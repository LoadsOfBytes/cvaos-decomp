/*
 * Manticore: body, overlay, tail tip and segments, shots and effects.
 */
#include "core.h"
#include "player_core.h"
#include "graphics.h"
#include "entity_util.h"
#include "structs/ewram.h"
#include "music.h"
#include "player_level.h"
#include "sound.h"
#include "gameplay.h"
#include "boss_defeat.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "hitbox.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"

extern void Manticore_UpdateWalk(struct EwramData_EntityData *entity);
extern void Manticore_UpdateAttack(struct EwramData_EntityData *entity);
extern s32 Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);

/**
 * @brief 72768 | Manticore update (pattern of EnemyKillerFishUpdate): wake
 * when on screen (Player_TrackEntity), hidden (hidden) more than 0x20 px above
 * or 0xF0 px below the screen, states 0 / 1 (Manticore_UpdateWalk / Manticore_UpdateAttack),
 * 2 dying (Enemy_UpdateFragmentDeath), 3 animation only; contact tests for itself and
 * its tail part (unk_4F8) except while dying.
 */
void EnemyManticoreUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[48];

    if (entity->state != 2 && Entity_IsInRect(entity, -16, 0, 272, 176))
        Player_TrackEntity(entity);
    if ((u16)(entity->posY.part16.integer + 0x20) > 0x110)
        entity->hidden = 1;
    else
        entity->hidden = 0;
    if (Enemy_IsStunned(entity))
    {
        if (entity->state != 2)
        {
            Hit_CheckWithPlayer(collision, entity);
            Hit_CheckWithPlayer(collision, entity->ext.manticore.tailTip);
        }
        return;
    }
    switch (entity->state)
    {
    case 0:
        Manticore_UpdateWalk(entity);
        break;
    case 1:
        Manticore_UpdateAttack(entity);
        break;
    case 2:
        Enemy_UpdateFragmentDeath(entity);
        break;
    case 3:
        Anim_Update(entity);
        return;
    }
    {
        u8 *frame = &entity->animFrame;
        u8 previous = *frame;
        Anim_Update(entity);
        if (previous != *frame)
            Entity_UpdateBoxes(entity);
    }
    if (entity->state != 2)
    {
        Hit_CheckWithPlayer(collision, entity);
        Hit_CheckWithPlayer(collision, entity->ext.manticore.tailTip);
    }
}

u8 Manticore_LoadGraphics(struct EwramData_EntityData *entity);
extern void Entity_EnableBoxes(struct EwramData_EntityData *entity, void *onHit, void *onHurt);
extern struct EwramData_EntityData *Manticore_CreateTailTip(struct EwramData_EntityData *entity);
extern struct EwramData_EntityData *Manticore_CreateTailSegment(struct EwramData_EntityData *entity, struct EwramData_EntityData *tail, s32 value, s32 index);
struct EwramData_EntityData *Manticore_CreateOverlay(struct EwramData_EntityData *parent);
extern void Manticore_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void Manticore_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit);

/**
 * @brief 72854 | Manticore create: the boss version (spawnParam0) is deleted
 * once beaten (bossEventFlags bit 10) unless in the boss rush mode bossRushMode,
 * where it gets +1000 HP and +20 to attack; palette 0x11; the tail head
 * Manticore_CreateTailTip (unk_4F8) and six segments Manticore_CreateTailSegment linked through
 * unk_504; frozen time gives state 3, otherwise any failed allocation
 * deletes it. The boss version starts song 0x10 and the boss HP bar
 * (Player_SetSelectedTarget).
 */
void EnemyManticoreCreate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *seg[6];
    s32 i;

    if (entity->spawnParam0 != 0 && !gEwramData->gameState.bossRushMode
        && (gEwramData->gameState.bossEventFlags & 0x400))
        goto delete;
    if ((u8)Manticore_LoadGraphics(entity) == 0)
        goto delete;
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 0x11;
    if (gEwramData->gameState.bossRushMode)
    {
        entity->hp += 1000;
        entity->attack += 20;
    }
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->ext.manticore.tailTip = Manticore_CreateTailTip(entity);
        for (i = 0; i <= 5; i++)
            seg[i] = Manticore_CreateTailSegment(entity, entity->ext.manticore.tailTip, 0, i);
        for (i = 0; i <= 4; i++)
            seg[i]->ext.manticoreTail.link.next = seg[i + 1];
        seg[5]->ext.manticoreTail.link.next = entity->ext.manticore.tailTip;
        Manticore_CreateOverlay(entity);
        entity->state = 3;
        return;
    }
    Entity_EnableBoxes(entity, Manticore_DamagePlayerOnContact, Manticore_OnHit);
    Entity_SnapToGround(entity);
    entity->ext.manticore.tailTip = Manticore_CreateTailTip(entity);
    if (entity->ext.manticore.tailTip == NULL)
        goto delete;
    for (i = 0; i <= 5; i++)
    {
        seg[i] = Manticore_CreateTailSegment(entity, entity->ext.manticore.tailTip, 0, i);
        if (seg[i] == NULL)
            goto delete;
    }
    for (i = 0; i <= 4; i++)
        seg[i]->ext.manticoreTail.link.next = seg[i + 1];
    seg[5]->ext.manticoreTail.link.next = entity->ext.manticore.tailTip;
    if (Manticore_CreateOverlay(entity) == 0)
    {
    delete:
        EntityDelete(entity);
        return;
    }
    entity->state = 0;
    if (entity->spawnParam0 != 0)
    {
        Music_SetCurrent(0x10);
        Player_SetSelectedTarget(entity);
        gEwramData->gameState.stateFlags |= 2;
    }
}

/* Enemy walker update (Manticore_UpdateWalk): approach, turn and attack decisions. */
extern s32 Target_IsInRect(s16, s16, s16, s16);
extern s32 Entity_MoveWithCollision(struct EwramData_EntityData *, s32, s32, s32, s32);

/* Walking step; one copy per animStep value so that every copy shares the 0x40
 * mask of the first flipX test (the copies are merged again by cross-jumping). */
#define STEP() do { s32 v = 0x18000; entity->velX.whole = v; if (!entity->flipX) entity->velX.whole = -v; } while (0)

/**
 * @brief Walk state machine (state 0): wake when the target is in the 240 x 80 box
 * or the Manticore is damaged, then walk at 1.5 px per frame and turn;
 * start the attack (state 1) at random when the target is in front within
 * 0x40 / 0x60 / 0x80 px.
 */
void Manticore_UpdateWalk(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg;
    s32 dx;
    s32 res;
    s32 found;
    s32 speed;
    do { /* FAKEMATCH: weights the refs of bg (allocated before the shared 0x40 mask) */
        bg = gEwramData->bgInfo + 1;
    } while (0);

    switch (entity->subState)
    {
    case 0:
        entity->flipX = Entity_IsTargetRight(entity);
        if (Target_IsInRect(entity->posX.part16.integer - 0x78, entity->posY.part16.integer - 0x28, 0xF0, 0x50) || Enemy_IsDamaged(entity))
        {
            entity->subState++;
            Anim_Start(entity, &sManticoreWalkAnimScript, 0, 1);
            entity->animStep = 9;
            entity->timer = 0x64;
            entity->velY.whole = 0x20000;
            entity->velX.whole = 0;
        }
        speed = 0x20000;
        entity->velY.whole = speed;
        Entity_MoveWithCollision(entity, speed, -0x200000, 0x80000, 0x10);
        break;
    case 1:
        if (entity->flipX)
            dx = 0x18;
        else
            dx = -0x18;
        if (Tile_Classify((s16)(bg->xPos.part16.integer + entity->posX.part16.integer + dx), (s16)(bg->yPos.part16.integer + entity->posY.part16.integer)))
        {
            entity->subState = 5;
            Anim_StartIndexed(entity, 8, 0, 0);
            entity->animDone = 0;
            entity->timer = 0;
            break;
        }
        if (entity->animTimer == 0)
        {
            if (entity->animStep == 2) STEP();
            else if (entity->animStep == 5) STEP();
            else if (entity->animStep == 8) STEP();
            else if (entity->animStep == 0xB) STEP();
        }
        res = Entity_MoveWithCollision(entity, 0x20000, -0x200000, 0x80000, 0x10);
        EntityDampVelocityX(entity, 4);
        if ((res & 4) && *(u16 *)&entity->animStep == 10)
        {
            if (entity->flipX != Entity_IsTargetRight(entity))
                goto turn;
            if (entity->flipX)
                dx = 0;
            else
                dx = -0x40;
            found = Target_IsInRect(entity->posX.part16.integer + dx, entity->posY.part16.integer - 0x3C, 0x40, 0x46);
            if (found)
            {
                if ((RandomNumberGenerator() & 0xFF) <= 0x7F)
                {
                    entity->state = 1;
                    entity->subState = 0;
                    break;
                }
            }
            else
            {
                do { /* FAKEMATCH: weights the refs of the flipX byte pointer (global-alloc priority) */
                    if (entity->flipX)
                        dx = 0;
                    else
                        dx = -0x60;
                } while (0);
                if (Target_IsInRect(entity->posX.part16.integer + dx, entity->posY.part16.integer - 0x3C, 0x60, 0x46) && (RandomNumberGenerator() & 0xFF) <= 0x3F)
                {
                    entity->state = 1;
                    do { /* FAKEMATCH: weights the refs of found (r4 before the 0x40 mask) */
                        entity->subState = found;
                    } while (0);
                    break;
                }
            }
            do { /* FAKEMATCH: weights the refs of the flipX byte pointer (global-alloc priority) */
                if (entity->flipX)
                    dx = 0;
                else
                    dx = -0x80;
            } while (0);
            if (Target_IsInRect(entity->posX.part16.integer + dx, entity->posY.part16.integer - 0x3C, 0x80, 0x46) && (RandomNumberGenerator() & 0xFF) <= 0x7F)
            {
                entity->state = 1;
                entity->subState = 10;
                break;
            }
        }
        if (EntityIsOutsideScreen(entity, 4))
        {
            entity->subState = 2;
            Anim_StartIndexed(entity, 0, 0, 1);
        }
        break;
    case 2:
        if (entity->flipX != Entity_IsTargetRight(entity))
        {
        turn:
            entity->subState = 3;
            Anim_StartIndexed(entity, 8, 0, 0);
            entity->animDone = 0;
            entity->timer = 0;
            break;
        }
        entity->subState = 1;
        Anim_Start(entity, &sManticoreWalkAnimScript, 0, 1);
        entity->animStep = 9;
        entity->timer = 0x64;
        speed = 0x20000;
        entity->velX.whole = 0;
        entity->velY.whole = speed;
        Entity_MoveWithCollision(entity, speed, -0x200000, 0x80000, 0x10);
        break;
    case 3:
        entity->timer++;
        if (entity->animDone)
        {
            entity->flipX ^= 1;
            entity->subState = 2;
            Anim_StartIndexed(entity, 0, 0, 1);
        }
        break;
    case 5:
        entity->timer++;
        if (entity->animDone)
        {
            entity->flipX ^= 1;
            entity->subState = 1;
            Anim_Start(entity, &sManticoreWalkAnimScript, 0, 1);
            entity->velY.whole = 0x20000;
            entity->velX.whole = 0;
        }
        break;
    }
}

/* Manticore_UpdateAttack: enemy attack sequence that spawns two projectiles on frame 14. */
extern void Manticore_SpawnShot(struct EwramData_EntityData *entity);
extern void Manticore_SpawnShotFlash(struct EwramData_EntityData *entity);

#define FRAME_PAIR(e) (*(u16 *)&(e)->animStep)

struct FramePairBits { u16 value:16; };
struct FramePairBits32 { u32 value:16; };

/**
 * @brief Attack state machine (state 1): animations 2 and 3, then the breath
 * (Manticore_SpawnShot + Manticore_SpawnShotFlash, sounds 0x7A / 0x77);
 * back to state 0.
 */
void Manticore_UpdateAttack(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.manticore.tailTip; /* tail tip */

    switch (entity->subState)
    {
    case 0:
        Anim_StartIndexed(entity, 2, 0, 0);
        entity->subState++;
        parent->ext.manticoreTail.step = 0;
        break;
    case 1:
        if (entity->animStep == 0)
            break;
        entity->animFlags |= 2;
        entity->subState++;
        break;
    case 2:
        if (parent->ext.manticoreTail.step <= 2)
            break;
        entity->animFlags &= ~2;
        entity->subState++;
        break;
    case 3:
        if (parent->ext.manticoreTail.step <= 4)
            break;
        Anim_StartIndexed(entity, 3, 0, 0);
        entity->subState++;
        entity->animDone = 0;
        entity->timer = 0;
        break;
    case 4:
        if (entity->timer++ <= 0x1D)
            break;
        entity->state = 0;
        entity->subState = 2;
        Anim_StartIndexed(entity, 0, 0, 1);
        break;
    case 10:
        Anim_StartIndexed(entity, 4, 0, 0);
        entity->subState++;
        parent->ext.manticoreTail.step = 0;
        entity->timer = 0;
        entity->ext.manticore.shotCount = 0;
        if ((RandomNumberGenerator() & 0xFF) <= 0x7F)
            entity->ext.manticore.shotTotal = 1;
        else
            entity->ext.manticore.shotTotal = (RandomNumberGenerator() % 2) + 2;
        break;
    case 11:
    {
        u32 m = 0xFFFF;
        if ((FRAME_PAIR(entity) & m) == 0xE)
        {
            PlaySong(0x7A);
            Manticore_SpawnShot(entity);
            Manticore_SpawnShotFlash(entity);
            PlaySong(0x77);
        }
        m &= FRAME_PAIR(entity);
        if (m == 0x14)
        {
            if ((s8)++entity->ext.manticore.shotCount < (s8)entity->ext.manticore.shotTotal)
            {
                Anim_StartIndexed(entity, 4, 0, 0);
                entity->animTimer = 0;
                entity->animStep = 2;
            }
        }
        if (entity->animDone)
        {
            entity->subState++;
            entity->timer = 0;
        }
        break;
    }
    case 12:
        if (entity->timer++ <= 3)
            break;
        entity->state = 0;
        entity->subState = 2;
        Anim_StartIndexed(entity, 0, 0, 1);
        break;
    }
}

extern void BossRecoveryPickup_Spawn(s32, s32, s32, s32, s32);

/**
 * @brief 72F40 | Enemy damage callback: subtract the damage from HP
 * (hp), flash (hitFlashTimer = 8); at 0 HP: Enemy_DropRewards, song 0x72, death
 * animation 7 with palette basePalette, state 2 for 40 frames; with a drop id
 * (spawnParam0) spawn BossDefeatEffect_Spawn and the item BossRecoveryPickup_Spawn (id 10) at its
 * x / 0x20, or at room (0x100, 0x350) when bossRushMode is 0, then
 * Player_SetSelectedTarget(NULL).
 */
void Manticore_OnDamaged(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit, s32 damage)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];

    entity->hitFlashTimer = 8;
    entity->hp -= damage;
    if ((s16)entity->hp > 0)
        return;
    Enemy_DropRewards(entity);
    PlaySong(0x72);
    Anim_StartIndexed(entity, 7, 0, 0);
    entity->palette = entity->basePalette;
    entity->timer = 0x28;
    entity->state = 2;
    entity->subState = 0;
    if (entity->spawnParam0 == 0)
        return;
    BossDefeatEffect_Spawn(entity->posX.whole, entity->posY.whole - 0x200000);
    if (gEwramData->gameState.bossRushMode)
        BossRecoveryPickup_Spawn((s16)entity->posX.part16.integer, 0x20, 0xA, 0x60, 0);
    else
        BossRecoveryPickup_Spawn((s16)(0x100 - bg->xPos.part16.integer), (s16)(0x350 - bg->yPos.part16.integer), 0xA, 0x60, 0);
    Player_SetSelectedTarget(NULL);
}

void ManticoreOverlay_Update(struct EwramData_EntityData *);

/**
 * @brief Create the overlay sprite drawn three layers above the Manticore that
 * copies its animation frame (ManticoreOverlay_Update); return it or NULL.
 */
struct EwramData_EntityData *Manticore_CreateOverlay(struct EwramData_EntityData *parent)
{
    struct EwramData_EntityData *child = Entity_CreateObject(ManticoreOverlay_Update);
    s32 slot;
    if (child != NULL)
    {
        child->posX.whole = parent->posX.whole;
        child->posY.whole = parent->posY.whole;
        child->spriteLayer = parent->spriteLayer + 3;
        slot = ObjGfx_Find(sManticoreGfx);
        Sprite_Attach(child, sManticoreOverlaySpriteSheet, slot, parent->basePalette);
        child->animFrame = parent->animFrame;
        child->scrollWithBg = 0;
        child->drawFunc = (u32 *)Sprite_DrawEntity;
        ENTITY_CREATOR(child) = parent;
        }
    else return NULL;
    return child;
}

/**
 * @brief Overlay update: copy the position, palette, facing, frame and visibility
 * of the owner (type 0x36); delete when the owner is gone.
 */
void ManticoreOverlay_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = ENTITY_CREATOR(entity);
    if (parent != NULL && parent->updateFunc != NULL && parent->enemyId == 0x36)
    {
        entity->posX.whole = parent->posX.whole;
        entity->posY.whole = parent->posY.whole;
        entity->palette = parent->palette;
        entity->flipX = parent->flipX;
        entity->animFrame = parent->animFrame;
        entity->hidden = parent->hidden;
    }
    else EntityDelete(entity);
}

extern void ManticoreTailTip_Update(struct EwramData_EntityData *);
extern void ManticoreTail_EnableHitBoxes(struct EwramData_EntityData *);
/**
 * @brief Create the tail tip of owner (affine sprite, contact damage through
 * ManticoreTail_EnableHitBoxes outside the bestiary preview); return it or
 * NULL when no affine slot is free.
 */
struct EwramData_EntityData *Manticore_CreateTailTip(struct EwramData_EntityData *owner)
{
    struct EwramData_EntityData *entity = Entity_CreateObject(ManticoreTailTip_Update);
    struct EwramData **ewram = &gEwramData;
    if (entity != NULL)
    {
        s32 graphics;
        u16 slot;
        entity->spriteLayer = owner->spriteLayer + 1;
        graphics = ObjGfx_Find(sManticoreGfx);
        Sprite_Attach(entity, sManticoreSpriteSheet, graphics, owner->basePalette);
        entity->flipY = 1;
        entity->animFrame = 0;
        entity->enemyId = owner->enemyId;
        entity->hp = 1;
        entity->scrollWithBg = 0;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        ENTITY_OWNER(entity) = owner;
        entity->ext.manticoreTail.kind = 6;
        entity->ext.manticoreTail.link.value = 0x4000;
        entity->attack = owner->attack;
        slot = ObjAffine_Alloc(1);
        if ((s16)slot < 0) { EntityDelete(entity); return NULL; }
        entity->affineIndex = slot;
        entity->affine = 1;
        if (!(*ewram)->gameState.bestiaryPreview) ManticoreTail_EnableHitBoxes(entity);
        return entity;
    }
    return NULL;
}

extern void ManticoreTail_UpdateSwing(struct EwramData_EntityData *);
/**
 * @brief Tail tip update: anchor at the owner's back, swing through
 * ManticoreTail_UpdateSwing and update the affine matrix; delete when the
 * owner (type 0x36) is gone.
 */
void ManticoreTailTip_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);
    if (owner == NULL || owner->updateFunc == NULL || owner->enemyId != 0x36)
        goto remove;
    entity->posX.whole = owner->posX.whole;
    entity->posY.whole = owner->posY.whole;
    if (owner->flipX)
    {
        entity->velX.whole = -(((u8)entity->ext.manticoreTail.kind << 19) + 0xC0000);
        entity->posX.whole -= 0xC0000;
    }
    else
    {
        entity->velX.whole = ((u8)entity->ext.manticoreTail.kind << 19) + 0xC0000;
        entity->posX.whole += 0xC0000;
    }
    entity->posY.whole -= 0xE0000;
    entity->flipX = owner->flipX == 0;
    entity->hidden = owner->hidden;
    ManticoreTail_UpdateSwing(entity);
    entity->palette = owner->palette;
    ObjAffine_Set(entity->velY.whole, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
    return;
remove:
    ObjAffine_Free(entity->affineIndex);
    EntityDelete(entity);
}

extern void ManticoreTailSegment_Update(struct EwramData_EntityData *);
struct EwramData_EntityData *Manticore_CreateTailSegment(struct EwramData_EntityData *owner, struct EwramData_EntityData *part, s32 value, s32 kind)
{
    struct EwramData_EntityData *entity = Entity_CreateObject(ManticoreTailSegment_Update);
    if (entity != NULL)
    {
        s32 graphics;
        u16 slot;
        entity->spriteLayer = part->spriteLayer + 1;
        graphics = ObjGfx_Find(sManticoreGfx);
        Sprite_Attach(entity, sManticoreSpriteSheet, graphics, owner->basePalette);
        if (kind == 5) entity->animFrame = 1;
        else entity->animFrame = 2;
        slot = ObjAffine_Alloc(1);
        if ((s16)slot >= 0)
        {
            entity->affineIndex = slot;
            entity->affine = 1;
            entity->enemyId = part->enemyId;
            entity->scrollWithBg = 0;
            entity->drawFunc = (u32 *)Sprite_DrawEntity;
            ENTITY_OWNER(entity) = owner;
            entity->ext.manticoreTail.anchor = part;
            entity->ext.manticoreTail.kind = kind;
            entity->ext.manticoreTail.link.value = value;
            return entity;
        }
        EntityDelete(entity);
    }
    return NULL;
}

extern u16 ArcTan2(s32, s32);
#define KIND(e) ((u8)e->ext.manticoreTail.kind)
#define DONE(e) (*(u8 *)&e->ext.manticoreTail.updated)
#define NEXT(e) (e->ext.manticoreTail.link.next)
static inline s32 ToPixels(s32 x) { if (x < 0) return (s16)-(-x >> 16); return x >> 16; }

/* Update a linked part, once per chain traversal, and face the next part. */
void ManticoreTailSegment_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);
    struct EwramData_EntityData *next;
    s32 zero, i, x, y, angle;
    if (owner == NULL || owner->updateFunc == NULL || owner->enemyId != 0x36)
        goto remove;
    if (KIND(entity) == 0)
    {
        next = entity;
        zero = 0;
        for (i = 5; i >= 0; i--)
        {
            DONE(next) = zero;
            next = NEXT(next);
        }
    }
    if (DONE(entity)) return;
    next = NEXT(entity);
    if (KIND(entity) != 5 && next->updateFunc != NULL)
        ((void (*)(struct EwramData_EntityData *))next->updateFunc)(next);
    DONE(entity) = 1;
    entity->posX.whole = owner->posX.whole;
    entity->posY.whole = owner->posY.whole;
    if (owner->flipX)
    {
        if (KIND(entity) == 0)
            entity->velX.whole = -((KIND(entity) << 19) + 0x60000);
        else
            entity->velX.whole = -((KIND(entity) << 19) + 0xA0000);
        entity->posX.whole -= 0xC0000;
    }
    else
    {
        if (KIND(entity) == 0)
            entity->velX.whole = (KIND(entity) << 19) + 0x60000;
        else
            entity->velX.whole = (KIND(entity) << 19) + 0xA0000;
        entity->posX.whole += 0xC0000;
    }
    entity->posY.whole -= 0x150000;
    entity->flipX = owner->flipX;
    entity->hidden = owner->hidden;
    ManticoreTail_UpdateSwing(entity);
    entity->palette = owner->palette;
    x = next->posX.whole - entity->posX.whole;
    y = next->posY.whole - entity->posY.whole;
    x = ToPixels(x);
    y = ToPixels(y);
    angle = (u16)ArcTan2(x, y);
    if (!entity->flipX) angle += 0x8000;
    ObjAffine_Set(angle, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
    return;
remove:
    ObjAffine_Free(entity->affineIndex);
    EntityDelete(entity);
}

#define TIMER(e) (e->ext.manticoreTail.timer)
#undef STEP
#define STEP(e) (e->ext.manticoreTail.step)
#define VALUE(e) (e->ext.manticoreTail.link.value)
#define SETANGLE(e) \
    do { \
        if ((e)->flipX) angle = -0x2000 - VALUE(e); \
        else angle = VALUE(e) + 0x12000; \
        (e)->velY.whole = angle; \
    } while (0)

/* Swing a linked part (chain link of ManticoreTailSegment_Update) around its owner depending on the owner's state. */
void ManticoreTail_UpdateSwing(struct EwramData_EntityData *e)
{
    struct EwramData_EntityData *owner = ENTITY_OWNER(e);
    struct EwramData_EntityData *part = e->ext.manticoreTail.anchor;
    s32 angle, x, y, r;

    if (Enemy_IsStunnedNoFlash(owner) == 0)
    {
        if (owner->animScriptIndex == 0)
        {
            if (KIND(e) == 6)
            {
                switch (owner->animStep)
                {
                case 0:
                case 5:
                case 6:
                case 7:
                    if (VALUE(e) < 0x4000) VALUE(e) += 0x60;
                    break;
                case 1:
                case 2:
                case 3:
                case 4:
                    if (VALUE(e) > 0x3800) VALUE(e) -= 0x60;
                    break;
                }
                SETANGLE(e);
            }
        }
        else if (owner->animScriptIndex == 1)
        {
            if (KIND(e) == 6)
            {
                switch (owner->animStep)
                {
                case 2:
                case 3:
                case 8:
                case 9:
                    if (VALUE(e) > 0x3800) VALUE(e) -= 0x60;
                    break;
                default:
                    if (VALUE(e) < 0x4000) VALUE(e) += 0x40;
                    break;
                }
                SETANGLE(e);
            }
        }
        else if ((u8)(owner->animScriptIndex - 2) <= 1)
        {
            if (KIND(e) == 6)
            {
                u8 step = STEP(e);
                s32 t;
                if (STEP(e) == 0)
                {
                    STEP(e) = step + 1;
                    TIMER(e) = 0;
                }
                else if (STEP(e) == 1)
                {
                    t = TIMER(e) + 1;
                    TIMER(e) = t;
                    if (t & 8)
                    {
                        s32 d = VALUE(e) - 0x8000;
                        s32 ad = d;
                        if (d < 0) ad = -d;
                        if (ad > 0x40)
                        {
                            if (d > 0) VALUE(e) -= 0x40;
                            else VALUE(e) += 0x40;
                        }
                        if (e->flipX) { t = TIMER(e); angle = ({ s32 a_ = -0x2000 - ((~t & 7) << 9); a_; }) - VALUE(e); }
                        else { t = TIMER(e); angle = VALUE(e) + ({ s32 a_ = ((~t & 7) << 9) + 0x12000; a_; }); }
                    }
                    else
                    {
                        if (e->flipX) { angle = ({ s32 a_ = -0x2000 - ((t & 7) << 9); a_; }) - VALUE(e); }
                        else { angle = VALUE(e) + ({ s32 a_ = ((t & 7) << 9) + 0x12000; a_; }); }
                    }
                    e->velY.whole = angle;
                    if ((u8)t > 0x1F)
                    {
                        STEP(e)++;
                        TIMER(e) = 0;
                    }
                }
                else if (STEP(e) == 2)
                {
                    TIMER(e)++;
                    if (TIMER(e) > 0x13)
                    {
                        TIMER(e) = 0;
                        STEP(e) = step + 1;
                        PlaySong(0x86);
                    }
                }
                else if (STEP(e) == 3)
                {
                    VALUE(e) += 0x800;
                    if (VALUE(e) > 0x7FFF)
                    {
                        STEP(e) = step + 1;
                        VALUE(e) = 0x8000;
                    }
                }
                else if (STEP(e) == 4)
                {
                    TIMER(e)++;
                    if (TIMER(e) > 0xB)
                    {
                        TIMER(e) = 0;
                        STEP(e) = step + 1;
                    }
                }
                else if (STEP(e) == 5)
                {
                    VALUE(e) -= 0x400;
                    if (VALUE(e) < 0x4000)
                    {
                        VALUE(e) = 0x4000;
                        STEP(e) = step + 1;
                        TIMER(e) = 0;
                    }
                }
                if (STEP(e) != 1) SETANGLE(e);
            }
        }
        else if (owner->animScriptIndex == 4)
        {
            if (KIND(e) == 6) SETANGLE(e);
        }
        else if (owner->animScriptIndex == 7)
        {
            if (KIND(e) == 6) SETANGLE(e);
        }
        else if (owner->animScriptIndex == 8)
        {
            u8 kind = e->ext.raw.unk_500.whole; /* word read of ext.manticoreTail.kind (the byte member changes the code) */
            if (kind == 6)
            {
                if (owner->timer >= 10 && owner->timer <= 14)
                {
                    e->posX.whole = owner->posX.whole;
                    e->posY.whole = owner->posY.whole;
                    if (owner->flipX)
                    {
                        e->posX.whole += 0x80000;
                        e->velX.whole = (KIND(e) << 19) + 0xC0000;
                    }
                    else
                    {
                        e->posX.whole -= 0x80000;
                        e->velX.whole = -((KIND(e) << 19) + 0xC0000);
                    }
                    e->posY.whole -= 0xE0000;
                    e->flipX = owner->flipX;
                }
                else if (owner->timer > 14)
                {
                    e->posX.whole = owner->posX.whole;
                    e->posY.whole = owner->posY.whole;
                    if (owner->flipX)
                    {
                        e->posX.whole += 0xC0000;
                        e->velX.whole = (KIND(e) << 19) + 0xC0000;
                    }
                    else
                    {
                        e->posX.whole -= 0xC0000;
                        e->velX.whole = -((KIND(e) << 19) + 0xC0000);
                    }
                    e->posY.whole -= 0xE0000;
                    e->flipX = owner->flipX;
                }
                SETANGLE(e);
            }
            else
            {
                if (owner->timer >= 10 && owner->timer <= 14)
                {
                    e->posX.whole = owner->posX.whole;
                    e->posY.whole = owner->posY.whole;
                    if (owner->flipX)
                    {
                        if (kind == 0) e->velX.whole = (KIND(e) << 19) + 0x60000;
                        else e->velX.whole = (KIND(e) << 19) + 0xA0000;
                        e->posX.whole += 0x80000;
                    }
                    else
                    {
                        if (kind == 0) e->velX.whole = -((KIND(e) << 19) + 0x60000);
                        else e->velX.whole = -((KIND(e) << 19) + 0xA0000);
                        e->posX.whole -= 0x80000;
                    }
                }
                else if (owner->timer > 14)
                {
                    e->posX.whole = owner->posX.whole;
                    e->posY.whole = owner->posY.whole;
                    if (owner->flipX)
                    {
                        if (kind == 0) e->velX.whole = (KIND(e) << 19) + 0x60000;
                        else e->velX.whole = (KIND(e) << 19) + 0xA0000;
                        e->posX.whole += 0xC0000;
                    }
                    else
                    {
                        if (kind == 0) e->velX.whole = -((KIND(e) << 19) + 0x60000);
                        else e->velX.whole = -((KIND(e) << 19) + 0xA0000);
                        e->posX.whole -= 0xC0000;
                    }
                }
                else
                    goto tail;
                e->posY.whole -= 0x150000;
                e->flipX = owner->flipX == 0;
            }
        }
    }
    else
    {
        if (KIND(e) == 6) SETANGLE(e);
    }
tail:
    if (KIND(e) == 6)
    {
        x = (VALUE(e) - 0x7000) / 4;
        x = FixedMultiply(e->velX.whole, x + 0x4000);
        x = FixedDivide(x, 0x4000);
        e->velX.whole = x;
        if (e->flipX) angle = 0x10000 - VALUE(e);
        else angle = VALUE(e);
        x = Sin(angle + 0x4000);
        y = Sin(angle);
        x = FixedMultiply(x, e->velX.whole);
        y = FixedMultiply(y, e->velX.whole);
        e->posX.whole += x;
        e->posY.whole += y;
    }
    else
    {
        /* do-while(0) wrappers raise register priority of r (allocation as in ROM) */
        do { r = VALUE(part); } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
        x = (r - 0x4000) * 5 / 6 * 24 + 0x70000;
        y = (KIND(e) << 16) + x - 0x60000;
        r = FixedMultiply(r, y);
        do { r = FixedDivide(r, x); } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
        x = (VALUE(part) - 0x7000) / 4;
        x = FixedMultiply(e->velX.whole, x + 0x4000);
        x = FixedDivide(x, 0x4000);
        e->velX.whole = x;
        do { if (!e->flipX) r = 0x10000 - r; } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
        e->velY.whole = angle;
        x = Sin(r + 0x4000);
        do { y = Sin(r); } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
        x = FixedMultiply(x, e->velX.whole);
        y = FixedMultiply(y, e->velX.whole);
        e->posX.whole += x;
        e->posY.whole += y;
        e->velX.whole = 0;
        e->velY.whole = 0;
    }
}

extern void ManticoreShot_Update(struct EwramData_EntityData *entity);
extern s32 ManticoreShot_LoadGraphics(struct EwramData_EntityData *entity, struct EwramData_EntityData *shot);
extern void ManticoreShot_EnableHitBoxes(struct EwramData_EntityData *entity);

/**
 * @brief 73B54 | Create a shot ManticoreShot_Update of the Manticore code
 * (0x08073xxx): set up by ManticoreShot_LoadGraphics (deleted when that fails, but still
 * initialised afterwards as in the original) and ManticoreShot_EnableHitBoxes, layer one
 * below the owner (unk_4F4), 44 px in front and 15 px above it, velX
 * 0x40000 in its facing, hidden (hidden), owner's spawnIndex, palette and
 * attack.
 */
void Manticore_SpawnShot(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *shot = EntityCreateInRange(0x1B, 0x48, (u32 *)ManticoreShot_Update);

    if (shot == NULL)
        return;
    if ((u8)ManticoreShot_LoadGraphics(entity, shot) == 0)
        EntityDelete(shot);
    shot->scrollWithBg = 1;
    ManticoreShot_EnableHitBoxes(shot);
    shot->spriteLayer = entity->spriteLayer - 1;
    ENTITY_CREATOR(shot) = entity;
    shot->flipX = entity->flipX;
    if (entity->flipX)
    {
        shot->posX.part16.integer = entity->posX.part16.integer + 0x2C;
        shot->velX.whole = 0x40000;
    }
    else
    {
        shot->posX.part16.integer = entity->posX.part16.integer - 0x2C;
        shot->velX.whole = -0x40000;
    }
    shot->posY.part16.integer = entity->posY.part16.integer - 0xF;
    shot->velY.whole = 0;
    shot->accelX.whole = 0;
    shot->accelY.whole = 0;
    shot->hidden = 1;
    shot->spawnIndex = entity->spawnIndex;
    shot->basePalette = entity->basePalette;
    shot->attack = entity->attack;
}

/* Projectile: wait 16 frames, then (state 1) fall until off screen; in
 * state 2 it falls emitting two random sparks per frame (Effect_SpawnFadingParticle /
 * Effect_SpawnParticleAnim0) and vanishes after a few frames. */

/**
 * @brief Breath shot update: wait 16 frames hidden, then fly (animation 6) until
 * off screen; state 2 bursts into particles and vanishes.
 */
void ManticoreShot_Update(struct EwramData_EntityData *entity)
{
    u32 buffer[4];
    s32 vx, vy;

    switch (entity->state)
    {
    case 0:
        if (entity->timer++ > 15)
        {
            entity->state++;
            Anim_StartIndexed(entity, 6, 0, 1);
            entity->hit.flags &= ~4;
            entity->hit.flags &= ~2;
            entity->hidden = 0;
        }
        break;
    case 1:
        EntityAccelerateAndMove(entity, 0x80000);
        if (!EntityIsOutsideScreen(entity, 1))
            break;
        goto delete;
    case 2:
        EntityAccelerateAndMove(entity, 0x80000);
        vx = entity->flipX ? 0x30000 : -0x30000;
        vx += RandomNumberGenerator() % 0x30000 - 0x18000;
        vy = RandomNumberGenerator() % 0x30000 - 0x18000;
        Effect_SpawnFadingParticle(entity->posX.whole, entity->posY.whole, vx, vy, 10, 1);
        vx = entity->flipX ? 0x30000 : -0x30000;
        vx += RandomNumberGenerator() % 0x30000 - 0x18000;
        vy = RandomNumberGenerator() % 0x30000 - 0x18000;
        Effect_SpawnParticleAnim0(entity->posX.whole, entity->posY.whole, vx, vy, 10);
        if (entity->timer++ > 1)
            entity->hidden = 1;
        if (entity->timer > 5)
        {
        delete:
            EntityDelete(entity);
            return;
        }
        break;
    }
    if (entity->state != 2)
        Hit_CheckWithPlayer(buffer, entity);
    Anim_Update(entity);
}

/* Spawn an effect 31 px in front of and 15 px above the owner (graphics
 * 081CBF54 / 082145BC in the owner palette, animation 5), one priority
 * level above it; updated by ManticoreEffect_Update. */
extern void ManticoreEffect_Update(struct EwramData_EntityData *);

/**
 * @brief Spawn the muzzle flash 31 px in front of and 15 px above owner; it
 * vanishes after its animation (ManticoreEffect_Update).
 */
void Manticore_SpawnShotFlash(struct EwramData_EntityData *owner)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)ManticoreEffect_Update);
    s32 graphics;

    if (entity == NULL)
        return;
    entity->spriteLayer = owner->spriteLayer - 1;
    entity->flipX = owner->flipX;
    if (entity->flipX)
        entity->posX.part16.integer = owner->posX.part16.integer + 0x1F;
    else
        entity->posX.part16.integer = owner->posX.part16.integer - 0x1F;
    entity->posY.part16.integer = owner->posY.part16.integer - 0xF;
    graphics = ObjGfx_Find(sManticoreGfx);
    Sprite_Attach(entity, sManticoreSpriteSheet, graphics, owner->basePalette);
    Anim_StartIndexed(entity, 5, 0, 0);
    Anim_Update(entity);
    entity->scrollWithBg = 1;
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    entity->timer = 0;
}

/* Box callbacks and hit-effect setup, 0x08073E58..0x08073EE7. */
void ManticoreTail_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void ManticoreTail_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit);

void ManticoreTail_EnableHitBoxes(struct EwramData_EntityData *entity)
{
    Hit_Enable(entity, 8, ManticoreTail_DamagePlayerOnContact, (s32)ManticoreTail_OnHit);
    HitBox_SetBodyRaw(entity, 0x0C0CFAFA); /* box {-6, -6, 12, 12} */
    HitBox_SetAttackRaw(entity, 0x0C0CFAFA);
    entity->hit.flags &= ~4;
    entity->hit.flags &= ~2;
}

/**
 * @brief Contact of a tail part: Player_ApplyContactDamage with attribute 0x40,
 * the owner's full attack during the sting (state 1, sub-states <= 4), else
 * half of it.
 */
void ManticoreTail_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage effect;
    struct EwramData_EntityData *parent = ENTITY_OWNER(entity);
    if (parent->state == 1 && parent->subState <= 4)
        effect.attack = parent->attack;
    else
        effect.attack = parent->attack >> 1;
    effect.attributes = 0x40;
    effect.unused = 0;
    effect.reaction = 1;
    effect.sourceX = entity->posX.whole;
    effect.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&effect);
}

/* ManticoreTail_OnHit (Manticore body part hit). */
extern s16 Hit_ResolveDamage(struct EwramData_EntityData *entity, struct EwramData_EntityData *other, u16 *hit, s32 inputDamage);
extern void Entity_ShowDamage(struct EwramData_EntityData *owner, s16 value, s32 kind);

/**
 * @brief 73EE8 | A part was hit: compute the damage for the parent
 * (unk_4F8) as if it stood at the part's position, show it on the part
 * (Entity_ShowDamage) and pass the hit on to the parent (Manticore_OnDamaged).
 */
void ManticoreTail_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    struct EwramData_EntityData *manticore = ENTITY_OWNER(entity);
    s32 oldX = manticore->posX.whole;
    s32 oldY = manticore->posY.whole;
    s16 damage;
    s16 attackPower;

    manticore->posX.whole = entity->posX.whole;
    manticore->posY.whole = entity->posY.whole;
    attackPower = Player_CalculateAttackPower(hit[2]);
    damage = Hit_ResolveDamage(ENTITY_OWNER(entity), attacker, hit, attackPower);
    manticore->posX.whole = oldX;
    manticore->posY.whole = oldY;
    Entity_ShowDamage(entity, damage, 0);
    Manticore_OnDamaged(ENTITY_OWNER(entity), attacker, hit, damage);
}

/* Manticore helpers 0x08073F54..0x0807412F. */
extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);

void ManticoreShot_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void ManticoreShot_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);

/**
 * @brief 73F54 | Give shot Manticore's graphics (palette row of entity),
 * animation 6; 0 when no graphics slot is free.
 */
s32 ManticoreShot_LoadGraphics(struct EwramData_EntityData *entity, struct EwramData_EntityData *shot)
{
    s32 gfx = ObjGfx_Find(sManticoreGfx);

    if (gfx < 0)
        return 0;
    Sprite_Attach(shot, sManticoreSpriteSheet, gfx, entity->basePalette);
    Anim_StartIndexed(shot, 6, 0, 0);
    Anim_Update(shot);
    shot->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 73FA4 | Register the hit box (contact ManticoreShot_DamagePlayerOnContact, hit
 * ManticoreShot_OnHit) of size 0x1818F4F4.
 */
void ManticoreShot_EnableHitBoxes(struct EwramData_EntityData *entity)
{
    Hit_Enable(entity, 10, ManticoreShot_DamagePlayerOnContact, (s32)ManticoreShot_OnHit);
    HitBox_SetBodyRaw(entity, 0x1818F4F4);
    HitBox_SetAttackRaw(entity, 0x1818F4F4);
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
}

/**
 * @brief 73FE4 | Contact: Player_ApplyContactDamage(attack, 2, 0, 1), then go to state 2
 * with sound 0x76.
 */
void ManticoreShot_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 2;
    contactDamage.unused = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
    entity->state = 2;
    entity->subState = 0;
    entity->timer = 0;
    PlaySong(0x76);
}

/**
 * @brief 74028 | Hit: hit effect, then go to state 2 with sound 0x76.
 */
void ManticoreShot_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    Player_CalculateAttackPower(hit[2]);
    entity->state = 2;
    entity->subState = 0;
    entity->timer = 0;
    PlaySong(0x76);
}

/**
 * @brief 74048 | Delete the entity when 0x53D bit 0 is set, else animate
 * (same as Chaos2Effect_Update).
 */
void ManticoreEffect_Update(struct EwramData_EntityData *entity)
{
    if (entity->animDone)
        EntityDelete(entity);
    else
        Anim_Update(entity);
}

/**
 * @brief 7406C | Load the graphics of Manticore; 0 when no slot is free.
 */
u8 Manticore_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sManticoreGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sManticorePalette, 0);
    Sprite_Attach(entity, sManticoreSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 740B8 | Player_ApplyContactDamage with (attack / 2, 0, 0, 0) at the entity
 * position.
 */
void Manticore_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack >> 1;
    contactDamage.attributes = 0;
    contactDamage.unused = 0;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief 740E8 | Hit: damage from hit, damage number, then Manticore_OnDamaged.
 */
void Manticore_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    s16 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));

    Entity_ShowDamage(entity, damage, 0);
    Manticore_OnDamaged(entity, attacker, hit, damage);
}
