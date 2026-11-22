#include "code_08009A0.h"
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
    struct EwramData_EntityData *owner = entity->unk_4F8.unk_4F8_entity;
    if (entity->state == 0)
    {
        if (!Entity_LoadGraphics(entity, sObject15Gfx, sUnk_0820A1C4, 8, sObject15SpriteSheet)) { EntityDelete(entity); return; }
        entity->animFrame = 1;
        entity->spriteLayer = 31;
        entity->state = 1;
    }
    entity->velX.whole = FixedMulFast(Sin(owner->unk_4FC.part16.unk_4FC + 0x4000), owner->unk_500.whole);
    entity->velY.whole = FixedMulFast(Sin(owner->unk_4FC.part16.unk_4FC), owner->unk_500.whole);
    entity->posX.whole = owner->posX.whole + entity->velX.whole;
    entity->posY.whole = owner->posY.whole + entity->velY.whole;
}
