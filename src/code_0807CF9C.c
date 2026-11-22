/* Update Alura Une and clear the room event when she leaves its bounds. */
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
extern s32 Enemy_PreUpdate(struct EwramData_EntityData *);
extern void Sound_StopTracked(s32);
extern void AluraUne_UpdateIdle(struct EwramData_EntityData *);
extern void AluraUne_UpdateAttack(struct EwramData_EntityData *);
extern void AluraUne_UpdatePreview(struct EwramData_EntityData *);
extern void Enemy_UpdateFragmentDeath(struct EwramData_EntityData *);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
void EnemyAluraUneUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    if (Enemy_PreUpdate(entity))
    {
        if (!Entity_IsInRect(entity, -128, -64, 496, 288)) Sound_StopTracked(0x105);
        goto finished;
    }
    switch (entity->state)
    {
    case 0: AluraUne_UpdateIdle(entity); break;
    case 1: AluraUne_UpdateAttack(entity); break;
    case 2: Enemy_UpdateFragmentDeath(entity); break;
    case 3:
        AluraUne_UpdatePreview(entity);
        Anim_Update(entity);
        goto finished;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
finished:
    return;
}
