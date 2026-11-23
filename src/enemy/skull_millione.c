/*
 * Skull Millione family (Skull Millione, Cagnazzo, Lubicant): body and parts.
 */
#include "core.h"
#include "player_core.h"
#include "graphics.h"
#include "entity_util.h"
#include "structs/entity_template.h"
#include "structs/enemy_variant_b7e80.h"
#include "macros.h"
#include "sound.h"
#include "structs/ewram.h"
#include "dead_warrior.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "entity_motion.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_palette.h"
#include "effects.h"

/* Rotating attached part: follow the parent during animation 3, then fall
 * and release its affine slot when the parent dies. */

/**
 * @brief Follow the owner and rotate while its animation 3 is active.
 * On owner death, detach and fall; release the affine slot when removed.
 * The visual identity of this attached part is unresolved.
 */
void SkullMillioneFamilyPart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = ENTITY_OWNER(entity);
    u16 offset;
    s32 rotation;
    s32 value;
    if (entity->state <= 1)
    {
        if (parent->state == 3) entity->state = 2;
        else if (gEwramData->gameState.timeStopTimer != 0)
        {
            struct EntityTemplate *entry = (struct EntityTemplate *)(parent->enemyId * sizeof(*entry) + (u32)sEnemyData);
            if (!(entry->flags_1C & 0x400)) return;
        }
    }
    switch (entity->state)
    {
    case 0:
        if (parent->animScriptIndex != 3) break;
        entity->ext.skullMillionePart.angle = 0;
        entity->hidden = 0;
        entity->state = 1;
        /* fall through */
    case 1:
        if (parent->animScriptIndex != 3)
        {
            entity->hidden = 1;
            entity->state = 0;
            break;
        }
        entity->flipX = parent->flipX;
        offset = -12;
        rotation = 0xEE00;
        if (entity->flipX)
        {
            offset = 12;
            rotation = -(s16)rotation;
        }
        {
            u16 *position = &parent->posX.part16.integer;
            value = (s16)offset;
            value += *position;
            entity->posX.part16.integer = value;
        }
        entity->posY.part16.integer = parent->posY.part16.integer - 6;
        value = (s16)rotation;
        value += (u16)entity->ext.skullMillionePart.angle;
        entity->ext.skullMillionePart.angle = value;
        break;
    case 2:
        entity->hidden = 0;
        entity->flipX = parent->flipX;
        offset = -12;
        if (entity->flipX) offset = 12;
        {
            u16 *position = &parent->posX.part16.integer;
            value = (s16)offset;
            value += *position;
            entity->posX.part16.integer = value;
        }
        entity->posY.part16.integer = parent->posY.part16.integer - 6;
        entity->velY.whole = (RandomSigned(0x10) << 12) - 0x38000;
        entity->accelY.whole = 0x2000;
        entity->velX.whole = RandomSigned(0x40) << 12;
        entity->accelX.whole = 0;
        entity->ext.skullMillionePart.angle = 0;
        entity->state = 3;
        /* fall through */
    case 3:
        rotation = 0xB00;
        if (entity->flipX) rotation = (u16)-rotation;
        value = (s16)rotation;
        value += (u16)entity->ext.skullMillionePart.angle;
        entity->ext.skullMillionePart.angle = value;
        EntityAccelerateAndMove(entity, 0x80000);
        if (!EntityIsOutsideScreen(entity, 4)) break;
        /* fall through */
    case 4:
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
        return;
    }
    ObjAffine_Set(entity->ext.skullMillionePart.angle, 0x10000, 0x10000, entity->flipX, 0, entity->affineIndex);
}

/* Shared Skull Millione / Cagnazzo / Lubicant setup and movement. */
#define BACKGROUND ((struct BgLayerState *)((u8 *)gEwramData + OFFSET_OF(struct EwramData, bgInfo[1])))
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern struct EwramData_EntityData *Entity_CreateObject(void *);
extern s32 Hit_EnableContact(struct EwramData_EntityData *, s32, void *, s32);
extern void SkullMillioneFamily_OnHit(struct EwramData_EntityData *);
extern void Entity_FaceTarget(struct EwramData_EntityData *);
extern s32 Target_IsNearEntity(struct EwramData_EntityData *, s32, s32);
extern s32 Target_GetPixelX(struct EwramData_EntityData *);
extern s32 Target_GetPixelY(struct EwramData_EntityData *);
extern s32 Target_GetVelocityX(void);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void SkullMillioneFamily_UpdateAttack(struct EwramData_EntityData *);
extern s32 SkullMillioneFamily_UpdateDeath(struct EwramData_EntityData *);
extern u8 sSkullMillioneFamilyGfx[], sSkullMillioneFamilyPalette[];
s32 SkullMillioneFamily_CreatePart(struct EwramData_EntityData *);
void SkullMillioneFamily_UpdateWait(struct EwramData_EntityData *);
void SkullMillioneFamily_UpdateApproach(struct EwramData_EntityData *);

/**
 * @brief Create a hidden attached sprite with an affine slot and the selected
 * variant graphics. Return 1 on success, 0 on allocation or graphics failure.
 * Preserve the existing graphics-failure cleanup behavior.
 */
s32 SkullMillioneFamily_CreatePart(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child = Entity_CreateObject(SkullMillioneFamilyPart_Update);
    const struct SkullMillioneFamilyVariant *info = &sSkullMillioneFamilyVariants[(u8)entity->ext.skullMillione.variant];
    u16 slot;
    if (child == NULL || !Entity_LoadGraphics(child, sSkullMillioneFamilyGfx, sSkullMillioneFamilyPalette, info->palette, info->sprites)) return 0;
    slot = ObjAffine_Alloc(1);
    if ((s16)slot < 0)
    {
        EntityDelete(child);
        return 0;
    }
    child->affineIndex = slot;
    child->affine = 1;
    child->animFrame = 0xD;
    child->spriteLayer = entity->spriteLayer;
    child->scrollWithBg = 1;
    ENTITY_OWNER(child) = entity;
    child->hidden = 1;
    return 1;
}

/**
 * @brief Wait state (0): bob vertically during the variant delay, then
 * activate near the target and select animation 3 for the applicable variant.
 */
void SkullMillioneFamily_UpdateWait(struct EwramData_EntityData *entity)
{
    const struct SkullMillioneFamilyVariant *info = &sSkullMillioneFamilyVariants[(u8)entity->ext.skullMillione.variant];
    switch (entity->subState)
    {
    case 0:
        entity->subState = 1;
        entity->ext.skullMillione.bobFrame = 0x8000;
        entity->timer = info->activationDelay;
        /* fall through */
    case 1:
        entity->ext.skullMillione.bobFrame++;
        entity->velY.whole = Sin((0x5B0 * entity->ext.skullMillione.bobFrame) & 0xFFFF) / 2;
        entity->posY.whole += entity->velY.whole;
        if (entity->timer != 0) entity->timer--;
        else
        {
            entity->posY.part16.integer += 0x28;
            if (Target_IsNearEntity(entity, 0xDC, 0xA0))
            {
                Entity_FaceTarget(entity);
                entity->state = 1;
                entity->stage = 0;
                entity->subState = 0;
                if ((info->behaviorFlags & 1) && !(info->behaviorFlags & 2))
                    Anim_StartIndexed(entity, 3, 0, 1);
            }
            entity->posY.part16.integer -= 0x28;
        }
        break;
    }
}

/**
 * @brief Approach state (1): increase the aiming speed and move toward a
 * swaying point above and beside the target; enter attack state on arrival.
 */
void SkullMillioneFamily_UpdateApproach(struct EwramData_EntityData *entity)
{
    const struct SkullMillioneFamilyVariant *info = &sSkullMillioneFamilyVariants[(u8)entity->ext.skullMillione.variant];
    s32 x;
    s32 y;
    s32 arrived;
    switch (entity->subState)
    {
    case 0:
        entity->ext.skullMillione.speed = 0;
        entity->subState = 1;
        /* fall through */
    case 1:
        if (entity->animTimer == 0) Entity_FaceTarget(entity);
        if (entity->ext.skullMillione.speed < info->approachSpeed) entity->ext.skullMillione.speed += 0x2000;
        x = ((s16)BACKGROUND->xPos.part16.integer + (s16)Target_GetPixelX(entity)) << 16;
        y = ((s16)BACKGROUND->yPos.part16.integer + (s16)Target_GetPixelY(entity) - 0x50) << 16;
        if (Target_GetX(entity) > entity->posX.whole) x -= entity->ext.skullMillione.speed;
        else x += entity->ext.skullMillione.speed;
        x += Target_GetVelocityX() * info->swayMultiplier;
        arrived = Entity_AimVelocityAtPoint(entity, x, y, entity->ext.skullMillione.speed);
        EntityMove(entity);
        if (arrived)
        {
            entity->state = 2;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    }
}

/* Attack and death states shared by Skull Millione, Cagnazzo and Lubicant. */

/**
 * @brief Attack state (2): perform the variant dive and recovery sequence.
 * Behavior flag 2 adds a timed tracking phase before the dive.
 */
void SkullMillioneFamily_UpdateAttack(struct EwramData_EntityData *entity)
{
    const struct SkullMillioneFamilyVariant *info = &sSkullMillioneFamilyVariants[(u8)entity->ext.skullMillione.variant];
    switch (entity->subState)
    {
    case 0:
        if (info->behaviorFlags & 2)
        {
            Anim_StartIndexed(entity, 3, 0, 1);
            entity->timer = 0x40;
            entity->subState = 5;
            break;
        }
        /* fall through */
    case 1:
        Anim_StartIndexed(entity, 1, 0, 0);
        entity->velY.whole = -0x20000;
        entity->accelY.whole = 0x1000;
        entity->accelX.whole = -entity->velX.whole / 16;
        entity->subState = 2;
        entity->timer = 0x20;
        /* fall through */
    case 2:
        EntityMove(entity);
        if (entity->timer != 0)
        {
            entity->timer--;
            entity->velX.whole += entity->accelX.whole;
        }
        entity->velY.whole += entity->accelY.whole;
        if (entity->velY.whole > 0) entity->velY.whole = 0;
        if (entity->animStep == 4)
        {
            entity->subState = 3;
            entity->velY.whole = info->diveSpeed;
            entity->accelY.whole = -0x2000;
            PlaySong(0x87);
            if ((u8)entity->ext.skullMillione.variant == 1) PlaySong(0x1BA);
        }
        break;
    case 3:
        EntityAccelerateAndMoveY(entity, 0x30000);
        if (entity->animFlags & 4)
        {
            entity->accelY.whole = -0x2000;
            entity->subState = 4;
        }
        break;
    case 4:
        entity->posY.whole += entity->velY.whole;
        entity->velY.whole += entity->accelY.whole;
        if (entity->velY.whole < 0)
        {
            entity->velY.whole = 0;
            entity->state = 0;
            entity->stage = 0;
            entity->subState = 0;
            Anim_StartIndexed(entity, 0, 0, 1);
        }
        break;
    case 5:
        if (*(u16 *)&entity->animStep == 0) Entity_FaceTarget(entity);
        if (Target_GetX(entity) > entity->posX.whole) entity->accelX.whole = 0x3000;
        else entity->accelX.whole = -0x3000;
        if (Target_GetY(entity) - 0x680000 > entity->posY.whole)
            entity->accelY.whole = 0x1800;
        else
        {
            if (entity->velY.whole > -0x1000) entity->velY.whole = -0x1000;
            entity->accelY.whole = -0x3000;
        }
        EntityAccelerateAndMoveX(entity, 0x28000);
        EntityAccelerateAndMoveY(entity, 0x8000);
        if (entity->timer != 0) entity->timer--;
        else entity->subState = 1;
        break;
    }
}

/**
 * @brief Death state (3): emit particles, then fall with fragment effects.
 * Return nonzero outside the removal rectangle.
 */
s32 SkullMillioneFamily_UpdateDeath(struct EwramData_EntityData *entity)
{
    s32 x;
    struct EwramData_EntityData *particle;
    switch (entity->subState)
    {
    case 0:
        PlaySong(0x1BC);
        Anim_StartIndexed(entity, 2, 0, 0);
        entity->subState = 1;
        entity->timer = 0x20;
        /* fall through */
    case 1:
        if (!(entity->timer & 0xF))
        {
            x = entity->posX.whole + (RandomSigned(8) << 16);
            particle = Effect_SpawnShared(x, entity->posY.whole + ((RandomSigned(0x10) - 0x10) << 16), 0xD, 8);
            if (particle != NULL)
            {
                particle->flipX = RandomNumberGenerator() & 1;
                particle->spriteLayer = 4;
            }
        }
        if (entity->timer != 0) entity->timer--;
        else
        {
            entity->velY.whole = -0x8000;
            entity->accelY.whole = 0x1000;
            entity->subState = 2;
            entity->timer = 0x32;
        }
        break;
    case 2:
        EntityAccelerateAndMoveY(entity, 0x80000);
        Enemy_UpdateFragmentDeath(entity);
        break;
    }
    if (!Entity_IsInRect(entity, -0x80, -0x40, 0x1F0, 0x120)) return 1;
    return 0;
}

/* Shared Skull Millione / Cagnazzo / Lubicant setup and movement. */
s32 SkullMillioneFamily_CreatePart(struct EwramData_EntityData *);
void SkullMillioneFamily_UpdateWait(struct EwramData_EntityData *);
void SkullMillioneFamily_UpdateApproach(struct EwramData_EntityData *);

/**
 * @brief Initialize variant 0 (Skull Millione), 1 (Cagnazzo), or 2 (Lubicant).
 * Set up graphics, hit callbacks and an optional attached part; bestiary
 * preview uses state 4. Return 0 on success, 1 on failure.
 */
s32 SkullMillioneFamily_Init(struct EwramData_EntityData *entity, u8 variant)
{
    const struct SkullMillioneFamilyVariant *info = &sSkullMillioneFamilyVariants[variant];
    if (!Entity_LoadGraphics(entity, sSkullMillioneFamilyGfx, sSkullMillioneFamilyPalette, info->palette, info->sprites)) return 1;
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 4;
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    entity->ext.skullMillione.variant = variant;
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 4;
        return 0;
    }
    else
    {
        Hit_EnableContact(entity, 8, SkullMillioneFamily_DamagePlayerOnContact, (s32)SkullMillioneFamily_OnHit);
        Entity_UpdateBoxes(entity);
        entity->scrollWithBg = 1;
        entity->state = 0;
        if ((info->behaviorFlags & 1) && !SkullMillioneFamily_CreatePart(entity)) return 1;
    }
    return 0;
}

/**
 * @brief Shared variant dispatcher: update visibility, tracking, stun handling,
 * state logic, hit checks and animation; remove the completed death state.
 */
void SkullMillioneFamily_Update(struct EwramData_EntityData *entity)
{
    u32 box[4];
    u32 outside = EntityIsOutsideScreen(entity, 4);
    outside = (entity->hidden = outside);
    if (!outside) Player_TrackEntity(entity);
    if (Enemy_IsStunned(entity))
    {
        if (entity->state != 3) Hit_CheckWithPlayer(box, entity);
        return;
    }
    switch (entity->state)
    {
    case 0: SkullMillioneFamily_UpdateWait(entity); break;
    case 1: SkullMillioneFamily_UpdateApproach(entity); break;
    case 2: SkullMillioneFamily_UpdateAttack(entity); break;
    case 3:
        if (SkullMillioneFamily_UpdateDeath(entity)) { EntityDelete(entity); return; }
        break;
    case 4:
        Anim_Update(entity);
        return;
    }
    if (entity->state != 3)
    {
        Hit_CheckWithPlayer(box, entity);
        Entity_AnimateWithBoxes(entity);
    }
    Anim_Update(entity);
}

/* Skull Millione / Cagnazzo / Lubicant helpers 0x080B8004..0x080B80BB. */
extern s32 Enemy_TakeDamage(struct EwramData_EntityData *entity);

/**
 * @brief Apply enemy damage; on defeat drop rewards, restore the palette
 * and enter death state 3 with the substate counters reset.
 */
void SkullMillioneFamily_OnHit(struct EwramData_EntityData *entity)
{
    if (Enemy_TakeDamage(entity))
    {
        Enemy_DropRewards(entity);
        entity->palette = entity->basePalette;
        entity->state = 3;
        entity->stage = 0;
        entity->subState = 0;
    }
}

/**
 * @brief B8044 | Skull Millione create: SkullMillioneFamily_Init variant 0, delete the
 * entity when that fails.
 */
void EnemySkullMillioneCreate(struct EwramData_EntityData *entity)
{
    if (SkullMillioneFamily_Init(entity, 0))
        EntityDelete(entity);
}

/**
 * @brief B8060 | Skull Millione update.
 */
void EnemySkullMillioneUpdate(struct EwramData_EntityData *entity)
{
    SkullMillioneFamily_Update(entity);
}

/**
 * @brief B806C | Cagnazzo create: variant 1.
 */
void EnemyCagnazzoCreate(struct EwramData_EntityData *entity)
{
    if (SkullMillioneFamily_Init(entity, 1))
        EntityDelete(entity);
}

/**
 * @brief B8088 | Cagnazzo update: same as Skull Millione.
 */
void EnemyCagnazzoUpdate(struct EwramData_EntityData *entity)
{
    SkullMillioneFamily_Update(entity);
}

/**
 * @brief B8094 | Lubicant create: variant 2.
 */
void EnemyLubicantCreate(struct EwramData_EntityData *entity)
{
    if (SkullMillioneFamily_Init(entity, 2))
        EntityDelete(entity);
}

/**
 * @brief B80B0 | Lubicant update: same as Skull Millione.
 */
void EnemyLubicantUpdate(struct EwramData_EntityData *entity)
{
    SkullMillioneFamily_Update(entity);
}
