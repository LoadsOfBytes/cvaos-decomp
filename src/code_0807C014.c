/* Fall during the reaction, then resume the landing phase. */
#include "code/code_08039340.h"
extern s32 Entity_MoveWithCollision(struct EwramData_EntityData *, s32, s32, s32, s16);
/**
 * @brief Knock-down state (state 3): fall with gravity and, on landing, resume
 * the walk state (sub-state 6, animation 5).
 */
void Persephone_UpdateKnockdown(struct EwramData_EntityData *entity)
{
    s32 gravity = 0x2800;
    s32 zero;
    s32 initialSpeed;
    entity->accelY.whole = gravity;
    zero = 0;
    entity->velX.whole = zero;
    entity->accelX.whole = zero;
    initialSpeed = 0x10000;
    if (Entity_MoveWithCollision(entity, initialSpeed, -0x200000, 0x80000, 8) & 4)
    {
        entity->state = zero;
        entity->subState = 6;
        entity->timer = zero;
        entity->velY.whole = initialSpeed;
        entity->accelY.whole = gravity;
        Anim_StartIndexed(entity, 5, 0, 1);
    }
}
