/*
 * Winged Skeleton: body, thrown shots and death pieces.
 */
#include "core.h"
#include "player_core.h"
#include "graphics.h"
#include "entity_util.h"
#include "structs/ewram.h"
#include "structs/entity_template.h"
#include "gameplay.h"
#include "structs/entity_render_flags.h"
#include "sound.h"

extern s32 Enemy_IsStunned(struct EwramData_EntityData *entity);
extern void Enemy_UpdateFragmentDeath(struct EwramData_EntityData *entity);
extern s32 Target_GetFlipXOrOpposite(struct EwramData_EntityData *entity);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *entity);
extern void WingedSkeleton_UpdateFly(struct EwramData_EntityData *entity);
extern void WingedSkeleton_UpdateAttack(struct EwramData_EntityData *entity);
extern void WingedSkeleton_SpawnDeathPieces(struct EwramData_EntityData *entity);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *entity);
extern void EntityUpdateNothing(void);
extern void EnemyWingedSkeletonCreate(struct EwramData_EntityData *entity);
extern s32 Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);

/**
 * @brief 74910 | Winged Skeleton update (as EnemyMedusaHeadUpdate): a
 * skeleton (states 0 / 1, WingedSkeleton_UpdateFly / WingedSkeleton_UpdateAttack) dies off screen
 * (state 2, WingedSkeleton_SpawnDeathPieces when HP is gone) and then decrements the count of
 * its spawner (unk_4FC, enemyId 0x14); the spawner (state 3) with 1/32
 * chance creates one below the limit spawnParam0: x off the room edge farther
 * from the player, y above / below / in the view depending on the vertical
 * camera speed of bgInfo[1].
 */
void EnemyWingedSkeletonUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    struct EwramData_EntityData *parent;
    struct EwramData_EntityData *head;
    s32 x;
    struct BgLayerState *bg = &gEwramData->bgInfo[1];

    if (entity->state != 3)
    {
        if (entity->state != 2 && Entity_IsInRect(entity, -16, 0, 272, 176))
            Player_TrackEntity(entity);
        if (EntityIsOutsideScreen(entity, 4))
        {
            entity->state = 2;
            entity->timer = 1;
        }
    }
    if (Enemy_IsStunned(entity))
    {
        if (entity->state != 2)
            Hit_CheckWithPlayer(collision, entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
        WingedSkeleton_UpdateFly(entity);
        break;
    case 1:
        WingedSkeleton_UpdateAttack(entity);
        break;
    case 2:
        entity->timer = 1;
        if ((s16)entity->hp <= 0)
            WingedSkeleton_SpawnDeathPieces(entity);
        Enemy_UpdateFragmentDeath(entity);
        if (entity->deleteRequested)
        {
            parent = entity->ext.wingedSkeleton.spawner;
            if (parent != NULL && parent->updateFunc != NULL && parent->enemyId == 0x14
                && parent->ext.wingedSkeletonSpawner.childCount != 0)
                parent->ext.wingedSkeletonSpawner.childCount--;
        }
        break;
    case 3:
        if ((RandomNumberGenerator() & 0x1F) == 0 && (s32)entity->ext.wingedSkeletonSpawner.childCount < entity->spawnParam0)
        {
            head = EntityCreateInRange(0x1B, 0x48, (u32 *)EntityUpdateNothing);
            if (head != NULL)
            {
                entity->ext.wingedSkeletonSpawner.childCount++;
                if (gEwramData->playerState.player->posX.whole <= 0x1FFFFF)
                    head->posX.whole = 0x1000000;
                else if (gEwramData->playerState.player->posX.whole <= 0xD00000 && (RandomNumberGenerator() & 1))
                    head->posX.whole = 0x1000000;
                else
                    head->posX.whole = -0x100000;
                if (bg->yVel.whole < 0)
                    head->posY.part16.integer = RandomNumberGenerator() % 0x50 - 0x40;
                else
                {
                    if (bg->yVel.whole > 0)
                        x = (RandomNumberGenerator() & 0x3F) + 0x90;
                    else
                        x = (RandomNumberGenerator() & 0x3F) + 0x30;
                    head->posY.part16.integer = x;
                }
                head->scrollWithBg = 1;
                head->spawnIndex = 0;
                head->enemyId = entity->enemyId;
                head->spawnParam1 = 0;
                head->drawFunc = (u32 *)Sprite_DrawEntity;
                head->hp = sEnemyData[entity->enemyId].hp;
                head->mp = sEnemyData[entity->enemyId].mp;
                head->updateFunc = entity->updateFunc;
                EnemyWingedSkeletonCreate(head);
                head->ext.wingedSkeleton.spawner = entity;
            }
        }
        return;
    case 4:
        Anim_Update(entity);
        return;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
}

extern u8 WingedSkeleton_LoadGraphics(struct EwramData_EntityData *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *);
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern void WingedSkeleton_DamagePlayerOnContact(struct EwramData_EntityData *);
void WingedSkeleton_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
void EnemyWingedSkeletonCreate(struct EwramData_EntityData *entity)
{
    if (!WingedSkeleton_LoadGraphics(entity)) { EntityDelete(entity); return; }
    Anim_StartIndexed(entity, 2, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bestiaryPreview) { entity->state = 4; return; }
    entity->spriteLayer = 3;
    Hit_Enable(entity, 8, WingedSkeleton_DamagePlayerOnContact, (s32)WingedSkeleton_OnHit);
    Entity_UpdateBoxes(entity);
    if (entity->spawnParam1 != 0)
    {
        entity->state = 3;
        entity->hidden = 1;
        entity->hit.flags |= 4;
        entity->hit.flags |= 2;
    }
    else
    {
        entity->state = 0;
        entity->flipX = Entity_IsTargetRight(entity);
        if (entity->flipX) entity->velX.whole = (RandomNumberGenerator() & 0x3FFF) + 0xC000;
        else entity->velX.whole = -0xC000 - (RandomNumberGenerator() & 0x3FFF);
    }
}

/* Patrolling enemy (Entity_GetTargetDistanceX/BDEC distance checks, Tile_Classify probe); per-copy pinned bg->xPos reads keep the three tracking copies distinct. */
extern void EntityMove(struct EwramData_EntityData *);
extern s32 Entity_GetTargetDistanceX(struct EwramData_EntityData *);
extern s32 Entity_GetTargetDistanceY(struct EwramData_EntityData *);
extern s16 Target_GetPixelX(struct EwramData_EntityData *);
extern s32 Tile_Classify(s16, s16);

#define ANIM552 (*(u16 *)&entity->animStep)
static inline s32 Abs(s32 v)
{
    if (v < 0)
        v = -v;
    return v;
}

#define ANIMATE()                                                                 \
    if (entity->animScriptIndex == 2)                                                     \
    {                                                                             \
        if (ANIM552 == 0 && RandomNumberGenerator() % 3 == 0)              \
        {                                                                         \
            Anim_StartIndexed(entity, 0, 0, 0);                                        \
            entity->animDone = 0;                                                \
        }                                                                         \
    }                                                                             \
    else if (entity->animScriptIndex == 0 && entity->animDone)                           \
        Anim_StartIndexed(entity, 2, 0, 1);

#define TRACK0()                                                                   \
    {                                                                             \
        s32 a, b, t;                                                               \
        if (Abs((s16)Entity_GetTargetDistanceY(entity)) <= 0x10)                                     \
            break;                                                                \
        a = Entity_GetTargetDistanceX(entity);                                                 \
        b = Entity_GetTargetDistanceY(entity);                                                 \
        t = Abs((s16)a) + 8; if (Abs(t - Abs((s16)b)) > 0x1F)                                    \
            break;                                                                \
        if (entity->flipX)                                                        \
        {                                                                         \
            if ((s16)Entity_GetTargetDistanceX(entity) >= 0)                                        \
                break;                                                            \
            if (Tile_Classify(({ s32 t_ = entity->posX.part16.integer + ({ register u32 b_ asm("r2") = bg->xPos.part16.integer; b_; }); t_; }) + 0x15, bg->yPos.part16.integer + entity->posY.part16.integer - 9) != 2) entity->ext.wingedSkeleton.blocked = 1; /* FAKEMATCH: pinned register */ \
        }                                                                         \
        else                                                                      \
        {                                                                         \
            if ((s16)Entity_GetTargetDistanceX(entity) <= 0)                                        \
                break;                                                            \
            if (Tile_Classify(({ s32 t_ = entity->posX.part16.integer + (bg->xPos.part16.integer); t_; }) - 0x15, bg->yPos.part16.integer + entity->posY.part16.integer - 9) != 2) entity->ext.wingedSkeleton.blocked = 1; \
        }                                                                         \
    }

#define TRACK1()                                                                   \
    {                                                                             \
        s32 a, b, t;                                                               \
        if (Abs((s16)Entity_GetTargetDistanceY(entity)) <= 0x10)                                     \
            break;                                                                \
        a = Entity_GetTargetDistanceX(entity);                                                 \
        b = Entity_GetTargetDistanceY(entity);                                                 \
        t = Abs((s16)a) + 8; if (Abs(t - Abs((s16)b)) > 0x1F)                                    \
            break;                                                                \
        if (entity->flipX)                                                        \
        {                                                                         \
            if ((s16)Entity_GetTargetDistanceX(entity) >= 0)                                        \
                break;                                                            \
            if (Tile_Classify(({ s32 t_ = entity->posX.part16.integer + (bg->xPos.part16.integer); t_; }) + 0x15, bg->yPos.part16.integer + entity->posY.part16.integer - 9) != 2) entity->ext.wingedSkeleton.blocked = 1; \
        }                                                                         \
        else                                                                      \
        {                                                                         \
            if ((s16)Entity_GetTargetDistanceX(entity) <= 0)                                        \
                break;                                                            \
            if (Tile_Classify(({ s32 t_ = entity->posX.part16.integer + ({ register u32 b_ asm("r2") = bg->xPos.part16.integer; b_; }); t_; }) - 0x15, bg->yPos.part16.integer + entity->posY.part16.integer - 9) != 2) entity->ext.wingedSkeleton.blocked = 1; /* FAKEMATCH: pinned register */ \
        }                                                                         \
    }

#define TRACK2()                                                                   \
    {                                                                             \
        s32 a, b, t;                                                               \
        if (Abs((s16)Entity_GetTargetDistanceY(entity)) <= 0x10)                                     \
            break;                                                                \
        a = Entity_GetTargetDistanceX(entity);                                                 \
        b = Entity_GetTargetDistanceY(entity);                                                 \
        t = Abs((s16)a) + 8; if (Abs(t - Abs((s16)b)) > 0x1F)                                    \
            break;                                                                \
        if (entity->flipX)                                                        \
        {                                                                         \
            if ((s16)Entity_GetTargetDistanceX(entity) >= 0)                                        \
                break;                                                            \
            if (Tile_Classify(({ s32 t_ = entity->posX.part16.integer + (bg->xPos.part16.integer); t_; }) + 0x15, bg->yPos.part16.integer + entity->posY.part16.integer - 9) != 2) entity->ext.wingedSkeleton.blocked = 1; \
        }                                                                         \
        else                                                                      \
        {                                                                         \
            if ((s16)Entity_GetTargetDistanceX(entity) <= 0)                                        \
                break;                                                            \
            if (Tile_Classify(({ s32 t_ = entity->posX.part16.integer + ({ register u32 b_ asm("r2") = bg->xPos.part16.integer; b_; }); t_; }) - 0x15, bg->yPos.part16.integer + entity->posY.part16.integer - 9) != 2) entity->ext.wingedSkeleton.blocked = 1; /* FAKEMATCH: pinned register */ \
        }                                                                         \
    }

#define TURN(next)                                                                \
    {                                                                             \
        u16 anim = ANIM552;                                               \
        u8 f;                                                                     \
        if (anim != 0)                                                            \
            break;                                                                \
        f = *(ENTITY_RENDER_FLAGS_ADDRESS(entity)) & 0x40;                                     \
        if (f)                                                                    \
        {                                                                         \
            if ((s16)Entity_GetTargetDistanceX(entity) < 0)                                         \
            {                                                                     \
                entity->state = 1;                                              \
                entity->subState = next;                                           \
                break;                                                            \
            }                                                                     \
            entity->ext.wingedSkeleton.blocked = 0;                                \
        }                                                                         \
        else                                                                      \
        {                                                                         \
            if ((s16)Entity_GetTargetDistanceX(entity) > 0)                                         \
            {                                                                     \
                entity->state = 1;                                              \
                entity->subState = next;                                           \
                break;                                                            \
            }                                                                     \
            entity->ext.wingedSkeleton.blocked = 0;                                \
        }                                                                         \
    }

/**
 * @brief Flight state machine (state 0): fly across the view toward the target and
 * turn; start the throw (state 1) when the target is roughly diagonal.
 */
void WingedSkeleton_UpdateFly(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];

    switch (entity->subState)
    {
    case 0:
        EntityMove(entity);
        ANIMATE()
        if (ANIM552 == 0 && Abs((s16)Entity_GetTargetDistanceX(entity)) <= Abs((s16)Entity_GetTargetDistanceY(entity)) + 0x20)
        {
            entity->subState = 1;
            Anim_StartIndexed(entity, 3, 0, 0);
            entity->animDone = 0;
            break;
        }
        if (entity->ext.wingedSkeleton.blocked == 0)
            TRACK0()
        else
            TURN(10)
        break;
    case 1:
        if (entity->animDone)
        {
            Anim_StartIndexed(entity, 4, 0, 1);
            entity->subState++;
        }
        EntityMove(entity);
        break;
    case 2:
        EntityMove(entity);
        if (ANIM552 == 0)
        {
            if (entity->velX.whole > 0)
            {
                if (Target_GetPixelX(entity) - (s16)entity->posX.part16.integer >= -0x10)
                    goto track2;
            }
            else
            {
                if (Target_GetPixelX(entity) - (s16)entity->posX.part16.integer <= 0x10)
                    goto track2;
            }
            entity->subState = 3;
            Anim_StartIndexed(entity, 2, 0, 1);
            entity->animDone = 0;
            break;
        }
    track2:
        if (entity->ext.wingedSkeleton.blocked == 0)
            TRACK1()
        else
            TURN(0)
        break;
    case 3:
        EntityMove(entity);
        ANIMATE()
        if (entity->ext.wingedSkeleton.blocked == 0)
            TRACK2()
        else
            TURN(10)
        break;
    case 4:
        EntityMove(entity);
        break;
    }
}

extern s16 Entity_GetTargetDistance(struct EwramData_EntityData *entity);
extern void WingedSkeleton_ThrowShot(struct EwramData_EntityData *entity);

/**
 * @brief 75078 | Winged Skeleton attack sub-state machine (subState): animation 5
 * (player farther than 0x50) or 6, then on frame 3 of the throw animation
 * spawn the projectile WingedSkeleton_ThrowShot (song 0x85); states 10..12 do the same
 * with animation 7. When the animation ends (animDone) animation 1 is set
 * and the main state returns to 0 with sub-state 4.
 */
void WingedSkeleton_UpdateAttack(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        if (Entity_GetTargetDistance(entity) > 0x50)
        {
            entity->subState = 1;
            Anim_StartIndexed(entity, 5, 0, 0);
            entity->animDone = 0;
        }
        else
        {
            entity->subState = 2;
            Anim_StartIndexed(entity, 6, 0, 0);
            entity->animDone = 0;
        }
        break;
    case 1:
        if (entity->animDone)
        {
            entity->subState = 2;
            Anim_StartIndexed(entity, 6, 0, 0);
            entity->animDone = 0;
        }
        break;
    case 2:
        if (entity->animStep > 2)
        {
            entity->subState++;
            WingedSkeleton_ThrowShot(entity);
            entity->animDone = 0;
            PlaySong(0x85);
        }
        break;
    case 10:
        Anim_StartIndexed(entity, 7, 0, 1);
        entity->subState = 11;
        break;
    case 11:
        if (entity->animStep > 2)
        {
            entity->subState++;
            entity->animDone = 0;
            WingedSkeleton_ThrowShot(entity);
            PlaySong(0x85);
        }
        break;
    case 3:
    case 12:
        if (entity->animDone)
        {
            Anim_StartIndexed(entity, 1, 0, 1);
            entity->state = 0;
            entity->subState = 4;
        }
        break;
    }
}

/* WingedSkeleton_OnHit (hit). */
extern u16 Player_CalculateAttackPower(u16 attackCategory);
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *entity, s32 other, u16 *hit, s32 inputDamage);
extern void Entity_ShowDamage(struct EwramData_EntityData *owner, s16 value, s32 kind);
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);

/**
 * @brief 751B4 | Hit: flash (hitFlashTimer = 8), subtract the damage from the HP
 * and show it; at HP <= 0 die (Enemy_DropRewards, song 0x6B, palette into
 * 0x53E bits 3-6, timer 40, state 2).
 */
void WingedSkeleton_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s32 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));

    entity->hitFlashTimer = 8;
    {
        u16 hp = entity->hp;

        // (s16)damage, written so that agbcc sign-extends after the HP load
        damage <<= 16;
        damage >>= 16;
        entity->hp = hp - damage;
    }
    Entity_ShowDamage(entity, damage, 0);
    if ((s16)entity->hp <= 0)
    {
        Enemy_DropRewards(entity);
        PlaySong(0x6B);
        entity->palette = entity->basePalette;
        entity->timer = 0x28;
        entity->state = 2;
        entity->subState = 0;
    }
}

extern void WingedSkeletonShot_Update(struct EwramData_EntityData *entity);
extern s32 WingedSkeletonShot_LoadGraphics(struct EwramData_EntityData *entity, struct EwramData_EntityData *child);
extern void WingedSkeletonShot_EnableHitBoxes(struct EwramData_EntityData *entity);

/**
 * @brief 75230 | Spawn a WingedSkeletonShot_Update projectile (Winged Skeleton graphics) from entity (its owner in
 * unk_4F4): graphics via WingedSkeletonShot_LoadGraphics (deleted if that fails, though setup
 * continues), WingedSkeletonShot_EnableHitBoxes, layer + 1, facing of the owner, 0x15 px in
 * front and 9 px above it, velX +-0x20000, velY 0x20000, no acceleration;
 * copies spawnIndex, basePalette and attack from the owner.
 */
void WingedSkeleton_ThrowShot(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x1B, 0x48, (u32 *)WingedSkeletonShot_Update);

    if (child == NULL)
        return;
    if (!(u8)WingedSkeletonShot_LoadGraphics(entity, child))
        EntityDelete(child);
    child->scrollWithBg = 1;
    WingedSkeletonShot_EnableHitBoxes(child);
    child->spriteLayer = entity->spriteLayer + 1;
    ENTITY_CREATOR(child) = entity;
    child->flipX = entity->flipX;
    if (entity->flipX)
    {
        child->posX.part16.integer = entity->posX.part16.integer + 0x15;
        child->velX.whole = 0x20000;
    }
    else
    {
        child->posX.part16.integer = entity->posX.part16.integer - 0x15;
        child->velX.whole = -0x20000;
    }
    child->posY.part16.integer = entity->posY.part16.integer - 9;
    child->velY.whole = 0x20000;
    child->accelX.whole = 0;
    child->accelY.whole = 0;
    child->spawnIndex = entity->spawnIndex;
    child->basePalette = entity->basePalette;
    child->attack = entity->attack;
}

extern s32 Entity_MoveWithPointCollision(struct EwramData_EntityData *entity, s32 landingVelocity, s32 ceilingOffset);
extern s32 Entity_GetVisibilityPattern(s32 step);

/**
 * @brief 75314 | Projectile spawned by WingedSkeleton_ThrowShot: while flying the hit
 * box cycles through three boxes (0x0404FC00 / 0x0404F00C / 0x0404E418)
 * and it moves with Entity_MoveWithPointCollision (0x10000, -0x20000), testing contact
 * (Hit_CheckWithPlayer); on landing (bits 0-2) it lies 60 frames with its box
 * disabled, blinking (Entity_GetVisibilityPattern -> hidden) for the last 40. Deleted
 * off screen or at the end.
 */
void WingedSkeletonShot_Update(struct EwramData_EntityData *entity)
{
    u32 contact[4];
    u32 box0 = 0x0404FC00;
    u32 box1 = 0x0404F00C;
    u32 box2 = 0x0404E418;
    s32 r;
    s8 t;
    u8 m;
    switch (entity->state)
    {
    case 0:
        entity->timer++;
        m = entity->timer % 3;
        if (m == 0)
            HitBox_SetAttack(entity, &box0);
        else if (m == 1)
            HitBox_SetAttack(entity, &box1);
        else
            HitBox_SetAttack(entity, &box2);
        r = Entity_MoveWithPointCollision(entity, 0x10000, -0x20000);
        if (EntityIsOutsideScreen(entity, 1))
            goto delete;
        if (r & 7)
        {
            entity->state = 1;
            entity->subState = 0;
            entity->timer = 0x3C;
            entity->hit.flags |= 4;
            entity->hit.flags |= 2;
            return;
        }
        break;
    case 1:
        t = entity->timer--;
        if (t <= 0x28)
        {
            entity->hidden = Entity_GetVisibilityPattern(entity->timer);
            if ((s8)entity->timer <= 0)
            {
            delete:
                EntityDelete(entity);
                return;
            }
        }
        break;
    }
    if (entity->state == 0)
        Hit_CheckWithPlayer(contact, entity);
    Anim_Update(entity);
}

/* Burst of seven fragments (frames 0x24-0x2A) from the owner position with
 * random velocities, gravity 0x1800, the owner palette and facing, and a
 * lifetime of 40-43 frames (WingedSkeletonPiece_Update). */
extern void WingedSkeletonPiece_Update(struct EwramData_EntityData *);
extern u8 sWingedSkeletonGfx[], sWingedSkeletonSpriteSheet[];

void WingedSkeleton_SpawnDeathPieces(struct EwramData_EntityData *owner)
{
    s32 i;
    for (i = 0; i <= 6; i++)
    {
        struct EwramData_EntityData *effect = EntityCreateInRange(0x49, 0xCF, (u32 *)WingedSkeletonPiece_Update);
        if (effect != NULL)
        {
            effect->posX.whole = owner->posX.whole;
            effect->posY.whole = owner->posY.whole;
            effect->velX.whole = (RandomNumberGenerator() & 0x3FFFF) - 0x20000;
            effect->velY.whole = -(RandomNumberGenerator() & 0xFFFF) - 0x4000;
            effect->accelY.whole = 0x1800;
            effect->spriteLayer = owner->spriteLayer;
            Sprite_Attach(effect, sWingedSkeletonSpriteSheet, ObjGfx_Find(sWingedSkeletonGfx), owner->basePalette);
            effect->animFrame = i + 0x24;
            effect->flipX = owner->flipX;
            effect->scrollWithBg = 1;
            effect->drawFunc = (u32 *)Sprite_DrawEntity;
            effect->timer = RandomNumberGenerator() % 4 + 0x28;
        }
    }
}

/* Winged Skeleton helpers 0x080754E0..0x0807565F. */
extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);
extern void EntityDampVelocityX(struct EwramData_EntityData *entity, s32 factor);
extern void EntityAccelerateAndMove(struct EwramData_EntityData *entity, s32 maxSpeed);

/* Graphics of the Winged Skeleton. */
extern u8 sWingedSkeletonGfx[];
extern u8 sWingedSkeletonPalette[];
extern u8 sWingedSkeletonSpriteSheet[];

void WingedSkeletonShot_DamagePlayerOnContact(struct EwramData_EntityData *entity);

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
 * @brief 754E0 | Load the graphics of the Winged Skeleton; 0 when no slot is free.
 */
u8 WingedSkeleton_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sWingedSkeletonGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sWingedSkeletonPalette, 0);
    Sprite_Attach(entity, sWingedSkeletonSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 7552C | Player_ApplyContactDamage with (attack, 0, 0, 0) at the entity position
 * (same as Chaos2Part_DamagePlayerOnContact).
 */
void WingedSkeleton_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/**
 * @brief 75558 | Give child the Winged Skeleton graphics (palette row of entity),
 * animation 8; 0 when no graphics slot is free.
 */
s32 WingedSkeletonShot_LoadGraphics(struct EwramData_EntityData *entity, struct EwramData_EntityData *child)
{
    s32 gfx = ObjGfx_Find(sWingedSkeletonGfx);

    if (gfx < 0)
        return 0;
    Sprite_Attach(child, sWingedSkeletonSpriteSheet, gfx, entity->basePalette);
    Anim_StartIndexed(child, 8, 0, 1);
    Anim_Update(child);
    child->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 755A8 | Register the hit box 0x0404FC00 (contact WingedSkeletonShot_DamagePlayerOnContact).
 */
void WingedSkeletonShot_EnableHitBoxes(struct EwramData_EntityData *entity)
{
    u32 box = 0x0404FC00;

    Hit_Enable(entity, 8, WingedSkeletonShot_DamagePlayerOnContact, 0);
    HitBox_SetAttack(entity, &box);
    entity->hit.flags &= ~2;
    entity->hit.flags |= 4;
}

/**
 * @brief 755E4 | Contact: Player_ApplyContactDamage with (attack, 0, 0, 1).
 */
void WingedSkeletonShot_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0;
    contactDamage.unused = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief Death piece: slow down, fall, blink during the last 0x28 frames and
 * delete after timer.
 */
void WingedSkeletonPiece_Update(struct EwramData_EntityData *entity)
{
    EntityDampVelocityX(entity, 8);
    EntityAccelerateAndMove(entity, 0x80000);
    if (entity->timer <= 0x28)
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
    if ((s8)--entity->timer <= 0)
        EntityDelete(entity);
}
