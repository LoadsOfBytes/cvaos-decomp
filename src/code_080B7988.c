/* Shared Skull Millione / Cagnazzo / Lubicant setup and movement. */
#include "code_08009A0.h"
#include "skull_millione_entity.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/enemy_variant_b7e80.h"
#include "macros.h"
#define BACKGROUND ((struct BgLayerState *)((u8 *)gEwramData + OFFSET_OF(struct EwramData, bgInfo[1])))
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void SkullMillioneFamilyPart_Update(struct EwramData_EntityData *);
extern struct EwramData_EntityData *Entity_CreateObject(void *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *);
extern s32 Hit_EnableContact(struct EwramData_EntityData *, s32, void *, s32);
extern void SkullMillioneFamily_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void SkullMillioneFamily_OnHit(struct EwramData_EntityData *);
extern void Entity_FaceTarget(struct EwramData_EntityData *);
extern s32 Target_IsNearEntity(struct EwramData_EntityData *, s32, s32);
extern s32 Target_GetPixelX(struct EwramData_EntityData *);
extern s32 Target_GetPixelY(struct EwramData_EntityData *);
extern s32 Target_GetX(struct EwramData_EntityData *);
extern s32 Target_GetVelocityX(void);
extern s32 Entity_AimVelocityAtPoint(struct EwramData_EntityData *, s32, s32, s32);
extern void EntityMove(struct EwramData_EntityData *);
extern s32 Enemy_IsStunned(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void SkullMillioneFamily_UpdateAttack(struct EwramData_EntityData *);
extern s32 SkullMillioneFamily_UpdateDeath(struct EwramData_EntityData *);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);
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
    const struct SkullMillioneFamilyVariant *info = &sSkullMillioneFamilyVariants[(u8)SKULL_MILLIONE_VARIANT(entity)];
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
    const struct SkullMillioneFamilyVariant *info = &sSkullMillioneFamilyVariants[(u8)SKULL_MILLIONE_VARIANT(entity)];
    switch (entity->subState)
    {
    case 0:
        entity->subState = 1;
        SKULL_MILLIONE_BOB_FRAME(entity) = 0x8000;
        entity->timer = info->activationDelay;
        /* fall through */
    case 1:
        SKULL_MILLIONE_BOB_FRAME(entity)++;
        entity->velY.whole = Sin((0x5B0 * SKULL_MILLIONE_BOB_FRAME(entity)) & 0xFFFF) / 2;
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
    const struct SkullMillioneFamilyVariant *info = &sSkullMillioneFamilyVariants[(u8)SKULL_MILLIONE_VARIANT(entity)];
    s32 x;
    s32 y;
    s32 arrived;
    switch (entity->subState)
    {
    case 0:
        SKULL_MILLIONE_SPEED(entity) = 0;
        entity->subState = 1;
        /* fall through */
    case 1:
        if (entity->animTimer == 0) Entity_FaceTarget(entity);
        if (SKULL_MILLIONE_SPEED(entity) < info->approachSpeed) SKULL_MILLIONE_SPEED(entity) += 0x2000;
        x = ((s16)BACKGROUND->xPos.part16.integer + (s16)Target_GetPixelX(entity)) << 16;
        y = ((s16)BACKGROUND->yPos.part16.integer + (s16)Target_GetPixelY(entity) - 0x50) << 16;
        if (Target_GetX(entity) > entity->posX.whole) x -= SKULL_MILLIONE_SPEED(entity);
        else x += SKULL_MILLIONE_SPEED(entity);
        x += Target_GetVelocityX() * info->swayMultiplier;
        arrived = Entity_AimVelocityAtPoint(entity, x, y, SKULL_MILLIONE_SPEED(entity));
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

