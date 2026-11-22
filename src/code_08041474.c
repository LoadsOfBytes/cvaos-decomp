#include "code_08033CAC.h"

/**
 * @brief 41474 | Test whether the hitboxes of two entities overlap
 *
 * @return 1 on overlap, 0 otherwise (also when either hitbox is 1 pixel wide or high)
 */
u32 HitBox_Overlap(struct EwramData_EntityData *e1, struct HitBoxDef *hb1, struct EwramData_EntityData *e2, struct HitBoxDef *hb2)
{
    s32 l1, r1, l2, r2;

    if (hb1->width == 1)
        return 0;
    if (hb1->height == 1)
        return 0;
    if (hb2->width == 1)
        return 0;
    if (hb2->height == 1)
        return 0;

    if (!e1->flipX)
    {
        l1 = (s16)e1->posX.part16.integer + hb1->x;
        {
            s32 tmp = l1 - 1;
            r1 = tmp + hb1->width;
        }
    }
    else
    {
        r1 = (s16)e1->posX.part16.integer - hb1->x;
        {
            s32 tmp = r1 + 1;
            l1 = tmp - hb1->width;
        }
    }
    if (!e2->flipX)
    {
        l2 = (s16)e2->posX.part16.integer + hb2->x;
        {
            s32 tmp = l2 - 1;
            r2 = tmp + hb2->width;
        }
    }
    else
    {
        r2 = (s16)e2->posX.part16.integer - hb2->x;
        {
            s32 tmp = r2 + 1;
            l2 = tmp - hb2->width;
        }
    }
    if (l1 > r2)
        return 0;
    if (l2 > r1)
        return 0;

    if (!e1->flipY)
    {
        l1 = (s16)e1->posY.part16.integer + hb1->y;
        {
            s32 tmp = l1 - 1;
            r1 = tmp + hb1->height;
        }
    }
    else
    {
        r1 = (s16)e1->posY.part16.integer - hb1->y;
        {
            s32 tmp = r1 + 1;
            l1 = tmp - hb1->height;
        }
    }
    if (!e2->flipY)
    {
        l2 = (s16)e2->posY.part16.integer + hb2->y;
        {
            s32 tmp = l2 - 1;
            r2 = tmp + hb2->height;
        }
    }
    else
    {
        r2 = (s16)e2->posY.part16.integer - hb2->y;
        {
            s32 tmp = r2 + 1;
            l2 = tmp - hb2->height;
        }
    }
    if (l1 > r2)
        return 0;
    if (l2 <= r1)
        return 1;
    return 0;
}
