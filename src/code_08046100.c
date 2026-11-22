#include "code_08009A0.h"
#include "code_08001194.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"


/**
 * @brief Advance animation and draw the source, optionally draw frame 0x31 six pixels above it under frame-counter bit one, then restore frame and position.
 */
void PickupSource_Draw(struct EwramData_EntityData *entity)
{
    Anim_Update(entity);
    Sprite_Draw(entity);
    if (gEwramData->frameCounter & 2)
    {
        u8 saved = entity->animFrame;
        entity->posY.part16.integer -= 6;
        entity->animFrame = 0x31;
        Sprite_Draw(entity);
        entity->animFrame = saved;
        entity->posY.part16.integer += 6;
    }
}

extern u8 sPickupIconTiles[];
extern u8 sCommonObjectGfx[];

/**
 * @brief Upload two icon rows to the shared OBJ graphics unless the icon index matches the cached value; retain the cache and index assumptions.
 */
void PickupSource_LoadIcon(s32 icon)
{
    struct GameState *data = &gEwramData->gameState;
    u8 *src;
    s32 tile;

    if (data->cachedPickupIcon == icon)
        return;
    data->cachedPickupIcon = icon;
    src = sPickupIconTiles;
    tile = ObjTiles_GetSlotTile((u8)ObjGfx_Find(sCommonObjectGfx)) << 5;
    src += (icon & 1) * 0x100 + (icon / 2) * 0x400;
    DmaQueue_IndirectCopy(0xC0, (u32 *)src, (u32 *)(tile + 0x06010800));
    src += 0x200;
    DmaQueue_IndirectCopy(0xC0, (u32 *)src, (u32 *)(tile + 0x06010C00));
}
