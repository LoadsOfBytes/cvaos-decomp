/* Select an approach or attack on animation events, then apply movement. */
#include "structs/ewram.h"
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Target_IsInRect(s32, s32, s32, s32);
extern s32 Entity_MoveWithCollision(struct EwramData_EntityData *, s32, s32, s32, s32);
/**
 * @brief Walk state (state 0): on the step frames 3 / 7 face the target and attack
 * (state 1) when it is in the 100 x 70 box; otherwise keep walking.
 */
void ZombieOfficer_UpdateWalk(struct EwramData_EntityData *entity)
{
    if (entity->subState != 0) return;
    if ((entity->animStep == 3 || entity->animStep == 7) && entity->animTimer == 0)
    {
        entity->flipX = Entity_IsTargetRight(entity);
        if (Target_IsInRect((s16)(entity->posX.part16.integer - 50), (s16)(entity->posY.part16.integer - 64), 100, 70))
        {
            entity->state = 1;
            entity->subState = 0;
            return;
        }
        if (entity->flipX) entity->velX.whole = 0xA000;
        else entity->velX.whole = -0xA000;
        entity->velY.whole = 0x10000;
        entity->accelY.whole = 0x2800;
    }
    if ((entity->animStep == 2 || entity->animStep == 6) && entity->animTimer == 0) entity->velX.whole = 0;
    Entity_MoveWithCollision(entity, 0x10000, -0x200000, 0x80000, 10);
}
