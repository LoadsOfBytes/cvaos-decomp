/* Convert both active entity boxes into world-space rectangles. */
#include "structs/ewram.h"
#include "code_08033CAC.h"
#include "structs/entity_boxes.h"
struct RectangleWords { u32 a, b; };
extern void HitBox_SetHitRect2(struct EwramData_EntityData *, struct HitBoxDef *);
/**
 * @brief Unreferenced: when the entity hit is enabled, store the rectangles of
 * its two hit boxes in first/second and return 1, else 0.
 */
s32 HitBox_GetBothRects(struct RectangleWords *first, struct RectangleWords *second, struct EwramData_EntityData *entity)
{
    struct EntityHit *state = &entity->hit;
    if (state->flags & 1)
    {
        struct HitBoxDef *boxes = (struct HitBoxDef *)&gEntityBoxes[entity->hit.boxSlot];
        struct HitBoxDef *secondBox = boxes + 1;
        HitBox_SetHitRect2(entity, boxes);
        *first = *(struct RectangleWords *)&gEwramData->hitRectLeft;
        HitBox_SetHitRect2(entity, secondBox);
        *second = *(struct RectangleWords *)&gEwramData->hitRectLeft;
        return 1;
    }
    return 0;
}
