#include "code_08011DD0.h"
#include "code_08009A0.h"
#include "code_08001004.h"
#include "code_08008750.h"
#include "code_0800CB00.h"
#include "code_0800F1FC.h"
#include "code/code_080211F0.h"
#include "code_08032444.h"
#include "code_08032E4C.h"
#include "code_08033CAC.h"
#include "code_08035930.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code_080D73B8.h"
#include "gba.h"
#include "macros.h"
#include "agb_sram.h"
#include "syscalls.h"

/*
 * Item icon entities (ItemIconEntity_Update, ItemIcon_AttachSprite /
 * _Free): item id, palette and the itemIconIds cache slot it uses.
 * Token-identical expansion.
 */
#define ITEM_ICON_ID(e) e->unk_4F8.part8.unk_4F8
#define ITEM_ICON_PALETTE(e) e->unk_4F8.part8.unk_4F9
#define ITEM_ICON_SLOT(e) e->unk_4F4.part8.unk_4F4

#include "constants/main.h"

#include "data/data_0E0334.h"
#include "data/data_0E3464.h"

#include "structs/agb_sram.h"
#include "structs/ewram.h"
#include "structs/main.h"

// Note: BgCmdBuffer_WriteString is implicitly called

/**
 * @brief Count set visited-cell bits across the 64 by 40 map grid.
 */
s32 Map_CountVisitedCells(struct GameState *gameState)
{
    // TODO: unused? looks like code in Map_GetCompletionTenths and Map_DrawCompletion, but probably can't be an inline
    s32 i;
    s32 j;
    s32 count;

    count = 0;

    for (i = 0; i < 40; i++)
    {
        for (j = 0; j < 0x40; j++)
        {
            if (gameState->mapCells[j >> 5][i].visited & (1 << (j & 0x1F)))
            {
                count += 1;
            }
        }
    }

    return count;
}

/**
 * @brief Return visited-cell count times 1000 divided by 0x350, representing tenths of a percent; preserve the unused parameter and absence of clamping.
 */
s32 Map_GetCompletionTenths(struct GameState *gameState, s32 param_1)
{
    s32 i;
    s32 j;
    s32 count;

    count = 0;

    for (i = 0; i < 40; i++)
    {
        for (j = 0; j < 0x40; j++)
        {
            if (gameState->mapCells[j >> 5][i].visited & (1 << (j & 0x1F)))
            {
                count += 1;
            }
        }
    }

    return Div(count * 1000, 0x350);
}

// TODO: is this an asset?
const u8 sMapCompletionText[0xE4] = {
	0x2e, 0x20, 0x25, 0x00, 0xf8, 0xf0, 0x00, 0x00, 0x00, 0x70, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00,
	0xf8, 0xf0, 0x00, 0x00, 0x10, 0x70, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0xf8, 0xf0, 0x00, 0x00,
	0x20, 0x70, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0xf8, 0xf0, 0x00, 0x00, 0x30, 0x70, 0x10, 0x10, 
	0x10, 0x00, 0x00, 0x00, 0xf8, 0xf0, 0x00, 0x00, 0x40, 0x70, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 
	0xf8, 0xf0, 0x00, 0x00, 0x50, 0x70, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0xf8, 0xf0, 0x00, 0x00, 
	0x60, 0x70, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0xf8, 0xf0, 0x00, 0x00, 0x70, 0x70, 0x10, 0x10, 
	0x10, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x10, 0x10, 0x0e, 0x08, 0x00, 0xf8, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x1c, 0x10, 0x0e, 0x08, 0x00, 0xf8, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x28, 0x10, 0x0e, 0x08, 0x00, 0xf8, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x34, 0x10, 0x0e, 0x08, 0x00, 0xf8, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x40, 0x10, 0x0e, 0x08, 0x00, 0xf8, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x4c, 0x10, 0x0e, 0x08, 0x00, 0xf8, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x58, 0x10, 0x0e, 0x08, 0x00, 0xf8, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x64, 0x10, 0x0e, 0x08, 
};

/**
 * @brief Draw map completion using visited count times 1000 divided by 0x350, with three integer digits and one fractional digit through the background command buffer.
 */
void Map_DrawCompletion(struct GameState *gameState, s32 param_1, s32 column, s32 rowOffset)
{
    s32 i;
    s32 j;
    s32 count;
    s32 tenths;

    count = 0;

    for (i = 0; i < 40; i++)
    {
        for (j = 0; j < 0x40; j++)
        {
            if (gameState->mapCells[j >> 5][i].visited & (1 << (j & 0x1F)))
            {
                count += 1;
            }
        }
    }

    tenths = Div(count * 1000, 0x350);
    BgCmdBuffer_WriteString(column + 3, rowOffset, 0, sMapCompletionText);
    BgCmdBuffer_WriteNumber(column, rowOffset, 0, 3, Div(tenths, 10));
    BgCmdBuffer_WriteNumber(column + 4, rowOffset, 0, 1, Mod(tenths, 10));
}

/**
 * @brief Store the destination room definition, camera position and player
 * position for the next room jump (roomJumpDestination..roomJumpPlayerY).
 */
void RoomJump_SetDestination(void *room, s32 cameraX, s32 cameraY, s32 playerX, s32 playerY)
{
    gEwramData->gameState.roomJumpDestination = (struct RoomDefinition *) room;
    gEwramData->gameState.roomJumpCameraX = cameraX;
    gEwramData->gameState.roomJumpCameraY = cameraY;
    gEwramData->gameState.roomJumpPlayerX = playerX;
    gEwramData->gameState.roomJumpPlayerY = playerY;
}

/**
 * @brief When priority is at least the current priority, store priority and signed X/Y components without changing the existing duration.
 */
void ScreenShake_RequestOffset(u8 priority, s8 param_1, s8 param_2)
{
    if (priority >= gEwramData->gameState.screenShakePriority)
    {
        gEwramData->gameState.screenShakePriority = priority;
        gEwramData->gameState.screenShakeComponentX = param_1;
        gEwramData->gameState.screenShakeComponentY = param_2;
    }
}

/**
 * @brief When priority is at least the current priority, store priority, duration and X/Y random masks.
 */
void ScreenShake_Request(u8 priority, u8 param_1, u8 param_2, u8 param_3)
{
    if (priority >= gEwramData->gameState.screenShakePriority)
    {
        gEwramData->gameState.screenShakePriority = priority;
        gEwramData->gameState.screenShakeTimer = param_1;
        gEwramData->gameState.screenShakeComponentX = param_2;
        gEwramData->gameState.screenShakeComponentY = param_3;
    }
}

/**
 * @brief Clear the four request-state bytes; existing output offsets are left for the update routine to replace.
 */
void ScreenShake_Clear(void)
{
    gEwramData->gameState.screenShakePriority = 0;
    gEwramData->gameState.screenShakeTimer = 0;
    gEwramData->gameState.screenShakeComponentX = 0;
    gEwramData->gameState.screenShakeComponentY = 0;
}

/**
 * @brief Set the indexed flag for nonnegative indices; retain original unchecked upper indexing.
 */
void GameFlag_Set(s32 flag)
{
    u32 *temp_r2;

    if (flag >= 0)
    {
        // TODO: is temp_r2 fake or inline?
        temp_r2 = &gEwramData->gameState.gameFlags[flag >> 5];
        *temp_r2 |= 1 << (flag & 0x1F);
    }
}

/**
 * @brief Return the raw indexed bit mask, or zero for a negative index; do not normalize a set high bit to one.
 */
s32 GameFlag_GetMask(s32 flag)
{
    u32 *word;

    if (flag < 0)
    {
        return 0;
    }
    else
    {
        word = &gEwramData->gameState.gameFlags[flag >> 5];
        return *word & (1 << (flag & 0x1F));
    }
}

// TODO: GameFlag_SetInline is the same as GameFlag_Set
static inline void GameFlag_SetInline(s32 flag)
{
    u32 *word;

    if (flag >= 0)
    {
        word = &gEwramData->gameState.gameFlags[flag >> 5];
        *word |= 1 << (flag & 0x1F);
    }
}

// TODO: GameFlag_TestAndSetInline is the same as GameFlag_GetMask
static inline s32 GameFlag_TestAndSetInline(s32 flag)
{
    u32 *word;

    if (flag < 0)
    {
        return 0;
    }
    else
    {
        word = &gEwramData->gameState.gameFlags[flag >> 5];
        return *word & (1 << (flag & 0x1F));
    }
}

/**
 * @brief Return the previous raw flag mask and set the flag when clear; a negative index returns zero without writing.
 */
s32 GameFlag_TestAndSet(s32 flag)
{
    s32 wasSet;
    u32 *temp_r2;

    wasSet = 0;
    wasSet = GameFlag_TestAndSetInline(flag);
    if (wasSet == 0)
    {
        GameFlag_SetInline(flag);
    }
    return wasSet;
}

extern s8 sGameFlagMapCoordinates[][2];

/**
 * @brief Search the signed-byte coordinate table for the supplied map X/Y pair; return its flag index or -1.
 */
s32 GameFlag_FindMapCoordinate(s32 x, s32 y)
{
    s32 i;

    for (i = 0; sGameFlagMapCoordinates[i][0] >= 0; i++)
    {
        if ((x == sGameFlagMapCoordinates[i][0]) && (y == sGameFlagMapCoordinates[i][1]))
        {
            return i;
        }
    }

    return -1;
}

/**
 * @brief Add room-map origin to room coordinates shifted by eight, apply the mode-dependent coordinate transform and return the matching flag index or -1.
 */
s32 GameFlag_FindRoomCoordinate(s32 x, s32 y, s32 side)
{
    s32 cellX;
    s32 cellY;
    s32 i;
    struct GameState *gameState;

    gameState = &gEwramData->gameState;
    cellX = gameState->roomMapXPos + (x >> 8);
    cellY = gameState->roomMapYPos + (y >> 8);

    switch (side)
    {
        case 2:
            cellX += 1;
            /* fallthrough */
        case 1:
            cellY = -cellY;
            break;
        case 4:
            cellY += 1;
            break;
    }

    for (i = 0; sGameFlagMapCoordinates[i][0] >= 0; i++)
    {
        if ((cellX == sGameFlagMapCoordinates[i][0]) && (cellY == sGameFlagMapCoordinates[i][1]))
        {
            return i;
        }
    }

    return -1;
}

/**
 * @brief Clear the indexed flag for nonnegative indices; retain original unchecked upper indexing.
 */
void GameFlag_Clear(s32 flag)
{
    u32 *word;

    if (flag >= 0)
    {
        word = &gEwramData->gameState.gameFlags[flag >> 5];
        *word &= ~(1 << (flag & 0x1F));
    }
}

/**
 * @brief Unless the uploaded bit (itemIconCacheFlags bit 1) is set, queue DMA copies of
 * both 0x200-byte cached icon tile rows to the icon object tiles and set it.
 */
void ItemIconCache_Upload(void)
{
    s32 tileOffset;
    u8 *tiles;

    if (!(gEwramData->itemIconCacheFlags & 2))
    {
        tiles = gEwramData->itemIconTiles;
        tileOffset = ObjTiles_GetSlotTile(gEwramData->itemIconGfxEntry) << 5;
        DmaQueue_IndirectCopy(0x200, (u32 *) tiles, (u32*)(VRAM_BASE + 0x13800 + tileOffset));
        DmaQueue_IndirectCopy(0x200, (u32 *) &tiles[0x200], (u32*)(VRAM_BASE + 0x13800 + 0x400 + tileOffset));
        gEwramData->itemIconCacheFlags |= 2;
    }
}

/**
 * @brief Find the icon object graphics slot, set itemIconCacheFlags to 1 and clear the
 * current and saved icon IDs and both icon tile buffers.
 */
void ItemIconCache_Init(void)
{
    s32 i;

    gEwramData->itemIconGfxEntry = ObjGfx_Find((u8 *)0x081C15F4);
    gEwramData->itemIconCacheFlags = 1;

    for (i = 0; i <= 7; i++)
    {
        gEwramData->itemIconIds[i] = 0;
        gEwramData->savedItemIconIds[i] = 0;
    }

    DMA_FILL_32(3, 0, &gEwramData->itemIconTiles, 0x400);
    DMA_FILL_32(3, 0, &gEwramData->savedItemIconTiles, 0x400);
}

void *sItemIconSheetData[] = {
    [0] = (void*)0x081C5E00,
    [1] = (void*)0x081C7E04,
    [2] = (void*)0x081C9E08
};

/**
 * @brief Load the 16x16 icon id into one of 8 cached icon slots (reusing a slot
 * that already holds it) and request the upload.
 *
 * @param id Icon id (1-based, 64 icons per sheet in sItemIconSheetData), 0 = none
 * @return Slot index, -1 if no slot is free or id is 0
 */
s32 ItemIconCache_Load(s32 id)
{
    s32 slot;
    s32 i;
    s32 sheet;
    void *src;
    void *dst;
    void *sheetData;
    s32 rowSize;

    rowSize = 0x200;
    slot = -1;
    if (id != 0)
    {
        for (i = 0; i < 8; i++)
        {
            if (gEwramData->itemIconIds[i] == id)
                return i;
            if (slot == -1 && gEwramData->itemIconIds[i] == 0)
                slot = i;
        }

        if (slot < 0)
            return -1;

        gEwramData->itemIconIds[slot] = (u8)id;
        sheet = (id - 1) / 64;
        sheetData = sItemIconSheetData[sheet];
        src = sheetData + (((((id - (sheet << 6)) - 1) >> 3) << 10) + 4) + ((((id - (sheet << 6)) - 1) & 7) << 6);
        dst = &gEwramData->itemIconTiles[slot << 6];
        DMA_COPY_32(3, src, dst, 0x40);
        DMA_COPY_32(3, src + rowSize, dst + 0x200, 0x40);
        ItemIconCache_Upload();
    }

    return slot;
}

// TODO: figure out struct
struct Unk_080E10F0 {
    u8 pad_0[0x10];
};

// TODO: is this asset?
const struct Unk_080E10F0 sItemIconSpriteSheet[] = {
    0x01, 0x00, 0x00, 0x00, 0x70, 0x10, 0x0E, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x01, 0x00, 0x00, 0x00, 0x80, 0x10, 0x0E, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x01, 0x00, 0x00, 0x00, 0x90, 0x10, 0x0E, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x01, 0x00, 0x00, 0x00, 0xA0, 0x10, 0x0E, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x01, 0x00, 0x00, 0x00, 0xB0, 0x10, 0x0E, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x01, 0x00, 0x00, 0x00, 0xC0, 0x10, 0x0E, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x01, 0x00, 0x00, 0x00, 0xD0, 0x10, 0x0E, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x01, 0x00, 0x00, 0x00, 0xE0, 0x10, 0x0E, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};

/**
 * @brief (Unused) Icon entity: load icon unk_4F8 into the cache, attach its
 * cache-slot sprite with palette unk_4F9 (default 6); state 3 releases the
 * slot and frees the sprite.
 */
void ItemIconEntity_Update(struct EwramData_EntityData *entity)
{
    u32 slot;
    s32 palette;

    switch (entity->state)
    {
        case 0:
            entity->drawFunc = (u32 *) Sprite_DrawEntity;
            entity->spriteLayer = 13;
            ITEM_ICON_PALETTE(entity) = 6;
            /* fallthrough */
        case 1:
            gEwramData->itemIconCacheFlags &= ~2;
            slot = ItemIconCache_Load(ITEM_ICON_ID(entity));
            palette = (s8)ITEM_ICON_PALETTE(entity);
            if (slot < 8)
            {
                ITEM_ICON_SLOT(entity) = slot;
                Sprite_Attach(entity, (u8*)&sItemIconSpriteSheet[slot], gEwramData->itemIconGfxEntry, palette);
                entity->animFrame = 0;
            }
            entity->scrollWithBg = 1;
            entity->state = 2;
            break;

        case 3:
            gEwramData->itemIconIds[(s8)ITEM_ICON_SLOT(entity)] = 0;
            Sprite_Free(entity, 0);
            /* fallthrough */
        case 2:
            break;
    }
}

/**
 * @brief Clear the current and saved cached icon IDs; tile buffers are kept.
 */
void ItemIconCache_ClearIds(void)
{
    DMA_FILL_16(3, 0, &gEwramData->itemIconIds, 8);
    DMA_FILL_16(3, 0, &gEwramData->savedItemIconIds, 8);
}

/**
 * @brief Attach the sprite of cache slot slot (0..7) with the given palette to
 * the entity and record the slot; return 0 for slots 8 and above.
 */
s32 ItemIcon_AttachSprite(struct EwramData_EntityData *entity, u32 slot, s32 palette)
{
    // TODO: inline for ItemIconEntity_Update?
    s32 result;

    if (slot >= 8)
        return 0;

    ITEM_ICON_SLOT(entity) = slot;
    result = Sprite_Attach(entity, (u8*)&sItemIconSpriteSheet[slot], gEwramData->itemIconGfxEntry, palette);
    entity->animFrame = 0;
    return result;
}

/**
 * @brief Free the icon entity's sprite, first releasing its cache slot when
 * release is nonzero.
 */
void ItemIcon_Free(struct EwramData_EntityData *entity, s32 release)
{
    // TODO: inline for ItemIconEntity_Update?
    if (release != 0)
    {
        gEwramData->itemIconIds[(s8)ITEM_ICON_SLOT(entity)] = 0;
    }
    Sprite_Free(entity, 0);
}

/**
 * @brief Clear the icon ID held by cache slot slot.
 */
void ItemIconCache_ReleaseSlot(s32 slot)
{
    // TODO: inline for ItemIconEntity_Update?
    gEwramData->itemIconIds[slot] = 0;
}

/**
 * @brief (Unused) Return the cache slot holding icon id, or -1; ids of 8 and
 * above return -1 before the search (original guard kept).
 */
s32 ItemIconCache_FindId(u32 id)
{
    // TODO: inline for ItemIconCache_Load or actually unused?
    s32 slot;

    if (id >= 8)
        return -1;

    for (slot = 0; slot < 8; slot++)
    {
        if (gEwramData->itemIconIds[slot] == id)
        {
            return slot;
        }
    }

    return -1;
}

/**
 * @brief Copy the cached icon IDs and tiles to the saved copy, then clear the
 * current IDs.
 */
void ItemIconCache_Save(void)
{
    DMA_COPY_16(3, &gEwramData->itemIconIds, &gEwramData->savedItemIconIds, 0x8);
    DMA_COPY_16(3, &gEwramData->itemIconTiles, &gEwramData->savedItemIconTiles, 0x400);
    DMA_FILL_16(3, 0, &gEwramData->itemIconIds, 0x8);
}

/**
 * @brief Restore the cached icon IDs and tiles from the saved copy and request
 * the upload.
 */
void ItemIconCache_Restore(void)
{
    DMA_COPY_16(3, &gEwramData->savedItemIconIds, &gEwramData->itemIconIds, 0x8);
    DMA_COPY_16(3, &gEwramData->savedItemIconTiles, &gEwramData->itemIconTiles, 0x400);
    ItemIconCache_Upload();
}

/**
 * @brief Clear the uploaded bit so the next ItemIconCache_Upload copies the tiles
 * again.
 */
void ItemIconCache_InvalidateUpload(void)
{
    gEwramData->itemIconCacheFlags &= ~2;
}

/**
 * @brief Store the current area, room, player position and camera (BG1
 * scroll, 0x30 higher for one-screen-high rooms) in the progress
 * block before saving.
 */
void SaveData_StorePlayerPosition(struct GameState *gameState)
{
    struct BgMapMetadata *metadata;
    s16 roomX;
    s16 roomY;
    struct BgLayerState *bg;

    metadata = gEwramData->bgInfo[1].pBgMetadata;

    roomX = GetEntityRoomXPositionInteger(gEwramData->playerState.player);
    roomY = GetEntityRoomYPositionInteger(gEwramData->playerState.player);
    gameState->currentArea = GetAreaFromMapPosition(roomX, roomY);
    gameState->currentRoom = GetRoomFromMapPosition(roomX, roomY);

    gameState->savedPlayerX = gEwramData->playerState.player->posX.part16.integer;
    gameState->savedPlayerY = gEwramData->playerState.player->posY.part16.integer;

    bg = &gEwramData->bgInfo[1];
    gameState->savedCameraX = bg->xPos.part16.integer;
    gameState->savedCameraY = bg->yPos.part16.integer;

    if (metadata->heightCells < 2)
    {
        gameState->savedCameraY -= 0x30;
    }
}
