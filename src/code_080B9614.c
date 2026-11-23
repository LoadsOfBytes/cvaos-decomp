/* Witch activation and attack selection. */
#include "code_08009A0.h"
#include "structs/ewram.h"
extern s32 Target_IsNearEntity(struct EwramData_EntityData *, u16, u16);
extern s32 Target_GetPixelX(struct EwramData_EntityData *);
extern s16 Target_GetPixelY(struct EwramData_EntityData *);

#define ABS(x) ((x) < 0 ? -(x) : (x))

/**
 * @brief Wait state (0): activate near the target on an animation boundary.
 * Choose charge when facing and height tests plus randomness allow it;
 * otherwise enter the spell approach state.
 */
void Witch_UpdateWait(struct EwramData_EntityData *entity)
{
    s32 width = 160;
    s32 height = 220;

    if (entity->ext.witch.flags & 4)
    {
        height += 180;
        width = 240;
    }
    if (*(u16 *)&entity->animStep == 0 && Target_IsNearEntity(entity, height, width))
    {
        s32 facing;
        s32 left;
        s32 t;
        s32 x;
        s16 *px;

        entity->ext.witch.flags |= 4;
        facing = entity->flipX;
        px = &entity->posX.part16.integer;
        t = Target_GetPixelX(entity);
        x = *px;
        left = (s16)t;
        if (x >= left)
            left = 0;
        else
            left = 1;
        if (facing == left
            && (RandomNumberGenerator() & 2)
            && ABS(Target_GetPixelY(entity) - ({ s32 t_ = entity->posY.part16.integer + 0x30; t_; })) < 0x20)
        {
            entity->state = 2;
            entity->stage = 0;
            entity->subState = 0;
        }
        else
        {
            entity->state = 1;
            entity->stage = 0;
            entity->subState = 0;
        }
    }
}
