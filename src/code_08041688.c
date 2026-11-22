#include "code_08033CAC.h"

/**
 * @brief 41688 | Test whether an entity's hitbox overlaps the rectangle at EWRAM 0x13074
 *
 * @param entity Entity (position and flip flags)
 * @param box Hitbox (signed offset, width, height)
 * @return 1 on overlap, 0 otherwise (also when the rectangle or the hitbox is degenerate)
 */
u32 HitBox_OverlapsHitRect(struct EwramData_EntityData *entity, struct HitBoxDef *box)
{
    s32 left;
    s32 right;
    s32 tmp;
    s32 top;
    s32 bottom;

    if ((s16)gEwramData->hitRectLeft == (s16)gEwramData->hitRectRight)
        return 0;
    if ((s16)gEwramData->hitRectTop == (s16)gEwramData->hitRectBottom)
        return 0;
    if (box->width == 1)
        return 0;
    if (box->height == 1)
        return 0;

    if (!entity->flipX)
    {
        tmp = (s16)entity->posX.part16.integer;
        top = box->x;
        left = tmp + top;
        right = left - 1;
        tmp = box->width;
        right += tmp;
    }
    else
    {
        right = (s16)entity->posX.part16.integer - box->x;
        left = right + 1;
        tmp = box->width;
        left -= tmp;
    }
    if ((s16)gEwramData->hitRectLeft > right)
        return 0;
    if (left > (s16)gEwramData->hitRectRight)
        return 0;

    if (!entity->flipY)
    {
        top = (s16)entity->posY.part16.integer + box->y;
        bottom = top - 1;
        tmp = box->height;
        bottom += tmp;
    }
    else
    {
        bottom = (s16)entity->posY.part16.integer - box->y;
        top = bottom + 1;
        tmp = box->height;
        top -= tmp;
    }
    if ((s16)gEwramData->hitRectTop > bottom)
        return 0;
    if (top <= (s16)gEwramData->hitRectBottom)
        return 1;
    return 0;
}
