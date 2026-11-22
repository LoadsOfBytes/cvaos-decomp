#include "code/code_08039340.h"
#include "code_08009A0.h"
#include "structs/ewram.h"

extern void Object37Child_Update(struct EwramData_EntityData *);
extern u8 sObject37Gfx[], sObject37SpriteSheet[];
static inline s32 Abs(s32 value) { if (value < 0) value = -value; return value; }

/**
 * @brief Create the child entity of object 0x37 (Object37Child_Update).
 */
void Object37_SpawnChild(struct EwramData_EntityData *owner)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)Object37Child_Update);
    struct EwramData **ewram = &gEwramData;
    s32 graphics;
    u16 transform;
    s32 two, scale;
    if (!entity)
        return;
    entity->posX.whole = owner->posX.whole;
    entity->posY.whole = owner->posY.whole;
    entity->spriteLayer = owner->spriteLayer + 1;
    graphics = ObjGfx_Find(sObject37Gfx);
    entity->unk_510.part8.basePalette = owner->unk_510.part8.basePalette;
    Sprite_Attach(entity, sObject37SpriteSheet, graphics, entity->unk_510.part8.basePalette);
    entity->animFrame = 2;
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
    transform = ObjAffine_Alloc(1);
    if ((s16)transform < 0)
    {
        EntityDelete(entity);
        return;
    }
    entity->affineIndex = transform;
    entity->affine = 1;
    ObjAffine_Set(0, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
    entity->unk_4F8.unk_4F8_entity = owner;
    entity->scrollWithBg = 1;
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    if (Abs(entity->posX.whole - (*ewram)->playerState.player->posX.whole) < 0x300000)
    {
        entity->unk_4FC.whole = -0x2000;
        entity->subState = 1;
        ObjAffine_Set(-0x2000, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
    }
}
