/* Short animation phase. */
#include "code/code_08039340.h"
#include "structs/ewram.h"

/**
 * @brief Wall attack state (6): play animation 0x19 and set weapon flag
 * 0x10. Wait for the animation and for the weapon wall-traversal helper
 * to clear that flag, then return to hover state 3.
 */
void Death_UpdateWallAttack(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        Anim_StartIndexed(entity, 0x19, 2, 0);
        entity->ext.death.flags |= 0x10;
        entity->subState = 1;
        break;
    case 1:
        if (entity->animFlags & 4)
            entity->subState = 2;
        break;
    case 2:
        if (!(entity->ext.death.flags & 0x10))
        {
            Anim_StartIndexed(entity, 7, 2, 1);
            entity->state = 3;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    }
}
