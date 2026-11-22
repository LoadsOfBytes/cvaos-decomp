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

void EnemyDryadUpdate(struct EwramData_EntityData *entity)
{
    if (Enemy_PreUpdate(entity)) return;
    switch (entity->state)
    {
    case 0: Dryad_UpdateIdle(entity); break;
    case 1: Dryad_UpdateAttack(entity); break;
    case 2:
        if (entity->animFlags & 4)
        {
            entity->state = 0;
            entity->subState = 0;
            entity->stage = 0;
        }
        break;
    case 3: Dryad_UpdateDeath(entity); break;
    case 4:
        Anim_Update(entity);
        return;
    }
    Enemy_CheckHitPlayer(entity);
    Entity_AnimateWithBoxes(entity);
}
