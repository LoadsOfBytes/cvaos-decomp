#include "code_08009A0.h"
#include "code/code_08039340.h"
extern struct EwramData_EntityData *Entity_CreateObject(void *);
extern u8 sManticoreGfx[], sManticoreOverlaySpriteSheet[];
void ManticoreOverlay_Update(struct EwramData_EntityData *);

/**
 * @brief Create the overlay sprite drawn three layers above the Manticore that
 * copies its animation frame (ManticoreOverlay_Update); return it or NULL.
 */
struct EwramData_EntityData *Manticore_CreateOverlay(struct EwramData_EntityData *parent)
{
    struct EwramData_EntityData *child = Entity_CreateObject(ManticoreOverlay_Update);
    s32 slot;
    if (child != NULL)
    {
        child->posX.whole = parent->posX.whole;
        child->posY.whole = parent->posY.whole;
        child->spriteLayer = parent->spriteLayer + 3;
        slot = ObjGfx_Find(sManticoreGfx);
        Sprite_Attach(child, sManticoreOverlaySpriteSheet, slot, parent->unk_510.part8.basePalette);
        child->animFrame = parent->animFrame;
        child->scrollWithBg = 0;
        child->drawFunc = (u32 *)Sprite_DrawEntity;
        ENTITY_CREATOR(child) = parent;
        }
    else return NULL;
    return child;
}

/**
 * @brief Overlay update: copy the position, palette, facing, frame and visibility
 * of the owner (type 0x36); delete when the owner is gone.
 */
void ManticoreOverlay_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = ENTITY_CREATOR(entity);
    if (parent != NULL && parent->updateFunc != NULL && parent->unk_518.part8.enemyId == 0x36)
    {
        entity->posX.whole = parent->posX.whole;
        entity->posY.whole = parent->posY.whole;
        entity->palette = parent->palette;
        entity->flipX = parent->flipX;
        entity->animFrame = parent->animFrame;
        entity->hidden = parent->hidden;
    }
    else EntityDelete(entity);
}
