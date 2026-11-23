#include "code_08009A0.h"
#include "save_point_entity.h"
#include "code_0800CB00.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "structs/entity_render_flags.h"

extern void Entity_SnapToGround(struct EwramData_EntityData *entity);
extern void SavePoint_SpawnAnimParts(struct EwramData_EntityData *entity);
extern void SavePoint_DrawPromptMarkers(struct EwramData_EntityData *entity);
extern u8 sSaveWarpRoomGfx[], sSaveWarpRoomPalette[], sSaveWarpRoomSpriteSheet[];

/**
 * @brief 5464C | Object 0x1C init: Hud_CloseMessageWindow, Text_SetTileBuffer(VRAM
 * 0x06005000), graphics sSaveWarpRoomGfx (ObjGfx_Find or newly loaded,
 * deleted when neither works) in unk_4FC, palette sSaveWarpRoomPalette to slot 10
 * (unk_4FE), affine slot in unk_4FB with scale unk_500, screen position
 * (0x78, 0x50), layer 0x11, Entity_SnapToGround / SavePoint_SpawnAnimParts; plus a helper
 * entity (layer 3, hidden, drawn by SavePoint_DrawPromptMarkers with sprite
 * sSaveWarpRoomSpriteSheet) whose unk_4F4 byte is kept in byte 1 of hp.
 */
void Object1CCreate(struct EwramData_EntityData *entity)
{
    u8 *gfx = sSaveWarpRoomGfx;
    u8 *pal = sSaveWarpRoomPalette;
    s32 graphics;
    struct EwramData_EntityData *child;

    Hud_CloseMessageWindow();
    Text_SetTileBuffer((void *)0x06005000);
    graphics = ObjGfx_Find(gfx);
    if (graphics < 0 && (graphics = ObjGfx_LoadResource(gfx, 0)) < 0)
    {
        EntityDelete(entity);
        return;
    }
    entity->ext.savePoint.graphics = graphics;
    ObjPalette_LoadRange(pal, 0, 4, 10);
    entity->ext.savePoint.frameBase = 10;
    entity->ext.savePoint.affine = ObjAffine_Alloc(1);
    entity->affineIndex = entity->ext.savePoint.affine;
    ObjAffine_Set(0, entity->ext.savePoint.scale, entity->ext.savePoint.scale, 0, 0, entity->ext.savePoint.affine);
    entity->posX.part16.integer = 0x78;
    entity->posY.part16.integer = 0x50;
    entity->spriteLayer = 0x11;
    Entity_SnapToGround(entity);
    SavePoint_SpawnAnimParts(entity);
    child = EntityCreateInRange(0x1B, 0x48, (u32 *)EntityUpdateNothing);
    if (child != NULL)
    {
        child->hidden = 1;
        child->spriteLayer = 3;
        SAVE_POINT_MARKERS_INDEX(entity) = ENTITY_INDEX(child);
        ENTITY_PARENT_INDEX(child) = ENTITY_INDEX(entity);
        child->drawFunc = (u32 *)SavePoint_DrawPromptMarkers;
        child->updateFunc = (u32 *)EntityUpdateNothing;
        Sprite_Attach(child, sSaveWarpRoomSpriteSheet, entity->ext.savePoint.graphics, 0xE);
    }
}
