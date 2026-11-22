#include "code_08033CAC.h"

struct Rect16 { s16 left; s16 top; s16 right; s16 bottom; };

u32 HitBox_OverlapsRect(struct Rect16 *rect, struct EwramData_EntityData *entity, struct HitBoxDef *box)
{
    s32 left;
    s32 right;
    s32 top;
    s32 bottom;
    s32 tmp;

    if (rect->left == rect->right)
        return 0;
    if (rect->top == rect->bottom)
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
    if (rect->left > right)
        return 0;
    if (left > rect->right)
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
    if (rect->top > bottom)
        return 0;
    if (top <= rect->bottom)
        return 1;
    return 0;
}
