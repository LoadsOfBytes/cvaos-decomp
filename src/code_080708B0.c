/* Oscillate horizontally while the contact classification permits movement. */
#include "code_08009A0.h"
#include "structs/ewram.h"
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);
extern s32 Entity_MoveWithClassifiedPointCollision(struct EwramData_EntityData *, s32, s32, s32);
/**
 * @brief Damp and alternate horizontal velocity; delete when movement contact classification no longer matches 0x80 under mask 0x88.
 */
void KillerFishEffect_UpdateOscillating(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
    case 0:
        entity->state = 1;
        entity->timer = 0;
        /* fall through */
    case 1:
        entity->timer++;
        EntityDampVelocityX(entity, 8);
        if ((entity->timer & 7) == 0)
        {
            if (entity->timer & 8) entity->velX.whole = -0x4000;
            else entity->velX.whole = 0x4000;
        }
        if ((Entity_MoveWithClassifiedPointCollision(entity, 0, -0x20000, 1) & 0x88) != 0x80)
            EntityDelete(entity);
        break;
    }
}
