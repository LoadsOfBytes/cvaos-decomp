/*
 * Killer Fish swarm: swarm fish, effects and rising sprites. Kept apart from killer_fish.c: it calls Target_GetPixelX/Y with a halfword (s16) result, the body part with the s32 one.
 */
#include "code_08009A0.h"
#include "syscalls.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "code/code_080211F0.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

/* Killer Fish part: follow its owner, steer toward targets and handle reactions.
 *
 * Compiler-sensitive constructs:
 * - Target_GetPixelX/Target_GetPixelY return s16 pixel coordinates (their bodies end
 *   in a halfword); with the halfword call value as a register and the camera
 *   field first, the truncated sum is done in HImode and combine folds the
 *   camera load into a (subreg:SI (mem:HI)) operand, so reload loads it into
 *   the camera pointer's reload register (ldr rX, [sp]; ldrh rX, [rX, #6]).
 * - `distance` is reused for the collision flags of Entity_MoveWithBoxCollision; the longer
 *   live range moves it to r5 and velocityX to r4. */
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);
extern s32 Entity_MoveWithBoxCollision(struct EwramData_EntityData *, s32, s32, s32, s32, s32, s32);
extern s32 Collision_FindWaterSurfaceOffset(s32, s32);
extern s32 Target_IsInRect(s16, s16, s16, s16);
extern s16 Target_GetPixelX(struct EwramData_EntityData *);
extern s16 Target_GetPixelY(struct EwramData_EntityData *);
extern s32 Target_GetX(struct EwramData_EntityData *);
extern s32 Target_GetY(struct EwramData_EntityData *);
extern s32 Tile_Classify(s16, s16);
extern s32 Entity_GetVisibilityPattern(s32);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
#define SCATTER(e) (e->ext.killerFishSwarm.scatter)
#define OWNER_LOST(e) (*(s8 *)&e->ext.killerFishSwarm.ownerLost)
#define STEER_TIMER(e) (e->ext.killerFishSwarm.steerTimer)
#define ROOM_X(e) ((s16)(camera->xPos.part16.integer + Target_GetPixelX(e)))
#define ROOM_Y(e) ((s16)(camera->yPos.part16.integer + Target_GetPixelY(e)))

static inline s32 DeltaPixels(s32 target, s32 base)
{
    s32 delta = target - base;
    if (delta < 0) return -((base - target) >> 16);
    return delta >> 16;
}

static inline s32 CoordinateDelta(s32 target, s32 base)
{
    if (target < 0) return -((-target) >> 16) - base;
    return (target >> 16) - base;
}

/**
 * @brief Swarm fish update: swim to random points around the owner (type 7) and,
 * when the target is close and in water, dart at it; scatter when the owner
 * attacks (state 4) or dies; state 2 blinks away.
 */
void KillerFishSwarm_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = ENTITY_CREATOR(entity);
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    u32 bounds[4];
    s32 targetX, targetY, dx, dy, velocityX, velocityY;
    s32 distance;

    if (entity->state != 2)
    {
        if (EntityIsOutsideScreen(entity, 4)) return;
        if (Entity_IsInRect(entity, -16, 0, 272, 176)) Player_TrackEntity(entity);
    }
    if (EntityIsOutsideScreen(entity, 4)) entity->hidden = 1;
    else entity->hidden = 0;
    if (owner && owner->updateFunc && owner->enemyId == 7)
    {
        if (owner->state == 4) SCATTER(entity) = 1;
        if ((s16)owner->hp <= 0 || entity->spawnIndex != owner->spawnIndex)
            OWNER_LOST(entity) = 1;
    }
    if (entity->state != 2 && gEwramData->gameState.timeStopTimer)
    {
        Hit_CheckWithPlayer(bounds, entity);
        return;
    }
restart:
    switch (entity->state)
    {
    case 0:
        entity->state = 1;
        entity->velX.whole = 0;
        entity->velY.whole = 0;
        entity->accelX.whole = 0;
        entity->accelY.whole = 0;
        STEER_TIMER(entity) = 0;
        entity->subState = 0;
        /* fall through */
    case 1:
    case 4:
        if (SCATTER(entity) == 1 || OWNER_LOST(entity) == 1)
        {
            entity->state = 3;
            entity->subState = 0;
            goto restart;
        }
        if (--STEER_TIMER(entity) <= 0)
        {
            STEER_TIMER(entity) = RandomNumberGenerator() % 20 + 20;
            {
                s32 random = RandomNumberGenerator();
                s32 base = owner->posX.whole;
                base -= 0x80000;
                targetX = base + (random & 0xFFFFF);
            }
            {
                s32 random = RandomNumberGenerator();
                s32 base = owner->posY.whole;
                base -= 0x80000;
                targetY = base + (random & 0xFFFFF);
            }
            dx = DeltaPixels(targetX, entity->posX.whole);
            dx *= dx;
            dy = DeltaPixels(targetY, entity->posY.whole);
            dy *= dy;
            distance = Sqrt(dx + dy);
            velocityX = (DeltaPixels(targetX, entity->posX.whole) << 17) / distance;
            velocityY = (DeltaPixels(targetY, entity->posY.whole) << 17) / distance;
            if (entity->state == 4 && distance <= 9) entity->state = 1;
            entity->velX.whole = velocityX;
            entity->velY.whole = velocityY;
        }
        if (entity->velX.whole > 0) entity->flipX = 1;
        else entity->flipX = 0;
        EntityDampVelocityX(entity, 20);
        EntityDampVelocityY(entity, 20);
        distance = Entity_MoveWithBoxCollision(entity, 0, -4, -4, 8, 8, 0x60000) & 0x80;
        if (!distance)
        {
            s32 correction = Collision_FindWaterSurfaceOffset((s16)entity->posX.part16.integer, entity->posY.part16.integer - 3);
            entity->posY.part16.integer += correction;
            entity->velY.whole = distance;
        }
        if (entity->state != 4 && Target_IsInRect((s16)(entity->posX.part16.integer - 80), (s16)(entity->posY.part16.integer - 32), 160, 64) && gEwramData->playerState.enemyTarget)
        {
            s32 x = ROOM_X(entity);
            s32 y = ROOM_Y(entity);
            if (Tile_Classify(x, y) == 3)
            {
                entity->state = 3;
                entity->subState = 0;
            }
        }
        Hit_CheckWithPlayer(bounds, entity);
        break;
    case 2:
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if (--entity->timer == 0) entity->deleteRequested = 1;
        break;
    case 3:
        switch (entity->subState)
        {
        case 0:
            entity->ext.killerFishSwarm.bitPlayer = 0;
            entity->subState++;
            STEER_TIMER(entity) = 0;
            entity->velX.whole = 0;
            entity->velY.whole = 0;
            entity->accelX.whole = 0;
            entity->accelY.whole = 0;
            entity->timer = 0;
            /* fall through */
        case 1:
            if (--STEER_TIMER(entity) <= 0)
            {
                entity->timer++;
                STEER_TIMER(entity) = RandomNumberGenerator() % 20 + 20;
                targetX = 0;
                if (gEwramData->playerState.enemyTarget)
                {
                    s32 x = ROOM_X(entity);
                    s32 y = ROOM_Y(entity);
                    if (Tile_Classify(x, y) == 3)
                    {
                        s32 random = RandomNumberGenerator();
                        {
                            s32 base = Target_GetX(entity);
                            random &= 0xFFFFF;
                            base -= 0x80000;
                            targetX = random + base;
                        }
                        {
                            s32 base = Target_GetY(entity);
                            base -= RandomNumberGenerator() & 0xFFFFF;
                            targetY = -0x100000 + base;
                        }
                    }
                }
                if (!targetX || SCATTER(entity) == 1)
                {
                    s32 random = RandomNumberGenerator() & 0xFFFFF;
                    { s32 base = entity->posX.whole; base -= 0x80000; targetX = random + base; }
                    random = RandomNumberGenerator() & 0xFFFFF;
                    { s32 base = entity->posY.whole; base -= 0x80000; targetY = random + base; }
                }
                dx = CoordinateDelta(targetX, (s16)entity->posX.part16.integer);
                dx *= dx;
                dy = CoordinateDelta(targetY, entity->posY.part16.integer);
                dy *= dy;
                distance = Sqrt(dx + dy);
                velocityX = (CoordinateDelta(targetX, (s16)entity->posX.part16.integer) << 17) / distance;
                velocityY = (CoordinateDelta(targetY, entity->posY.part16.integer) << 17) / distance;
                entity->velX.whole = velocityX;
                entity->velY.whole = velocityY;
            }
            if (entity->velX.whole > 0) entity->flipX = 1;
            else if (entity->velX.whole < 0) entity->flipX = 0;
            EntityDampVelocityX(entity, 20);
            EntityDampVelocityY(entity, 20);
            distance = Entity_MoveWithBoxCollision(entity, 0, -4, -4, 8, 8, 0x60000) & 0x80;
            if (!distance)
            {
                s32 correction = Collision_FindWaterSurfaceOffset((s16)entity->posX.part16.integer, entity->posY.part16.integer - 3);
                entity->posY.part16.integer += correction;
                entity->velY.whole = distance;
            }
            if (entity->timer > 5) entity->state = 4;
            if (entity->ext.killerFishSwarm.bitPlayer) entity->state = 4;
            Hit_CheckWithPlayer(bounds, entity);
            break;
        }
        break;
    }
    Anim_Update(entity);
}

/* KillerFishSwarm_EnableHitBoxes (hit boxes). */
extern void KillerFishSwarm_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void KillerFishSwarm_OnHit(struct EwramData_EntityData *entity, void *attacker, u16 *hit);

/**
 * @brief 70774 | Activate box slot 8 with KillerFishSwarm_DamagePlayerOnContact / KillerFishSwarm_OnHit and
 * set both boxes to the bytes of 0x0808FCFC (x -4, y -4, w 8, h 8 - not a
 * ROM address); clear flags bits 2 and 1.
 */
void KillerFishSwarm_EnableHitBoxes(struct EwramData_EntityData *entity)
{
    u32 box = 0x0808FCFC;

    Hit_Enable(entity, 8, KillerFishSwarm_DamagePlayerOnContact, (s32)KillerFishSwarm_OnHit);
    HitBox_SetBody(entity, &box);
    HitBox_SetAttack(entity, &box);
    entity->hit.flags &= ~4;
    entity->hit.flags &= ~2;
}

/* Create an oscillating Killer Fish effect with supplied position and velocity. */
extern void KillerFishEffect_UpdateOscillating(struct EwramData_EntityData *);
extern u8 sKillerFishGfx[], sKillerFishSpriteSheet[];
/**
 * @brief Create a sprite effect with supplied position, velocity and frame; reject it unless its initial tile classification is 3.
 */
void KillerFish_SpawnOscillatingEffect(struct EwramData_EntityData *parent, s32 x, s32 y, s32 frame, s32 vx, s32 vy)
{
    struct EwramData_EntityData *part = EntityCreateInRange(0x49, 0xCF, (u32 *)KillerFishEffect_UpdateOscillating);
    struct BgLayerState *background = &gEwramData->bgInfo[1];
    if (part)
    {
        s32 graphics;
        part->posX.whole = x;
        part->posY.whole = y;
        part->velX.whole = vx;
        part->velY.whole = vy;
        part->flipX = RandomNumberGenerator() % 2;
        if (RandomNumberGenerator() % 2 == 0) part->spriteLayer = parent->spriteLayer + 1;
        else part->spriteLayer = parent->spriteLayer - 1;
        graphics = ObjGfx_Find(sKillerFishGfx);
        Sprite_Attach(part, sKillerFishSpriteSheet, graphics, parent->basePalette);
        part->animFrame = frame;
        part->scrollWithBg = 1;
        part->drawFunc = (u32 *)Sprite_DrawEntity;
        if (Tile_Classify((s16)(background->xPos.part16.integer + part->posX.part16.integer),
                          (s16)(background->yPos.part16.integer + part->posY.part16.integer)) != 3)
            EntityDelete(part);
    }
}

/* Oscillate horizontally while the contact classification permits movement. */
extern s32 Entity_MoveWithClassifiedPointCollision(struct EwramData_EntityData *, s32, s32, s32);
/**
 * @brief Damp and alternate horizontal velocity; delete when movement contact classification no longer matches 0x80 under mask 0x88.
 */
void KillerFishEffect_UpdateOscillating(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
    case 0:
        entity->state = 1;
        entity->timer = 0;
        /* fall through */
    case 1:
        entity->timer++;
        EntityDampVelocityX(entity, 8);
        if ((entity->timer & 7) == 0)
        {
            if (entity->timer & 8) entity->velX.whole = -0x4000;
            else entity->velX.whole = 0x4000;
        }
        if ((Entity_MoveWithClassifiedPointCollision(entity, 0, -0x20000, 1) & 0x88) != 0x80)
            EntityDelete(entity);
        break;
    }
}

/* Animate a detached sprite for twelve ticks while moving upward. */
extern void EntityMove(struct EwramData_EntityData *);
/**
 * @brief Move upward with alternating horizontal velocity, decrement the frame at tick 6 and delete after tick 11.
 */
void KillerFishEffect_UpdateRising(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
    case 0:
        entity->state = 1;
        entity->timer = 0;
        /* fall through */
    case 1:
        entity->timer++;
        if ((entity->timer & 7) == 0)
        {
            if (entity->timer & 8) entity->velX.whole = -0x4000;
            else entity->velX.whole = 0x4000;
        }
        entity->velY.whole = -0x4000;
        EntityMove(entity);
        if (entity->timer == 6) entity->animFrame--;
        if (entity->timer > 11) EntityDelete(entity);
        break;
    }
}

/* Load shared Killer Fish graphics and start a part's animation. */
/**
 * @brief Give part the Killer Fish graphics in the palette of parent, animation 0;
 * return 0 when the graphics are not loaded, else 1.
 */
u8 KillerFishSwarm_LoadGraphics(struct EwramData_EntityData *parent, struct EwramData_EntityData *part)
{
    s32 graphics = ObjGfx_LoadResource(sKillerFishGfx, 0);
    if (graphics < 0) return 0;
    Sprite_Attach(part, sKillerFishSpriteSheet, graphics, parent->basePalette);
    Anim_StartIndexed(part, 0, 0, 1);
    Anim_Update(part);
    part->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/* Emit the part's configured effect at its current position. */
/**
 * @brief Contact: mark the bite (unk_500 = 1) and
 * Player_ApplyContactDamage(attack, 0, 0, 1) at the fish position.
 */
void KillerFishSwarm_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage effect;
    effect.attack = entity->attack;
    effect.attributes = 0;
    effect.unused = 0;
    effect.reaction = 1;
    effect.sourceX = entity->posX.whole;
    effect.sourceY = entity->posY.whole;
    entity->ext.killerFishSwarm.bitPlayer = 1;
    Player_ApplyContactDamage(&effect);
}

/* Spawn the part's death sprite and start its forty-tick death state. */
extern void KillerFish_SpawnRisingSprite(struct EwramData_EntityData *, s32, s32, s32);
/**
 * @brief Hit: rising sprite 15 at the fish position, then state 2 (40 frames).
 */
void KillerFishSwarm_OnHit(struct EwramData_EntityData *entity, void *attacker, u16 *hit)
{
    Player_CalculateAttackPower(hit[2]);
    KillerFish_SpawnRisingSprite(entity, entity->posX.whole, entity->posY.whole, 15);
    entity->state = 2;
    entity->timer = 40;
    entity->subState = 0;
}

/* Create a detached Killer Fish sprite using the parent's cached graphics. */
/**
 * @brief Create a detached sprite with parent graphics and palette at the supplied position and frame, one layer above the parent.
 */
void KillerFish_SpawnRisingSprite(struct EwramData_EntityData *parent, s32 x, s32 y, s32 frame)
{
    struct EwramData_EntityData *part = EntityCreateInRange(0x49, 0xCF, (u32 *)KillerFishEffect_UpdateRising);
    if (part)
    {
        s32 graphics;
        part->posX.whole = x;
        part->posY.whole = y;
        part->flipX = RandomNumberGenerator() % 2;
        part->spriteLayer = parent->spriteLayer - 1;
        graphics = ObjGfx_Find(sKillerFishGfx);
        Sprite_Attach(part, sKillerFishSpriteSheet, graphics, parent->basePalette);
        part->animFrame = frame;
        part->scrollWithBg = 1;
        part->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   KillerFish_LoadGraphics = Object25_LoadGraphics
 *   KillerFish_DamagePlayerOnContact = Enemy_DamagePlayerOnContact */

extern u8 sKillerFishGfx[];
extern u8 sKillerFishPalette[];
extern u8 sKillerFishSpriteSheet[];
extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);

static inline void Entity_DamagePlayerOnContact(struct EwramData_EntityData *entity, u16 attributes, u16 unused, u16 reaction)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = attributes;
    contactDamage.unused = unused;
    contactDamage.reaction = reaction;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief 70AC4 | Same as Object25_LoadGraphics.
 */
u8 KillerFish_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sKillerFishGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sKillerFishPalette, 0);
    Sprite_Attach(entity, sKillerFishSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 70B10 | Same as Enemy_DamagePlayerOnContact.
 */
void KillerFish_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}
