#include "code_080109F4.h"
#include "code_08009A0.h"
#include "code_08001004.h"
#include "code_08001194.h"
#include "code_08008750.h"
#include "code_0800CB00.h"
#include "code_0800F1FC.h"
#include "code_08011DD0.h"
#include "code_08013960.h"
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

#include "constants/main.h"
#include "constants/songs.h"

#include "data/data_0E0334.h"
#include "data/data_0E3464.h"

#include "structs/agb_sram.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern u16 sMapCellRoomInfo[];

static inline u16* MapScreen_GetCellTileAddress(s32 tile, s32 x, s32 y)
{
    return (u16*)0x06000000 + ((tile + 0x22) << 4) + ((y & 1) << 3) + (x & 1);
}

/**
 * @brief Draw the top edge of map cell (x, y) on the map screen: wall, door or
 * open, using the edge type nibble; flag-gated doors use the event flag
 * of the cell.
 */
void MapScreen_DrawCellTopEdge(s32 x, s32 y, s32 color, s32 border, s32 edge)
{
    s32 current;
    s32 pattern;
    u16 *pixel;
    s32 tile;

    if (sMapCellRoomInfo[(y << 6) + x] == 0xFFFF)
    {
        color = 0;
    }

    tile = (x >> 1) + ((y >> 1) << 5);
    edge = edge >> 2;
    if (edge == 1)
    {
        if (GameFlag_GetMask(GameFlag_FindMapCoordinate(x, y)) != 0)
        {
            edge = 2;
        }
        else
        {
            edge = 3;
        }
    }

    switch (edge)
    {
        case 0:
            pattern = (color << 4) | (color << 8) | (color << 12);
            break;

        case 2:
            pattern = 0xBBB0;
            break;

        case 3:
            pattern = (border << 4) | (border << 8) | (border << 12);
            break;
    }

    pixel = MapScreen_GetCellTileAddress(tile, x, y);
    current = *pixel & 0xF;
    if ((current == border) || (edge & 2))
    {
        *pixel = pattern | border;
    }
    else
    {
        *pixel = pattern | current;
    }

    if (edge & 2)
    {
        tile = ((x + 1) >> 1) + ((y >> 1) << 5);
        pixel = MapScreen_GetCellTileAddress(tile, x + 1, y);
        *pixel = (*pixel & 0xFFF0) | border;
    }
}

/**
 * @brief Draw the left edge (four pixel rows) of map cell (x, y) on the map
 * screen from its edge type nibble.
 */
void MapScreen_DrawCellLeftEdge(s32 x, s32 y, s32 color, s32 border, s32 edge)
{
    s32 current;
    s32 value;
    s32 value2;
    s32 i;
    u16 *pixel;
    s32 tile;
    s32 below;

    if (sMapCellRoomInfo[(y << 6) + x] == 0xFFFF)
    {
        color = 0;
    }

    edge = edge & 3;
    if (edge == 1)
    {
        if (GameFlag_GetMask(GameFlag_FindMapCoordinate(x, -y)) != 0)
        {
            edge = 2;
        }
        else
        {
            edge = 3;
        }
    }

    tile = (x >> 1) + ((y >> 1) << 5);
    pixel = MapScreen_GetCellTileAddress(tile, x, y);
    
    for (i = 0; i < 4; i++)
    {
        current = *pixel & 0xF;

        switch (i)
        {
            case 0:
                if (edge != 2)
                {
                    if (((edge & 2) || (current == 5)))
                    {
                        value = border;
                    }
                    else
                    {
                        value = color;
                    }
                }
                else
                {
                    value = 0xB;
                }
                break;

            case 2:
                if (edge == 2)
                {
                   value = 0xB;
                }
                else
                {
                    if (edge == 3)
                    {
                        value = border;
                    }
                    else
                    {
                        value = color;
                    }
                }
                break;

            default:
                if (edge == 2)
                {
                    value = 0xB;
                }
                else
                {
                    if (edge & 2)
                    {
                        value = border;
                    }
                    else
                    {
                        value = color;
                    }
                }
                break;
        }

        if (current != border)
        {
            *pixel = (*pixel & 0xFFF0) | value;
        }
        pixel += 2;
    }

    tile = (x >> 1) + (((y + 1) >> 1) << 5);
    pixel = MapScreen_GetCellTileAddress(tile, x, y + 1);

    below = *pixel & 0xF;
    if (below)
    {
        if (below == 5 || below == 1)
        {
            value2 = border;
        }
        else
        {
            value2 = color;
        }
    }
    else
    {
        if (edge & 2)
        {
            value2 = border;
        }
        else
        {
            value2 = color;
        }
    }
    *pixel = (*pixel & 0xFFF0) | value2;
}


extern u8 sMapCellEdges[];

/**
 * @brief Fill map cell (x, y) with colour and draw its four edges from
 * sMapCellEdges.
 */
void MapScreen_DrawCell(struct GameState *gameState, s32 x, s32 y, s32 color)
{
    u16 *pixels;
    s32 i;
    s32 edges;
    s32 border;
    s32 var_0;
    s32 tile;
    
    if (color == 4)
    {
        border = 5;
    }
    else
    {
        border = 1;
    }

    tile = (x >> 1) + ((y >> 1) << 5);
    pixels = MapScreen_GetCellTileAddress(tile, x, y);

    edges = color | (color << 4) | (color << 8) | (color << 12);
    for (i = 1; i < 4; i++)
    {
        pixels[i * 2] = edges;
    }

    edges = sMapCellEdges[(y << 5) + (x >> 1)];
    if (x & 1)
    {
        edges = edges & 0xF;
    }
    else
    {
        edges = edges >> 4;
    }
    MapScreen_DrawCellTopEdge(x, y, color, border, edges);
    MapScreen_DrawCellLeftEdge(x, y, color, border, edges);

    edges = sMapCellEdges[((y + 1) << 5) + (x >> 1)];
    if (x & 1)
    {
        edges = edges & 0xF;
    }
    else
    {
        edges = edges >> 4;
    }
    MapScreen_DrawCellTopEdge(x, y + 1, color, border, edges);

    edges = sMapCellEdges[((x + 1) >> 1) + (y << 5)];
    if (!(x & 1))
    {
        edges = edges & 0xF;
    }
    else
    {
        edges = edges >> 4;
    }
    MapScreen_DrawCellLeftEdge(x + 1, y, color, border, edges);
}

u8 sMapScreenExcludedCells[][2] = {
    [0] = {
        4, 13
    },
    [1] = {
        12, 13
    },
    [2] = {
        13, 13
    },
    [3] = {
        15, 22
    },
    [4] = {
        47, 20
    },
    [5] = {
        0, 0
    }
};

static inline struct EwramData_EntityData* Get_unk_13110()
{
    return gEwramData->playerState.player;
}

static inline s32 MapScreen_IsCellNotExcluded(struct GameState *gameState, s32 cellX, s32 cellY)
{
    s32 i;

    for (i = 0; sMapScreenExcludedCells[i][0] != 0; i += 1)
    {
        if ((cellX == sMapScreenExcludedCells[i][0]) && (cellY == sMapScreenExcludedCells[i][1]))
            return FALSE;
    }
    return TRUE;
}

/**
 * @brief Set up the map tilemap, print the completion and draw every visited
 * cell, plus the cells revealed by map items 0x1D/0x1E.
 */
void MapScreen_DrawMap(struct GameState *gameState)
{
    u16 *tilemap;
    s32 tile;
    s32 roomColor;
    s32 color;
    s32 j;
    s32 i;
    s32 cellType;
    s32 cellInfo;

    tilemap = (u16 *)0x0600E000;
    tile = 0;

    for (i = 0; i < 20; i++)
    {
        for (j = 0; j < 0x20; j++)
        {
            *tilemap = tile;
            tile++;
            tilemap++;
        }
    }

    DMA_FILL_32(3, 0, 0x06000000, 0x6000);
    Map_DrawCompletion(gameState, gameState->unk_8D_6, 0x18, 1);
    if (GetAreaFromMapPosition(GetEntityRoomXPositionInteger(Get_unk_13110()), GetEntityRoomYPositionInteger(Get_unk_13110())) == 11)
    {
        return;
    }

    if (gEwramData->gameState.bossRushMode)
    {
        return;
    }

    for (i = 0; i < 0x23; i++)
    {
        for (j = 0; j < 0x40; j++)
        {
            cellType = sMapCellRoomInfo[j + (i << 6)];
            if (cellType == 0xFFFF)
            {
                continue;
            }

            cellType = (cellType >> 6) & 0xF;
            if (gameState->mapCells[j >> 5][i].visited & (1 << (j & 0x1F)))
            {
                MapScreen_DrawCell(gameState, j, i, 4);
            }
            else
            {
                if ((gEwramData->playerStatus.itemInventory[0x1D] != 0) && (cellType == 0 || cellType == 1))
                {
                    if (MapScreen_IsCellNotExcluded(gameState, j, i))
                    {
                        MapScreen_DrawCell(gameState, j, i, 4);
                        continue;
                    }
                }

                if ((gEwramData->playerStatus.itemInventory[0x1E] != 0) && (cellType == 3))
                {
                    if (MapScreen_IsCellNotExcluded(gameState, j, i))
                    {
                        MapScreen_DrawCell(gameState, j, i, 4);
                        continue;
                    }
                }

                if ((gEwramData->playerStatus.itemInventory[0x1F] != 0) && (cellType == 5))
                {
                    if (MapScreen_IsCellNotExcluded(gameState, j, i))
                    {
                        MapScreen_DrawCell(gameState, j, i, 4);
                        continue;
                    }
                }
            }
        }
    }

    
    for (i = 0; i < 35; i++)
    {
        for (j = 0; j < 0x40; j++)
        {
            cellInfo = sMapCellRoomInfo[(i << 6) + j];
            if (cellInfo == 0xFFFF)
                continue;

            if (gameState->mapCells[j >> 5][i].visitedThisRun & (1 << (j & 0x1F)))
            {
                // Is save room
                if (cellInfo & 0x4000)
                {
                    roomColor = 3;
                }
                else
                {
                    roomColor = 9;
                }

                // Is warp room
                if (!(cellInfo & 0x8000))
                {
                    color = roomColor;
                }
                else
                {
                    color = 2;
                }
                MapScreen_DrawCell(gameState, j, i, color);
            }
        }
    }

    gDisplayRegisters.bgCnt[0] = CREATE_BGCNT(0, 28, BGCNT_HIGH_PRIORITY, BGCNT_SIZE_256x256) | BGCNT_MOSAIC;
    gDisplayRegisters.bgOfs[0].hOfs = 2;
}


/**
 * @brief Draw the blinking circular window marker at the player cell when the
 * player is in the shown area.
 */
void MapScreen_UpdatePlayerMarker(struct GameState *gameState)
{
    s32 markerY;
    s32 markerX;
    struct EwramData_EntityData *entity;
    s32 bossRush;
    s32 phase;
    s32 radius;

    entity = Get_unk_13110();
    if (GetAreaFromMapPosition(GetEntityRoomXPositionInteger(gEwramData->playerState.player), GetEntityRoomYPositionInteger(Get_unk_13110())) != 11)
    {
        bossRush = gEwramData->gameState.bossRushMode != 0;
        if (!bossRush)
        {
            markerX = (gameState->roomMapXPos + (GetEntityRoomXPositionInteger(entity) >> 8) + 4) * 4;
            markerY = ((gameState->roomMapYPos + (GetEntityRoomYPositionInteger(entity) >> 8) + 2) * 4) + 2;
            phase = gEwramData->frameCounter >> 2;
            radius = 0x10 - (phase & 0xF);
            if ((gameState->unk_8D_6 == 1 && gameState->unk_8E_0) || (gameState->unk_8D_6 == 3 && !gameState->unk_8E_0))
            {
                gDisplayRegisters.bldY = 0;
            }
            else
            {
                gDisplayRegisters.bldY = (phase & 0xF) + 1;
            }
            Window1_SetCircle(markerX, markerY, radius);
        }
    }
}

/**
 * @brief Sub-mode 4 steps: draw the castle map, handle scrolling input until
 * B or Select, then restore the room display; return 1 when closed.
 */
s32 MapScreen_Update(struct GameState *gameState)
{
    struct RoomDefinition *room;
    s32 new_input;
    s32 done;
    struct BgLayerState *bg;

    room = gameState->currentRoomDefinition;
    new_input = gEwramData->inputData.newInput;
    done = 0;

    switch (gameState->subModeStep)
    {
        case 0:
            Entity_UpdateAll();
            gEwramData->bgControlMinWriteLevel = gEwramData->unk_A074_4 = 3;
            gameState->subModeStep = 8;
            break;

        case 8:
            gDisplayRegisters.bgOfs[0].hOfs = 0;
            gDisplayRegisters.bgOfs[0].vOfs = 0;
            Palette_SaveState();
            BgPalette_Load((u8 *)0x081183CC);
            MapScreen_DrawMap(gameState);
            gameState->subModeStep = 1;
            break;

        case 1:
            gUnk_03002CB0.dispCnt = DCNT_BG0;
            gDisplayRegisters.bldY -= 4;
            if (gDisplayRegisters.bldY == 0)
            {
                gUnk_03002CB0.dispCnt |= DCNT_WIN1;
                gDisplayRegisters.bldCnt = BLDCNT_SCREEN_FIRST_TARGET | BLDCNT_BRIGHTNESS_INCREASE_EFFECT;
                gDisplayRegisters.winIn_H = WIN1_ALL >> 8;
                gDisplayRegisters.winOut_L = WIN0_ALL_NO_COLOR_EFFECT;
                gDisplayRegisters.win1H = C_16_2_8(SCREEN_SIZE_X, SCREEN_SIZE_X + 1);
                gDisplayRegisters.win1V = C_16_2_8(SCREEN_SIZE_Y, SCREEN_SIZE_Y + 1);
                Window1_EnableHBlank();
                gameState->subModeStep += 1;
            }
            break;

        case 2:
            MapScreen_UpdatePlayerMarker(gameState);
            if (new_input & (KEY_B | KEY_SELECT))
            {
                gDisplayRegisters.bldCnt = BLDCNT_SCREEN_FIRST_TARGET | BLDCNT_BRIGHTNESS_DECREASE_EFFECT;
                gDisplayRegisters.winIn_H = 0;
                gDisplayRegisters.bldY = 0;
                Window1_Disable();
                gameState->subModeStep += 1;
            }
            break;

        case 3:
            gDisplayRegisters.bldY += 4;
            if (gDisplayRegisters.bldY == BLDY_MAX_VALUE)
            {
                gameState->subModeStep += 1;
            }
            break;
        case 4:
            Palette_RestoreState();
            Hud_Init();
            bg = &gEwramData->bgInfo[1];
            // TODO: A094 types
            BgMap_Setup(1, (struct BgMapMetadata *) bg->pBgMetadata, bg->xPos.part16.integer, bg->yPos.part16.integer);
            gUnk_03002CB0.dispCnt = 0;
            Room_LoadGraphics(room);
            Room_LoadPalettes(room);
            gameState->subModeStep += 1;
            break;

        case 5:
            Display_RestoreState();
            // TODO: 7864 and 8C6C
            DMA_COPY_32(3, &gEwramData->hBlankEffect_backup, &gEwramData->hBlankEffect, sizeof(gEwramData->hBlankEffect));
            Entity_UpdateAll();
            gDisplayRegisters.bldY = 0;
            gDisplayRegisters.bldCnt |= BLDCNT_SCREEN_SECOND_TARGET;
            gEwramData->startPauseEnabled = 1;
            gEwramData->paused = 0;
            Sound_ReplayTrackedWithVolume();
            gEwramData->bgControlMinWriteLevel = gEwramData->unk_A074_4 = 0;
            done = 1;
            break;

        case 6:
            if (gDisplayRegisters.bldY < BLDY_MAX_VALUE)
            {
                gDisplayRegisters.bldY += 4;
            }
            else
            {
                MapScreen_DrawMap(gameState);
                gameState->subModeStep = 1;
            }
            break;
    }

    gEwramData->hBlankEffect.requestStop = 1;
    return done;
}

/**
 * @brief Select in game: dismiss effects, black out and pause, enter in-game
 * sub-mode 4, back up the H-blank state, stop tracked sounds and load
 * the map tile graphics.
 */
void MapScreen_Open(struct GameState *gameState)
{
    Display_SaveState();
    gDisplayRegisters.bldCnt = BLDCNT_SCREEN_FIRST_TARGET | BLDCNT_BRIGHTNESS_DECREASE_EFFECT;
    gUnk_03002CB0.dispCnt &= ~DCNT_WIN0;
    gDisplayRegisters.bldY = BLDY_MAX_VALUE;
    gEwramData->startPauseEnabled = 0;
    gEwramData->paused = 1;
    gameState->subMode = 4;
    gameState->subModeStep = 0;
    gDisplayRegisters.bldY = BLDY_MAX_VALUE;
    DMA_COPY_32(3, &gEwramData->hBlankEffect, &gEwramData->hBlankEffect_backup, sizeof(gEwramData->hBlankEffect_backup));
    gDisplayRegisters.bgOfs[0].hOfs = 0;
    gDisplayRegisters.bgOfs[0].vOfs = 0;
    gEwramData->hBlankEffect.requestStop = 1;
    DMA_FILL_32(3, 0, &gEwramData->hBlankEffect.hBlankBuffer, sizeof(gEwramData->hBlankEffect.hBlankBuffer[0]));
    Sound_StopTrackedWithVolume();
    DMA_COPY_32(3, sDebugFontTiles, VRAM_BASE + 0x6000, sizeof(sDebugFontTiles));
}

/**
 * @brief Under the original screen-X, area and mode guards, mark the player map cell in both map bit planes using room-map origin plus room position.
 */
void Map_MarkCurrentCell(struct GameState *gameState)
{
    struct EwramData_EntityData *player;
    s32 roomX;
    s32 roomY;
    s32 cellX;
    s16 cellY;

    player = gEwramData->playerState.player;
    if ((s16)player->posX.part16.integer < SCREEN_SIZE_X)
    {
        roomX = GetEntityRoomXPositionInteger(player);
        roomY = GetEntityRoomYPositionInteger(player);
        if ((GetAreaFromMapPosition(GetEntityRoomXPositionInteger(gEwramData->playerState.player), GetEntityRoomYPositionInteger(gEwramData->playerState.player)) != 11))
        {
            if (!gEwramData->gameState.bossRushMode)
            {
                cellX = gameState->roomMapXPos + (roomX >> 8);
                cellY = gameState->roomMapYPos + (roomY >> 8);
                gameState->mapCells[cellX >> 5][cellY].visitedThisRun |= 1 << (cellX & 0x1F);
                gameState->mapCells[cellX >> 5][cellY].visited |= 1 << (cellX & 0x1F);
            }
        }
    }
}

/**
 * @brief Move the warp choice selectedWarpIndex to the previous/next activated warp room
 * (bits of activatedWarpFlags); 0 when none is activated.
 */
void WarpMenu_HandleCursorInput(void)
{
    u16 repeatedInput;

    repeatedInput = gEwramData->inputData.repeatedInput;

    if (gEwramData->gameState.activatedWarpFlags[0] == 0)
    {
        gEwramData->gameState.selectedWarpIndex = gEwramData->gameState.activatedWarpFlags[0];
        return;
    }

    if (repeatedInput & (KEY_UP | KEY_LEFT))
    {
        PlaySong(SE_241);
        do
        {
            if (--gEwramData->gameState.selectedWarpIndex < 0)
            {
                gEwramData->gameState.selectedWarpIndex = 7;
            }
        }
        while (!(gEwramData->gameState.activatedWarpFlags[gEwramData->gameState.selectedWarpIndex >> 5] & (1 << (gEwramData->gameState.selectedWarpIndex & 0x1F))));
    }

    if (repeatedInput & (KEY_DOWN | KEY_RIGHT))
    {
        PlaySong(SE_241);
        do
        {
            if (++gEwramData->gameState.selectedWarpIndex > 7)
            {
                gEwramData->gameState.selectedWarpIndex = 0;
            }
        }
        while (!(gEwramData->gameState.activatedWarpFlags[gEwramData->gameState.selectedWarpIndex >> 5] & (1 << (gEwramData->gameState.selectedWarpIndex & 0x1F))));
    }
}

/**
 * @brief Enter in-game sub-mode 7 from the warp-room object: black out,
 * pause, back up the H-blank state, stop tracked sounds and load the
 * map tile graphics.
 */
void WarpMenu_Open(struct GameState *gameState)
{
    Display_SaveState();
    gDisplayRegisters.bldCnt = BLDCNT_SCREEN_FIRST_TARGET | BLDCNT_BRIGHTNESS_DECREASE_EFFECT;
    gEwramData->startPauseEnabled = 0;
    gEwramData->paused = 1;
    gameState->subMode = 7;
    gameState->subModeStep = 0;
    gDisplayRegisters.bldY = BLDY_MAX_VALUE;
    DMA_COPY_32(3, &gEwramData->hBlankEffect, &gEwramData->hBlankEffect_backup, sizeof(gEwramData->hBlankEffect_backup));
    gDisplayRegisters.bgOfs[0].hOfs = 0;
    gDisplayRegisters.bgOfs[0].vOfs = 0;
    gEwramData->hBlankEffect.requestStop = 1;
    DMA_FILL_32(3, 0, &gEwramData->hBlankEffect.hBlankBuffer, sizeof(gEwramData->hBlankEffect.hBlankBuffer[0]));
    Sound_StopTrackedWithVolume();
    DMA_COPY_32(3, sDebugFontTiles, VRAM_BASE + 0x6000, sizeof(sDebugFontTiles));
}

/**
 * @brief Sub-mode 7 steps: draw the map, preselect the current warp room, let
 * the player cycle through the activated warp rooms (activatedWarpFlags bits) and
 * set the destination; selectedWarpIndex = -1 means cancelled. Return 1 when done.
 */
s32 WarpMenu_Update(struct GameState *gameState)
{
    s32 done;
    u16 markerX;
    u16 y;
    s32 phase;
    u16 input;
    struct RoomDefinition *room;
    struct WarpRoomEntry *warp;
    struct BgLayerState *bg;
    s32 radius;

    room = gameState->currentRoomDefinition;
    input = gEwramData->inputData.newInput;
    done = 0;

    switch (gameState->subModeStep)
    {
        case 0:
            Entity_UpdateAll();
            gEwramData->bgControlMinWriteLevel = gEwramData->unk_A074_4 = 3;
            if (gDisplayRegisters.bldY == BLDY_MAX_VALUE)
            {
                input = gEwramData->gameState.roomMapXPos + (GetEntityRoomXPositionInteger(gEwramData->playerState.player) >> 8);
                y = gEwramData->gameState.roomMapYPos + (GetEntityRoomYPositionInteger(gEwramData->playerState.player) >> 8);
                BgPalette_Load((u8 *)0x081183CC);
                MapScreen_DrawMap(gameState);
                gameState->subModeStep += 1;
                gEwramData->gameState.selectedWarpIndex = Warp_FindRoomIndex(input, y);
                if (gEwramData->gameState.selectedWarpIndex <= -1)
                {
                    gEwramData->gameState.selectedWarpIndex = 0;
                }
                if (!(gEwramData->gameState.activatedWarpFlags[0] & 1))
                {
                    gEwramData->gameState.activatedWarpFlags[0] |= 1;
                    gEwramData->gameState.selectedWarpIndex = 0;
                }
                gDisplayRegisters.bgOfs[0].hOfs = 0;
                gDisplayRegisters.bgOfs[0].vOfs = 0;
                Palette_SaveState();
            }
            break;

        case 1:
            gUnk_03002CB0.dispCnt = DCNT_WIN1 | DCNT_BG0;
            gDisplayRegisters.bldY -= 4;
            if (gDisplayRegisters.bldY == 0)
            {
                gDisplayRegisters.bldCnt = BLDCNT_SCREEN_FIRST_TARGET | BLDCNT_BRIGHTNESS_INCREASE_EFFECT;
                gDisplayRegisters.winIn_H = WIN1_ALL >> 8;
                gDisplayRegisters.winOut_L = WIN0_ALL_NO_COLOR_EFFECT;
                gDisplayRegisters.win1H = C_16_2_8(SCREEN_SIZE_X, SCREEN_SIZE_X + 1);
                gDisplayRegisters.win1V = C_16_2_8(SCREEN_SIZE_Y, SCREEN_SIZE_Y + 1);
                Window1_EnableHBlank();
                gameState->subModeStep += 1;
            }
            break;

        case 2:
            WarpMenu_HandleCursorInput();
            warp = Warp_GetRoomEntry(gEwramData->gameState.selectedWarpIndex);
            markerX = ((warp->mapX + 4) * 4) + 2;
            y = ((warp->mapY + 2) * 4) + 2;
            phase = gEwramData->frameCounter >> 2;
            radius = 8 - (phase & 3);
            gDisplayRegisters.bldY = (phase & 7) + 8;
            Window1_SetCircle(markerX, y, radius);
            if (input & (KEY_B | KEY_SELECT))
            {
                PlaySong(SE_240);
                gDisplayRegisters.bldCnt = BLDCNT_SCREEN_FIRST_TARGET | BLDCNT_BRIGHTNESS_DECREASE_EFFECT;
                gDisplayRegisters.winIn_H = 0;
                gDisplayRegisters.bldY = 0;
                Window1_Disable();
                gameState->subModeStep += 1;
                gEwramData->gameState.selectedWarpIndex = -1;
            }
            else if (input & KEY_A)
            {
                PlaySong(SE_244);
                RoomJump_SetDestination(warp->roomDefinition, 0, 0, 0x78, 0x80);
                gDisplayRegisters.bldCnt = BLDCNT_SCREEN_FIRST_TARGET | BLDCNT_BRIGHTNESS_DECREASE_EFFECT;
                gDisplayRegisters.winIn_H = 0;
                gDisplayRegisters.bldY = 0;
                Window1_Disable();
                gameState->subModeStep += 1;
            }
            break;

        case 3:
            gDisplayRegisters.bldY += 4;
            if (gDisplayRegisters.bldY == BLDY_MAX_VALUE)
            {
                gameState->subModeStep += 1;
            }
            break;

        case 4:
            Palette_RestoreState();
            Hud_Init();
            bg = &gEwramData->bgInfo[1];
            BgMap_Setup(1, (struct BgMapMetadata *) bg->pBgMetadata, bg->xPos.part16.integer, bg->yPos.part16.integer);
            gUnk_03002CB0.dispCnt = 0;
            Room_LoadGraphics(room);
            Room_LoadPalettes(room);
            gameState->subModeStep += 1;
            break;

        case 5:
            Display_RestoreState();
            DMA_COPY_32(3, &gEwramData->hBlankEffect_backup, &gEwramData->hBlankEffect, sizeof(gEwramData->hBlankEffect));
            Entity_UpdateAll();
            gDisplayRegisters.bldY = 0;
            gDisplayRegisters.bldCnt |= BLDCNT_SCREEN_SECOND_TARGET;
            gEwramData->startPauseEnabled = 1;
            gEwramData->paused = 0;
            Sound_ReplayTrackedWithVolume();
            gEwramData->bgControlMinWriteLevel = gEwramData->unk_A074_4 = 0;
            done = 1;
            break;

        case 6:
            if (gDisplayRegisters.bldY < BLDY_MAX_VALUE)
            {
                gDisplayRegisters.bldY += 4;
            }
            else
            {
                MapScreen_DrawMap(gameState);
                gameState->subModeStep = 1;
            }
            break;
    }

    gEwramData->hBlankEffect.requestStop = 1;
    return done;
}

/**
 * @brief Return 0 when the living player is beyond the room width, above
 * 0x30 or below the room height, or stands on exit tile 0xF0; else 1.
 */
s32 Room_IsPlayerInside(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg;
    struct BgMapMetadata *metadata;
    u32 x;
    u32 y;
    u32 maxY;
    u32 maxX;

    bg = &gEwramData->bgInfo[1];
    metadata = bg->pBgMetadata;
    x = (s16)entity->posX.part16.integer + (s16)bg->xPos.part16.integer;
    y = (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer;
    if (gEwramData->playerStatus.currentHP != 0)
    {
        if (metadata->widthCells > 1)
        {
            maxX = metadata->widthCells << 8;
        }
        else
        {
            maxX = 0xF0;
        }

        if (metadata->heightCells > 1)
        {
            maxY = (metadata->heightCells << 8) - 0x30;
        }
        else
        {
            maxY = 0xD0;
        }

        if (x > maxX)
            return 0;

        if (y < 0x30 || y > maxY)
            return 0;

        if (Collision_GetTileAtEntity(gEwramData->playerState.player, 0, 0) == 0xF0)
            return 0;
    }
    return 1;
}

/**
 * @brief Black out, pause and enter in-game sub-mode 8 to jump to the stored
 * destination (warp rooms, objects and boss defeat scripts).
 */
void RoomJump_Begin(struct GameState *gameState)
{
    gDisplayRegisters.bldCnt = BLDCNT_SCREEN_FIRST_TARGET | BLDCNT_BRIGHTNESS_DECREASE_EFFECT;
    gDisplayRegisters.bldY = BLDY_MAX_VALUE;
    gUnk_03002CB0.dispCnt &= ~DCNT_WIN0;
    gEwramData->startPauseEnabled = 0;
    gEwramData->paused = 1;
    gameState->subMode = 8;
    gameState->subModeStep = 0;
}

/**
 * @brief Sub-mode 8 steps: clear the old room, place the player, load the
 * destination room at the stored camera position, store the entry
 * position and restore the display; return 1 when done.
 */
s32 RoomJump_Update(struct GameState *gameState)
{
    s32 done;
    struct RoomDefinition *room;

    done = 0;

    switch (gameState->subModeStep)
    {
        case 0:
            Entity_UpdateAll();
            gEwramData->bgControlMinWriteLevel = gEwramData->unk_A074_4 = 3;
            gameState->subModeStep += 1;
            break;

        case 1:
            Room_ClearForTransition(gameState);
            DMA_FILL_32(3, 0, 0x0600E800, 0x800);
            DMA_FILL_32(3, 0, 0x0600F000, 0x800);
            DMA_FILL_32(3, 0, 0x0600F800, 0x800);
            gEwramData->playerState.player->posX.part16.integer = gEwramData->gameState.roomJumpPlayerX;
            gEwramData->playerState.player->posY.part16.integer = gEwramData->gameState.roomJumpPlayerY;
            gEwramData->playerState.player->velX.whole = 0;
            gEwramData->playerState.player->velY.whole = 0;
            gEwramData->gameState.currentRoomDefinition = gEwramData->gameState.roomJumpDestination;
            Room_LoadAtCamera(gEwramData->gameState.roomJumpCameraX, gEwramData->gameState.roomJumpCameraY);
            Room_StoreEntryPosition(gameState);
            gEwramData->paused = 0;
            Entity_UpdateAll();
            gEwramData->paused = 1;
            gameState->subModeStep += 1;
            /* fallthrough */
        case 2:
            room = gameState->currentRoomDefinition;
            if (!(gEwramData->gameState.stateFlags & 0x800000))
            {
                gEwramData->startPauseEnabled = 1;
            }
            gEwramData->gameState.stateFlags &= ~0x800000;
            gEwramData->paused = 0;
            gEwramData->bgControlMinWriteLevel = gEwramData->unk_A074_4 = 0;
            gUnk_03002CB0.dispCnt = room->displayControl;
            gDisplayRegisters.bldY = BLDY_MAX_VALUE / 2;
            gDisplayRegisters.bldCnt = room->blendControl;
            Display_SetDefaultBlend();
            gDisplayRegisters.mosaic = 0;
            done = 1;
            break;
    }

    return done;
}

/**
 * @brief Unless paused, update output offsets from duration and random masks or one-shot signed components, then expire the request state as in the original.
 */
void ScreenShake_Update(void)
{
    if (gEwramData->paused)
        return;

    if (gEwramData->gameState.screenShakeTimer != 0)
    {
        gEwramData->gameState.screenShakeTimer -= 1;

        if (gEwramData->gameState.screenShakeComponentX != 0)
        {
            gEwramData->screenShakeOffsetX = RandomNumberGenerator() & gEwramData->gameState.screenShakeComponentX;
        }
        if (gEwramData->gameState.screenShakeComponentY != 0)
        {
            gEwramData->screenShakeOffsetY = RandomNumberGenerator() & gEwramData->gameState.screenShakeComponentY;
        }

        if (gEwramData->gameState.screenShakeTimer == 0)
        {
            gEwramData->gameState.screenShakeComponentX = 0;
            gEwramData->gameState.screenShakeComponentY = 0;
            gEwramData->screenShakeOffsetX = gEwramData->gameState.screenShakeComponentX;
            gEwramData->screenShakeOffsetY = gEwramData->gameState.screenShakeComponentY;
            gEwramData->gameState.screenShakePriority = 0;
        }
    }
    else
    {
        gEwramData->screenShakeOffsetX = gEwramData->gameState.screenShakeComponentX;
        gEwramData->screenShakeOffsetY = gEwramData->gameState.screenShakeComponentY;
        gEwramData->gameState.screenShakePriority = 0;
        gEwramData->gameState.screenShakeComponentX = 0;
        gEwramData->gameState.screenShakeComponentY = 0;
    }
}
