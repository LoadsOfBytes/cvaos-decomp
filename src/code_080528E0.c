#include "code_08009A0.h"
#include "room_object_entity.h"
#include "code/code_08039340.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern u8 sObject15Gfx[], sUnk_0820A1C4[], sObject15SpriteSheet[];
static inline s32 FixedMulFast(s32 a, s32 b) { return (a >> 8) * (b >> 8); }
/**
 * @brief Sprite of object 0x15 placed around its owner at the owner angle
 * unk_4FC and radius unk_500.
 */
void Object15Orbiter_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);
    if (entity->state == 0)
    {
        if (!Entity_LoadGraphics(entity, sObject15Gfx, sUnk_0820A1C4, 8, sObject15SpriteSheet)) { EntityDelete(entity); return; }
        entity->animFrame = 1;
        entity->spriteLayer = 31;
        entity->state = 1;
    }
    entity->velX.whole = FixedMulFast(Sin(OBJECT15_ANGLE(owner) + 0x4000), OBJECT15_RADIUS(owner));
    entity->velY.whole = FixedMulFast(Sin(OBJECT15_ANGLE(owner)), OBJECT15_RADIUS(owner));
    entity->posX.whole = owner->posX.whole + entity->velX.whole;
    entity->posY.whole = owner->posY.whole + entity->velY.whole;
}
