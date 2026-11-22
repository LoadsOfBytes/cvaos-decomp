/* HitBox_GetBoxes. */
#include "code_08009A0.h"
#include "structs/entity_boxes.h"

/**
 * @brief 42030 | Copy the entity's attack and body boxes
 *
 * @param attack Destination of the attack box
 * @param body Destination of the body box
 * @param entity Entity
 * @return 1, or 0 when the hit block is inactive
 */
s32 HitBox_GetBoxes(struct SpriteBox *attack, struct SpriteBox *body, struct EwramData_EntityData *entity)
{
    struct SpriteBox *src;
    struct EntityHit *hit = &entity->hit;

    if (hit->flags & 1)
    {
        src = &gEntityBoxes[entity->hit.boxSlot].attack.box;
        attack->x = src->x;
        attack->y = src->y;
        attack->width = src->width;
        attack->height = src->height;
        src = &gEntityBoxes[entity->hit.boxSlot].body.box;
        body->x = src->x;
        body->y = src->y;
        body->width = src->width;
        body->height = src->height;
        return 1;
    }
    return 0;
}
