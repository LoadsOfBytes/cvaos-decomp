#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Dryad_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void Dryad_OnHit(struct EwramData_EntityData *, s32, u16 *);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern s32 Hit_EnableContact(struct EwramData_EntityData *, s32, void *, s32);
extern s32 Enemy_PreUpdate(struct EwramData_EntityData *);
extern void Dryad_UpdateIdle(struct EwramData_EntityData *);
extern void Dryad_UpdateAttack(struct EwramData_EntityData *);
extern void Dryad_UpdateDeath(struct EwramData_EntityData *);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);
extern u8 sDryadGfx[], sDryadPalette[], sDryadSpriteSheet[];

void EnemyDryadCreate(struct EwramData_EntityData *entity)
{
    if (!Entity_LoadGraphics(entity, sDryadGfx, sDryadPalette, 0, sDryadSpriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    if (gEwramData->gameState.bestiaryPreview) entity->state = 4;
    else
    {
        Enemy_ResetFromTemplate(entity);
        Hit_EnableContact(entity, 8, Dryad_DamagePlayerOnContact, (s32)Dryad_OnHit);
        Entity_UpdateBoxes(entity);
        Entity_SnapToGround(entity);
        entity->posY.part16.integer++;
        entity->scrollWithBg = 1;
        entity->state = 0;
    }
}
