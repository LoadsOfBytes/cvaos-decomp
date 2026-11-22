/* Bael collision motion, rotation, and update dispatcher. */
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 Entity_Walk(struct EwramData_EntityData *entity,s32 speed,void *profile,s32 flags);
extern s32 Entity_IsOffsetOutsideRoom(struct EwramData_EntityData *entity,s32 x,s32 y);
extern void Bael_UpdateState4Rotation(struct EwramData_EntityData *entity);
extern void Bael_UpdateDeath(struct EwramData_EntityData *entity);
extern void Bael_UpdateEntry(struct EwramData_EntityData *entity);
extern void Bael_UpdateActive(struct EwramData_EntityData *entity);
extern s32 Enemy_IsStunned(struct EwramData_EntityData *entity);
extern void Hit_CheckWithPlayer(void *buffer,struct EwramData_EntityData *entity);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *entity);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *entity);
extern s16 sBaelWalkProfile[];
extern s32 Collision_GetPushUp(s32 x,s32 y);
extern void Bael_UpdateActive(struct EwramData_EntityData *entity);
void EnemyBaelUpdate(struct EwramData_EntityData *entity)
{
    u8 buffer[16];
    if (entity->state == 4)
    {
        Bael_UpdateState4Rotation(entity);
        return;
    }
    if (entity->state == 3)
    {
        Bael_UpdateDeath(entity);
        return;
    }
    Player_TrackEntity(entity);
    if (Enemy_IsStunned(entity))
    {
        Hit_CheckWithPlayer(buffer,entity);
        return;
    }
    switch (entity->state)
    {
        case 1: Bael_UpdateEntry(entity); break;
        case 2: Bael_UpdateActive(entity); break;
        case 3: Bael_UpdateDeath(entity); break;
    }
    Hit_CheckWithPlayer(buffer,entity);
}


