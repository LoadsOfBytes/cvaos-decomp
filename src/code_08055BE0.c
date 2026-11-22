#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern u8 sWarpRoomGfx[], sWarpRoomPalette[], sWarpRoomSpriteSheet[];
extern void WarpRoom_UpdateGatePart(struct EwramData_EntityData *);
extern void WarpRoom_ResetGatePart(struct EwramData_EntityData *);

/**
 * @brief Create the nine gate part sprites (update WarpRoom_UpdateGatePart)
 * and reset each. Return 1 when any was created.
 */
s32 WarpRoom_CreateGateParts(struct EwramData_EntityData *owner)
{
    s32 created = 0;
    s32 graphics = ObjGfx_Find(sWarpRoomGfx);
    s32 i;
    ObjPalette_LoadRange(sWarpRoomPalette, 2, 1, 0xA);
    for (i = 0, i = created; i <= 8; i++)
    {
        struct EwramData_EntityData *entity = EntityCreateInRange(0x1B, 0x48, (u32 *)EntityUpdateNothing);
        if (entity != NULL)
        {
            created = 1;
            Sprite_Attach(entity, sWarpRoomSpriteSheet, graphics, 0xA);
            entity->scrollWithBg = 1;
            entity->animFrame = i;
            ENTITY_PARENT_INDEX(entity) = ENTITY_INDEX(owner);
            if (i == 0)
            {
                register u8 *layer asm("r0") = &entity->spriteLayer; /* FAKEMATCH: pinned register */
                *layer = 0x1C;
            }
            if (i >= 1 && i <= 4)
                entity->spriteLayer = 0x1D;
            if (i >= 5 && i <= 8)
                entity->spriteLayer = 0x1E;
            entity->updateFunc = (u32 *)WarpRoom_UpdateGatePart;
            entity->affineIndex = ObjAffine_Alloc(1);
            entity->drawFunc = (u32 *)Sprite_DrawEntity;
            WarpRoom_ResetGatePart(entity);
        }
    }
    return created;
}
