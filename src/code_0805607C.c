#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern u8 sWarpRoomGfx[], sWarpRoomPalette[], sWarpRoomSpriteSheet[];
extern void WarpRoom_UpdateSideLight(struct EwramData_EntityData *);

/**
 * @brief Create the two side light sprites (update WarpRoom_UpdateSideLight)
 * at x 0x54 and 0x9C. Return 1 when any was created.
 */
s32 WarpRoom_CreateSideLights(struct EwramData_EntityData *owner)
{
    s32 created = 0;
    s32 graphics = ObjGfx_Find(sWarpRoomGfx);
    s32 i;
    ObjPalette_LoadRange(sWarpRoomPalette, 4, 1, 0xB);
    for (i = 0, i = created; i <= 1; i++)
    {
        struct EwramData_EntityData *entity = EntityCreateInRange(0x1B, 0x48, (u32 *)EntityUpdateNothing);
        if (entity != NULL)
        {
            created = 1;
            Sprite_Attach(entity, sWarpRoomSpriteSheet, graphics, 0xB);
            if (i == 0)
                entity->posX.part16.integer = 0x54;
            else
                entity->posX.part16.integer = 0x9C;
            entity->posY.part16.integer = 0x80;
            entity->scrollWithBg = 1;
            entity->animFrame = 9;
            entity->spriteLayer = 0x15;
            entity->updateFunc = (u32 *)WarpRoom_UpdateSideLight;
            ENTITY_PARENT_INDEX(entity) = ENTITY_INDEX(owner);
            entity->ext.warpRoomSideLight.frame = 0;
            entity->state = 0;
            entity->drawFunc = (u32 *)Sprite_DrawEntity;
        }
    }
    return created;
}
