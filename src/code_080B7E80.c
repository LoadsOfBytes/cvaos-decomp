/* Shared Skull Millione / Cagnazzo / Lubicant setup and movement. */
#include "code_08009A0.h"
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
    entity->unk_4F8.part8.unk_4F8 = variant;
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
