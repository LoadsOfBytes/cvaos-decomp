/* Create and retire an affine afterimage with its own matrix allocation. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
extern struct EwramData_EntityData *Entity_CreateObject(u32 *);
extern u8 sGiantGhostGfx[], sGiantGhostSpriteSheet[];
extern void GiantGhostAuraTrail_Update(struct EwramData_EntityData *);
/**
 * @brief Trail update: hide off screen vertically, keep the affine scale, animate;
 * free the matrix and delete when the timer ends.
 */
void GiantGhostAuraTrail_Update(struct EwramData_EntityData *entity)
{
    if ((u16)(entity->posY.part16.integer + 28) > 220) entity->hidden = 1;
    else entity->hidden = 0;
    ObjAffine_SetBiased(0, entity->velX.whole, entity->velY.whole, entity->flipX, entity->flipY, entity->affineIndex);
    Anim_Update(entity);
    if ((s8)entity->timer-- <= 0)
    {
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
    }
}
