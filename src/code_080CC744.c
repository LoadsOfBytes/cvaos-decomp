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
        ENTITY_PARENT_INDEX(child) = ENTITY_INDEX(entity);
        ENTITY_PARENT_INDEX(entity) = ENTITY_INDEX(child);
        child->spriteLayer = entity->spriteLayer - 1;
        child->basePalette = entity->basePalette;
        child->flipX = entity->flipX;
        Sprite_Attach(child, sBaelBuerSpriteSheet, Sprite_GetTileSlot(entity), child->basePalette);
        child->animFrame = 1;
        child->scrollWithBg = 1;
        child->drawFunc = (u32 *)Sprite_DrawEntity;
        child->posX.whole = entity->posX.whole;
        child->posY.whole = entity->posY.whole;
        if ((s32)entity->ext.wheel.speed >= 0)
            child->flipX = 1;
        else
            child->flipX = 0;
    }
}
