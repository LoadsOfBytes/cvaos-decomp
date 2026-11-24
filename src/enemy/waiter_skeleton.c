/*
 * Waiter Skeleton: tray, throw, walk, hits; thrown tray and Skull Archer arrow contact / hit callbacks.
 * Built without -f2003-patch (the hit callbacks need it off; the rest compiles the same either way).
 */
#include "waiter_skeleton.h"
#include "core.h"
#include "graphics.h"
#include "structs/ewram.h"
#include "sound.h"
#include "gameplay.h"
#include "structs/entity_render_flags.h"
#include "player_core.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "hitbox.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "skull_archer.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"
#include "entity_hit.h"

extern u8 Entity_LoadGraphics(struct EwramData_EntityData *entity, u8 *resource, void *palette, u8 variant, void *sheet);
extern s32 Target_IsInRect(s16, s16, s32, s32);
extern void Entity_FaceTarget(struct EwramData_EntityData *);

void WaiterSkeletonThrownTray_DamagePlayerOnContact(struct EwramData_EntityData *);
void WaiterSkeletonThrownTray_OnHit(struct EwramData_EntityData *);
void WaiterSkeleton_OnHit(struct EwramData_EntityData *, s32, u16 *);

/* Attached animated effect: follows a sprite part while its owner's flag 2 is set. */
void WaiterSkeletonTray_Update(struct EwramData_EntityData *entity)
{
    u16 offset[2];
    struct EwramData_EntityData *parent = entity->ext.waiterTray.owner;
    if (parent->state == 3 || !(parent->ext.waiterSkeleton.flags & 2))
    {
        entity->deleteRequested = 1;
        return;
    }
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sWaiterSkeletonGfx, sSkeletonWaiterSkeletonPalette, 0, sWaiterSkeletonSpriteSheet))
        {
            EntityDelete(entity);
            return;
        }
        entity->scrollWithBg = 1;
        entity->spriteLayer = parent->spriteLayer;
        Anim_StartIndexed(entity, 17, 0, 1);
        entity->state++;
        break;
    case 1:
        {
            u16 *partOffset = offset;
            Sprite_GetPartCornerOffset(parent, 0, partOffset);
            entity->flipX = parent->flipX;
            entity->posX.part16.integer = parent->posX.part16.integer + offset[0];
            entity->posY.part16.integer = parent->posY.part16.integer + partOffset[1];
            if ((parent->ext.waiterSkeleton.flags & 8) && entity->animScriptIndex != 8)
                Anim_StartIndexed(entity, 8, 0, 1);
            break;
        }
    case 2:
        break;
    }
    entity->palette = parent->palette;
    Anim_Update(entity);
}

/* Parent-linked affine entity part: attachment, release, motion and deletion. */
/* This halfword bitfield view preserves the original slot update access width. */

static inline void BoxA(struct EwramData_EntityData *e, s32 box) { HitBox_SetAttackRaw(e, box); }
static inline void BoxB(struct EwramData_EntityData *e, s32 box) { HitBox_SetBodyRaw(e, box); }

void WaiterSkeletonThrownTray_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.waiterTray.owner;
    struct HitResult result;
    s16 offset[2];
    if (entity->state != 5 && (s16)entity->hp <= 0)
        entity->state = 5;
    if (entity->state <= 2 && ((u8)ENTITY_INDEX(entity) != (u8)ENTITY_INDEX(parent) || parent->state == 5)) {
        entity->hidden = 0;
        entity->spriteLayer = 19;
        entity->state = 3;
        Hit_Enable(entity, 10, WaiterSkeletonThrownTray_DamagePlayerOnContact, WaiterSkeletonThrownTray_OnHit);
        BoxA(entity, 0x0808FCFC);
        BoxB(entity, 0x0808FCFC);
        entity->hit.flags &= 0xFB;
        entity->hit.flags |= 2;
    }
    if (gEwramData->gameState.timeStopTimer && entity->state != 5) {
        Hit_CheckWithPlayer(&result, entity);
        return;
    }
    switch (entity->state) {
    case 0: {
        s16 slot;
        if (!Entity_LoadGraphics(entity, sWaiterSkeletonGfx, sSkeletonWaiterSkeletonPalette, 0, sWaiterSkeletonSpriteSheet))
            goto delete;
        slot = ObjAffine_Alloc(1);
        if (slot < 0)
            goto delete;
        entity->affineIndex = slot;
        entity->affine = 1;
        entity->scrollWithBg = 1;
        entity->spriteLayer = parent->spriteLayer;
        Anim_StartIndexed(entity, entity->ext.waiterTray.anim, 0, 1);
        entity->flipX = parent->flipX;
        entity->accelX.part16.integer = 10;
        if (entity->flipX)
            entity->accelX.part16.integer = -entity->accelX.part16.integer;
        { u16 *origin = &parent->posX.part16.integer;
          u16 x = entity->accelX.part16.integer;
          u16 parentX = *origin;
          x += parentX;
          entity->posX.part16.integer = x; }
        entity->posY.part16.integer = parent->posY.part16.integer - 16;
        if (parent->ext.waiterSkeleton.flags & 1) {
            entity->hidden = 1;
            entity->state = 2;
            break;
        }
        entity->state = 1;
    }
    case 1:
        if ((u8)ENTITY_INDEX(entity) != (u8)ENTITY_INDEX(parent)) {
            entity->state = 5;
            break;
        }
        if ((u8)(parent->animScriptIndex - 2) <= 4) {
            const struct PartOffset *point = &sWaiterSkeletonTrayOffsets[parent->animScriptIndex];
            Sprite_GetPartCornerOffset(parent, point->index, offset);
            entity->flipX = parent->flipX;
            { u16 *origin = &parent->posX.part16.integer;
          u16 x = offset[0];
          u16 parentX = *origin;
          x += parentX;
          entity->posX.part16.integer = x; }
            entity->posY.part16.integer = offset[1] + parent->posY.part16.integer;
            if (entity->flipX)
                entity->posX.part16.integer -= point->x;
            else
                entity->posX.part16.integer += point->x;
            entity->posY.part16.integer += point->y;
            if (parent->ext.waiterSkeleton.flags & 4) {
                s32 vx;
                entity->state = 4;
                PlaySong(0x89);
                Hit_Enable(entity, 10, WaiterSkeletonThrownTray_DamagePlayerOnContact, WaiterSkeletonThrownTray_OnHit);
                BoxA(entity, 0x0808FCFC);
                BoxB(entity, 0x0808FCFC);
                entity->hit.flags &= 0xFB;
                if (Target_GetX(entity) - entity->posX.whole < 0)
                    vx = -0x14000 - ((entity->posX.whole - Target_GetX(entity)) >> 6);
                else
                    vx = -0x14000 - ((Target_GetX(entity) - entity->posX.whole) >> 6);
                entity->velX.whole = vx;
                entity->accelY.whole = 0x2000;
                if (entity->flipX)
                    entity->velX.whole = -vx;
                entity->velY.whole = -0x20000;
            }
        }
        break;
    case 2:
        if ((u8)ENTITY_INDEX(entity) != (u8)ENTITY_INDEX(parent)) {
            entity->state = 5;
            break;
        }
        if (parent->state == 3) {
            entity->state = 3;
            break;
        }
        switch (parent->animStep) {
        case 1:
            entity->hidden = 0;
            entity->spriteLayer = parent->spriteLayer + 1;
            break;
        case 3:
            entity->spriteLayer = parent->spriteLayer;
            entity->flipX = parent->flipX;
            entity->accelX.part16.integer = -10;
            if (entity->flipX)
                entity->accelX.part16.integer = 10;
            { u16 *origin = &parent->posX.part16.integer;
          u16 x = entity->accelX.part16.integer;
          u16 parentX = *origin;
          x += parentX;
          { u16 *destination = &entity->posX.part16.integer;
            *destination = x;
            { u16 y = parent->posY.part16.integer - 16;
              /* Recompute the field address: pointer arithmetic folds into a different store. */
              destination = &entity->posY.part16.integer; *destination = y; } } }
            entity->state = 1;
            break;
        }
        break;
    case 3:
        entity->velX.whole = 0x8000;
        if (entity->flipX)
            entity->velX.whole = -entity->velX.whole;
        entity->velY.whole = -0x28000;
        entity->accelY.whole = 0x2000;
        PlaySong(0x89);
        entity->state = 4;
    case 4:
        entity->ext.waiterTray.angle += entity->velX.whole / 64;
        EntityDampVelocityX(entity, 60);
        EntityAccelerateY(entity, 0x80000);
        EntityMoveX(entity);
        EntityMoveY(entity);
        if (!EntityIsOutsideScreen(entity, 4))
            break;
        goto release;
    case 5:
        Effect_SpawnRandom(entity->posX.whole, entity->posY.whole, 0xC0000, 0xC0000, entity->spriteLayer - 1);
release:
        ObjAffine_Free(entity->affineIndex);
delete:
        EntityDelete(entity);
        return;
    }
    ObjAffine_Set(entity->ext.waiterTray.angle, 0x10000, 0x10000, entity->flipX, 0, entity->affineIndex);
    if (entity->state != 5)
        Hit_CheckWithPlayer(&result, entity);
    Anim_Update(entity);
}

/* Waiter Skeleton: choose and create a thrown object, then resume its walk. */
void WaiterSkeleton_UpdateThrow(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        if ((entity->ext.waiterSkeleton.flags & 2) && RandomNumberGenerator() % 4 == 0)
            entity->subState = 1;
        else
            entity->subState = 3;
        break;
    case 1:
        {
            struct EwramData_EntityData *child = Entity_CreateObject(WaiterSkeletonThrownTray_Update);
            if (child != NULL)
            {
                child->ext.waiterTray.owner = entity;
                *(u8 *)&child->ext = *(u8 *)&entity->ext;
                child->attack = entity->attack;
                child->hp = entity->hp;
                child->ext.waiterTray.anim = 17;
            }
            Anim_StartIndexed(entity, 6, 0, 0);
            entity->subState = 2;
        }
        break;
    case 2:
        entity->ext.waiterSkeleton.flags &= ~2;
        goto finishThrow;
    case 3:
        {
            struct EwramData_EntityData *child = Entity_CreateObject(WaiterSkeletonThrownTray_Update);
            if (child != NULL)
            {
                child->ext.waiterTray.owner = entity;
                *(u8 *)&child->ext = *(u8 *)&entity->ext;
                child->attack = entity->attack;
                child->hp = entity->hp;
                entity->ext.waiterSkeleton.flags |= 1;
                child->ext.waiterTray.anim = 9;
            }
            Anim_StartIndexed(entity, 7, 0, 0);
            entity->subState = 4;
        }
        break;
    case 4:
finishThrow:
        if (entity->animFlags & 4)
        {
            Anim_StartIndexed(entity, 3, 0, 1);
            entity->state = 2;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    }
}

/* Waiter Skeleton walking/approach states and breakup animation. */
void WaiterSkeleton_UpdateWalk(struct EwramData_EntityData *entity)
{
    s32 speed = 0x8000;
    switch (entity->subState)
    {
    case 0:
        Entity_Walk(entity, speed, sWaiterSkeletonWalkProfile, 12);
        if (Target_IsInRect(entity->posX.part16.integer - 100, entity->posY.part16.integer - 40, 200, 80))
        {
            Anim_StartIndexed(entity, 4, 0, 1);
            if ((RandomNumberGenerator() & 3) != 0)
                entity->subState = 1;
            else
            {
                entity->subState = 3;
                entity->timer = 20;
                Anim_StartIndexed(entity, 4, 0, 1);
            }
        }
        break;
    case 1:
        speed = 0x10000;
        Entity_Walk(entity, speed, sWaiterSkeletonWalkProfile, 12);
        if (Target_IsInRect(entity->posX.part16.integer - 80, entity->posY.part16.integer - 40, 160, 80))
        {
            Anim_StartIndexed(entity, 5, 0, 0);
            entity->subState = 2;
        }
        break;
    case 2:
        if (entity->animStep == 2)
            entity->ext.waiterSkeleton.flags |= 4;
        if (entity->animFlags & 4)
        {
            entity->ext.waiterSkeleton.flags &= ~5;
            entity->subState = 5;
            Anim_StartIndexed(entity, 1, 0, 1);
            entity->timer = (RandomNumberGenerator() & 15) + 60;
        }
        break;
    case 3:
        speed = 0x10000;
        Entity_Walk(entity, speed, sWaiterSkeletonWalkProfile, 12);
        if (entity->timer != 0)
            entity->timer--;
        else
        {
            entity->subState = 4;
            entity->timer = 50;
            Anim_StartIndexed(entity, 3, 0, 1);
        }
        break;
    case 4:
        Entity_Walk(entity, -speed, sWaiterSkeletonWalkProfile, 12);
        if (entity->timer != 0)
            entity->timer--;
        else
            entity->subState = 0;
        break;
    case 5:
        Entity_Walk(entity, -speed, sWaiterSkeletonWalkProfile, 12);
        if (entity->timer != 0)
            entity->timer--;
        else
        {
            entity->state = 0;
            entity->stage = 0;
            entity->subState = 0;
            Anim_StartIndexed(entity, 0, 0, 1);
        }
        break;
    }
}

/* Spawn fragments using frames 10..16 (or 11..16 without flag 2). */
s32 WaiterSkeleton_SpawnDeathPieces(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        {
            s32 frame;
            PlaySong(0x6B);
            frame = 10;
            if (!(entity->ext.waiterSkeleton.flags & 2))
                frame = 11;
            for (; frame <= 16; frame++)
            {
                struct EwramData_EntityData *part = EntityCreateInRange(0x49, 0xCF, Effect_UpdateSpriteDebris);
                if (part != NULL)
                {
                    s32 tiles;
                    part->posX.whole = entity->posX.whole;
                    part->posY.whole = entity->posY.whole;
                    part->velX.whole = ((RandomNumberGenerator() & 15) - 8) << 14;
                    part->velY.whole = (((RandomNumberGenerator() & 15) - 8) << 12) - 0x28000;
                    part->accelY.whole = 0x2000;
                    part->spriteLayer = entity->spriteLayer;
                    tiles = ObjGfx_Find(sWaiterSkeletonGfx);
                    Sprite_Attach(part, sWaiterSkeletonSpriteSheet, tiles, entity->basePalette);
                    part->flipX = entity->flipX;
                    part->drawFunc = Sprite_DrawEntity;
                    part->scrollWithBg = 1;
                    part->timer = (RandomNumberGenerator() & 15) + 40;
                    part->subState = 1;
                    Anim_StartIndexed(part, (u16)frame, 0, 1);
                    Anim_Update(part);
                }
            }
            entity->hidden = 1;
            entity->subState = 1;
        }
        break;
    case 1:
        return 1;
    }
    return 0;
}

/* Waiter Skeleton creation and attached sprite setup. */
void EnemyWaiterSkeletonCreate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *effect;

    if (!Entity_LoadGraphics(entity, sWaiterSkeletonGfx, sSkeletonWaiterSkeletonPalette, 0, sWaiterSkeletonSpriteSheet))
        goto failed;
    entity->ext.waiterSkeleton.flags |= 2;
    effect = Entity_CreateObject(WaiterSkeletonTray_Update);
    if (effect == NULL)
    {
    failed:
        EntityDelete(entity);
        return;
    }
    effect->ext.waiterTray.owner = entity;
    *(u8 *)&effect->ext = *(u8 *)&entity->ext;
    effect->attack = entity->attack;
    *(u16 *)&effect->hp = *(u16 *)&entity->hp;
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 4;
        return;
    }
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 19;
    Entity_EnableBoxes(entity, Enemy_OnContact, WaiterSkeleton_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    entity->posY.part16.integer++;
    entity->scrollWithBg = 1;
    entity->state = 0;
}

void EnemyWaiterSkeletonUpdate(struct EwramData_EntityData *entity)
{
    if (Enemy_PreUpdate(entity)) return;
    switch (entity->state)
    {
    case 0:
        Entity_FaceTarget(entity);
        if (entity->subState == 0
            && Target_IsInRect((s16)(entity->posX.part16.integer - 0xA0),
                            (s16)(entity->posY.part16.integer - 0x28), 0x140, 0x50))
        {
            entity->state = 1;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    case 1: WaiterSkeleton_UpdateThrow(entity); break;
    case 2: WaiterSkeleton_UpdateWalk(entity); break;
    case 3:
        if (WaiterSkeleton_SpawnDeathPieces(entity)) EntityDelete(entity);
        return;
    case 4:
        Anim_Update(entity);
        return;
    }
    Enemy_CheckHitPlayer(entity);
    Entity_AnimateWithBoxes(entity);
}

/* Enemy helpers decompiled from assembly. */

void WaiterSkeleton_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    if (Enemy_TakeDamage(entity, attacker, hit))
    {
        Enemy_DropRewards(entity);
        entity->palette = entity->basePalette;
        entity->timer = 0x28;
        entity->state = 3;
        entity->stage = 0;
        entity->subState = 0;
    }
}

/* Notify the parent when its low unk_4F4 byte still matches the child's. */
void WaiterSkeletonThrownTray_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage effect;
    struct EwramData_EntityData *parent = entity->ext.waiterTray.owner;
    effect.attributes = 0;
    effect.unused = 0;
    effect.attack = entity->attack;
    effect.reaction = 0;
    effect.sourceX = entity->posX.whole;
    effect.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&effect);
    if (ENTITY_INDEX(parent) == ENTITY_INDEX(entity))
        parent->ext.waiterSkeleton.flags |= 8;
}

void WaiterSkeletonThrownTray_OnHit(struct EwramData_EntityData *entity)
{
    entity->hp = 0;
}

/* Record the parent and initial relative position for the child effect. */
void SkullArcherArrow_DamagePlayerOnContact(struct EwramData_EntityData *entity, struct EwramData_EntityData *parent)
{
    struct PlayerContactDamage effect;
    effect.attributes = 0;
    effect.unused = 0;
    effect.attack = entity->attack;
    effect.reaction = 1;
    effect.sourceX = entity->posX.whole;
    effect.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&effect);
    entity->hit.flags |= 2;
    entity->hit.flags |= 4;
    entity->ext.skullArcherArrow.owner = parent;
    entity->accelX.whole = entity->posX.whole - parent->posX.whole;
    entity->accelY.whole = entity->posY.whole - parent->posY.whole;
    entity->state = 1;
    entity->timer = 0x1E;
}

/* Projectile: move or follow its parent, then expire or break on impact. */
void SkullArcherArrow_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.skullArcherArrow.owner;
    struct HitResult result;
    if (entity->state != 2 && (s16)entity->hp <= 0)
        entity->state = 2;
    if (gEwramData->gameState.timeStopTimer != 0 && entity->state != 2)
        goto collision;
    if (EntityIsOutsideScreen(entity, 4))
    {
        EntityDelete(entity);
        return;
    }
    switch (entity->state)
    {
        case 0:
            Entity_SetVelocityXFromFacing(entity, 0x30000);
            EntityMoveX(entity);
        collision:
            Hit_CheckWithPlayer(&result, entity);
            break;
        case 1:
            entity->posX.whole = parent->posX.whole + entity->accelX.whole;
            entity->posY.whole = parent->posY.whole + entity->accelY.whole;
            if (entity->timer != 0)
            {
                if (--entity->timer == 10)
                    entity->drawSkip = 1;
            }
            else
                entity->deleteRequested = 1;
            break;
        case 2:
            Effect_SpawnRandom(entity->posX.whole + (entity->velX.whole << 3),
                         entity->posY.whole - 0x160000, 0x40000, 0x40000,
                         entity->spriteLayer - 1);
            EntityDelete(entity);
            break;
    }
}

void SkullArcher_UpdateIdle(struct EwramData_EntityData *entity)
{
    Entity_FaceTarget(entity);
    if (entity->subState == 0 && Target_IsNearEntity(entity, 240, 35))
    {
        s32 index = entity->ext.skullArcher.patternStep / 2;
        if (++entity->ext.skullArcher.patternStep > 4)
            entity->ext.skullArcher.patternStep = 0;
        entity->state = 1;
        entity->stage = 0;
        entity->subState = 0;
        entity->ext.skullArcher.pattern = sUnk_08528614[index];
        Anim_StartIndexed(entity, index + 2, 0, 0);
    }
}

/* Emit an arrow when the next four-byte pattern entry matches the
 * animation frame; after the animation, wait before the next state. */
void SkullArcher_UpdateShoot(struct EwramData_EntityData *entity)
{
    s8 *pattern = (s8 *)entity->ext.skullArcher.pattern;
    switch (entity->subState)
    {
        case 0:
            if (pattern[0] == entity->animStep && entity->animTimer == 0)
            {
                struct EwramData_EntityData *arrow = Entity_CreateObject(SkullArcherArrow_Update);
                if (arrow != NULL)
                {
                    s32 graphics;
                    PlaySong(0x85);
                    graphics = ObjGfx_Find(sSkullArcherGfx);
                    Sprite_Attach(arrow, sSkullArcherSpriteSheet, graphics, entity->basePalette);
                    arrow->animFrame = 0x19;
                    arrow->flipX = entity->flipX;
                    arrow->drawFunc = Sprite_DrawEntity;
                    arrow->spriteLayer = entity->spriteLayer;
                    arrow->posX.part16.integer = entity->posX.part16.integer;
                    arrow->posY.part16.integer = pattern[1] + entity->posY.part16.integer;
                    arrow->scrollWithBg = 1;
                    arrow->attack = entity->attack;
                    arrow->hp = 1;
                    Hit_Enable(arrow, 10, SkullArcherArrow_DamagePlayerOnContact, SkullArcherArrow_OnHit);
                    HitBox_SetAttackRaw(arrow, 0x0202E8EE);
                    HitBox_SetBodyRaw(arrow, 0x0202E8EE);
                    arrow->hit.flags &= ~4;
                }
                pattern += 4;
                entity->ext.skullArcher.pattern = (s32)pattern;
            }
            if (entity->animFlags & 4)
            {
                entity->subState = 1;
                entity->timer = 0x20;
            }
            break;
        case 1:
            if (entity->timer != 0)
                entity->timer--;
            else if (entity->spawnParam0 == 0)
            {
                entity->state = 0;
                entity->stage = 0;
                entity->subState = 0;
                Anim_StartIndexed(entity, 0, 0, 1);
            }
            else
            {
                entity->state = 2;
                entity->stage = 0;
                entity->subState = 0;
                Anim_StartIndexed(entity, 1, 0, 1);
            }
            break;
    }
}

/**
 * @brief AFA9C | Skull Archer patrol: substate 0 walks (Entity_Walk,
 * turning every 128 frames) until the target is in front (Entity_GetTargetDirection
 * 1 when facing right, -1 when facing left) and closer than 80 pixels;
 * substate 1 backs off until 100 pixels away and then shoots (state 1,
 * one of the patterns in sUnk_08528614, sprite 2 + pattern).
 * Only matches without -f2003-patch. */
void SkullArcher_UpdatePatrol(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            if (entity->timer++ > 128)
            {
                entity->timer = 0;
                entity->flipX ^= 1;
            }
            Entity_Walk(entity, 0x4000, sSkullArcherWalkProfile, 12);
            if (entity->flipX && (s8)Entity_GetTargetDirection(entity) == 1)
                goto close;
            if (!entity->flipX && (s8)Entity_GetTargetDirection(entity) == -1)
                goto close;
            break;
        close:
            if (Entity_GetTargetDistance(entity) <= 79)
                entity->subState = 1;
            break;
        case 1:
            Entity_Walk(entity, -0xC000, sSkullArcherWalkProfile, 13);
            if (Entity_GetTargetDistance(entity) > 99)
            {
                s32 index = entity->ext.skullArcher.patternStep / 2;
                if (++entity->ext.skullArcher.patternStep > 4)
                    entity->ext.skullArcher.patternStep = 0;
                entity->state = 1;
                entity->stage = 0;
                entity->subState = 0;
                entity->ext.skullArcher.pattern = sUnk_08528614[index];
                Anim_StartIndexed(entity, index + 2, 0, 0);
            }
            break;
    }
}
