/*
 * Skeleton: patrol, combat movement, idle, attack, throw, death, projectiles; the Imp create.
 * Built without -f2003-patch (Skeleton_UpdatePatrol and the projectile / Imp create file need it off; the rest compiles the same either way).
 */
#include "core.h"
#include "graphics.h"
#include "structs/ewram.h"
#include "sound.h"
#include "gameplay.h"
#include "structs/entity_render_flags.h"
#include "entity_util.h"
#include "player_core.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "hitbox.h"
#include "imp.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "skeleton.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"
#include "damage_numbers.h"
#include "entity_hit.h"
#include "bg_and_collision.h"


void SkeletonProjectile_Update(struct EwramData_EntityData *);
void Skeleton_SpawnDeathPieces(struct EwramData_EntityData *);
void SkeletonProjectile_DamagePlayerOnContact(struct EwramData_EntityData *);
s32 Imp_LoadGraphics(struct EwramData_EntityData *entity);

/* Patrol and attack-state selection. */
/* Keep -f2003-patch disabled: signed distance comparisons must retain asrs. */

/**
 * @brief Patrol the stored room bounds; turn on obstacles and enter state 1 when the target is near.
 */
void Skeleton_UpdatePatrol(struct EwramData_EntityData *entity)
{
    s32 blocked = 0;
    u16 left = entity->ext.skeleton.patrolLeft;
    u16 right = entity->ext.skeleton.patrolRight;

    switch (entity->subState)
    {
        case 0:
            if (entity->animScriptIndex != 1)
                Anim_StartIndexed(entity,1,0,1);
            entity->subState++;
            break;
        case 1:
        {
            u16 distance = entity->ext.skeleton.homeX - GetEntityRoomXPositionIntegerWide(entity);
            if (entity->flipX)
            {
                if ((s16)right < GetEntityRoomXPositionIntegerWide(entity) || (s16)distance < -40)
                    entity->flipX = 0;
            }
            else
            {
                if ((s16)left > GetEntityRoomXPositionIntegerWide(entity) || (s16)distance > 40)
                    entity->flipX = 1;
            }
            if (Entity_Walk(entity,0x6000,sSkeletonWalkProfile,12) & 3)
                blocked = 1;
            if (Entity_ClampXToRoom(entity,1))
                blocked = 1;
            if (blocked)
                entity->flipX = 1 - entity->flipX;
            if (Target_IsNearEntity(entity,512,128))
            {
                entity->velX.whole = 0;
                entity->state = 1;
                entity->stage = 0;
                entity->subState = 0;
            }
            break;
        }
    }
}

/**
 * @brief Face the target, choose short walking intervals and select attack or idle states from proximity and random checks.
 */
void Skeleton_UpdateCombatMovement(struct EwramData_EntityData *entity)
{
    s32 state = entity->subState;
    s32 d0;
    s32 d;
    s32 d2;
    s32 anim;
    u32 r;

    switch (state)
    {
    case 0:
        d0 = (s16)Entity_GetTargetDistanceY(entity);
        if (d0 < 0)
            d0 = -d0;
        if (d0 > 0x40)
        {
            d = (s16)Entity_GetTargetDistanceX(entity);
            if (d < 0)
                d = -d;
            if (d > 0x200)
            {
                entity->state = 0;
                entity->stage = 0;
                entity->subState = 0;
                break;
            }
        }
        entity->ext.skeleton.cooldown = RandomNumberGenerator() % 32;
        if ((entity->ext.skeleton.flags & 0xA) == 0xA)
            entity->ext.skeleton.cooldown = 0x40;
        entity->ext.skeleton.cooldown = 1;
        entity->subState++;
        entity->stage = 0;
    case 1:
        Entity_FaceTarget(entity);
        entity->ext.skeleton.walkSpeed = 0x8000;
        if (!entity->flipX)
            entity->velX.whole = -entity->velX.whole;
        if (Entity_IsTargetInCenteredBox(entity, RandomNumberGenerator() % 64 + 0x80, 0x80))
            entity->ext.skeleton.walkSpeed = -entity->ext.skeleton.walkSpeed;
        anim = (entity->ext.skeleton.flags & 8) ? 2 : 1;
        if (entity->animScriptIndex != anim)
            Anim_StartIndexed(entity, anim, 0, 1);
        if ((entity->ext.skeleton.flags & 0xA) == 0xA)
        {
            entity->velX.whole = 0;
            entity->animFlags |= 2;
        }
        else
        {
            entity->animFlags &= ~2;
        }
        entity->timer = RandomNumberGenerator() % 8 + 1;
        entity->subState++;
        entity->stage = 0;
    case 2:
        if (entity->ext.skeleton.cooldown)
            entity->ext.skeleton.cooldown--;
        if ((entity->ext.skeleton.flags & 6) && !(RandomNumberGenerator() % 16))
            entity->ext.skeleton.flags |= 0x20;
        else
            entity->ext.skeleton.flags &= ~0x20;
        entity->ext.skeleton.walkSpeed -= entity->ext.skeleton.walkSpeed >> 4;
        if ((entity->ext.skeleton.flags & 0xA) == 0xA && entity->ext.skeleton.cooldown == 0)
        {
            r = RandomNumberGenerator() & 3;
            if (r)
            {
                entity->state = 4;
                entity->stage = 0;
                entity->subState = 0;
            }
            else
            {
                entity->state = 2;
                entity->stage = r;
                entity->subState = r;
            }
            break;
        }
        if (entity->timer == 0)
        {
            if (entity->ext.skeleton.cooldown == 0 && Entity_IsTargetInCenteredBox(entity, 0xC4, 0x80))
            {
                if (entity->ext.skeleton.flags & 6)
                {
                    entity->state = 4;
                    entity->stage = 0;
                    entity->subState = 0;
                }
                else
                {
                    entity->state = 3;
                    entity->stage = 0;
                    entity->subState = 0;
                }
                entity->velX.whole = 0;
                break;
            }
            entity->ext.skeleton.throwUrge++;
            if (RandomNumberGenerator() % 40 < entity->ext.skeleton.throwUrge)
            {
                d2 = (s16)Entity_GetTargetDistanceX(entity);
                if (d2 < 0)
                    d2 = -d2;
                if (d2 > 0x30)
                {
                    entity->ext.skeleton.throwUrge = 0;
                    if (RandomNumberGenerator() % 50 <= 14)
                    {
                        entity->state = 2;
                        entity->stage = 0;
                        entity->subState = 0;
                    }
                    break;
                }
                entity->ext.skeleton.throwUrge >>= 1;
            }
            entity->subState--;
            entity->stage = 0;
        }
        else
        {
            entity->timer--;
        }
        Entity_Walk(entity, entity->ext.skeleton.walkSpeed, (void *)sSkeletonWalkProfile, 0xC);
        break;
    }
}

/* Skeleton_UpdateIdle. */
/**
 * @brief Start animation 0 and count down 60 ticks before returning to combat movement state 1.
 */
void Skeleton_UpdateIdle(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            if (entity->animScriptIndex)
                Anim_StartIndexed(entity, 0, 0, 1);
            entity->timer = 0x3C;
            entity->subState++;
        case 1:
            if (--entity->timer == 0)
            {
                entity->state = 1;
                entity->stage = 0;
                entity->subState = 0;
            }
            break;
    }
}

/* Idle choice between two attacks. */

/* attack flags, see Skeleton_UpdateThrow */
#define FLAGS(e) ((e)->ext.skeleton.flags)

/**
 * @brief Select preparation animation 5 or 6 and pending attack flags; optionally create an attached part, then return to state 1.
 */
void Skeleton_UpdateAttackPreparation(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        if ((RandomNumberGenerator() & 7) == 0 && (FLAGS(entity) & 1))
        {
            Anim_StartIndexed(entity, 5, 0, 0);
            FLAGS(entity) = (FLAGS(entity) & ~1) | 2;
        }
        else
        {
            Anim_StartIndexed(entity, 6, 0, 0);
            if (Entity_CreateLinkedAndHide(entity, SkeletonAttachedPart_Update) == NULL)
            {
                EntityDelete(entity);
                entity->state = 1;
                entity->stage = 0;
                entity->subState = 0;
                break;
            }
            FLAGS(entity) |= 4;
        }
        entity->subState++;
    case 1:
        if (entity->animFlags & 4)
        {
            entity->state = 1;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    }
}

/* Attack phase spawning a projectile. */

/* attack flags: bit 3 alternate animation, bits 1/2 pending shots */
#undef FLAGS
#undef FLAGS
#define FLAGS(e) (e->ext.skeleton.flags)

/**
 * @brief Start animation 3 or 4, create a counted projectile on step 2 and return to state 1 when the animation finishes.
 */
void Skeleton_UpdateThrow(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *e;

    switch (entity->subState)
    {
    case 0:
        entity->subState = 1;
    case 1:
        if (FLAGS(entity) & 8)
        {
            Anim_StartIndexed(entity, 3, 0, 0);
            FLAGS(entity) &= ~8;
        }
        else
            Anim_StartIndexed(entity, 4, 0, 0);
        entity->subState++;
    case 2:
        if (entity->animStep == 2)
        {
            e = Entity_CreateObject(SkeletonProjectile_Update);
            if (e != NULL)
            {
                e->ext.skeletonPiece.ownerIndex = ENTITY_INDEX_PAIR(entity);
                if (FLAGS(entity) & 2)
                    e->ext.skeletonBone.kind = 1;
                entity->ext.skeleton.throwCount++;
            }
            entity->subState++;
            if (FLAGS(entity) & 2)
                FLAGS(entity) &= ~2;
            else
                FLAGS(entity) &= ~4;
        }
        break;
    case 3:
        if (entity->animFlags & 4)
        {
            entity->state = 1;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    }
}

/* Death sequence. */

/**
 * @brief Hide the entity, spawn death pieces and drop rewards; count down the byte timer, then free the affine slot and request deletion.
 */
void Skeleton_UpdateDeath(struct EwramData_EntityData *entity)
{
    entity->hidden = 1;
    switch (entity->subState)
    {
    case 0:
        Skeleton_SpawnDeathPieces(entity);
        Enemy_DropRewards(entity);
        entity->palette = entity->basePalette;
        entity->timer = 0xFF;
        entity->state = 0xFF;
        entity->subState++;
    case 1:
        if (--entity->timer == 0xFF)
        {
            ObjAffine_Free((u16)entity->ext.skeleton.affine);
            entity->deleteRequested = 1;
        }
        break;
    }
}

/* Hit callback that knocks the entity away. */

/**
 * @brief Clear health and affine mode; fall or reverse vertical velocity and bounce sideways according to projectile variant.
 */
void SkeletonProjectile_OnHit(struct EwramData_EntityData *entity, s32 hit, u16 *data)
{
    s32 speed;

    Player_CalculateAttackPower(data[2]);
    entity->hp = 0;
    entity->affine = 0;
    entity->hitFlashTimer = 8;
    if ((u8)entity->ext.skeletonBone.kind == 0)
    {
        entity->timer = 0x32;
        entity->velX.whole = 0;
        entity->accelX.whole = 0;
        entity->velY.whole = 0x4000;
        entity->accelY.whole = 0x2000;
        PlaySong(0x151);
    }
    else
    {
        if ((s8)Entity_GetTargetDirection(entity) > 0)
        {
            speed = entity->velX.whole;
            if (speed < 0)
                speed = -speed;
            entity->velX.whole = -(speed * 12);
        }
        else
        {
            speed = entity->velX.whole;
            if (speed < 0)
                speed = -speed;
            entity->velX.whole = speed * 12;
        }
        entity->velY.whole = -entity->velY.whole;
        PlaySong(0x15D);
    }
    entity->palette = entity->basePalette;
    entity->state = 3;
}

/* SkeletonProjectile_Update: debris piece thrown by its parent entity (falls until off screen). */

static inline s32 IsBelow(struct EwramData_EntityData *entity)
{
    if (entity->posY.part16.integer > 0xAA)
        return 1;
    return 0;
}

/**
 * @brief Attach to the parent graphics and affine slot, launch and fall; decrement the parent projectile count before deletion.
 */
void SkeletonProjectile_Update(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    struct EwramData_EntityData *parent = &gEwramData->entityData[(u8)entity->ext.skeletonPiece.ownerIndex];
    u32 state;
    u8 flag;

    switch (entity->state)
    {
    case 0:
        entity->scrollWithBg = 1;
        entity->basePalette = parent->basePalette;
        Sprite_Attach(entity, sSkeletonSpriteSheet, state = Sprite_GetTileSlot(parent), entity->basePalette);
        entity->drawFunc = Sprite_DrawEntity;
        entity->hp = 1;
        entity->mp = 1;
        entity->affineIndex = parent->ext.skeleton.affine;
        Hit_Enable(entity, 0xA, SkeletonProjectile_DamagePlayerOnContact, SkeletonProjectile_OnHit);
        HitBox_SetBodyRaw(entity, 0x1010F8F8);
        HitBox_SetAttackRaw(entity, 0x0202FFFF);
        entity->attack = parent->attack;
        entity->state++;
    case 1:
        entity->affine = 1;
        if (parent->flipX)
        {
            entity->velX.whole = 0xD000;
            entity->accelX.whole = -0x1000;
        }
        else
        {
            entity->velX.whole = -0xD000;
            entity->accelX.whole = 0x1000;
        }
        entity->posX.part16.integer = parent->posX.part16.integer;
        entity->posY.part16.integer = parent->posY.part16.integer - 0x1E;
        entity->velY.whole = -0x30000;
        entity->scrollWithBg = 1;
        if (parent->flipX)
            entity->velX.whole = 0x8AAA;
        else
            entity->velX.whole = -0x8AAA;
        if ((u8)entity->ext.skeletonBone.kind == 0)
        {
            PlaySong(0x88);
            entity->animFrame = 0x12;
        }
        else
        {
            PlaySong(0x89);
            entity->animFrame = 0x1B;
        }
        entity->state = 2;
        break;
    case 2:
    case 3:
        if (entity->state != 4)
            Enemy_UpdateHitFlash(entity);
        if (entity->state != 3)
        {
            if (Entity_IsTimeStopped(entity))
            {
                Hit_CheckWithPlayer(&result, entity);
                entity->affine = 0;
                break;
            }
            entity->affine = 1;
        }
        flag = entity->hitFlashTimer;
        state = entity->state;
        if (flag && (u8)state != 3 && (u8)state != 4)
        {
            Hit_CheckWithPlayer(&result, entity);
            break;
        }
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        entity->velY.whole += 0x1800;
        if ((u8)state != 3)
            Hit_CheckWithPlayer(&result, entity);
        if (IsBelow(entity))
            entity->state = 4;
        break;
    case 4:
        parent->ext.skeleton.throwCount--;
        entity->deleteRequested = 1;
        break;
    }
}

/* Hide the owner and spawn up to seven debris pieces from its sprite. */
/**
 * @brief Hide the owner and create up to seven table-configured sprite pieces, tracking each in the owner count.
 */
void Skeleton_SpawnDeathPieces(struct EwramData_EntityData *owner)
{
    s32 i, x, graphics;
    struct EwramData_EntityData *child;
    owner->hidden = 1;
    if ((owner->ext.skeleton.flags & 1) || (owner->ext.skeleton.flags & 2))
        i = 0;
    else
        i = 1;
    for (; i <= 6; i++)
    {
        child = EntityCreateInRange(0x49, 0xCF, Effect_UpdateSpriteDebris);
        if (child != NULL)
        {
            child->ext.skeletonPiece.ownerIndex = ENTITY_INDEX(owner);
            child->ext.skeletonPiece.index = i;
            owner->ext.skeletonDeath.pieces++;
            child->spriteLayer = owner->spriteLayer;
            child->scrollWithBg = 1;
            graphics = Sprite_GetTileSlot(owner);
            Sprite_Attach(child, sSkeletonSpriteSheet, graphics, ObjPalette_FindOrLoad(sSkeletonWaiterSkeletonPalette));
            child->drawFunc = Sprite_DrawEntity;
            child->posX.whole = owner->posX.whole;
            child->posY.whole = owner->posY.whole;
            x = sSkeletonDeathPieceOffsets[child->ext.skeletonPiece.index][0];
            child->posY.part16.integer += sSkeletonDeathPieceOffsets[child->ext.skeletonPiece.index][1];
            child->animFrame = sSkeletonDeathPieceOffsets[child->ext.skeletonPiece.index][2];
            child->velX.whole = (u8)sSkeletonDeathPieceOffsets[child->ext.skeletonPiece.index][3] << 12;
            child->velY.whole = -(sSkeletonDeathPieceOffsets[child->ext.skeletonPiece.index][4] << 12) - 16;
            child->accelY.whole = 0x3000;
            child->timer = sSkeletonDeathPieceOffsets[child->ext.skeletonPiece.index][5];
            child->flipX = owner->flipX;
            if ((s8)Entity_GetTargetDirection(owner) > 0)
            {
                child->velX.whole = -(child->velX.whole < 0 ? -child->velX.whole : child->velX.whole);
                x = -(x < 0 ? -x : x);
            }
            child->posX.part16.integer += x;
        }
    }
}

static inline u8 BelowScreen(struct EwramData_EntityData *entity)
{
    s32 y = entity->posY.part16.integer; return y > 170;
}

/**
 * @brief Unreferenced per-piece update that sets up a Skeleton death piece from
 * sSkeletonDeathPieceOffsets (the table of Skeleton_SpawnDeathPieces) on the owner's
 * graphics and lets it fly.
 */
void SkeletonDeathPiece_UpdateUnused(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = &gEwramData->entityData[(u8)entity->ext.skeletonPiece.ownerIndex];
    s32 x, graphics, reason, below;
    switch (entity->state)
    {
    case 0:
        entity->spriteLayer = owner->spriteLayer;
        entity->scrollWithBg = 1;
    {
        u8 *sprite = sSkeletonSpriteSheet;
        graphics = Sprite_GetTileSlot(owner);
        Sprite_Attach(entity, sprite, graphics, ObjPalette_FindOrLoad(sSkeletonWaiterSkeletonPalette));
    }
        entity->drawFunc = Sprite_DrawEntity;
        entity->posX.whole = owner->posX.whole;
        entity->posY.whole = owner->posY.whole;
        x = sSkeletonDeathPieceOffsets[entity->ext.skeletonPiece.index][0];
        entity->posY.part16.integer += sSkeletonDeathPieceOffsets[entity->ext.skeletonPiece.index][1];
        entity->posY.part16.integer += 16;
        entity->animFrame = sSkeletonDeathPieceOffsets[entity->ext.skeletonPiece.index][2];
        entity->velX.whole = (u8)sSkeletonDeathPieceOffsets[entity->ext.skeletonPiece.index][3] << 12;
        entity->velY.whole = -(sSkeletonDeathPieceOffsets[entity->ext.skeletonPiece.index][4] << 12) - 16;
        entity->timer = sSkeletonDeathPieceOffsets[entity->ext.skeletonPiece.index][5];
        entity->flipX = owner->flipX;
        if ((s8)Entity_GetTargetDirection(owner) > 0)
        {
            entity->velX.whole = -(entity->velX.whole < 0 ? -entity->velX.whole : entity->velX.whole);
            x = -(x < 0 ? -x : x);
        }
        entity->posX.part16.integer += x;
        entity->state++;
        break;
    case 1:
        reason = --entity->timer == 0;
        below = BelowScreen(entity);
        if (below) reason = 2;
        if (reason != 0)
        {
            owner->ext.skeletonDeath.pieces--;
            entity->deleteRequested = 1;
        }
        break;
    }
    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
    entity->velY.whole += 0x3000;
}

/* Attached part: mirrors its parent's flags, follows joint 0 / table point and switches animation. */

/**
 * @brief Follow the Skeleton owner at joint 0, then table-selected joints
 * and offsets. Mirror owner visibility, palette and lifetime; select the
 * part animation from owner flags. Distinct from SkeletonAttachedPart_Update.
 */
void SkeletonLinkedPart_Update(struct EwramData_EntityData *entity)
{
    struct { s16 x; s16 y; } pt;
    struct EwramData_EntityData *parent = &gEwramData->entityData[(u8)entity->ext.skeletonPiece.ownerIndex];
    const struct PartPoint *t;

    entity->hidden = parent->hidden;
    entity->deleteRequested = parent->deleteRequested;
    if (entity->deleteRequested)
        return;
    if (ENTITY_INDEX_PAIR(entity) != ENTITY_INDEX_PAIR(parent))
    {
        entity->deleteRequested = 1;
        return;
    }
    if (parent->updateFunc == NULL)
    {
        entity->deleteRequested = 1;
        return;
    }
    if (parent->deleteRequested)
    {
        entity->deleteRequested = 1;
        return;
    }
    if (!(parent->ext.skeleton.flags & 3))
    {
        entity->deleteRequested = 1;
        return;
    }
    if (entity->state && Enemy_IsStunned(entity))
        return;
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sSkeletonGfx, sSkeletonWaiterSkeletonPalette, 0, sSkeletonSpriteSheet))
        {
            EntityDelete(entity);
            return;
        }
        entity->scrollWithBg = 1;
        entity->spriteLayer = parent->spriteLayer;
        Anim_StartIndexed(entity, 0x10, 0, 1);
        Anim_Update(entity);
        entity->state++;
        entity->hidden = 0;
    case 1:
        Sprite_GetPartCornerOffset(parent, 0, &pt.x);
        entity->flipX = parent->flipX;
        entity->posX.part16.integer = parent->posX.part16.integer + pt.x;
        entity->posY.part16.integer = parent->posY.part16.integer + pt.y;
        if (!(parent->ext.skeleton.flags & 1))
            entity->state++;
        break;
    case 2:
        t = &sSkeletonPartPoints[parent->animScriptIndex];
        Sprite_GetPartCornerOffset(parent, t->point, &pt.x);
        entity->flipX = parent->flipX;
        entity->posX.part16.integer = parent->posX.part16.integer + pt.x;
        entity->posY.part16.integer = parent->posY.part16.integer + pt.y;
        if (entity->flipX)
            entity->posX.part16.integer -= t->dx;
        else
            entity->posX.part16.integer += t->dx;
        entity->posY.part16.integer += t->dy;
        break;
    }
    if (parent->ext.skeleton.flags & 0x20)
    {
        if (entity->animScriptIndex != 7)
            Anim_StartIndexed(entity, 7, 0, 1);
    }
    else
    {
        if (entity->animScriptIndex != 0x10)
            Anim_StartIndexed(entity, 0x10, 0, 1);
    }
    entity->palette = parent->palette;
    Anim_Update(entity);
}

/* Attached part: follows its parent's joint (case 2), mirrors its flags and deletes itself with it. */
/**
 * @brief Follow the parent animation joint and palette; delete when the parent identity or attachment flags are invalid.
 */
void SkeletonAttachedPart_Update(struct EwramData_EntityData *entity)
{
    struct { s16 x; s16 y; } pt;
    struct EwramData_EntityData *parent = &gEwramData->entityData[(u8)entity->ext.skeletonPiece.ownerIndex];
    const struct PartPoint *t;

    if (!entity->hidden && parent->hidden)
        entity->hidden = parent->hidden;
    entity->deleteRequested = parent->deleteRequested;
    if (entity->deleteRequested)
        return;
    if (ENTITY_INDEX_PAIR(entity) != ENTITY_INDEX_PAIR(parent))
    {
        entity->deleteRequested = 1;
        return;
    }
    if (parent->updateFunc == NULL)
    {
        entity->deleteRequested = 1;
        return;
    }
    if (parent->deleteRequested)
    {
        entity->deleteRequested = 1;
        return;
    }
    if (!(parent->ext.skeleton.flags & 4))
    {
        entity->deleteRequested = 1;
        return;
    }
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sSkeletonGfx, sSkeletonWaiterSkeletonPalette, 0, sSkeletonSpriteSheet))
        {
            EntityDelete(entity);
            return;
        }
        entity->scrollWithBg = 1;
        entity->spriteLayer = parent->spriteLayer;
        Anim_StartIndexed(entity, 8, 0, 1);
        entity->state++;
        break;
    case 1:
        entity->state = 2;
    case 2:
        if ((*(u32 *)&parent->animFlags & 0xFFFF00) == 0x30600)
        {
            if (entity->hidden)
                entity->hidden = 0;
        }
        if (!entity->hidden)
        {
            if (parent->hidden)
                entity->deleteRequested = 1;
        }
        t = &sSkeletonPartPoints[parent->animScriptIndex];
        Sprite_GetPartCornerOffset(parent, t->point, &pt.x);
        entity->flipX = parent->flipX;
        entity->posX.part16.integer = parent->posX.part16.integer + pt.x;
        entity->posY.part16.integer = parent->posY.part16.integer + pt.y;
        if (entity->flipX)
            entity->posX.part16.integer -= t->dx;
        else
            entity->posX.part16.integer += t->dx;
        entity->posY.part16.integer += t->dy;
        break;
    }
    entity->palette = parent->palette;
    Anim_Update(entity);
}

/* Enemy helpers 0x080C45E8..0x080C467B. */

/**
 * @brief Start animation 0, clear hidden and advance animation. The gameplay meaning of state 5 remains unresolved.
 */
void Skeleton_UpdateState5Animation(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->hidden = 0;
            entity->subState++;
        case 1:
            Anim_Update(entity);
            break;
    }
}

/**
 * @brief Resolve damage, show it and flash; on lethal damage play the death sound and start the death sequence.
 */
void Skeleton_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s32 damage = Hit_ResolveDamage(entity, (struct EwramData_EntityData *)attacker, hit, Player_CalculateAttackPower(hit[2]));

    u16 hp = entity->hp; // HP, loaded first
    damage <<= 16;
    damage >>= 16;
    entity->hp = hp - damage;
    Entity_ShowDamage(entity, (s16)damage, 0);
    entity->hitFlashTimer = 8;
    if ((s16)entity->hp <= 0)
    {
        PlaySong(0x6B);
        entity->subState = 0;
        Skeleton_UpdateDeath(entity);
    }
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   SkeletonProjectile_DamagePlayerOnContact = Object28_OnPlayerContact */

/**
 * @brief Apply the Skeleton projectile damage with effect type 0 and
 * effect flags 1 at the entity position.
 */
void SkeletonProjectile_DamagePlayerOnContact(struct EwramData_EntityData *entity)
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

/* Tiny Devil spawn and create 0x080C46AC..0x080C4767. */

/**
 * @brief Create with the supplied update function, copy parent identity, attack and health fields, and hide the parent; NULL on allocation failure.
 */
struct EwramData_EntityData *Entity_CreateLinkedAndHide(struct EwramData_EntityData *entity, EntityFunc updateFunc)
{
    struct EwramData_EntityData *child = Entity_CreateObject(updateFunc);

    if (child == NULL)
        return NULL;
    child->ext.skeletonPiece.ownerIndex = ENTITY_INDEX_PAIR(entity);
    ENTITY_INDEX_PAIR(child) = ENTITY_INDEX_PAIR(entity);
    child->attack = entity->attack;
    child->hp = entity->hp;
    entity->hidden = 1;
    return child;
}

/**
 * @brief C46EC | Tiny Devil create: load the graphics (delete the enemy
 * when that fails), layer 4, hit boxes and Imp_ResetFlight; state 4 instead
 * when bestiaryPreview is set. Ends (0x53D bit 3) when Imp_CreateGrabIndicator fails.
 */
void EnemyImpCreate(struct EwramData_EntityData *entity)
{
    if (!Imp_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    entity->state = 0;
    entity->stage = 0;
    entity->subState = 0;
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 4;
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 4;
        entity->stage = 0;
        entity->subState = 0;
        return;
    }
    Entity_EnableBoxes(entity, Imp_OnContact, Imp_OnHit);
    Imp_ResetFlight(entity);
    if (!Imp_CreateGrabIndicator(entity))
        entity->deleteRequested = 1;
}

/* Graphics loader 0x080C4768. It only matches without -f2003-patch (see
 * NO_2003_PATCH_SRCS in ../Makefile). */

/**
 * @brief Find or load graphics and palette, attach the sprite and start animation 0; return 0 on failure, 1 on success.
 */
s32 Imp_LoadGraphics(struct EwramData_EntityData *entity)
{
    void *palette = sImpTinyDevilPalette;
    void *sprite = sImpTinyDevilSpriteSheet;
    u8 *graphics = sImpTinyDevilGfx;
    s32 gfx = ObjGfx_Find(graphics);
    s8 paletteIndex;

    if (gfx < 0)
    {
        gfx = ObjGfx_LoadResource(graphics, 0);
        if (gfx < 0)
            return 0;
    }
    paletteIndex = ObjPalette_FindOrLoadVariant(palette, 0);
    if (paletteIndex == -1)
        return 0;
    entity->basePalette = paletteIndex;
    if (!Sprite_Attach(entity, sprite, gfx, entity->basePalette))
        return 0;
    if (!Anim_StartIndexed(entity, 0, 0, 1))
        return 0;
    Anim_Update(entity);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}
