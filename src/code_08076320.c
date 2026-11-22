/* Harpy knockback and the small projectile's movement/cleanup. */
#include "code_08009A0.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
extern void EntityMove(struct EwramData_EntityData *);
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern void Enemy_UpdateFragmentDeath(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *,struct EwramData_EntityData *);
/**
 * @brief Death state (state 2): fall away from the target under gravity.
 */
void Harpy_UpdateDeath(struct EwramData_EntityData *entity)
{
    switch(entity->subState)
    {
        case 0:
            entity->flipX=Entity_IsTargetRight(entity);
            if(entity->flipX) entity->velX.whole=-0x10000;
            else entity->velX.whole=0x10000;
            entity->velY.whole=-0x20000;
            { s16 zero=0; entity->accelX.whole=zero; }
            entity->accelY.whole=0x2000;
            entity->subState++;
        case 1:
            EntityAccelerateAndMove(entity,0x80000);
            Enemy_UpdateFragmentDeath(entity);
            break;
    }
}
void HarpyFeather_Update(struct EwramData_EntityData *entity)
{
    s32 bounds[4];
    switch(entity->state)
    {
        case 0:
            EntityMove(entity);
            Hit_CheckWithPlayer(bounds,entity);
            if(!EntityIsOutsideScreen(entity,1)) break;
        case 1:
            EntityDelete(entity);
            break;
        case 2:
            EntityDelete(entity);
            break;
    }
}
