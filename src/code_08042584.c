/* Player hitbox rectangles: writes the two attack/body boxes of the current
 * player entity (gEntityBoxes slot boxSlot) into the IWRAM rectangles at
 * 0x03001A48 and 0x03001A50, mirrored by flipX/flipY. */
#include "gba.h"
#include "structs/ewram.h"
#include "structs/entity_boxes.h"

struct Rect16 { s16 left; s16 top; s16 right; s16 bottom; };

#define RECT0 ((struct Rect16 *)0x03001A48)
#define RECT1 ((struct Rect16 *)0x03001A50)

void HitBox_UpdatePlayerRects(void)
{
    struct EwramData_EntityData *p;
    struct SpriteBox *b0;
    struct SpriteBox *b1;
    struct Rect16 *r;
    struct Rect16 *q;

    p = gEwramData->playerState.player;
    b0 = &gEntityBoxes[p->hit.boxSlot].attack.box;
    b1 = &gEntityBoxes[p->hit.boxSlot].body.box;
    if (!p->flipX)
    {
        RECT0->left = b0->x + p->posX.part16.integer;
        RECT0->right = RECT0->left + b0->width;
        r = RECT0;
    }
    else
    {
        RECT0->right = p->posX.part16.integer - b0->x;
        RECT0->left = RECT0->right - b0->width;
        r = RECT0;
    }
    if (!p->flipY)
    {
        r->top = b0->y + p->posY.part16.integer;
        r->bottom = r->top + b0->height;
    }
    else
    {
        r->bottom = p->posY.part16.integer - b0->y;
        r->top = r->bottom - b0->height;
    }
    if (!p->flipX)
    {
        RECT1->left = b1->x + p->posX.part16.integer;
        RECT1->right = RECT1->left + b1->width;
        q = RECT1;
    }
    else
    {
        RECT1->right = p->posX.part16.integer - b1->x;
        RECT1->left = RECT1->right - b1->width;
        q = RECT1;
    }
    if (!p->flipY)
    {
        q->top = b1->y + p->posY.part16.integer;
        q->bottom = q->top + b1->height;
    }
    else
    {
        q->bottom = p->posY.part16.integer - b1->y;
        q->top = q->bottom - b1->height;
    }
}
