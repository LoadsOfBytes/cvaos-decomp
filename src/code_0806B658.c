#include "code_08009A0.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

struct HitRect {
    s16 left;
    s16 top;
    s16 right;
    s16 bottom;
};

/* Contact point of two entities: centre of the overlap of their boxes,
 * clamped to the second box. */
/**
 * @brief Estimate a contact point from the first attack box and second
 * body box, clamping the result to the second box. Y starts at
 * the first box center; missing boxes leave the outputs untouched.
 */
void Hit_GetContactPoint(struct EwramData_EntityData *a, struct EwramData_EntityData *b, s16 *outX, s16 *outY)
{
    struct HitRect ra;
    struct HitBoxRecord a0, a1;
    struct HitRect rb;
    struct HitBoxRecord b0, b1;
    struct HitRect *rect = &ra;
    struct HitRect *rect2;
    struct HitBoxRecord *box = &a0;

    if (!HitBox_GetBoxes(box, &a1, a))
        return;
    if (a->flipX)
    {
        { s32 dx = box->x; rect->right = a->posX.part16.integer - dx; }
        rect->left = rect->right - box->width;
    }
    else
    {
        rect->left = box->x + a->posX.part16.integer;
        rect->right = rect->left + box->width;
    }
    rect->top = box->y + a->posY.part16.integer;
    rect->bottom = rect->top + box->height;
    rect2 = &rb;
    if (!HitBox_GetBoxes(&b0, &b1, b))
        return;
    box = &b1;
    if (b->flipX)
    {
        { s32 dx = box->x; rect2->right = b->posX.part16.integer - dx; }
        rect2->left = rect2->right - box->width;
    }
    else
    {
        rect2->left = box->x + b->posX.part16.integer;
        rect2->right = rect2->left + box->width;
    }
    rect2->top = box->y + b->posY.part16.integer;
    rect2->bottom = rect2->top + box->height;
    if (ra.left < rb.left)
        *outX = ra.right - (ra.right - rb.left) / 2;
    else
        *outX = ra.left + (rb.right - ra.left) / 2;
    *outY = ra.top + (ra.bottom - ra.top) / 2;
    if (*outX < rb.left)
        *outX = rb.left;
    else if (*outX > rb.right)
        *outX = rb.right;
    if (*outY < rb.top)
        *outY = rb.top;
    else if (*outY > rb.bottom)
        *outY = rb.bottom;
}
