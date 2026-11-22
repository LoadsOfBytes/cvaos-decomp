/* Attached animated effect: follows a sprite part while its owner's flag 2 is set. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void Sprite_GetPartCornerOffset(struct EwramData_EntityData *, s32, void *);
extern u8 sWaiterSkeletonGfx[], sSkeletonWaiterSkeletonPalette[], sWaiterSkeletonSpriteSheet[];
void WaiterSkeletonTray_Update(struct EwramData_EntityData *entity)
{
    u16 offset[2];
    struct EwramData_EntityData *parent = entity->unk_4FC.unk_4FC_entity;
    if (parent->state == 3 || !(parent->unk_504.whole & 2))
    {
        entity->deleteRequested = 1;
        return;
    }
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sWaiterSkeletonGfx, sSkeletonWaiterSkeletonPalette, 0, sWaiterSkeletonSpriteSheet))
        {
            EntityDelete(entity);
            return;
        }
        entity->scrollWithBg = 1;
        entity->spriteLayer = parent->spriteLayer;
        Anim_StartIndexed(entity, 17, 0, 1);
        entity->state++;
        break;
    case 1:
        {
            u16 *partOffset = offset;
            Sprite_GetPartCornerOffset(parent, 0, partOffset);
            entity->flipX = parent->flipX;
            entity->posX.part16.integer = parent->posX.part16.integer + offset[0];
            entity->posY.part16.integer = parent->posY.part16.integer + partOffset[1];
            if ((parent->unk_504.whole & 8) && entity->animScriptIndex != 8)
                Anim_StartIndexed(entity, 8, 0, 1);
            break;
        }
    case 2:
        break;
    }
    entity->palette = parent->palette;
    Anim_Update(entity);
}
