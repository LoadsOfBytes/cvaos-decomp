/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Bael_CreateHead = Buer_CreateHead */
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void BaelHead_Update(struct EwramData_EntityData *entity);
extern struct EwramData_EntityData *Entity_CreateObject(u32 *updateFunc);
extern u8 sBaelBuerSpriteSheet[];

/**
 * @brief Create the linked frame-1 head sharing graphics and palette; store mutual slot indices and face it according to movement speed.
 */
void Bael_CreateHead(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child = Entity_CreateObject((u32 *)BaelHead_Update);

    if (child != NULL)
    {
        child->unk_4F4.part8.unk_4F5 = entity->unk_4F4.part8.unk_4F4;
        entity->unk_4F4.part8.unk_4F5 = child->unk_4F4.part8.unk_4F4;
        child->spriteLayer = entity->spriteLayer - 1;
        child->unk_510.part8.basePalette = entity->unk_510.part8.basePalette;
        child->flipX = entity->flipX;
        Sprite_Attach(child, sBaelBuerSpriteSheet, Sprite_GetTileSlot(entity), child->unk_510.part8.basePalette);
        child->animFrame = 1;
        child->scrollWithBg = 1;
        child->drawFunc = (u32 *)Sprite_DrawEntity;
        child->posX.whole = entity->posX.whole;
        child->posY.whole = entity->posY.whole;
        if ((s32)entity->unk_4FC.whole >= 0)
            child->flipX = 1;
        else
            child->flipX = 0;
    }
}
