#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void BlueSoulAfterimage_Update(struct EwramData_EntityData *);
/**
 * @brief Unreferenced: create a copy of the owner sprite frame one layer above
 * it (BlueSoulAfterimage_Update, 8 frames, blue soul slot from unk_508);
 * return it or NULL.
 */
struct EwramData_EntityData *BlueSoul_CreateAfterimage(struct EwramData_EntityData *owner)
{
    s32 graphics = Sprite_GetTileSlot(owner);
    u8 *index = &owner->spriteSlot;
    struct SpriteSheet *sprite = gEwramData->spriteSlots[*index].sheet;
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)BlueSoulAfterimage_Update);
    if (entity != NULL)
    {
        if (sprite != NULL)
        {
            Sprite_Attach(entity, sprite, graphics, owner->palette);
            entity->animFrame = owner->animFrame;
            entity->spriteLayer = owner->spriteLayer + 1;
            entity->flipX = owner->flipX;
            entity->timer = 8;
            entity->stage = owner->unk_508.part16.unk_508;
            entity->scrollWithBg = 1;
            return entity;
        }
        EntityDelete(entity);
    }
    return NULL;
}
