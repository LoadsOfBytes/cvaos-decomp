/*
 * Killer Fish: body, attack, death and spawning of the swarm fish.
 */
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "code/code_080211F0.h"
#include "code_08009A0.h"
#include "structs/ewram.h"
#include "code_080D73B8.h"

/* Dispatch Killer Fish states and refresh its frame-dependent collision data. */
extern s32 Enemy_IsStunned(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void KillerFish_UpdateSwimming(struct EwramData_EntityData *);
extern void KillerFish_UpdateAttack(struct EwramData_EntityData *);
extern void KillerFish_UpdateDeath(struct EwramData_EntityData *);
extern void Enemy_UpdateFragmentDeath(struct EwramData_EntityData *);
void EnemyKillerFishUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    if (entity->state != 2)
    {
        if (EntityIsOutsideScreen(entity, 4)) goto finished;
        if (Entity_IsInRect(entity, -16, 0, 272, 176))
            Player_TrackEntity(entity);
    }
    if (EntityIsOutsideScreen(entity, 4)) entity->hidden = 1;
    else entity->hidden = 0;
    if (Enemy_IsStunned(entity))
    {
        if ((u8)(entity->state - 2) > 1)
            Hit_CheckWithPlayer(collision, entity);
        goto finished;
    }
    switch (entity->state)
    {
    case 0: KillerFish_UpdateSwimming(entity); break;
    case 1: KillerFish_UpdateAttack(entity); break;
    case 2: KillerFish_UpdateDeath(entity); break;
    case 3: Enemy_UpdateFragmentDeath(entity); break;
    case 4: entity->deleteRequested = 1; goto finished;
    case 5: Anim_Update(entity); goto finished;
    }
    {
        u8 *frame = &entity->animFrame;
        u8 previous = *frame;
        if ((u8)(entity->state - 2) > 1)
            Anim_Update(entity);
        if (previous != *frame)
            Entity_UpdateBoxes(entity);
    }
    if ((u8)(entity->state - 2) > 1)
        Hit_CheckWithPlayer(collision, entity);
finished:
    return;
}

/* Initialize Killer Fish, choosing its ground or submerged setup. */
extern u8 KillerFish_LoadGraphics(struct EwramData_EntityData *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern void Entity_EnableBoxes(struct EwramData_EntityData *, void *, void *);
extern void KillerFish_DamagePlayerOnContact(struct EwramData_EntityData *);
void KillerFish_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit);
extern void KillerFish_SpawnSwarmFish(struct EwramData_EntityData *);
extern s32 Tile_Classify(s16, s16);
static inline void FillParts(struct EwramData_EntityData *entity)
{
    s32 index;
    for (index = 0; index < (s16)entity->spawnParam1; index++)
        KillerFish_SpawnSwarmFish(entity);
}
void EnemyKillerFishCreate(struct EwramData_EntityData *entity)
{
    struct BgLayerState *background = &gEwramData->bgInfo[1];
    if (!KillerFish_LoadGraphics(entity))
    {
        EntityDelete(entity);
        goto finished;
    }
    Anim_StartIndexed(entity, 1, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bestiaryPreview)
        entity->state = 5;
    else
    {
        Entity_EnableBoxes(entity, KillerFish_DamagePlayerOnContact, KillerFish_OnHit);
        if (Tile_Classify((s16)(background->xPos.part16.integer + entity->posX.part16.integer),
                          (s16)(background->yPos.part16.integer + entity->posY.part16.integer)) != 3)
        {
            Entity_SnapToGround(entity);
            entity->subState = 99;
            entity->flipY = 1;
            entity->posY.part16.integer -= 8;
        }
        else
        {
            FillParts(entity);
        }
        entity->state = 0;
    }
finished:
    return;
}

extern s32 Entity_MoveAndCollide(struct EwramData_EntityData *, s32, s32, s32, s32);
extern s32 Collision_FindWaterSurfaceOffset(s32, s32);
extern void KillerFish_SpawnOscillatingEffect(struct EwramData_EntityData *parent, s32 x, s32 y, s32 frame, s32 vx, s32 vy);
extern s32 Target_GetPixelX(struct EwramData_EntityData *);
extern s32 Target_GetPixelY(struct EwramData_EntityData *);
extern s32 Tile_Classify(s16 x, s16 y);
extern s32 Target_IsWithinDistance(struct EwramData_EntityData *, s32);

static inline s16 BgX(s32 v, struct BgLayerState *bg)
{
    v += bg->xPos.part16.integer;
    return v;
}

static inline s16 BgY(s32 v, struct BgLayerState *bg)
{
    /* the original loads bgY into the (dying) bg register */
    { register u32 y asm("r6") = bg->yPos.part16.integer; v += y; } /* FAKEMATCH: pinned register */
    return v;
}

static inline s32 TileAt(struct EwramData_EntityData *entity, struct BgLayerState *bg)
{
    s16 x = BgX(Target_GetPixelX(entity), bg);
    return Tile_Classify(x, BgY(Target_GetPixelY(entity), bg));
}

/**
 * @brief Swim with vertical oscillation and surface correction; turn after stalled movement and select attack when the target checks succeed.
 */
void KillerFish_UpdateSwimming(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    s32 res;

    switch (entity->subState)
    {
    case 0:
        if (entity->flipX)
            entity->velX.whole = 0x4000;
        else
            entity->velX.whole = -0x4000;
        entity->accelX.whole = 0;
        entity->velY.whole = 0;
        entity->accelY.whole = 0x400;
        entity->ext.killerFish.stallTimer = 0;
        entity->ext.killerFish.lastX = 0;
        entity->ext.killerFishEffect.timer = 0;
        entity->subState++;
        break;
    case 1:
        res = Entity_MoveAndCollide(entity, 0x80000, -0x70000, 1, 0x40000);
        if (entity->velY.whole > 0x4000)
            entity->accelY.whole = -0x400;
        else if (entity->velY.whole < -0x4000)
            entity->accelY.whole = 0x400;
        if (res & 4)
            entity->accelY.whole = -0x400;
        if (res & 8)
        {
            entity->velY.whole = 0;
            entity->accelY.whole = 0x400;
        }
        if (entity->velX.whole > 0)
        {
            if (entity->ext.killerFish.lastX == (s16)bg->xPos.part16.integer + (s16)entity->posX.part16.integer)
            {
                if (entity->ext.killerFish.stallTimer++ > 0x3B)
                {
                    entity->flipX ^= 1;
                    if (entity->flipX)
                        entity->velX.whole = 0x4000;
                    else
                        entity->velX.whole = -0x4000;
                    entity->ext.killerFish.stallTimer = 0;
                    entity->ext.killerFish.lastX = 0;
                    entity->velY.whole = 0;
                    entity->accelY.whole = 0x400;
                }
            }
            else
            {
                entity->ext.killerFish.lastX = bg->xPos.part16.integer + entity->posX.part16.integer;
                entity->ext.killerFish.stallTimer = 0;
            }
        }
        else
        {
            if (entity->ext.killerFish.lastX == (s16)bg->xPos.part16.integer + (s16)entity->posX.part16.integer)
            {
                if (entity->ext.killerFish.stallTimer++ > 0x3B)
                {
                    entity->flipX ^= 1;
                    if (entity->flipX)
                        entity->velX.whole = 0x4000;
                    else
                        entity->velX.whole = -0x4000;
                    entity->ext.killerFish.stallTimer = 0;
                    entity->ext.killerFish.lastX = 0;
                    entity->velY.whole = 0;
                    entity->accelY.whole = 0x400;
                }
            }
            else
            {
                entity->ext.killerFish.lastX = bg->xPos.part16.integer + entity->posX.part16.integer;
                entity->ext.killerFish.stallTimer = 0;
            }
        }
        if (!(res & 0x80))
        {
            entity->posY.part16.integer += Collision_FindWaterSurfaceOffset((s16)entity->posX.part16.integer, (s16)entity->posY.part16.integer - 7);
            entity->accelY.whole = 0x400;
        }
        if ((s16)--entity->ext.killerFishEffect.timer <= 0)
        {
            KillerFish_SpawnOscillatingEffect(entity, entity->flipX ? entity->posX.whole + 0x100000 : entity->posX.whole - 0x100000, entity->posY.whole, 0x10, 0, -0x4000);
            entity->ext.killerFishEffect.timer = RandomNumberGenerator() % 0x78 + 0x78;
        }
        if (gEwramData->playerState.enemyTarget)
        {
            if (TileAt(entity, bg) == 3 && Target_IsWithinDistance(entity, 0x500000))
            {
                entity->state = 1;
                entity->subState = 0;
            }
        }
        break;
    }
}

/* Aim, release four effects, approach the sampled target and recover. */
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Entity_AimVelocityAtPointByDistance(struct EwramData_EntityData *, s32, s32, s32);
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);
extern void KillerFish_SpawnOscillatingEffect(struct EwramData_EntityData *, s32, s32, s32, s32, s32);
static inline void CorrectSurface(struct EwramData_EntityData *entity, s32 flags)
{
    s32 classification = flags & 0x80;
    if (!classification)
    {
        entity->posY.part16.integer += Collision_FindWaterSurfaceOffset((s16)entity->posX.part16.integer, entity->posY.part16.integer - 7);
        entity->velY.whole = classification;
        entity->accelY.whole = 0x400;
    }
}
/**
 * @brief Sample a target point, wait, emit four effects, approach the sampled point and recover before returning to swimming.
 */
void KillerFish_UpdateAttack(struct EwramData_EntityData *entity)
{
    struct BgLayerState *background = &gEwramData->bgInfo[1];
    switch (entity->subState)
    {
    case 0:
        {
            s32 targetX;
            u32 random;
            entity->flipX = Entity_IsTargetRight(entity);
            entity->velX.whole = 0;
            entity->accelX.whole = 0;
            entity->velY.whole = 0;
            entity->accelY.whole = 0;
            entity->timer = 0;
            targetX = Target_GetPixelX(entity);
            { s32 t_ = targetX + ({ register u32 bx_ asm("r2") = background->xPos.part16.integer; bx_; }) + (RandomNumberGenerator() & 7); entity->ext.killerFish.attackX = t_ - 4; } /* FAKEMATCH: pinned register */
            { s32 v_ = Target_GetPixelY(entity); { register u32 by_ asm("r6") = background->yPos.part16.integer; v_ += by_; } entity->ext.killerFish.attackY = v_ - 16; } /* FAKEMATCH: pinned register */
            entity->subState++;
        }
        break;
    case 1:
        if (entity->timer++ > 39)
        {
            entity->timer = 0;
            entity->subState++;
            Anim_StartIndexed(entity, 2, 0, 0);
        }
        break;
    case 2:
        if (entity->animStep)
        {
            s32 index;
            entity->timer = 0;
            entity->subState++;
            /* FAKEMATCH: r2 looks live over `index = 0`, so its reload takes r0 like the original */
            { register u32 z_ asm("r2"); asm("" : "=r"(z_)); index = 0; asm("" :: "r"(z_)); } /* FAKEMATCH: pinned register + empty asm */
            for (; index < 4; index++)
            {
                s32 x, y, vx, vy;
                u32 random;
                if (entity->flipX)
                {
                    x = entity->posX.whole - 0xA0000;
                    vx = -0x30000 - (RandomNumberGenerator() & 0xFFFF);
                }
                else
                {
                    x = entity->posX.whole + 0xA0000;
                    vx = (RandomNumberGenerator() & 0xFFFF) + 0x30000;
                }
                random = RandomNumberGenerator();
                { s32 t_ = x - 0x20000; y = 0x3FFFF; x = t_ + (random & y); }
                do { do { do { y = entity->posY.whole - (RandomNumberGenerator() & y); } while (0); } while (0); } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
                if (index == 0)
                {
                    vy = -0x6000 - (RandomNumberGenerator() & 0x1FFF);
                    KillerFish_SpawnOscillatingEffect(entity, x, y, 15, vx, vy);
                }
                else
                {
                    vy = -0x4000 - (RandomNumberGenerator() & 0x1FFF);
                    KillerFish_SpawnOscillatingEffect(entity, x, y, 16, vx, vy);
                }
            }
        }
        break;
    case 3:
        if (Entity_AimVelocityAtPointByDistance(entity, (s16)entity->ext.killerFish.attackX << 16,
                         (s16)entity->ext.killerFish.attackY << 16, 0x20000))
        {
            entity->subState++;
            entity->timer = 0;
            Anim_StartIndexed(entity, 1, 0, 1);
        }
        if (entity->timer++ > 59)
        {
            entity->subState++;
            entity->timer = 0;
            Anim_StartIndexed(entity, 1, 0, 1);
        }
        CorrectSurface(entity, Entity_MoveAndCollide(entity, 0x80000, -0x70000, 1, 0x40000));
        break;
    case 4:
        EntityDampVelocityX(entity, 8);
        EntityDampVelocityY(entity, 8);
        CorrectSurface(entity, Entity_MoveAndCollide(entity, 0x80000, -0x70000, 1, 0x40000));
        if (entity->timer++ > 29)
        {
            entity->state = 0;
            entity->subState = 0;
            Anim_StartIndexed(entity, 1, 0, 1);
        }
        break;
    }
}

/* Run Killer Fish's rising, floating and fading death sequence. */
extern s32 Entity_GetVisibilityPattern(s32);
extern void KillerFish_SpawnRisingSprite(struct EwramData_EntityData *, s32, s32, s32);
/**
 * @brief Invert the sprite, rise to the surface, oscillate and emit detached sprites; apply the visibility table before deletion.
 */
void KillerFish_UpdateDeath(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        entity->flipY = 1;
        entity->velX.whole = 1;
        entity->accelX.whole = 0;
        entity->velY.whole = 0;
        entity->accelY.whole = -0x320;
        entity->subState++;
        break;
    case 1:
        {
            s32 flags = Entity_MoveAndCollide(entity, 0, -0x70000, 1, 0x40000);
            if (entity->velY.whole < -0x4000) entity->velY.whole = -0x4000;
            if ((flags & 0x88) != 0x80)
            {
                entity->posY.part16.integer += Collision_FindWaterSurfaceOffset((s16)entity->posX.part16.integer, entity->posY.part16.integer - 7);
                entity->velX.whole = 0;
                entity->accelX.whole = 0;
                entity->velY.whole = 0;
                entity->accelY.whole = 0x200;
                entity->subState++;
                entity->timer = 0;
                PlaySong(0x6C);
            }
        }
        break;
    case 2:
        {
            s32 x, y;
            EntityAccelerateAndMove(entity, 0x20000);
            if (entity->velY.whole > 0x2000) entity->accelY.whole = -0x200;
            else if (entity->velY.whole < -0x2000) entity->accelY.whole = 0x200;
            x = entity->posX.whole + (RandomNumberGenerator() & 0x1FFFFF) - 0x100000;
            y = entity->posY.whole + (RandomNumberGenerator() & 0xFFFFF) - 0x80000;
            KillerFish_SpawnRisingSprite(entity, x, y, 15);
            if (entity->timer++ > 29)
                entity->hidden = Entity_GetVisibilityPattern(70 - entity->timer);
            if (entity->timer > 69) entity->deleteRequested = 1;
        }
        break;
    }
}

/* Apply a Killer Fish hit and select its death state when HP reaches zero. */
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *, struct EwramData_EntityData *, u16 *, s16);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s32, s32);
extern void Enemy_DropRewards(struct EwramData_EntityData *);
void KillerFish_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    s32 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));
    entity->hitFlashTimer = 8;
    {
        s32 hp = entity->hp;
        damage = (s16)damage;
        entity->hp = hp - damage;
    }
    Entity_ShowDamage(entity, damage, 0);
    if ((s16)entity->hp <= 0)
    {
        Enemy_DropRewards(entity);
        Anim_StartIndexed(entity, 1, 0, 0);
        Anim_Update(entity);
        entity->palette = entity->basePalette;
        entity->timer = 40;
        if (entity->subState == 99) entity->state = 3;
        else entity->state = 2;
        entity->subState = 0;
        entity->spawnIndex = 255;
    }
}

/* Spawn and attach a Killer Fish part in the reserved enemy slots. */
extern void KillerFishSwarm_Update(struct EwramData_EntityData *);
extern u8 KillerFishSwarm_LoadGraphics(struct EwramData_EntityData *, struct EwramData_EntityData *);
extern void KillerFishSwarm_EnableHitBoxes(struct EwramData_EntityData *);
static inline void SetRelativePosition(struct EwramData_EntityData *parent, struct EwramData_EntityData *part, s16 x, s16 y)
{
    if (parent->flipX)
        part->posX.part16.integer = parent->posX.part16.integer - x;
    else
        part->posX.part16.integer = parent->posX.part16.integer + x;
    part->posY.part16.integer = parent->posY.part16.integer + y;
}
/**
 * @brief Spawn one swarm fish around parent (EnemyKillerFishCreate spawns
 * spawnParam1 of them) on a random layer in front of or behind it, with a
 * quarter of its attack (attack >> 2).
 */
void KillerFish_SpawnSwarmFish(struct EwramData_EntityData *parent)
{
    struct EwramData_EntityData *part = EntityCreateInRange(0x1B, 0x48, (u32 *)KillerFishSwarm_Update);
    if (part)
    {
        if (!KillerFishSwarm_LoadGraphics(parent, part)) EntityDelete(part);
        part->scrollWithBg = 1;
        KillerFishSwarm_EnableHitBoxes(part);
        if (RandomNumberGenerator() % 2 == 0)
            part->spriteLayer = parent->spriteLayer + 1;
        else
            part->spriteLayer = parent->spriteLayer - 1;
        ENTITY_CREATOR(part) = parent;
        SetRelativePosition(parent, part, 0, 0);
        part->spawnIndex = parent->spawnIndex;
        part->basePalette = parent->basePalette;
        part->attack = parent->attack >> 2;
    }
}
