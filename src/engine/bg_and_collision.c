#include "bg_and_collision.h"
#include "gba.h"
#include "macros.h"
#include "syscalls.h"

#include "data/interrupt_table.h"
#include "data/debug_font.h"

#include "structs/agb_sram.h"
#include "structs/ewram.h"
#include "structs/main.h"
#include "data/data_0E5364.h"
#include "data/data_4F1444.h"

/**
 * @brief 1194 | Transfer data from BG Command Buffer to VRAM
 * 
 */
void BgCmdBuffer_TransferToVram(void)
{
    u16 bgCmd;
    u16 bgCnt;
    u16 tilesPerRow;
    u16 *vramAddr;
    u16 *buf;
    u16 bgNum;
    u16 length;
    u16 *dst;
    u32 modBitmask;
    u32 nextEntry;
    s32 i;
    u16 tilesPerCol;
    s32 newDst;
    u16 *src;

    buf = gUnk_03002CB0.bgCmdBuffer;
    while (buf < gUnk_03002CB0.pBgCmdBuffer)
    {
        bgCmd = (*buf >> 8) & 0xFF;
        length = (u8)(*buf);
        bgNum = bgCmd & 3;
        bgCnt = gDisplayRegisters.bgCnt[bgNum];
        buf++;
        vramAddr = (VRAM_BASE + *buf);
        buf++;
        if (bgNum == 0 || bgNum == 1)
        {
            dst = vramAddr;
            src = buf;
            tilesPerRow = (bgCnt & (BGCNT_SIZE_512x256 << BGCNT_SCREEN_SIZE_SHIFT)) ? 64 : 32;
            tilesPerCol = (bgCnt & (BGCNT_SIZE_256x512 << BGCNT_SCREEN_SIZE_SHIFT)) ? 64 : 32;

            if (bgCmd & 0x80)
            {
                // Vertical
                modBitmask = tilesPerRow * (tilesPerCol - 1) * sizeof(u16);
                nextEntry = tilesPerRow * sizeof(u16);
            }
            else
            {
                // Horizontal
                modBitmask = (tilesPerRow * sizeof(u16)) - 1;
                nextEntry = sizeof(u16);
            }

            for (i = 0; i < length; i++)
            {
                *dst = *src;
                newDst = (((u32)dst & ~modBitmask)) | ((((u32)dst + nextEntry) & modBitmask)); // Next entry, wrap row/col
                dst = (u16*)newDst;
                src += 1;
            }
            buf = src;
        }
    }
    gUnk_03002CB0.pBgCmdBuffer = gUnk_03002CB0.bgCmdBuffer;
}

/**
 * @brief 125C | Add string of characters to BG Command Buffer
 * 
 * @param tileIndex Tile index into VRAM
 * @param rowOffset Row offset from base
 * @param tileInfo Low nybble is palette number, high nybble is tile index flag
 * @param string Pointer to string
 * @return u16* Pointer to next data
 */
u16* BgCmdBuffer_WriteString(u16 tileIndex, u16 rowOffset, u8 tileInfo, const u8 *string)
{
    u16 *end;
    u16 palNum;
    u16 *strDst;
    u8 length;
    u32 vramAddr;
    u8 *src;
    u16 *dst;
    u32 tile;
    u32 vramTileBase;

    palNum = tileInfo << 12;
    strDst = (u16*)gEwramData->sharedScratchBuffer;
    
    for (length = 0; string[length] != 0; length++)
    {
        tile = palNum;
        if (!(tileInfo & 0xF0))
        {
            tile |= string[length] + 0x300;
        }
        *strDst++ = tile;
    }

    vramTileBase = (u32)(VRAM_BASE + 0xE000) + (tileIndex * sizeof(u16));
    if (gDisplayRegisters.bgCnt[0] & (BGCNT_SIZE_512x256 << BGCNT_SCREEN_SIZE_SHIFT))
    {
        vramAddr = vramTileBase + (rowOffset << 7);
    }
    else
    {
        vramAddr = vramTileBase + (rowOffset << 6);
    }
    src = gEwramData->sharedScratchBuffer;
    dst = gUnk_03002CB0.pBgCmdBuffer;

    if ((u32) (&dst[length] + 2) >= (u32) &gUnk_03002CB0.bgCmdBuffer[sizeof(gUnk_03002CB0.bgCmdBuffer)/sizeof(u16)])
    {
        end = NULL;
    }
    else 
    {
        *dst++ = length;
        *dst++ = vramAddr;
        DMA_SET(3, src, dst, C_32_2_16(DMA_ENABLE, length));
        dst += length;
        gUnk_03002CB0.pBgCmdBuffer = dst;
        end = dst;
    }
    return end;
}

/**
 * @brief 1350 | Add signed integer to BG Command Buffer
 * 
 * @param tileIndex Tile index into VRAM
 * @param rowOffset Row offset from base
 * @param palNum Palette number
 * @param numChars Number of characters
 * @param value Signed integer to write
 * @return u16* Pointer to next data
 */
u16* BgCmdBuffer_WriteNumber(u16 tileIndex, u16 rowOffset, u8 palNum, s32 numChars, s32 value)
{
    s32 isNegative;
    u8 length;
    u16 *valDst;
    s32 digit;
    s32 charPos;
    u16 *end;
    u16 baseTile;
    u32 vramAddr;
    u8 *src;
    u16 *dst;
    u32 vramTileBase;

    isNegative = FALSE;
    if (value < 0)
    {
        isNegative = TRUE;
        value = -value;
    }

    baseTile = (palNum << 0xC) | 0x300;
    valDst = (u16*)&gEwramData->linkPartnerWeapon + numChars;
    charPos = 0;

    // Write digits
    do
    {
        digit = Mod(value, 10);
        value = Div(value, 10);
        *valDst = (digit + 0x30) | baseTile;
        valDst -= 1;
        charPos += 1;
    } while (charPos < numChars && value != 0);

    // Write negative sign if negative
    if (charPos < numChars && isNegative)
    {
        *valDst = baseTile | 0x2D;
        valDst -= 1;
        charPos += 1;
    }

    // Write spaces for remaining places
    while (charPos < numChars)
    {
        *valDst = 0x20; // spaces
        charPos += 1;
        valDst -= 1;
    }

    vramTileBase = (u32)(VRAM_BASE + 0xE000) + (tileIndex * sizeof(u16));
    if (gDisplayRegisters.bgCnt[0] & (BGCNT_SIZE_512x256 << BGCNT_SCREEN_SIZE_SHIFT))
    {
        vramAddr = vramTileBase + (rowOffset << 7);
    }
    else
    {
        vramAddr = vramTileBase + (rowOffset << 6);
    }
    length = numChars;
    src = gEwramData->sharedScratchBuffer;
    dst = gUnk_03002CB0.pBgCmdBuffer;

    if ((u32) (&dst[length] + 2) >= (u32) &gUnk_03002CB0.bgCmdBuffer[sizeof(gUnk_03002CB0.bgCmdBuffer)/sizeof(u16)])
    {
        end = NULL;
    }
    else
    {
        *dst++ = length;
        *dst++ = vramAddr;
        DMA_SET(3, src, dst, C_32_2_16(DMA_ENABLE, length));
        dst += length;
        gUnk_03002CB0.pBgCmdBuffer = dst;
        end = dst;
    }
    return end;
}

/**
 * @brief 148C | (Duplicate of BgCmdBuffer_WriteString) Add string of characters to BG Command Buffer
 * 
 * @param tileIndex Tile index into VRAM
 * @param rowOffset Row offset from base
 * @param tileInfo Low nybble is palette number, high nybble is tile index flag
 * @param string Pointer to string
 * @return u16* Pointer to next data
 */
u16* BgCmdBuffer_WriteString_Duplicate(u16 tileIndex, u16 rowOffset, u8 tileInfo, u8 *string)
{
    u16 *end;
    u16 palNum;
    u16 *strDst;
    u8 length;
    u32 vramAddr;
    u8 *src;
    u16 *dst;
    u32 tile;
    u32 vramTileBase;

    palNum = tileInfo << 12;
    strDst = (u16*)gEwramData->sharedScratchBuffer;
    
    for (length = 0; string[length] != 0; length++)
    {
        tile = palNum;
        if (!(tileInfo & 0xF0))
        {
            tile |= string[length] + 0x300;
        }
        *strDst++ = tile;
    }

    vramTileBase = (u32)(VRAM_BASE + 0xE000) + (tileIndex * sizeof(u16));
    if (gDisplayRegisters.bgCnt[0] & (BGCNT_SIZE_512x256 << BGCNT_SCREEN_SIZE_SHIFT))
    {
        vramAddr = vramTileBase + (rowOffset << 7);
    }
    else
    {
        vramAddr = vramTileBase + (rowOffset << 6);
    }
    src = gEwramData->sharedScratchBuffer;
    dst = gUnk_03002CB0.pBgCmdBuffer;

    if ((u32) (&dst[length] + 2) >= (u32) &gUnk_03002CB0.bgCmdBuffer[sizeof(gUnk_03002CB0.bgCmdBuffer)/sizeof(u16)])
    {
        end = NULL;
    }
    else 
    {
        *dst++ = length;
        *dst++ = vramAddr;
        DMA_SET(3, src, dst, C_32_2_16(DMA_ENABLE, length));
        dst += length;
        gUnk_03002CB0.pBgCmdBuffer = dst;
        end = dst;
    }
    return end;
}

/**
 * @brief 1580 | Write data to BG Command Buffer
 * 
 * @param bgCmd Low nybble is background number, high nybble is vertical flag
 * @param length Length of data in bytes
 * @param src Pointer to source data
 * @param vramAddr VRAM address that will be written to
 * @return u16* Pointer to next data
 */
u16* BgCmdBuffer_WriteData(u8 bgCmd, u8 length, u8* src, u32 vramAddr)
{
    u16 *dst;
    u16 *end;

    dst = gUnk_03002CB0.pBgCmdBuffer;
    if ((u32) (&dst[length] + 2) < (u32) &gUnk_03002CB0.bgCmdBuffer[sizeof(gUnk_03002CB0.bgCmdBuffer)/sizeof(u16)])
    {
        *dst++ = (bgCmd << 8) | length;
        *dst++ = vramAddr;
        DMA_SET(3, src, dst, C_32_2_16(DMA_ENABLE, length));
        dst += length;
        gUnk_03002CB0.pBgCmdBuffer = dst;
        end = dst;
    }
    else
    {
        end = NULL;
    }
    return end;
}

/**
 * @brief 15E4 | Perform all DMA requests in queue
 * 
 */
void DmaQueue_Process(void)
{
    struct DmaQueueInfo *dmaQueueInfo;
    u8 *nextDmaEntry;

    dmaQueueInfo = gUnk_03002CB0.dmaQueue;

    while (dmaQueueInfo < gUnk_03002CB0.dmaQueueEnd)
    {
        if (dmaQueueInfo->directSize != 0)
        {
            // Direct transfer (data is embedded)
            DMA_COPY_16(3, &dmaQueueInfo->dataOrSize.directData, dmaQueueInfo->dst, dmaQueueInfo->directSize);
            nextDmaEntry = (u8*)dmaQueueInfo + dmaQueueInfo->directSize + 0xC;
        }
        else
        {
            // Indirect transfer (data comes from pointer)
            DMA_COPY_16(3, dmaQueueInfo->src, dmaQueueInfo->dst, dmaQueueInfo->dataOrSize.indirectSize);
            nextDmaEntry = (u8*)dmaQueueInfo + 0x10;
        }
        dmaQueueInfo = (struct DmaQueueInfo *)((s32)(nextDmaEntry + 3) & ~3);
    }

    gUnk_03002CB0.dmaQueueEnd = &gUnk_03002CB0.dmaQueue[0];
}

/**
 * @brief 1668 | Add direct DMA to queue (data is copied into buffer)
 * 
 * @param size Size in bytes
 * @param src Source
 * @param dst Destination
 * @return struct DmaQueueInfo* Pointer to end of DMA queue
 */
struct DmaQueueInfo* DmaQueue_DirectCopy(u16 size, u32 *src, u32 *dst)
{
    struct DmaQueueInfo *dmaQueueInfo;

    dmaQueueInfo = gUnk_03002CB0.dmaQueueEnd;

    // check if enough room in queue and size is nonzero
    if (((s32) ((u8*)dmaQueueInfo + (size + 0xC)) >= (s32) (&gUnk_03002CB0.dmaQueue[0x80])) || (size == 0))
    {
        return NULL;
    }

    dmaQueueInfo->src = (u32*)&dmaQueueInfo->dataOrSize.directData;
    dmaQueueInfo->dst = dst;
    dmaQueueInfo->directSize = size;
    DMA_COPY_16(3, src, dmaQueueInfo->src, size);

    gUnk_03002CB0.dmaQueueEnd = (struct DmaQueueInfo*)((u32)((u8*)dmaQueueInfo + size + 0xC + 3) & ~3);
    return gUnk_03002CB0.dmaQueueEnd;
}

/**
 * @brief 16D0 | Add indirect DMA to queue (data is pointed to)
 * 
 * @param size Size in bytes
 * @param src Source
 * @param dst Destination
 * @return struct DmaQueueInfo* Pointer to end of DMA queue
 */
struct DmaQueueInfo* DmaQueue_IndirectCopy(u32 size, u32 *src, u32 *dst)
{
    struct DmaQueueInfo *dmaQueueInfo;

    dmaQueueInfo = gUnk_03002CB0.dmaQueueEnd;

    // check if enough room in queue
    if ((u32) ((u8*)dmaQueueInfo + 0x10) >= (u32) (&gUnk_03002CB0.dmaQueue[0x80]))
    {
        return NULL;
    }

    dmaQueueInfo->src = src;
    dmaQueueInfo->dst = dst;
    dmaQueueInfo->directSize = 0;
    dmaQueueInfo->dataOrSize.indirectSize = size;

    gUnk_03002CB0.dmaQueueEnd = (struct DmaQueueInfo*)((u32)((u8*)dmaQueueInfo + 0x10 + 3) & ~3);
    return gUnk_03002CB0.dmaQueueEnd;
}

/**
 * @brief 1718 | Setup H-Blank effect
 * 
 * @param numEntries Number of entries to allocate for buffer
 * @param vcountSetting Target V-Counter value
 * @param writeSize Amount of bytes to write to
 * @param destReg The register to write to
 */
void HBlankEffectSetup(u8 numEntries, u8 vcountSetting, u8 writeSize, void *destReg)
{
    struct HBlankEffect *hBlankEffect;

    hBlankEffect = &gEwramData->hBlankEffect;

    hBlankEffect->vcountSetting = vcountSetting;
    hBlankEffect->writeSize = writeSize;
    hBlankEffect->destReg = destReg;

    if (numEntries != 0)
    {
        DMA_FILL_16(3, 0, &hBlankEffect->hBlankBuffer[0], (numEntries * writeSize));
        DMA_FILL_16(3, 0, &hBlankEffect->hBlankBuffer[1], (numEntries * writeSize));
    }
}

/**
 * @brief 1780 | Check if incoming room is a transition room
 * 
 * @param xOffset X Offset
 * @param yOffset Y offset
 * @return bool, room is a transition room
 */
s32 CheckRoomTransition(s32 xOffset, s32 yOffset)
{
    u16 room;
    u16 area;
    u32 *roomPtr;
    u32 i;
    u32 isTransitionRoom;

    isTransitionRoom = FALSE;

    area = sMapCellRoomInfo[(gEwramData->gameState.roomMapXPos + (xOffset >> 8)) + ((gEwramData->gameState.roomMapYPos + (yOffset >> 8)) << 6)];
    area = (area >> 6) & 0xF;

    room = sMapCellRoomInfo[(gEwramData->gameState.roomMapXPos + (xOffset >> 8)) + ((gEwramData->gameState.roomMapYPos + (yOffset >> 8)) << 6)];
    room = room & 0x3F;

    roomPtr = GetRoomPointer(area, room);
    for (i = 0; sTransitionRooms[i] != 0; i++)
    {
        if (sTransitionRooms[i] == roomPtr)
        {
            isTransitionRoom = TRUE;
            break;
        }
    }
    return isTransitionRoom;
}

/**
 * @brief Return the 8x8 tile index inside the 4x4 block at map tile (x, y),
 * honouring the block flip bits (stored in blockFlipFlags); -1 for an empty
 * block.
 */
s32 BgMap_GetBlockTileIndex(struct BgMapMetadata *metadata, u16 tileX, u16 tileY)
{
    u16 block;
    u16 flip;
    u8 mask;
    u8 subX;
    u8 blockX;
    u8 blockY;
    u8 subY;

    blockX = tileX >> 0x2;
    blockY = tileY >> 0x2;
    block = metadata->blockMap[blockX + ((blockY << 0x3) * metadata->widthCells)];
    flip = block & 0xC000;
    gUnk_03002CB0.blockFlipFlags = flip >> 0xC;

    block = block & 0x3FFF;
    if (block == 0)
    {
        return -1;
    }

    block -= 1;
    mask = 3; // TODO: static inline also works

    subX = tileX & mask;
    if (flip & 0x4000)
    {
        subX = 3 - subX;
    }

    subY = tileY & mask;
    if (flip & 0x8000)
    {
        subY = 3 - subY;
    }

    return (block << 4) + subX + (subY << 2);
}

/**
 * @brief 1894 | Get the area index from the current room on the map
 * 
 * @param xOffset X offset
 * @param yOffset Y offset
 * @return s32 Area index
 */
s32 GetAreaFromMapPosition(s32 xOffset, s32 yOffset)
{
    u16 tmp;

    tmp = sMapCellRoomInfo[gEwramData->gameState.roomMapXPos + (xOffset >> 8) + ((gEwramData->gameState.roomMapYPos + (yOffset >> 8)) << 6)];
    return (tmp >> 6) & 0xF;
}

/**
 * @brief 18D0 | Get the room index from the current room on the map
 * 
 * @param xOffset X offset
 * @param yOffset Y offset
 * @return s32 Room index
 */
s32 GetRoomFromMapPosition(s32 xOffset, s32 yOffset)
{
    u16 tmp;

    tmp = sMapCellRoomInfo[gEwramData->gameState.roomMapXPos + (xOffset >> 8) + ((gEwramData->gameState.roomMapYPos + (yOffset >> 8)) << 6)];
    return tmp & 0x3F;
}

/**
 * @brief 190C | Get the save room flag from the current room on the map
 * 
 * @param xOffset X offset
 * @param yOffset Y offset
 * @return s32 Is save room flag
 */
s32 GetSaveRoomFlagFromMapPosition(s32 xOffset, s32 yOffset)
{
    u16 tmp;

    tmp = sMapCellRoomInfo[gEwramData->gameState.roomMapXPos + (xOffset >> 8) + ((gEwramData->gameState.roomMapYPos + (yOffset >> 8)) << 6)];
    return tmp >> 0xF;
}

/**
 * @brief 1944 | Get the warp room flag from the current room on the map
 * 
 * @param xOffset X offset
 * @param yOffset Y offset
 * @return s32 Is warp room flag
 */
s32 GetWarpRoomFlagFromMapPosition(s32 xOffset, s32 yOffset)
{
    u16 tmp;

    tmp = sMapCellRoomInfo[gEwramData->gameState.roomMapXPos + (xOffset >> 8) + ((gEwramData->gameState.roomMapYPos + (yOffset >> 8)) << 6)];
    return (tmp >> 0xE) & 1;
}

/**
 * @brief 1980 | Get the pointer to the room
 * 
 * @param area Area
 * @param room Room
 * @return u32* Pointer to room
 */
u32* GetRoomPointer(s32 area, s32 room)
{
    return sAreaRoomTables[area][room];
}

/**
 * @brief Resolve tile coordinates through the map index helper, select raw or cached tile data and XOR the room flip bits; return zero for an invalid map index.
 */
u32 BgMap_GetTileEntry(struct BgMapMetadata *metadata, u16 tileX, u16 tileY)
{
    s32 tileIndex;
    u16 *tiles;
    u16 *entry;

    tileIndex = BgMap_GetBlockTileIndex(metadata, tileX, tileY);
    if (tileIndex < 0)
    {
        return 0;
    }

    if (!(metadata->formatFlags & 2))
    {
        tiles = metadata->tileData;
    }
    else if (metadata == gEwramData->bgInfo[1].pBgMetadata)
    {
        tiles = gEwramData->bg1TileCache;
    }
    else
    {
        tiles = gEwramData->sharedBgTileCache;
    }
    // entry = (void*)(tileIndex*2 + (u32)tiles); // alternative
    entry = tiles - - tileIndex; // - - instead of + to match
    return (gUnk_03002CB0.blockFlipFlags << 8) ^ *entry;
}

/**
 * @brief 1A00 | Collision flags of the BG1 tile at a room position
 *
 * Flags: bit 1 solid, bit 3 (with bit 0) platform-like, bits 6-7 slope
 * steepness, bit 2 slope mirrored, bits 4-5 slope base; 0xFF = solid 16x16.
 * Slopes are mirrored by the room flip flags (gUnk_03002CB0.blockFlipFlags).
 * Collision_SetBlock cells make empty or platform tiles solid (3).
 *
 * @param x Room x in pixels
 * @param y Room y in pixels
 * @return u8 Tile collision flags
 */
u8 Collision_GetTile(s32 x, s32 y)
{
    s32 limit;
    s32 flag;
    s32 tileX;
    s32 tileY;
    struct BgMapMetadata *bg;
    u8 *map;
    u8 tile;

    bg = gEwramData->bgInfo[1].pBgMetadata;
    if (bg->collisionData == NULL)
    {
        tile = 0;
    }
    else
    {
        tileX = x >> 3;
        if (bg->widthCells != 1)
        {
            limit = bg->widthCells << 5;
        }
        else
        {
            limit = 0x1E;
        }

        if (tileX < 0)
        {
            tileX = 0;
        }
        else if (tileX >= limit)
        {
            tileX = limit - 1;
        }

        tileY = y >> 3;
        if (bg->heightCells != 1)
        {
            limit = bg->heightCells << 5;
        }
        else
        {
            limit = 0x1A;
        }

        if (tileY < 0)
        {
            tileY = 0;
        }
        else if (tileY >= limit)
        {
            tileY = limit - 1;
        }

        if (bg->formatFlags & 2)
        {
            map = gEwramData->bg1CollisionCache;
        }
        else
        {
            map = bg->collisionData;
        }

        tile = map[BgMap_GetBlockTileIndex(bg, tileX, tileY)];
        if (tile & 0xC0)
        {
            if (tile & 2)
            {
                flag = 1;
            }
            else
            {
                flag = 1;
            }
        }
        else
        {
            flag = 0;
        }

        if (flag != 0)
        {
            if (gUnk_03002CB0.blockFlipFlags & 4)
            {
                tile ^= 4;
            }
            if (gUnk_03002CB0.blockFlipFlags & 3)
            {
                tile ^= 2;
            }
        }
    }

    if (gEwramData->collisionBlocksActive)
    {
        if (tile == 0)
        {
            flag = 1;
        }
        else
        {
            if (tile & 8)
            {
                if (tile & 1)
                {
                    flag = 1;
                }
                else
                {
                    flag = 1;
                }
            }

            else
            {
                flag = 0;
            }
        }

        if (flag != 0)
        {
            tileX = tileX >> 1;
            tileY = tileY >> 1;
            if (gEwramData->collisionBlocks[((tileY << 2) + ((tileX >> 5) << 9)) >> 2] & (1 << (tileX & 0x1F)))
            {
                tile = 3;
            }
        }
    }

    return tile;
}

/**
 * @brief 1B40 | Height offset of a slope tile at a pixel column
 *
 * @param tile Collision flags
 * @param x Pixel x (only the position inside the tile matters)
 * @return s16 Offset in pixels
 */
s16 Collision_GetSlopeHeight(u8 tile, s16 x)
{
    s32 isSlope;
    s16 height;
    u32 tmp;

    height = (tile & 0x30) >> 3;

    if (tile & 0xC0)
    {
        if (tile & 2)
        {
            isSlope = 1;
        }
        else
        {
            isSlope = 1;
        }
    }
    else
    {
        isSlope = 0;
    }

    if (isSlope != 0)
    {
        // tmp = (tile >> 6) - 1;
        // unusedBase = height;
        // if (tile & 4)
        // {
        //     unusedX = 7 - x;
        // }
        // else
        // {
        //     unusedX = x;
        // }
        // height = unusedBase + ((unusedX & 7) >> tmp);
        height = height + ((((tile & 4) ? (7 - x) : x) & 7) >> ((tile >> 6) - 1));
    }
    return height;
}

/**
 * @brief 1BA0 | Adjust tile flags for a collision mode (platform handling)
 *
 * @param tile Collision flags
 * @param mode 1 or 2 (other values keep the flags)
 * @return u8 Adjusted flags
 */
u8 Collision_FilterTile(u8 tile, s16 mode)
{
    s32 kind;

    switch (mode)
    {
        case 1:
            if (tile & 8)
            {
                if (tile & mode)
                {
                    kind = 2;
                }
                else
                {
                    kind = 1;
                }
            }
            else
            {
                kind = 0;
            }

            switch (kind)
            {
                case 1:
                    tile = (tile & ~8) | 1;
                    break;

                case 2:
                    tile = 9;
                    break;
            }
            break;

        case 2:
            if (tile & 8)
            {
                if (tile & 1)
                {
                    kind = 2;
                }
                else
                {
                    kind = 1;
                }
            }
            else
            {
                kind = 0;
            }

            switch (kind)
            {
                case 1:
                    tile = (tile & ~8) | 1;
                    break;

                case 2:
                    tile = tile ^ 10;
                    break;

                case 0:
                    tile = 0;
                    break;
            }
            break;
    }
    return tile;
}

/**
 * @brief 1C1C | Pixels to move down until the point leaves solid tiles (max ~8)
 *
 * Also stores the first tile's flags in gUnk_03002CB0.collisionProbeFlags.
 *
 * @param x Room x
 * @param y Room y
 * @return s16 Distance (0 when not inside a solid tile)
 */
s16 Collision_GetPushDown(s32 x, s32 y)
{
    s16 step;
    s16 push;
    u32 slope;
    u8 tile;

    for (push = 0; push < 9;)
    {
        tile = Collision_GetTile(x, y);

        if (push == 0)
        {
            gUnk_03002CB0.collisionProbeFlags = tile;
        }

        if ((tile & 2) == 0)
            break;

        if (tile == 0xFF)
        {
            step = 0x10 - (y & 0xF);
            push += step;
            y += step;
            continue;
        }

        if (tile & 0xC0)
        {
            if (tile & 2)
            {
                slope = 2;
            }
            else
            {
                slope = 1;
            }
        }
        else
        {
            slope = 0;
        }

        if (slope == 2)
        {
            push += 1 - (y & 7) + Collision_GetSlopeHeight(tile, x);

            if (push < 0)
            {
                push = 0;
            }
            break;
        }

        step = 8 - (y & 7);
        push += step;
        y += step;
    }
    return push;
}

/**
 * @brief 1CCC | Collision_GetPushDown with Collision_FilterTile(mode)
 *
 * @return s16 Distance
 */
s16 Collision_GetPushDownFiltered(s32 x, s32 y, s32 mode)
{
    s16 step;
    s16 push;
    u32 slope;
    u8 tile;

    for (push = 0; push < 9;)
    {
        tile = Collision_GetTile(x, y);

        if (push == 0)
        {
            gUnk_03002CB0.collisionProbeFlags = tile;
        }

        tile = Collision_FilterTile(tile, mode);
        if ((tile & 2) == 0)
            break;

        if (tile == 0xFF)
        {
            step = 0x10 - (y & 0xF);
            push += step;
            y += step;
            continue;
        }

        if (tile & 0xC0)
        {
            if (tile & 2)
            {
                slope = 2;
            }
            else
            {
                slope = 1;
            }
        }
        else
        {
            slope = 0;
        }

        if (slope == 2)
        {
            push += 1 - (y & 7) + Collision_GetSlopeHeight(tile, x);

            if (push < 0)
            {
                push = 0;
            }
            break;
        }

        step = 8 - (y & 7);
        push += step;
        y += step;
    }
    return push;
}

/**
 * @brief 1D94 | Pixels to move up (negative) until the point leaves solid tiles
 *
 * @param x Room x
 * @param y Room y
 * @return s16 Distance (<= 0)
 */
s16 Collision_GetPushUp(s32 x, s32 y)
{
    s16 step;
    s16 push;
    u32 slope;
    u8 tile;

    for (push = 0; push >= -8;)
    {
        tile = Collision_GetTile(x, y);

        if (push == 0)
        {
            gUnk_03002CB0.collisionProbeFlags = tile;
        }

        if ((tile & 1) == 0)
            break;

        if (tile == 0xFF)
        {
            step = -(y & 0xF) - 1;
            push += step;
            y += step;
            continue;
        }

        if (tile & 0xC0)
        {
            if (tile & 2)
            {
                slope = 2;
            }
            else
            {
                slope = 1;
            }
        }
        else
        {
            slope = 0;
        }

        if (slope == 1)
        {
            push += -(y & 7) - 1 + Collision_GetSlopeHeight(tile, x);

            if (push > 0)
            {
                push = 0;
            }
            break;
        }

        step = -(y & 7) - 1;
        push += step;
        y += step;
    }
    return push;
}

/**
 * @brief 1E58 | Collision_GetPushUp with Collision_FilterTile(mode)
 *
 * @return s16 Distance (<= 0)
 */
s16 Collision_GetPushUpFiltered(s32 x, s32 y, s32 mode)
{
    s16 step;
    s16 push;
    u32 slope;
    u8 tile;

    for (push = 0; push >= -8;)
    {
        tile = Collision_GetTile(x, y);

        if (push == 0)
        {
            gUnk_03002CB0.collisionProbeFlags = tile;
        }

        tile = Collision_FilterTile(tile, mode);
        if ((tile & 1) == 0)
            break;

        if (tile == 0xFF)
        {
            step = ~(y & 0xF);
            push += step;
            y += step;
            continue;
        }

        if (tile & 0xC0)
        {
            if (tile & 2)
            {
                slope = 2;
            }
            else
            {
                slope = 1;
            }
        }
        else
        {
            slope = 0;
        }

        if (slope == 1)
        {
            push += -(y & 7) - 1 + Collision_GetSlopeHeight(tile, x);

            if (push > 0)
            {
                push = 0;
            }
            break;
        }

        step = -(y & 7) - 1;
        push += step;
        y += step;
    }
    return push;
}

/**
 * @brief Return 1 when the room point is inside solid collision, taking the
 * slope height of slope tiles into account.
 */
s32 Collision_IsPointSolid(s32 x, s32 y)
{
    s16 solid;
    u32 kind;
    u8 tile;
    s16 height;
    s32 diff;
    u32 slope;
    u32 platform;

    solid = 0;

    tile = Collision_GetTile(x, y);

    if (tile & 0xC0)
    {
        if (tile & 2)
        {
            kind = 2;
        }
        else
        {
            kind = 1;
        }
    }
    else
    {
        kind = 0;
    }
    slope = kind;

    if (tile & 8)
    {
        if (tile & 1)
        {
            kind = 2;
        }
        else
        {
            kind = 1;
        }
    }
    else
    {
        kind = 0;
    }
    platform = kind;

    if (slope != 0 && platform != 0)
    {
        height = Collision_GetSlopeHeight(tile, x);
        tile = y & 7;
        if (slope == 1)
        {
            height++,height--; // Fake
            diff = height - tile;
        }
        else
        {
            diff = tile - height;
        }
        height = diff;

        switch (platform)
        {
            case 1:
                if (height <= 0)
                {
                    solid = 1;
                }
                break;

            case 2:
                if (height >= 0)
                {
                    solid = 1;
                }
                break;
        }
    }

    else if (platform != 0)
    {
        solid = 1;
    }

    return solid;
}

/**
 * @brief 1FE8 | Collision_GetTile at an offset from an entity (screen to room)
 *
 * @param entity Entity
 * @param dx X offset in pixels
 * @param dy Y offset in pixels
 * @return u8 Tile collision flags
 */
u8 Collision_GetTileAtEntity(struct EwramData_EntityData *entity, s32 dx, s32 dy)
{
    struct BgLayerState *bg1Info;

    bg1Info = &gEwramData->bgInfo[1];
    return Collision_GetTile((s16)bg1Info->xPos.part16.integer + (s16)entity->posX.part16.integer + dx, (s16)bg1Info->yPos.part16.integer + entity->posY.part16.integer + dy);
}

/**
 * @brief 2028 | Collision_GetTile at a screen position
 *
 * @return u8 Tile collision flags
 */
u8 Collision_GetTileOnScreen(s32 x, s32 y)
{
    struct BgLayerState *bg1Info;

    bg1Info = &gEwramData->bgInfo[1];
    return Collision_GetTile((s16)bg1Info->xPos.part16.integer + x, (s16)bg1Info->yPos.part16.integer + y);
}

/**
 * @brief 2058 | Pixels to move left (negative) out of a solid non-slope tile
 *
 * @return s16 Distance (<= 0)
 */
s16 Collision_GetPushLeft(s32 x, s32 y)
{
    return Collision_TileEdgeLeft(x, y, Collision_GetTile(x, y));
}

/**
 * @brief 207C | Pixels to move right out of a solid non-slope tile
 *
 * @return s16 Distance (>= 0)
 */
s16 Collision_GetPushRight(s32 x, s32 y)
{
    return Collision_TileEdgeRight(x, y, Collision_GetTile(x, y));
}

/**
 * @brief For a tile with bit 0 (top surface) at room position (x, y), return the
 * negative distance -(y mod 16) - 1 for a solid 16x16 block (0xFF) or
 * -(y mod 8) - 1 otherwise; 0 when the tile has no top surface.
 */
s16 Collision_GetGroundOffset(s32 x, s32 y)
{
    s16 offset;
    s32 tile;

    offset = 0;

    tile = Collision_GetTile(x, y);
    if (tile & 1)
    {
        if (tile == 0xFF)
        {
            offset = ~MOD_AND(y, 16);
        }
        else
        {
            offset = ~MOD_AND(y, 8);
        }
    }

    return offset;
}

/**
 * @brief 20D8 | Collision_GetPushLeft with Collision_FilterTile(mode)
 *
 * @return s16 Distance
 */
s16 Collision_GetPushLeftFiltered(s32 x, s32 y, s32 mode)
{
    return Collision_TileEdgeLeft(x, y, Collision_FilterTile(Collision_GetTile(x, y), mode));
}

/**
 * @brief 210C | Collision_GetPushRight with Collision_FilterTile(mode)
 *
 * @return s16 Distance
 */
s16 Collision_GetPushRightFiltered(s32 x, s32 y, s32 mode)
{
    return Collision_TileEdgeRight(x, y, Collision_FilterTile(Collision_GetTile(x, y), mode));
}

/**
 * @brief Collision_GetGroundOffset with the tile flags passed through
 * Collision_FilterTile(mode).
 */
s16 Collision_GetGroundOffsetFiltered(s32 x, s32 y, s32 mode)
{
    s16 offset;
    s32 tile;

    offset = 0;

    tile = Collision_FilterTile( Collision_GetTile(x, y), mode);
    if (tile & 1)
    {
        if (tile == 0xFF)
        {
            offset = ~MOD_AND(y, 16);
        }
        else
        {
            offset = ~MOD_AND(y, 8);
        }
    }

    return offset;
}

/**
 * @brief 2188 | Calculate the integer X position of the entity in the room
 * 
 * @param entity Entity
 * @return s16 Integer X Position of entity in room
 */
s16 GetEntityRoomXPositionInteger(struct EwramData_EntityData *entity)
{
    return gEwramData->bgInfo[1].xPos.part16.integer + entity->posX.part16.integer;
}

/**
 * @brief 21A8 | Calculate the integer Y position of the entity in the room
 * 
 * @param entity Entity
 * @return s16 Integer Y Position of entity in room
 */
s16 GetEntityRoomYPositionInteger(struct EwramData_EntityData *entity)
{
    return gEwramData->bgInfo[1].yPos.part16.integer + entity->posY.part16.integer;
}

/**
 * @brief 21C8 | Calculate the whole X position of the entity in the room
 * 
 * @param entity Entity
 * @return u32 Whole X Position of entity in room
 */
u32 GetEntityRoomXPositionWhole(struct EwramData_EntityData *entity)
{
    return gEwramData->bgInfo[1].xPos.whole + entity->posX.whole;
}

/**
 * @brief 21E4 | Calculate the whole Y position of the entity in the room
 * 
 * @param entity Entity
 * @return u32 Whole Y Position of entity in room
 */
u32 GetEntityRoomYPositionWhole(struct EwramData_EntityData *entity)
{
    return gEwramData->bgInfo[1].yPos.whole + entity->posY.whole;
}

/**
 * @brief 2200 | Make the 16x16 cell at a room position solid (collisionBlocks)
 *
 * @param x Room x
 * @param y Room y
 */
void Collision_SetBlock(s32 x, s32 y)
{
    x >>= 4;
    y >>= 4;

    // TODO: cleanup
    gEwramData->collisionBlocks[((y << 2) + ((x >> 5) << 9)) >> 2] |= 1 << (x & 0x1F);
    gEwramData->collisionBlocksActive = 1;
}

/**
 * @brief 2248 | Clear the solid 16x16 cell at a room position
 *
 * @param x Room x
 * @param y Room y
 */
void Collision_ClearBlock(s32 x, s32 y)
{
    x >>= 4;
    y >>= 4;

    // TODO: cleanup
    gEwramData->collisionBlocks[((y << 2) + ((x >> 5) << 9)) >> 2] &= ~(1 << (x & 0x1F));
}

/**
 * @brief 2278 | Clear all solid cells of collisionBlocks
 */
void Collision_ClearAllBlocks(void)
{
    DMA_FILL_32(3, 0, gEwramData->collisionBlocks, sizeof(gEwramData->collisionBlocks));
}

/**
 * @brief 22A8 | Distance from x to the right edge of a solid tile (8 or 16 px grid)
 *
 * @param x Room x
 * @param y Room y (unused)
 * @param tile Collision flags of the tile
 * @return s16 Distance, 0 for non-solid or slope tiles
 */
s16 Collision_TileEdgeRight(s32 x, s32 y, u8 tile)
{
    s16 push;

    push = 0;

    if (tile & 2)
    {
        if (tile == 0xFF)
        {
            push = 16 - MOD_AND(x, 16);
        }
        else if (!(tile & 0xC0))
        {
            push = 8 - MOD_AND(x, 8);
        }
    }

    return push;
}

/**
 * @brief 22E8 | Negative distance from x to the left edge of a solid tile
 *
 * @param x Room x
 * @param y Room y (unused)
 * @param tile Collision flags of the tile
 * @return s16 Distance (<= -1), 0 for non-solid or slope tiles
 */
s16 Collision_TileEdgeLeft(s32 x, s32 y, u8 tile)
{
    s16 push;

    push = 0;

    if (tile & 2)
    {
        if (tile == 0xFF)
        {
            push = ~MOD_AND(x, 16);
        }
        else if (!(tile & 0xC0))
        {
            push = ~MOD_AND(x, 8);
        }
    }

    return push;
}

/**
 * @brief Fill the H-blank buffer with WIN1 horizontal bounds of a circle at
 * (x, y) with radius r and set WIN1V.
 */
void Window1_SetCircle(s16 centerX, s16 centerY, s16 radius)
{
    s16 *hBlankBuf;
    s32 clamped;
    u16 top;
    u16 bottom;
    s32 scanline;
    u16 halfWidth;
    u16 left;
    u16 right;
    
    hBlankBuf = (s16*)&gEwramData->hBlankEffect.hBlankBuffer[1 - gEwramData->hBlankEffect.currentBuffer];

    clamped = centerY - radius;
    if (clamped < 0)
    {
        clamped = 0;
    }
    top = clamped;

    clamped = centerY + radius + 1;
    if (clamped > SCREEN_SIZE_Y)
    {
        clamped = SCREEN_SIZE_Y;
    }
    bottom = clamped;

    gDisplayRegisters.win1V = (top << 8) | bottom;

    for (scanline = top; scanline < bottom; scanline += 1)
    {
        halfWidth = Sqrt(((radius * radius) - ((centerY - scanline) * (centerY - scanline))) << 8) >> 4;
        clamped = centerX - halfWidth;
        if (clamped < 0)
        {
            clamped = 0;
        }
        left = clamped;

        clamped = centerX + halfWidth + 1;
        if (clamped > SCREEN_SIZE_X)
        {
            clamped = SCREEN_SIZE_X;
        }
        right = clamped;

        hBlankBuf[scanline] = (left << 8) | right;
    }

    gEwramData->hBlankEffect.requestStart = 1;
}

/**
 * @brief Enable window 1 and its per-line H-blank WIN1H updates.
 */
void Window1_EnableHBlank(void)
{
    gUnk_03002CB0.dispCnt |= DCNT_WIN1;
    HBlankEffectSetup(160, SCREEN_SIZE_Y - 1, 2, REG_WIN1H);
}

/**
 * @brief Disable window 1 and stop the H-blank effect.
 */
void Window1_Disable(void)
{
    gUnk_03002CB0.dispCnt &= ~DCNT_WIN1;
    gEwramData->hBlankEffect.requestStop = 1;
}
