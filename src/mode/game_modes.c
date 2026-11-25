#include "game_modes.h"
#include "core.h"
#include "sram.h"
#include "main_menu.h"
#include "intro_cutscene.h"
#include "soul_trade.h"
#include "in_game.h"
#include "bg_effects.h"
#include "room_entities.h"
#include "map_flags.h"
#include "save_data.h"
#include "player_core.h"
#include "soul_inventory.h"
#include "player_effects.h"
#include "player_level.h"
#include "debug_1967.h"
#include "graphics.h"
#include "gameplay.h"
#include "entity_util.h"
#include "sound.h"
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
#include "entity_common.h"
#include "entity_motion.h"
#include "equipment_menu.h"
#include "hitbox.h"
#include "item_menu.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "palette.h"
#include "player_soma.h"
#include "system_menus.h"
#include "text12.h"
#include "data/data_118B48.h"
#include "data/data_0E5364.h"
#include "data/data_4F1444.h"
#include "menu_common.h"
#include "bg_and_collision.h"

struct DebugMenuOptions {
    const u8 *text;
    u32 gameMode;
};

struct DebugMenuOptions sDebugMenuOptions[] = {
    [0] = {
        .text = "Exit",
        .gameMode = GAME_MODE_DEBUG_EXIT
    },
    [1] = {
        .text = "Start",
        .gameMode = GAME_MODE_RESET
    },
    [2] = {
        .text = "Trade",
        .gameMode = GAME_MODE_SOUL_TRADE_MENU
    },
    [3] = {
        .text = "1967",
        .gameMode = GAME_MODE_DEBUG_1967
    },
    [4] = {
        .text = "2229",
        .gameMode = GAME_MODE_DEBUG_2229
    },
    [5] = {
        .text = "2739",
        .gameMode = GAME_MODE_DEBUG_2739
    },
    [6] = {
        .text = "4531",
        .gameMode = GAME_MODE_DEBUG_4531
    },
    [7] = {
        .text = "Staff Roll",
        .gameMode = GAME_MODE_CREDITS
    },
    [8] = {
        .text = NULL,
        .gameMode = GAME_MODE_KONAMI_LOGO
    },
};

struct DebugExitOptions {
    s32 gameMode;
    s32 saveFile;
    u32* roomDefinition; // Type?
    const u8* text;
};

struct DebugExitOptions sDebugExitOptions[] = {
    [0] = {
        .gameMode = GAME_MODE_IN_GAME,
        .saveFile = 0,
        .roomDefinition = (u32*)sDebugEventRoom12,
        .text = "0.Entrance"
    },
    [1] = {
        .gameMode = GAME_MODE_IN_GAME,
        .saveFile = 0,
        .roomDefinition = (u32*)sDebugExitOptionsRoom1,
        .text = "1.Adoration Hall"
    },
    [2] = {
        .gameMode = GAME_MODE_IN_GAME,
        .saveFile = 0,
        .roomDefinition = (u32*)sDebugExitOptionsRoom2,
        .text = "2.Library"
    },
    [3] = {
        .gameMode = GAME_MODE_IN_GAME,
        .saveFile = 0,
        .roomDefinition = (u32*)sDebugExitOptionsRoom3,
        .text = "3.Dance Hall"
    },
    [4] = {
        .gameMode = GAME_MODE_IN_GAME,
        .saveFile = 0,
        .roomDefinition = (u32*)sDebugExitOptionsRoom4,
        .text = "4.Harem"
    },
    [5] = {
        .gameMode = GAME_MODE_IN_GAME,
        .saveFile = 0,
        .roomDefinition = (u32*)sDebugEventRoom9,
        .text = "5.Babylon"
    },
    [6] = {
        .gameMode = GAME_MODE_IN_GAME,
        .saveFile = 0,
        .roomDefinition = (u32*)sDebugExitOptionsRoom6,
        .text = "6.Clock Tower"
    },
    [7] = {
        .gameMode = GAME_MODE_IN_GAME,
        .saveFile = 0,
        .roomDefinition = (u32*)sDebugExitOptionsRoom7,
        .text = "7.Water Vein"
    },
    [8] = {
        .gameMode = GAME_MODE_IN_GAME,
        .saveFile = 0,
        .roomDefinition = (u32*)sDebugExitOptionsRoom8,
        .text = "8.Arena"
    },
    [9] = {
        .gameMode = GAME_MODE_IN_GAME,
        .saveFile = 0,
        .roomDefinition = (u32*)sDebugExitOptionsRoom9,
        .text = "9.Saijo-kai"
    },
    [10] = {
        .gameMode = GAME_MODE_IN_GAME,
        .saveFile = 0,
        .roomDefinition = (u32*)sDebugExitOptionsRoom10,
        .text = " Chaos Start"
    },
    [11] = {
        .gameMode = GAME_MODE_IN_GAME,
        .saveFile = 0,
        .roomDefinition = (u32*)sDebugExitOptionsRoom11,
        .text = " Chaos Boss"
    },
    [12] = {
        .gameMode = GAME_MODE_IN_GAME,
        .saveFile = 0,
        .roomDefinition = (u32*)sDebugExitOptionsRoom12,
        .text = " Boss Rush 1P"
    },
    [13] = {
        .gameMode = GAME_MODE_BOSS_RUSH_MENU,
        .saveFile = 0,
        .roomDefinition = (u32*)sDebug4531Room2,
        .text = " Boss Rush 2P"
    },
    [14] = {
        .gameMode = GAME_MODE_IN_GAME,
        .saveFile = 0,
        .roomDefinition = (u32*)sDebugExitOptionsRoom14,
        .text = "  VS Legion"
    },
    [15] = {
        .gameMode = GAME_MODE_IN_GAME,
        .saveFile = 0,
        .roomDefinition = (u32*)sDebugEventRoom7,
        .text = "  VS Balor"
    },
    [16] = {
        .gameMode = GAME_MODE_IN_GAME,
        .saveFile = 0,
        .roomDefinition = (u32*)sDebugExitOptionsRoom16,
        .text = "  VS Collector"
    },
};

s32 sDebugExitOptionsLength = ARRAY_SIZE(sDebugExitOptions);

const u8 sDebugMenuSelectText[] = "Please select.\n";
const u8 sDebugExitEmptyText[] = " ";
const u8 sDebugExitCursorText[0x4] = ">";

extern struct unk_08506B38 *sTextEntries[0xB4F];

/**
 * @brief 2454 | Handle debug game mode
 * 
 * @return s32 Game mode, -1 is no change, -2 is next game mode, else the specified game mode
 */
s32 GameModeDebugUpdate(void)
{
    s32 option;
    s32 optionEnd;
    s32 gameMode;
    u16 repeatedInput;

    gameMode = GAME_MODE_SAME_MODE;
    repeatedInput = gEwramData->inputData.repeatedInput;

    switch (gEwramData->gameModeUpdateStage)
    {
        case 0:
            DMA_FILL_32(3, 0, VRAM_BASE + 0xE000, 0x800);
            BgCmdBuffer_WriteStringWide(1, 1, 0, sDebugMenuSelectText);
            gEwramData->gameModeUpdateStage += 1;
            gEwramData->gameModeSubStage = 0; // gameModeSubStage is used as the current option here
            break;

        case 1:
            for (optionEnd = 0; sDebugMenuOptions[optionEnd].text != NULL; optionEnd++);

            if (gEwramData->inputData.newInput & (KEY_START | KEY_A))
            {
                DMA_FILL_32(3, 0, VRAM_BASE + 0xE000, 0x800);
                gameMode = sDebugMenuOptions[gEwramData->gameModeSubStage].gameMode;

                // Only slot 8 has Konami Logo mode, which is not possible to reach
                if ((gameMode == GAME_MODE_KONAMI_LOGO) && (repeatedInput & KEY_SELECT))
                {
                    Game_SetStartPosition(0);
                    gameMode = GAME_MODE_IN_GAME;
                }
            }
            else
            {
                if (repeatedInput & KEY_UP)
                {
                    if (gEwramData->gameModeSubStage != 0)
                    {
                        gEwramData->gameModeSubStage -= 1;
                    }
                    else
                    {
                        // Wrap around
                        gEwramData->gameModeSubStage = optionEnd - 1;
                    }
                }
                else if (repeatedInput & KEY_DOWN)
                {
                    gEwramData->gameModeSubStage += 1;
                    if (gEwramData->gameModeSubStage >= optionEnd)
                    {
                        // Wrap around
                        gEwramData->gameModeSubStage = 0;
                    }
                }

                for (option = 0; option < optionEnd; option++)
                {
                    BgCmdBuffer_WriteStringWide(4, option + 3, (option == gEwramData->gameModeSubStage) ? 1 : 3, sDebugMenuOptions[option].text);
                }
            }
            break;
    }
    return gameMode;
}

/**
 * @brief 2588 | Handle exit debug game mode
 * 
 * @return s32 Game mode, -1 is no change, -2 is next game mode, else the specified game mode
 */
s32 GameModeDebugExitUpdate(void)
{
    s32 hasBackup;
    s32 option;
    s32 gameMode;
    u32 slotOffset;

    gameMode = GAME_MODE_SAME_MODE;

    switch (gEwramData->gameModeUpdateStage)
    {
        case 0:
            gUnk_03002CB0.dispCnt = DCNT_BG0;
            DMA_COPY_32(3, &sDebugFontTiles, VRAM_BASE + 0x6000, sizeof(sDebugFontTiles));
            DMA_COPY_32(3, &sDebugFontExtraTiles, VRAM_BASE + 0x8000, sizeof(sDebugFontExtraTiles));
            DMA_COPY_32(3, &sDebugPalette, PALRAM_BASE, sizeof(sDebugPalette));
            DMA_FILL_32(3, 0, VRAM_BASE + 0xE000, 0x800);
            gEwramData->gameModeSubStage = 0; // gameModeSubStage is used as the current option here

            for (option = 0; option < sDebugExitOptionsLength; option++)
            {
                BgCmdBuffer_WriteStringWide(5, option + 2, 0, sDebugExitOptions[option].text);
            }

            gEwramData->gameModeUpdateStage += 1;
            gEwramData->gameModeSubStage = 0;
            break;

        case 1:
            gEwramData->gameModeUpdateStage += 1;
            gEwramData->gameModeSubStage = 0;
            /* fallthrough */

        case 2:
            if (gEwramData->inputData.newInput & KEY_UP)
            {
                BgCmdBuffer_WriteStringWide(4, gEwramData->gameModeSubStage + 2, 2, (u8 *) sDebugExitEmptyText);
                if (--gEwramData->gameModeSubStage > sDebugExitOptionsLength)
                {
                    gEwramData->gameModeSubStage = sDebugExitOptionsLength - 1;
                }
            }
            else if (gEwramData->inputData.newInput & KEY_DOWN)
            {
                BgCmdBuffer_WriteStringWide(4, gEwramData->gameModeSubStage + 2, 2, (u8 *) sDebugExitEmptyText);
                if (++gEwramData->gameModeSubStage >= sDebugExitOptionsLength)
                {
                    gEwramData->gameModeSubStage = 0;
                }
            }

            BgCmdBuffer_WriteStringWide(4, gEwramData->gameModeSubStage + 2, 2, (u8 *) sDebugExitCursorText);
            if (gEwramData->inputData.newInput & KEY_A)
            {
                // If entrance option picked
                if (gEwramData->gameModeSubStage == 0)
                {
                    if (gEwramData->inputData.heldInput & KEY_LEFT)
                    {
                        gEwramData->gameState.currentSaveSlot = 2;
                    }
                    else if (gEwramData->inputData.heldInput & KEY_RIGHT)
                    {
                        gEwramData->gameState.currentSaveSlot = 4;
                    }
                    
                    Player_InitNewGameIfNeeded();
                    gEwramData->gameState.currentArea = 0;
                    gEwramData->gameState.currentRoom = 0;
                    gEwramData->gameState.savedCameraX = 0;
                    gEwramData->gameState.savedCameraY = 0x200;
                    gEwramData->gameState.savedPlayerX = 0x78;
                    gEwramData->gameState.savedPlayerY = 0x8D;
                    gEwramData->gameState.roomData = GetRoomPointer(gEwramData->gameState.currentArea, gEwramData->gameState.currentRoom);
                    gEwramData->gameState.currentRoomDefinition = 0;
                }
                else
                {
                    gEwramData->gameState.currentRoomDefinition = (struct RoomDefinition *)sDebugExitOptions[gEwramData->gameModeSubStage].roomDefinition;
                    gEwramData->gameState.shownAreaNameFlags = -1;
                    Player_InitNewGameIfNeeded();
                    Player_EquipItem(2, 0x3a);
                    Player_EquipItem(0, 8);
                    Player_EquipItem(1, 0x24);
                    Player_SetLevel(0x63);
                    Player_EquipSoulsAndRestore(5, 3, 2);
                    Player_RefreshEquipmentStats();
                }

                if ((gEwramData->gameState.currentRoomDefinition == 0) && (gEwramData->gameState.roomData == 0))
                {
                    slotOffset = 0;
                    gEwramData->gameState.currentSaveSlot = sDebugExitOptions[gEwramData->gameModeSubStage].saveFile * 2;
                    hasBackup = SaveData_ReadSlotHeader(gEwramData->gameState.currentSaveSlot + 1, 0);
                    if ((gEwramData->inputData.heldInput & KEY_RIGHT) && (hasBackup != 0))
                    {
                        slotOffset = gEwramData->saveHeaders[0].playTime != slotOffset;
                    }

                    if (SaveData_LoadSlotFromSram(gEwramData->gameState.currentSaveSlot + slotOffset) != 0)
                    {
                        gEwramData->gameState.roomData = GetRoomPointer(gEwramData->gameState.currentArea, gEwramData->gameState.currentRoom);
                    }
                    else
                    {
                        gEwramData->gameState.currentRoomDefinition = (struct RoomDefinition *)sDebugEventRoom12;
                    }
                }

                gEwramData->bgControlMinWriteLevel = gEwramData->unusedWriteLevel = 3;
                gDisplayRegisters.bldCnt = BLDCNT_SCREEN_SECOND_TARGET | BLDCNT_BRIGHTNESS_DECREASE_EFFECT | BLDCNT_SCREEN_FIRST_TARGET;
                gDisplayRegisters.bldY = BLDY_MAX_VALUE;
                gEwramData->gameModeUpdateStage += 1;
            }
            break;

        case 3:
            gameMode = sDebugExitOptions[gEwramData->gameModeSubStage].gameMode;
            gEwramData->gameState.linkPlay = 0;
            gEwramData->gameState.bossRushMode = 0;
            gEwramData->gameState.activatedWarpFlags[0] = -1;

            if (sDebugExitOptions[gEwramData->gameModeSubStage].roomDefinition != (u32*)sDebugEventRoom12)
            {
                GameFlag_Set((1 << 5) | 0);
            }
            break;
    }

    return gameMode;
}

const u16 sKonamiCodeInputs[] = {
    KEY_UP, KEY_UP, KEY_DOWN, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_LEFT, KEY_RIGHT, KEY_B, KEY_A
};

/**
 * @brief 2944 | Check and update konami code logic
 * 
 */
void KonamiCodeCheck(void)
{
    u16 newInput;

    newInput = gEwramData->inputData.newInput;
    if (gEwramData->konamiCodeInputsCounter >= 1 && gEwramData->konamiCodeInputsCounter <= ARRAY_SIZE(sKonamiCodeInputs))
    {
        if (newInput == sKonamiCodeInputs[gEwramData->konamiCodeInputsCounter - 1])
        {
            gEwramData->konamiCodeInputsCounter++;
        }
        else if (newInput != 0)
        {
            gEwramData->konamiCodeInputsCounter = 0;
        }
    }
}

/**
 * @brief 2990 | Handle Konami logo game mode
 * 
 * @return s32 Game mode, -1 is no change, -2 is next game mode, else the specified game mode
 */
s32 GameModeKonamiLogoUpdate(void)
{
    s32 gameMode;

    gameMode = GAME_MODE_SAME_MODE;

    switch (gEwramData->gameModeUpdateStage)
    {
        case 0:
            gUnk_03002CB0.dispCnt = DCNT_BG0;
            gDisplayRegisters.bgCnt[0] = CREATE_BGCNT(0, 28, BGCNT_HIGH_PRIORITY, BGCNT_SIZE_256x256);
            gDisplayRegisters.bldCnt = BLDCNT_SCREEN_FIRST_TARGET | BLDCNT_BRIGHTNESS_INCREASE_EFFECT;
            gDisplayRegisters.bldY = BLDY_MAX_VALUE;
            EntityDeleteAll();
            ObjTiles_Init();
            Obj_Init();
            Sprite_Init();
            Palette_Init();
            Text_Initialize();
            HitBox_ClearAll();
            if ((gEwramData->gameState.unk_380 & 0x3F) && (gEwramData->konamiCodeInputsCounter != UCHAR_MAX))
            {
                gEwramData->konamiCodeInputsCounter = 1;
            }
            else
            {
                gEwramData->konamiCodeInputsCounter = 0;
            }
            gEwramData->gameModeUpdateStage += 1;
            gEwramData->gameModeSubStage = 0;
            break;

        case 1:
            Gfx_CopyResource((u8 *)sKonamiLogoGfx, VRAM_BASE, 0);
            BgPalette_LoadRange((u8 *)sKonamiLogoBgPalette, 0, 1, 0);
            BgMap_Setup(0, (struct BgMapMetadata *)sKonamiLogoBgMap, 0, 0);
            gEwramData->gameModeUpdateStage += 1;
            gEwramData->gameModeSubStage = 0;
            break;

        case 2:
            if (gDisplayRegisters.bldY > 1)
            {
                gDisplayRegisters.bldY -= 1;
            }
            else
            {
                gDisplayRegisters.bldY = 0;
                gEwramData->gameModeTimer = 4 * 60; // Display for 4 seconds
                gEwramData->gameModeUpdateStage += 1;
                gEwramData->gameModeSubStage = 0;
            }
            break;

        case 3:
            KonamiCodeCheck();
            if ((--gEwramData->gameModeTimer == 0) || (gEwramData->inputData.newInput & (KEY_START | KEY_A)))
            {
                gDisplayRegisters.bldCnt = BLDCNT_SCREEN_FIRST_TARGET | BLDCNT_BRIGHTNESS_DECREASE_EFFECT;
                gEwramData->gameModeUpdateStage += 1;
                gEwramData->gameModeSubStage = 0;
            }
            break;
    
        case 4:
            if (gDisplayRegisters.bldY < BLDY_MAX_VALUE)
            {
                gDisplayRegisters.bldY += 1;
            }
            else
            {
                if (gEwramData->konamiCodeInputsCounter == (ARRAY_SIZE(sKonamiCodeInputs) + 1))
                {
                    PlaySong(SE_428);
                    gEwramData->konamiCodeInputsCounter = UCHAR_MAX;
                }
                gameMode = GAME_MODE_TITLE_SCREEN;
            }
            break;
    }

    return gameMode;
}

/**
 * @brief 2B54 | Handle Licensed by Nintendo game mode
 * 
 * @return s32 Game mode, -1 is no change, -2 is next game mode, else the specified game mode
 */
s32 GameModeLicensedByNintendoUpdate(void)
{
    s32 gameMode;

    gameMode = GAME_MODE_SAME_MODE;

    switch (gEwramData->gameModeUpdateStage)
    {
        case 0:
            gUnk_03002CB0.dispCnt = DCNT_BG0;
            gDisplayRegisters.bgCnt[0] = CREATE_BGCNT(0, 28, BGCNT_LOW_MID_PRIORITY, BGCNT_SIZE_256x256);
            gDisplayRegisters.bldCnt = BLDCNT_SCREEN_FIRST_TARGET | BLDCNT_BRIGHTNESS_INCREASE_EFFECT;
            gDisplayRegisters.bldY = BLDY_MAX_VALUE;
            gEwramData->gameModeUpdateStage += 1;
            gEwramData->gameModeSubStage = 0;
            break;

        case 1:
            Gfx_CopyResource((u8 *)sLicensedByNintendoGfx, VRAM_BASE, 0);
            BgPalette_Load((u8 *)sLicensedByNintendoBgPalette);
            BgMap_Setup(0, (struct BgMapMetadata *)sLicensedByNintendoBgMap, 0, 0);
            gEwramData->gameModeUpdateStage += 1;
            gEwramData->gameModeSubStage = 0;
            break;

        case 2:
            if (gDisplayRegisters.bldY > 1)
            {
                gDisplayRegisters.bldY -= 2;
            }
            else
            {
                gDisplayRegisters.bldY = 0;
                gEwramData->gameModeTimer = 2 * 60; // Display for 2 seconds
                gEwramData->gameModeUpdateStage += 1;
                gEwramData->gameModeSubStage = 0;
            }
            break;

        case 3:
            if (--gEwramData->gameModeTimer == 0)
            {
                gDisplayRegisters.bldCnt = BLDCNT_SCREEN_FIRST_TARGET | BLDCNT_BRIGHTNESS_INCREASE_EFFECT;
                gEwramData->gameModeUpdateStage += 1;
                gEwramData->gameModeSubStage = 0;
            }
            break;

        case 4:
            if (gDisplayRegisters.bldY < BLDY_MAX_VALUE)
            {
                gDisplayRegisters.bldY += 2;
            }
            else
            {
                gameMode = GAME_MODE_KONAMI_LOGO;
            }
            break;
    }

    return gameMode;
}

/**
 * @brief 2C88 | Handle reset game mode
 * 
 * @return s32 Game mode, -1 is no change, -2 is next game mode, else the specified game mode
 */
s32 GameModeResetUpdate(void)
{
    s32 prevSelected;
    s32 selected;
    s32 gameMode;
    struct GameState *gameState;
    u32 first;
    u32 last;

    gameState = &gEwramData->gameState;
    gameMode = GAME_MODE_SAME_MODE;
    first = 1;
    last = 3;

    switch (gEwramData->gameModeUpdateStage)
    {
        case 0:
            if (gameState->language != 1)
            {
                gameState->language = 1;
                SaveData_SaveLanguage();
            }
            gameMode = GAME_MODE_LICENSED_BY_NINTENDO;
            break;
    
        case 1:
            gUnk_03002CB0.dispCnt = DCNT_BG0;
            gDisplayRegisters.bgCnt[0] = CREATE_BGCNT(0, 28, BGCNT_LOW_MID_PRIORITY, BGCNT_SIZE_256x256);
            gDisplayRegisters.bldCnt = BLDCNT_SCREEN_FIRST_TARGET | BLDCNT_BRIGHTNESS_INCREASE_EFFECT;
            gDisplayRegisters.bldY = BLDY_MAX_VALUE;
            DMA_FILL_32(3, 0, VRAM_BASE + 0xE000, 0x800);
            DMA_FILL_32(3, 0, VRAM_BASE + 0x0000, 0x2000);
            gEwramData->gameModeUpdateStage += 1;
            gEwramData->gameModeSubStage = 0;
            break;

        case 2:
            HudMap_WriteTiles(0xA, 5, 0xF020, 0xA);
            HudMap_WriteTiles(0xA, 7, 0xF040, 0xA);
            HudMap_WriteTiles(0xA, 9, 0xF060, 0xA);
            BgPalette_LoadRange((u8 *)sTextBgPalette, 0, 1, 0xF);
            Text_Initialize();
            Text_SetForegroundColor(8);
            Text_SetBackgroundColor(0);
            Text8_SetRegion(0, 0, 0x1E, 4);
            Text8_ClearRegion();
            Text8_SetCursor(2, 1);
            Text8_Print(sTextEntries[0xB4C], 0); // "English\n"
            Text8_SetCursor(2, 2);
            Text8_Print(sTextEntries[0xB4D], 0); // "Français\n"
            Text8_SetCursor(2, 3);
            Text8_Print(sTextEntries[0xB4E], 0); // "Deutsch\n"

            if ((gameState->language == 0) || (gameState->language > last))
            {
                gameState->language = first;
            }
            Text8_SetCursor(0, gameState->language);
            Text8_PutChar(0x3E);
            gEwramData->gameModeUpdateStage += 1;
            gEwramData->gameModeSubStage = 0;
            break;

        case 3:
            if (gDisplayRegisters.bldY > 1)
            {
                gDisplayRegisters.bldY -= 2;
            }
            else
            {
                gDisplayRegisters.bldY = 0;
                gEwramData->gameModeTimer = 2 * 60; // 2 second timer? Seems unused
                gEwramData->gameModeUpdateStage += 1;
                gEwramData->gameModeSubStage = 0;
            }
            break;

        case 4:
            selected = gameState->language;
            prevSelected = selected;
            if (gEwramData->inputData.newInput & KEY_A)
            {
                gDisplayRegisters.bldCnt = BLDCNT_SCREEN_FIRST_TARGET | BLDCNT_BRIGHTNESS_INCREASE_EFFECT;
                gEwramData->gameModeUpdateStage += 1;
                gEwramData->gameModeSubStage = 0;
            }
            else if (gEwramData->inputData.newInput & KEY_B)
            {
                selected = first;
            }
            else if (gEwramData->inputData.newInput & KEY_UP)
            {
                if (selected == first)
                {
                    selected = last;
                }
                else
                {
                    selected -= 1;
                }
            }
            else if (gEwramData->inputData.newInput & KEY_DOWN)
            {
                if (selected == last)
                {
                    selected = first;
                }
                else
                {
                    selected += 1;
                }
            }

            if (prevSelected != selected)
            {
                gameState->language = selected;
                Text8_SetCursor(0, prevSelected);
                Text8_PutChar(0x20);
                Text8_PutChar(0x20);
                Text8_SetCursor(0, selected);
                Text8_PutChar(0x3E);
            }
            break;

        case 5:
            if (gDisplayRegisters.bldY < BLDY_MAX_VALUE)
            {
                gDisplayRegisters.bldY += 2;
            }
            else
            {
                SaveData_SaveLanguage();
                gameMode = GAME_MODE_LICENSED_BY_NINTENDO;
            }
            break;
    }
    return gameMode;
}

/**
 * @brief 2F44 | Fill the back H-blank buffer with BG1 horizontal offsets
 * for the top 3/4 of the screen: a sine wave of the given amplitude that
 * advances with the frame counter, odd lines mirrored (logo ripple; 0 = flat).
 *
 * @param amplitude Wave amplitude (16.16 scale of Sin >> 8)
 */
void TitleScreen_SetLogoWave(s32 amplitude)
{
    s16 *hBlankBuf;
    s32 angle;
    s32 scanline;
    u16 offset;
    u16 value;

    hBlankBuf = gEwramData->hBlankEffect.hBlankBuffer[1 - gEwramData->hBlankEffect.currentBuffer];
    angle = gEwramData->frameCounter << 10;
    HBlankEffectSetupWide(0, (s32)(SCREEN_SIZE_Y * 0.75f), 2, REG_BG1HOFS);
    
    for (scanline = 0; scanline < (s32)(SCREEN_SIZE_Y * 0.75f); scanline++)
    {
        if ((amplitude * (Sin(angle) >> 8)) < 0)
        {
            offset = -(-(amplitude * (Sin(angle) >> 8)) >> 0x10);
        }
        else
        {
            offset = (amplitude * (Sin(angle) >> 8)) >> 0x10;
        }

        value = offset;
        if (scanline & 1)
        {
            value = -(value << 0x10) >> 0x10;
        }
        *hBlankBuf = value;
        angle += 0x1000;
        hBlankBuf += 1;
    }

    gEwramData->hBlankEffect.requestStart = 1;
}

struct TitlePaletteBlendStep {
    u16* paletteResource; // Type?
    u16 sourcePalette;
    u16 duration;
};

struct TitlePaletteBlendStep sTitlePaletteBlendSteps[12] = {
    [0] = {
        .paletteResource = (void*)sTitleLogoBlendPalette,
        .sourcePalette = 0x0,
        .duration = 0x30
    },
    [1] = {
        .paletteResource = (void*)sTitleLogoBlendPalette,
        .sourcePalette = 0x5,
        .duration = 0x30
    },
    [2] = {
        .paletteResource = (void*)sTitleLogoBlendPalette,
        .sourcePalette = 0x6,
        .duration = 0x30
    },
    [3] = {
        .paletteResource = (void*)sTitleLogoBlendPalette,
        .sourcePalette = 0x7,
        .duration = 0x30
    },
    [4] = {
        .paletteResource = (void*)sTitleLogoBlendPalette,
        .sourcePalette = 0x8,
        .duration = 0x30
    },
    [5] = {
        .paletteResource = (void*)sTitleLogoBlendPalette,
        .sourcePalette = 0x9,
        .duration = 0x30
    },
    [6] = {
        .paletteResource = (void*)sTitleLogoBlendPalette,
        .sourcePalette = 0xA,
        .duration = 0x30
    },
    [7] = {
        .paletteResource = (void*)sTitleLogoBlendPalette,
        .sourcePalette = 0xB,
        .duration = 0x30
    },
    [8] = {
        .paletteResource = (void*)sTitleLogoBlendPalette,
        .sourcePalette = 0xC,
        .duration = 0x30
    },
    [9] = {
        .paletteResource = (void*)sTitleLogoBlendPalette,
        .sourcePalette = 0xD,
        .duration = 0x30
    },
    [10] = {
        .paletteResource = (void*)sTitleLogoBlendPalette,
        .sourcePalette = 0xE,
        .duration = 0x30
    },
    [11] = {
        .paletteResource = (void*)sTitleLogoBlendPalette,
        .sourcePalette = 0xF,
        .duration = 0x30
    },
};

/**
 * @brief 2FF8 | Title palette cycle: after the current step's duration load
 * its palette and start the blend towards the next entry of
 * sTitlePaletteBlendSteps (wrapping), timer and step kept in the logo entity.
 */
void TitleScreen_StepPaletteCycle(void)
{
    struct EwramData_EntityData *unk_4E4;
    struct TitlePaletteBlendStep *step;

    unk_4E4 = &gEwramData->entityData[0x12];
    step = &sTitlePaletteBlendSteps[unk_4E4->ext.titleLogo.paletteStep];

    if (unk_4E4->ext.titleLogo.paletteTimer++ > step->duration)
    {
        unk_4E4->ext.titleLogo.paletteTimer = 0;
        BgPalette_LoadRange((void*)step->paletteResource, step->sourcePalette, 1, 0);

        unk_4E4->ext.titleLogo.paletteStep += 1;
        if (unk_4E4->ext.titleLogo.paletteStep >= ARRAY_SIZE(sTitlePaletteBlendSteps))
        {
            unk_4E4->ext.titleLogo.paletteStep = 0;
        }

        step = &sTitlePaletteBlendSteps[unk_4E4->ext.titleLogo.paletteStep];
        BgPalette_StartBlend(0, (void*)step->paletteResource, step->sourcePalette, 0x10000, 0, step->duration);
    }
}

/**
 * @brief 3080 | Leave the title screen: stop the H-blank effect and palette
 * blend, free the logo graphics, delete all entities, reset the sprite,
 * palette, text and hit-box systems, clear two BG maps and set all BGCNTs.
 */
void TitleScreen_Shutdown(void)
{
    gEwramData->hBlankEffect.requestStop = 1;
    BgPalette_StopBlend(0);
    TitleScreen_FreeGraphics(&gEwramData->entityData[0x12]);
    EntityDeleteAll();
    ObjTiles_Init();
    Obj_Init();
    Sprite_Init();
    Palette_Init();
    Text_Initialize();
    HitBox_ClearAll();

    DMA_FILL_32(3, 0, VRAM_BASE + 0xE800, 0x800);
    DMA_FILL_32(3, 0, VRAM_BASE + 0xF000, 0x800);

    gDisplayRegisters.bgCnt[0] = CREATE_BGCNT(0, 28, BGCNT_LOW_MID_PRIORITY, BGCNT_SIZE_256x256);
    gDisplayRegisters.bgCnt[1] = CREATE_BGCNT(0, 29, BGCNT_LOW_MID_PRIORITY, BGCNT_SIZE_256x256);
    gDisplayRegisters.bgCnt[2] = CREATE_BGCNT(0, 30, BGCNT_LOW_MID_PRIORITY, BGCNT_SIZE_256x256);
    gDisplayRegisters.bgCnt[3] = CREATE_BGCNT(0, 31, BGCNT_LOW_MID_PRIORITY, BGCNT_SIZE_256x256);
}

u8 sTitleScreenNextGameMode[] = {
    GAME_MODE_MAIN_MENU,
    GAME_MODE_SOUL_TRADE_MENU,
    GAME_MODE_KONAMI_LOGO,
    GAME_MODE_BOSS_RUSH_MENU,
    GAME_MODE_SOUND_TEST_MENU,
};

static inline void TitleScreenSetAffine_inline(s16 scale)
{
    struct BgAffineSrcData src;

    src.texX = 0x8000;
    src.texY = 0x8000;
    src.scrX = SCREEN_SIZE_X / 2;
    src.scrY = SCREEN_SIZE_Y / 2;
    src.sx = scale;
    src.sy = scale;
    src.alpha = gEwramData->frameCounter * 0x10;
    BgAffineSet(&src, (struct BgAffineDstData *) &gDisplayRegisters.bg2PA, 1);
}

/* Per-frame update of the title logo (palette cycling). */
static inline void TitleScreenStep(struct EwramData_EntityData *logo)
{
    TitleScreen_StepPaletteCycle();
}
// Title screen game mode (upstream fakematch https://decomp.me/scratch/s3HhS)
s32 GameModeTitleScreenUpdate(void)
{
    s32 fade;
    s32 alpha;
    struct EwramData_EntityData *menu;
    struct EwramData_EntityData *logo;
    struct EwramData_EntityData *overlay;
    s32 gameMode;
    u16 tmp;
    u16 tmp2;

    logo = &gEwramData->entityData[0x12];
    overlay = &gEwramData->entityData[0x13];
    menu = &gEwramData->entityData[0x14];
    gameMode = GAME_MODE_SAME_MODE;

    switch (gEwramData->gameModeUpdateStage)
    {
        case 0:
            gUnk_03002CB0.dispCnt = 0;
            gDisplayRegisters.bgOfs[0].hOfs = 0;
            gDisplayRegisters.bgOfs[0].vOfs = 0;
            gDisplayRegisters.bgOfs[1].hOfs = 0;
            gDisplayRegisters.bgOfs[1].vOfs = 0;
            gDisplayRegisters.bgOfs[2].hOfs = 0;
            gDisplayRegisters.bgOfs[2].vOfs = 0;
            gDisplayRegisters.bldCnt = BLDCNT_SCREEN_FIRST_TARGET | BLDCNT_BRIGHTNESS_DECREASE_EFFECT;
            gDisplayRegisters.bldY = BLDY_MAX_VALUE;
            tmp = C_16_2_8(BLDALPHA_MAX_VALUE, 0);
            gDisplayRegisters.bldAlpha = tmp;

            gEwramData->gameModeCounter = 0;
            gEwramData->gameModeTimer = 0;
            gEwramData->startPauseEnabled = 0;
            gEwramData->paused = 0;
            gEwramData->gameModeUpdateStage = 1;
            gEwramData->gameModeSubStage = 0;
            break;
    
        case 1:
            PlaySong(MUS_27);
            EntityDeleteAll();
            ObjTiles_Init();
            Obj_Init();
            Sprite_Init();
            Palette_Init();
            Text_Initialize();
            HitBox_ClearAll();
            ObjPalette_StopColorFade();
            BgPalette_StopColorFade();

            gUnk_03002CB0.dispCnt = DCNT_WINOBJ | DCNT_OBJ | DCNT_BG2 | DCNT_MODE_1;
            gDisplayRegisters.bgCnt[0] = CREATE_BGCNT(2, 28, BGCNT_HIGH_PRIORITY, BGCNT_SIZE_256x256);
            gDisplayRegisters.bgCnt[1] = CREATE_BGCNT(2, 29, BGCNT_HIGH_MID_PRIORITY, BGCNT_SIZE_256x256);
            gDisplayRegisters.bgCnt[2] = CREATE_BGCNT(0, 30, BGCNT_LOW_MID_PRIORITY, BGCNT_SIZE_512x256) | BGCNT_256_COLOR;

            Obj_SetBgLayer(7, 0);
            Obj_SetBgLayer(6, 1);
            Obj_SetBgLayer(0x16, 2);
            Bg_SyncPriorities();
            Gfx_CopyResource((u8 *)sTitleScreenGfx2, VRAM_BASE, 0);
            Gfx_CopyResource((u8 *)sTitleScreenGfx3, VRAM_BASE + 0x8000, 0);
            BgPalette_Load((u8 *)sTitleScreenBgPalette);
            BgMap_Setup(0, (struct BgMapMetadata *)sTitleScreenBgMap2, 0, 0);
            BgMap_Setup(1, (struct BgMapMetadata *)sTitleScreenBgMap3, 0, 0);
            BgMap_Setup(2, (struct BgMapMetadata *)sTitleScreenBgMap, 0, 0);
            TitleScreenLogo_Create(logo);
            TitleScreen_CreateEffects(overlay);

            logo->posX.whole = 0x980000;
            logo->posY.whole = 0x700000;
            TitleScreenSetAffine_inline(0x108 - logo->ext.titleLogo.distance / 0x8000);

            gEwramData->gameModeUpdateStage = 2;
            gEwramData->gameModeSubStage = 0;
            /* fallthrough */
        case 2:
            TitleScreenStep(&gEwramData->entityData[0x12]);

            gDisplayRegisters.bldY = BLDY_MAX_VALUE - (gEwramData->gameModeTimer >> 1);
            if (gDisplayRegisters.bldY > 0)
            {
                gEwramData->gameModeTimer += 1;
            }
            else
            {
                gDisplayRegisters.bldCnt = BLDCNT_SCREEN_SECOND_TARGET | BLDCNT_ALPHA_BLENDING_EFFECT;
                gEwramData->gameModeUpdateStage = 3;
                gEwramData->gameModeSubStage = 0;
            }
            break;
    
        case 3:
            TitleScreenStep(&gEwramData->entityData[0x12]);

            if (logo->ext.titleLogo.distance != 0)
            {
                logo->ext.titleLogo.distance -= 0x2000;
                logo->posX.whole -= 0x2000;
                logo->posY.whole -= 0x2000;

                if (logo->ext.titleLogo.distance == 0x40000)
                {
                    gDisplayRegisters.bldY = BLDY_MAX_VALUE;
                    TitleScreen_SpawnFlash(0, 0x18, 0xEF);
                    TitleScreen_SpawnFlash(1, 0x20, 0x89);
                }

                if (logo->ext.titleLogo.distance < 0x30000)
                {
                    logo->ext.titleLogo.flags |= 1;
                    gEwramData->gameModeUpdateStage = 4;
                    gEwramData->gameModeSubStage = 0;
                }

                TitleScreenSetAffine_inline(0x108 - logo->ext.titleLogo.distance / 0x8000);
            }
            break;
    
        case 4:
            TitleScreenStep(&gEwramData->entityData[0x12]);

            if (logo->ext.titleLogo.distance != 0)
            {
                logo->ext.titleLogo.distance -= 0x2000;
                logo->posX.whole -= 0x2000;
                logo->posY.whole -= 0x2000;

                TitleScreenSetAffine_inline(0x108 - logo->ext.titleLogo.distance / 0x8000);
            }

            if ((logo->ext.titleLogo.flags & 2))
            {
                gEwramData->gameModeUpdateStage = 5;
                gEwramData->gameModeSubStage = 0;

            case 5:
                TitleScreenStep(&gEwramData->entityData[0x12]);
                TitleScreenSetAffine_inline(0x118 - logo->ext.titleLogo.zoomStep);
                if (logo->ext.titleLogo.flags & 2)
                {
                    logo->objMode = 2;
                }

                if (logo->ext.titleLogo.zoomStep < 0x40)
                {
                    logo->ext.titleLogo.zoomStep += 1;
                }
                else
                {
                    overlay->ext.titleEffects.fadeTimer = 0x88;
                    gDisplayRegisters.bldAlpha = C_16_2_8(BLDALPHA_MAX_VALUE, 0);
                    overlay->ext.titleEffects.flags |= 1;
                    gEwramData->gameModeUpdateStage = 6;
                    gEwramData->gameModeSubStage = 0;
                }
            }
            break;
    
        case 6:
            TitleScreenStep(&gEwramData->entityData[0x12]);

            logo->ext.titleLogo.wobblePhase += 0x40;
            TitleScreenSetAffine_inline(Sin(logo->ext.titleLogo.wobblePhase & 0x7FFF) / 0x1000 - (logo->ext.titleLogo.zoomStep - 0x118));

            if (overlay->ext.titleEffects.fadeTimer != 0)
            {
                overlay->ext.titleEffects.fadeTimer--;
                alpha = overlay->ext.titleEffects.fadeTimer / 8;
                gDisplayRegisters.bldAlpha = C_16_2_8(alpha, BLDALPHA_MAX_VALUE - alpha);
            }
            else
            {
                overlay->ext.titleEffects.flags |= 4;
                gDisplayRegisters.bldAlpha = C_16_2_8(BLDALPHA_MAX_VALUE, 0);
                gDisplayRegisters.bldCnt = BLDCNT_SCREEN_SECOND_TARGET | BLDCNT_ALPHA_BLENDING_EFFECT | BLDCNT_BG1_FIRST_TARGET_PIXEL;
                gUnk_03002CB0.dispCnt |= DCNT_BG1;
                gEwramData->gameModeUpdateStage = 7;
                gEwramData->gameModeSubStage = 0;
            }
            break;
    
        case 7:
            TitleScreenStep(&gEwramData->entityData[0x12]);

            logo->ext.titleLogo.wobblePhase += 0x40;
            TitleScreenSetAffine_inline(Sin(logo->ext.titleLogo.wobblePhase & 0x7FFF) / 0x1000 - (logo->ext.titleLogo.zoomStep - 0x118));

            if (logo->ext.titleLogo.waveAmplitude != 0)
            {
                if (!(overlay->ext.titleEffects.flags & 2) && (logo->ext.titleLogo.waveAmplitude <= 0x37F))
                {
                    overlay->ext.titleEffects.flags |= 2;
                }

                logo->ext.titleLogo.waveAmplitude -= 2 + logo->ext.titleLogo.waveAmplitude / 256 * 256 / 80;

                if (logo->ext.titleLogo.waveAmplitude < 0xC0)
                {
                    logo->ext.titleLogo.waveAmplitude = 0;
                }
                fade = logo->ext.titleLogo.waveAmplitude / 1280;
                gDisplayRegisters.bldAlpha = C_16_2_8(fade, BLDALPHA_MAX_VALUE - fade);
                TitleScreen_SetLogoWave(logo->ext.titleLogo.waveAmplitude);
            }
            else
            {
                EntityDeleteGroup(1);
                gUnk_03002CB0.dispCnt |= DCNT_BG0;
                logo->hidden = 1;
                logo->objMode = 0;
                TitleScreenMenu_Create();
                TitleScreenFrame_Create();
                BgPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0, 0x10000, 0x20);
                ObjPalette_StartColorFade(0x1F, 0x12, 0x12, 0x2000, 0x10000, 0x20);
                gEwramData->gameModeTimer = 0x258;
                gEwramData->gameModeUpdateStage = 8;
                gEwramData->gameModeSubStage = 0;
            }
            break;
    
        case 8:
            gDisplayRegisters.bldY = BLDY_MAX_VALUE;
            TitleScreenStep(&gEwramData->entityData[0x12]);

            logo->ext.titleLogo.wobblePhase += 0x40;
            TitleScreenSetAffine_inline(Sin(logo->ext.titleLogo.wobblePhase & 0x7FFF) / 0x1000 - (logo->ext.titleLogo.zoomStep - 0x118));

            if (gEwramData->inputData.repeatedInput & (KEY_DOWN | KEY_UP))
            {
                gEwramData->gameModeTimer = 10 * 60; // 10 second timer
            }

            if (--gEwramData->gameModeTimer == 0)
            {
                gEwramData->gameModeCounter = 1;
            }

            if (gEwramData->inputData.newInput & (KEY_START | KEY_A))
            {
                if (sTitleScreenNextGameMode[menu->ext.titleMenu.cursor + menu->ext.titleMenu.first] == GAME_MODE_KONAMI_LOGO)
                {
                    TitleScreenMenu_StartSlide(menu);
                    break;
                }
            }
            else if ((gEwramData->inputData.newInput & KEY_B) && (menu->ext.titleMenu.first != 0))
            {
                TitleScreenMenu_StartSlide(menu);
                break;
            }

            if ((menu->ext.titleMenu.sliding == 0) && ((gEwramData->gameModeCounter != 0) || (gEwramData->inputData.newInput & (KEY_START | KEY_A))))
            {
                PlaySong(SONG_MUTE);
                menu->ext.titleMenu.confirmed = 1;
                gEwramData->gameModeUpdateStage = 9;
                gEwramData->gameModeSubStage = 0;
                gEwramData->gameModeTimer = 0;
                gDisplayRegisters.bldCnt = BLDCNT_SCREEN_FIRST_TARGET | BLDCNT_BRIGHTNESS_DECREASE_EFFECT;
                gDisplayRegisters.bldY = 0;
            }
            break;
    
        case 9:
            TitleScreenStep(&gEwramData->entityData[0x12]);

            gDisplayRegisters.bldY = gEwramData->gameModeTimer >> 1;
            if (gDisplayRegisters.bldY == BLDY_MAX_VALUE)
            {
                gameMode = sTitleScreenNextGameMode[menu->ext.titleMenu.cursor + menu->ext.titleMenu.first];
                TitleScreen_Shutdown();
                gDisplayRegisters.bldY = BLDY_MAX_VALUE;
                gEwramData->gameModeUpdateStage = 0;
                if (gEwramData->gameModeCounter != 0)
                {
                    gameMode = GAME_MODE_INTRO_CUTSCENE;
                }
                return gameMode;
            }
            else
            {
                gEwramData->gameModeTimer += 1;
            }
            break;
    }

    if ((gEwramData->gameModeUpdateStage >= 2 && gEwramData->gameModeUpdateStage <= 7) && (gEwramData->inputData.newInput & (KEY_START | KEY_A)))
    {
        gDisplayRegisters.bldCnt = BLDCNT_OBJ_SECOND_TARGET_PIXEL | BLDCNT_BG2_SECOND_TARGET_PIXEL | BLDCNT_ALPHA_BLENDING_EFFECT | BLDCNT_BG1_FIRST_TARGET_PIXEL;
        gUnk_03002CB0.dispCnt |= DCNT_BG1 | DCNT_BG0;
        logo->posX.part16.integer = 0x78;
        logo->posY.part16.integer = 0x50;
        logo->ext.titleLogo.distance = 0;
        logo->ext.titleLogo.zoomStep = 0x40;
        tmp2 = C_16_2_8(0, BLDALPHA_MAX_VALUE);
        gDisplayRegisters.bldAlpha = tmp2;

        TitleScreen_SetLogoWave(0);
        EntityDeleteGroup(1);
        logo->hidden = 1;
        logo->objMode = 0;
        TitleScreenMenu_Create();
        TitleScreenFrame_Create();
        BgPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0, 0x10000, 0x20);
        ObjPalette_StartColorFade(0x1F, 0x17, 0x17, 0x2000, 0x10000, 0x20);
        gEwramData->gameModeTimer = 10 * 60; // 10 second timer
        gEwramData->gameModeUpdateStage = 8;
        gEwramData->gameModeSubStage = 0;
    }
    Entity_ScrollWithBg(1);
    Entity_UpdateAll();
    
    return gameMode;
}

/**
 * @brief Drift a title particle with its velocity and a slow acceleration
 * towards the screen.
 */
void TitleScreenParticle_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *logo;
    s32 waveY;
    s32 waveX;
    s32 y;

    logo = &gEwramData->entityData[0x13];
    if (entity->state == 0 && !(logo->ext.titleEffects.flags & 1))
        return;

    else if (entity->state == 0 && (logo->ext.titleEffects.flags & 1))
    {
        entity->state = 1;
        entity->hidden = 0;
    }

    if ((entity->objMode != 0) && (logo->ext.titleEffects.flags & 4))
    {
        entity->objMode = 0;
    }

    entity->ext.titleParticle.phaseX += entity->velX.part16.integer;
    entity->ext.titleParticle.phaseY += entity->velY.part16.integer;

    waveX = entity->accelX.whole * Sin(entity->ext.titleParticle.phaseX + 0x4000);
    if (waveX < 0)
    {
        waveX += 0xFFF;
    }
    entity->posX.whole += waveX >> 0xC;

    waveY = entity->accelY.whole * Sin(entity->ext.titleParticle.phaseY);
    if (waveY < 0)
    {
        waveY += 0xFFF;
    }
    y = entity->posY.whole + (waveY >> 0xC);
    entity->posX.whole += entity->ext.titleParticle.driftX;
    entity->posY.whole = y + entity->ext.titleParticle.driftY;

    Anim_Update(entity);
}

// TODO: combine/figure out inlines
static inline TitleObject_LoadGraphics(u8 *ptr0, u8 *ptr1, struct EwramData_EntityData *entity, s32 palette)
{
    s32 gfx;
    u8 loaded;
    s32 none;

    none = 0;
    gfx = ObjGfx_LoadResource(ptr0, 0);
    if (gfx < 0)
    {
        loaded = 0;
    }
    else
    {
        entity->basePalette = palette;
        Sprite_Attach(entity, ptr1, gfx, palette);
        entity->drawFunc = Sprite_DrawEntity;
        loaded = entity->group = 1;
    }
    return loaded != none;
}

static inline TitleObject_LoadGraphicsAlt(u8 *ptr0, u8 *ptr1, struct EwramData_EntityData *entity, s32 palette)
{
    s32 gfx;
    u8 loaded;

    gfx = ObjGfx_LoadResource(ptr0, 0);
    if (gfx < 0)
    {
        loaded = 0;
    }
    else
    {
        entity->basePalette = palette;
        Sprite_Attach(entity, ptr1, gfx, palette);
        entity->drawFunc = Sprite_DrawEntity;
        loaded = entity->group = 1;
    }
    return loaded != 0;
}

/**
 * @brief Attach the particle sprite (random animation phase, hidden, layer 7)
 * and give it an outward start offset, an inward velocity and random
 * motion parameters from its angle to the screen centre.
 */
void TitleScreenParticle_Init(struct EwramData_EntityData *entity)
{
    s16 angle;
    s32 driftX;
    s32 driftY;
    u8* tmp;
    u8* tmp2;
    s32 loaded;

    loaded = TitleObject_LoadGraphics((u8*)&sTitleScreenGfx, (u8*)&sTitleScreenSpriteSheet, entity, 0xD);

    if (!loaded)
    {
        EntityDelete(entity);
        return;
    }

    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    entity->animStep = RandomNumberGenerator() & 3;
    entity->animTimer = RandomNumberGenerator() & 3;

    entity->objMode = 1;
    entity->hidden = 1;
    entity->scrollWithBg = 1;
    entity->spriteLayer = 7;

    entity->accelX.whole = ((RandomNumberGenerator() & 0xFF) * 0x10) + 0x400;
    entity->accelY.whole = ((RandomNumberGenerator() & 0xFF) * 0x10) + 0x400;

    angle = ArcTan2(entity->posX.part16.integer - 0x78, entity->posY.part16.integer - 0x60);
    entity->ext.titleParticle.driftX = Sin(angle + 0x4000);
    entity->ext.titleParticle.driftY = Sin(angle);
    entity->posX.whole += entity->ext.titleParticle.driftX << 5;
    entity->posY.whole += entity->ext.titleParticle.driftY << 5;

    driftX = -entity->ext.titleParticle.driftX;
    if (driftX < 0)
    {
        driftX += 0xF;
    }
    entity->ext.titleParticle.driftX = driftX >> 4;

    driftY = -entity->ext.titleParticle.driftY;
    if (driftY < 0)
    {
        driftY += 0xF;
    }
    entity->ext.titleParticle.driftY = driftY >> 4;

    entity->ext.titleParticle.phaseX = (RandomNumberGenerator() & 0xFF) << 8;
    entity->ext.titleParticle.phaseY = entity->ext.titleParticle.phaseX + 0x8000 + ((RandomNumberGenerator() & 0x1F) << 4);
    entity->velX.part16.integer = (RandomNumberGenerator() & 0xFF) + 0x800;
    entity->velY.part16.integer = (RandomNumberGenerator() & 0xFF) + 0xA00;
}

/**
 * @brief Once the overlay sets flag 2, show the sprite and spiral it in
 * towards (0x7A, 0x1A) while its affine scale shrinks.
 */
void TitleScreenSpiralSprite_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *unk_4E4;
    s16 angle;
    s32 shrunk;
    s32 radius;
    s32 decay;
    s32 cosA;
    s32 radiusX;
    s32 sinA;
    s32 radiusY;

    unk_4E4 = &gEwramData->entityData[0x13];
    angle = entity->ext.titleSpiral.angle;
    switch (entity->state)
    {
        case 0:
            if (unk_4E4->ext.titleEffects.flags & 2)
            {
                entity->hidden = 0;
                entity->state = 1;
        case 1:
                if (entity->ext.titleSpiral.radius != 0)
                {
                    entity->ext.titleSpiral.angle -= 0xA0;
                    shrunk = entity->ext.titleSpiral.radius - 0x8000;
                    decay = entity->ext.titleSpiral.radius;
                    if (decay < 0)
                    {
                        decay += 0x3F;
                    }
                    radius = shrunk - (decay >> 6);
                    entity->ext.titleSpiral.radius = radius;
                    if (radius <= 0xFFF)
                    {
                        entity->ext.titleSpiral.radius = 0;
                    }
                }
                entity->ext.titleSpiral.scale = (entity->ext.titleSpiral.radius / 160) + 0x10000;
                // inlines?
                cosA = Sin(angle + 0x4000);
                radiusX = entity->ext.titleSpiral.radius;
                entity->posX.whole = ((cosA >> 8) * (radiusX >> 8)) + 0x7A0000;
                sinA = Sin(angle);
                radiusY = entity->ext.titleSpiral.radius;
                entity->posY.whole = ((sinA >> 8) * (radiusY >> 8)) + 0x1A0000;
                Anim_Update(entity);
                ObjAffine_Set(0, entity->ext.titleSpiral.scale, entity->ext.titleSpiral.scale, 0, 0, entity->affineIndex);
            }
        }
}

/**
 * @brief Attach the hidden affine sprite (animation 4, layer 6) with its
 * start angle and radius.
 */
void TitleScreenSpiralSprite_Init(struct EwramData_EntityData *entity)
{
    u16 affineIndex;
    u8* tmp;
    u8* tmp2;
    u8 loaded;

    loaded = TitleObject_LoadGraphics((u8 *)&sTitleScreenGfx, (u8 *)&sTitleScreenSpriteSheet, entity, 0xC);

    if (!loaded || ((s32) ((affineIndex = ObjAffine_Alloc(1)) << 0x10) < 0))
    {
        EntityDelete(entity);
        return;
    }

    entity->doubleSize = 1;
    entity->affineIndex = affineIndex;
    entity->affine = 1;
    Anim_StartIndexed(entity, 4, 0, 1);
    Anim_Update(entity);
    entity->hidden = 1;
    entity->scrollWithBg = 1;
    entity->spriteLayer = 6;
    entity->ext.titleSpiral.angle = 0x9800;
    entity->ext.titleSpiral.radius = 0xC00000;
    entity->ext.titleSpiral.unused = 0;
}

/**
 * @brief Turn the overlay entity into the effect group: load the title
 * graphics, create the spiral sprite and 34 particles.
 */
void TitleScreen_CreateEffects(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child;
    s32 i;

    EntityDelete(entity);
    ObjGfx_LoadResource((u8 *)&sTitleScreenGfx, 0);
    ObjGfx_LoadResource((u8 *)&sTitleFlashGfx, 0);
    ObjGfx_LoadResource((u8 *)&sTitleMenuGfx, 0);
    entity->updateFunc = TitleScreenEffects_Update;
    entity->group = 1;
    child = EntityCreateInRange(0x49, 0xCF, TitleScreenSpiralSprite_Update);
    if (child != 0)
    {
        TitleScreenSpiralSprite_Init(child);
    }

    for (i = 0; i < 0x22 && (child = EntityCreateInRange(0x49, 0xCF, TitleScreenParticle_Update)) != 0; i++)
    {
        child->posX.whole = (i * 0x3C3C3) + 0x380000 + (RandomSigned(0x10) << 0x10);
        child->posY.part16.integer = (RandomSigned(0x20) + 0x60);
        TitleScreenParticle_Init(child);
    }
}

/**
 * @brief Rotate and scale the flash sprite; the first flash also fades the
 * blend alpha. Delete it when its timer runs out.
 */
void TitleScreenFlash_Update(struct EwramData_EntityData *entity)
{
    s32 alpha;

    switch (entity->state)
    {
        case 0:
            entity->state = 1;
            entity->ext.titleFlash.scale = 0x20000;
            entity->ext.titleFlash.life = 0x8000;
            /* fallthrough */
        case 1:
            entity->ext.titleFlash.scale = entity->ext.titleFlash.size << 0xC;
            if (entity->ext.titleFlash.noBlend == 0)
            {
                alpha = 0x10 - (Sin(entity->ext.titleFlash.life) >> 0xC);
                gDisplayRegisters.bldAlpha = C_16_2_8(alpha, BLDALPHA_MAX_VALUE - alpha);
            }

            entity->ext.titleFlash.angle += entity->ext.titleFlash.spin;
            Anim_Update(entity);
            ObjAffine_Set((s32) entity->ext.titleFlash.angle, entity->ext.titleFlash.scale, entity->ext.titleFlash.scale, 0, 0, entity->affineIndex);

            if (entity->ext.titleFlash.life > 0)
            {
                entity->ext.titleFlash.life -= 0x180;
            }
            else
            {
                entity->deleteRequested = 1;
            }
    }
}

/**
 * @brief Create a semi-transparent rotating flash sprite at (0x4E, 0x26) with
 * the given blend flag, scale and rotation speed.
 */
void TitleScreen_SpawnFlash(s32 skipBlend, s32 scale, u16 spin)
{
    struct EwramData_EntityData *child;
    u16 affineIndex;
    u8* tmp;
    u8* tmp2;
    u8 loaded;

    child = EntityCreateInRange(0x49, 0xCF, TitleScreenFlash_Update);
    if (child == 0)
        return;

    loaded = TitleObject_LoadGraphics((u8 *)&sTitleFlashGfx, (u8 *)&sTitleFlashSpriteSheet, child, 0xB);

    if (!loaded || ((s32) ((affineIndex = ObjAffine_Alloc(1)) << 0x10) < 0))
    {
        EntityDelete((struct EwramData_EntityData *) child);
        return;
    }

    child->doubleSize = 1;
    child->affineIndex = affineIndex;
    child->affine = 1;
    Anim_StartIndexed(child, 4, 0, 0);
    Anim_Update(child);
    child->scrollWithBg = 1;
    child->spriteLayer = 6;
    child->ext.titleFlash.noBlend = skipBlend;
    child->ext.titleFlash.size = scale;
    child->ext.titleFlash.spin = spin;
    child->objMode = 1;
    child->posX.part16.integer = 0x4E;
    child->posY.part16.integer = 0x26;
}

/**
 * @brief After the logo start flag, cycle the logo palettes 4..7 through the
 * fade-in ramp, then set flag 2.
 */
void TitleScreenLogo_Update(struct EwramData_EntityData *entity)
{
    u8 tick;
    u8 tick3;
    u8 tick2;

    if (entity->hidden)
    {
        entity->state = 0x63;
    }

    switch (entity->state)
    {
        case 0:
            if (!(entity->ext.titleLogo.flags & 1))
            {
                break;
            }
            entity->state = 1;
            /* fallthrough */
        case 1:
            tick = entity->timer;
            if (tick == 0)
            {
                ObjPalette_LoadRange((u8 *)sTitleScreenLogoPalette, entity->subState + 3, 1, 4);
                entity->subState += 1;
                entity->timer = 1;
                if (entity->subState > 0xE)
                {
                    entity->subState = tick;
                    entity->timer = tick;
                    entity->state = 2;
                    break;
                }
            }
            entity->timer -= 1;
            break;
        case 2:
            tick2 = entity->timer;
            if (tick2 == 0)
            {
                ObjPalette_LoadRange((u8 *)sTitleScreenLogoPalette, entity->subState + 0x12, 1, 5);
                ObjPalette_LoadRange((u8 *)sTitleScreenLogoPalette, entity->subState + 3, 1, 6);
                entity->subState += 1;
                entity->timer = 1;
                if (entity->subState > 0xE)
                {
                    entity->subState = tick2;
                    entity->timer = tick2;
                    entity->state = 3;
                    break;
                }
            }
            entity->timer -= 1;
            break;

        case 3:
            tick3 = entity->timer;
            if (tick3 == 0)
            {
                ObjPalette_LoadRange((u8 *)sTitleScreenLogoPalette, entity->subState + 0x12, 1, 7);
                entity->subState += 1;
                entity->timer = 1;
                if (entity->subState > 0xE)
                {
                    entity->subState = tick3;
                    entity->timer = tick3;
                    entity->ext.titleLogo.flags |= 2;
                    entity->state = 4;
                    break;
                }
            }
            entity->timer -= 1;
            break;

        case 4:
            if (entity->timer != 0)
            {
                entity->timer -= 1;
                break;
            }
            ObjPalette_LoadRange((u8 *)sTitleScreenLogoPalette, 3, 1, 4);
            ObjPalette_LoadRange((u8 *)sTitleScreenLogoPalette, 3, 1, 5);
            ObjPalette_LoadRange((u8 *)sTitleScreenLogoPalette, 3, 1, 6);
            ObjPalette_LoadRange((u8 *)sTitleScreenLogoPalette, 3, 1, 7);
            entity->state = 5;
            break;
    }
}

/**
 * @brief Reset the object heap, load the logo graphics and palettes and set
 * the logo update/draw functions and start scale.
 */
void TitleScreenLogo_Create(struct EwramData_EntityData *entity)
{
    u8 result;
    u8* tmp;
    u8* tmp2;

    gEwramData->randomNumber = 0x1967;
    ObjTiles_Init();
    SharedObjPalette_ResetSlots();
    Heap_Reset();
    ObjPalette_LoadRange((u8 *)sTitleScreenLogoPalette, 3, 1, result = 4);
    entity->basePalette = result;

    result = TitleObject_LoadGraphics((u8*)&sTitleFlashGfx, (u8*)&sTitleFlashSpriteSheet, entity, 4);
    
    if (result == 0)
    {
        EntityDelete(entity);
        return;
    }

    entity->group = 0;
    ObjPalette_LoadRange((u8 *)sTitleScreenLogoPalette, 1, 1, 0xC);
    ObjPalette_LoadRange((u8 *)sTitleScreenLogoPalette, 0, 1, 0xD);
    ObjPalette_LoadRange((u8 *)sTitleScreenLogoPalette, 2, 1, 0xB);
    ObjPalette_LoadRange((u8 *)sTitleScreenLogoPalette, 3, 1, 5);
    ObjPalette_LoadRange((u8 *)sTitleScreenLogoPalette, 3, 1, 6);
    ObjPalette_LoadRange((u8 *)sTitleScreenLogoPalette, 3, 1, 7);
    entity->updateFunc = TitleScreenLogo_Update;
    entity->drawFunc = TitleScreenLogo_Draw;
    entity->scrollWithBg = 1;
    entity->spriteLayer = 0x11;
    entity->ext.titleLogo.distance = 0x200000;
    entity->ext.titleLogo.waveAmplitude = 0x5000;
    gDisplayRegisters.winOut_L = WIN0_ALL;
    gDisplayRegisters.winOut_H = (WIN1_ALL >> 8) & ~WIN0_BG2;
}

struct TitleSpritePart {
    u8 animFrame;
    u8 palette;
    s16 offsetX;
    s16 offsetY;
    u8 pad_6[0x8 - 0x6];
};

struct TitleSpritePart sTitleSpriteParts[8] = {
    [0] = {
        .animFrame = 0x0,
        .palette = 0x4,
        .offsetX = -32,
        .offsetY = -32
    },
    [1] = {
        .animFrame = 0x1,
        .palette = 0x5,
        .offsetX = 32,
        .offsetY = -32
    },
    [2] = {
        .animFrame = 0x2,
        .palette = 0x6,
        .offsetX = -32,
        .offsetY = 32
    },
    [3] = {
        .animFrame = 0x3,
        .palette = 0x7,
        .offsetX = 32,
        .offsetY = 32
    },
    [4] = {
        .animFrame = 0xC,
        .palette = 0xB,
        .offsetX = -32,
        .offsetY = -32
    },
    [5] = {
        .animFrame = 0xD,
        .palette = 0xB,
        .offsetX = 32,
        .offsetY = -32
    },
    [6] = {
        .animFrame = 0xE,
        .palette = 0xB,
        .offsetX = -32,
        .offsetY = 32
    },
    [7] = {
        .animFrame = 0xF,
        .palette = 0xB,
        .offsetX = 32,
        .offsetY = 32
    },
};

u8 sTitleMenuFrames[] = {
    0, 1, 2, 3, 4, 4,
    // padding?
    0, 0
};

/**
 * @brief Draw the visible menu entries, highlighting the selected one and
 * offsetting alternate rows by the slide distance.
 */
void TitleScreenMenu_Draw(struct EwramData_EntityData *entity)
{
    s32 end;
    s32 oldX;
    s32 oldY;
    s32 i;

    oldX = entity->posX.whole;
    oldY = entity->posY.whole;

    entity->posX.part16.integer = 0x78;
    entity->posY.part16.integer = 0x8C - (4 * entity->ext.titleMenu.count);

    end = entity->ext.titleMenu.first + entity->ext.titleMenu.count;

    for (i = entity->ext.titleMenu.first; i < end; i++)
    {
        entity->animFrame = sTitleMenuFrames[i];
        if ((entity->ext.titleMenu.cursor + entity->ext.titleMenu.first) == i)
        {
            entity->palette = 0xB;
        }
        else
        {
            entity->palette = 0xC;
        }
        entity->posX.whole = (oldX + entity->ext.titleMenu.slideOffset) - (2 * entity->ext.titleMenu.slideOffset * (i & 1));
        Sprite_Draw(entity);
        entity->posY.part16.integer += 9;
    }
    entity->posX.whole = oldX;
    entity->posY.whole = oldY;
}

/**
 * @brief Slide between the main and the extra entries and move the cursor
 * with Up/Down.
 */
void TitleScreenMenu_Update(struct EwramData_EntityData *entity)
{
    u8 busy;

    busy = entity->ext.titleMenu.confirmed;
    if (busy == 0)
    {
        switch (entity->ext.titleMenu.slidePhase)
        {
            case 1:
                entity->ext.titleMenu.slideOffset += 0x180000;
                if (entity->ext.titleMenu.slideOffset > 0x9FFFFF)
                {
                    if (entity->ext.titleMenu.first == 0)
                    {
                        entity->ext.titleMenu.first = 3;
                        entity->ext.titleMenu.count = entity->ext.titleMenu.extraCount;
                    }
                    else
                    {
                        entity->ext.titleMenu.first = busy;
                        entity->ext.titleMenu.count = 3;
                    }

                    entity->ext.titleMenu.cursor = 0;
                    entity->ext.titleMenu.slidePhase += 1;
                }
                break;
            case 2:
                entity->ext.titleMenu.slideOffset -= 0x100000;
                if (entity->ext.titleMenu.slideOffset <= 0)
                {
                    entity->ext.titleMenu.slideOffset = busy;
                    entity->ext.titleMenu.slidePhase = busy;
                    entity->ext.titleMenu.sliding = busy;
                }
                break;
        }

        if (gEwramData->inputData.repeatedInput & KEY_DOWN)
        {
            entity->ext.titleMenu.cursor = Mod(entity->ext.titleMenu.cursor + 1, entity->ext.titleMenu.count);
        }

        else if (gEwramData->inputData.repeatedInput & KEY_UP)
        {
            entity->ext.titleMenu.cursor -= 1;
            if (entity->ext.titleMenu.cursor < 0)
            {
                entity->ext.titleMenu.cursor = entity->ext.titleMenu.count - 1;
            }
        }
    }
}

/**
 * @brief Create the menu entity (slot 0x14) with the language-specific entry
 * graphics; two entries, or three with gameState.systemFlags bit 1.
 */
void TitleScreenMenu_Create(void)
{
    struct EwramData_EntityData *menu;
    u8 loaded;

    menu = &gEwramData->entityData[0x14];

    ObjPalette_LoadRange((u8 *)sTitleScreenLogoPalette, 3, 1, 4);
    menu->ext.titleMenu.first = 0;
    menu->ext.titleMenu.extraCount = 0;

    if (!(gEwramData->gameState.systemFlags & 2))
    {
        menu->ext.titleMenu.count = 2;
    }
    else
    {
        menu->ext.titleMenu.count = 3;
        menu->ext.titleMenu.extraCount = 2;
    }

    switch (gEwramData->gameState.language)
    {
        case 2:
            loaded = TitleObject_LoadGraphicsAlt((u8*)&sTitleMenuGfxLanguage2, (u8*)&sTitleMenuSpriteSheetLanguage2, menu, 0xB);
            break;

        case 3:
            loaded = TitleObject_LoadGraphicsAlt((u8*)&sTitleMenuGfxLanguage3, (u8*)&sTitleMenuSpriteSheetLanguage3, menu, 0xB);
            break;

        default:
            loaded = TitleObject_LoadGraphicsAlt((u8*)&sTitleMenuGfx, (u8*)&sTitleMenuSpriteSheet, menu, 0xB);
            break;
    }

    if (loaded == 0)
    {
        EntityDelete(menu);
        return;
    }

    menu->group = 0;
    menu->updateFunc = TitleScreenMenu_Update;
    menu->drawFunc = TitleScreenMenu_Draw;
    menu->scrollWithBg = 1;
    menu->spriteLayer = 0x11;
    menu->posX.part16.integer = 0x78;
    menu->posY.part16.integer = 0x50;
}

/**
 * @brief Draw the four frame parts around the centre and the bottom part.
 */
void TitleScreenFrame_Draw(struct EwramData_EntityData *entity)
{
    s32 oldX;
    s32 oldY;
    s32 baseX;
    s32 baseY;
    s32 i;
    struct TitleSpritePart *part;

    oldX = entity->posX.whole;
    oldY = entity->posY.whole;
    baseX = oldX;
    baseY = oldY;
    part = &sTitleSpriteParts[entity->stage];
    
    // for (i = 0; i < 4; i++)
    for (i = 3; i >= 0; i--)
    {
        entity->posX.whole = (part->offsetX << 0x10) + baseX;
        entity->posY.whole = (part->offsetY << 0x10) + baseY;
        entity->animFrame = part->animFrame;
        entity->palette = part->palette;
        Sprite_Draw(entity);
        part += 1;
    }
    entity->posX.whole = baseX;
    entity->posY.whole = baseY;
    entity->posX.part16.integer = SCREEN_SIZE_X / 2;
    entity->posY.part16.integer = SCREEN_SIZE_Y;
    entity->animFrame = 0xB;
    entity->palette = 0xB;
    Sprite_Draw(entity);
    entity->posX.whole = oldX;
    entity->posY.whole = oldY;
}

/**
 * @brief Create the frame entity (slot 0x15) drawn by TitleScreenFrame_Draw.
 */
void TitleScreenFrame_Create(void)
{
    s32 loaded;
    struct EwramData_EntityData *frame;

    frame = &gEwramData->entityData[0x15];

    loaded = TitleObject_LoadGraphics((u8*)&sTitleScreenGfx, (u8*)&sTitleScreenSpriteSheet, frame, 0xB);
    if (loaded == 0)
    {
        EntityDelete(frame);
        return;
    }

    frame->group = 0;
    frame->updateFunc = EntityUpdateNothing;
    frame->drawFunc = TitleScreenFrame_Draw;
    frame->scrollWithBg = 1;
    frame->spriteLayer = 0x12;
    frame->posX.part16.integer = 0x78;
    frame->posY.part16.integer = 0x50;
    frame->stage = 4;
}

/**
 * @brief Start sliding to the other menu page unless a slide or the exit runs.
 */
void TitleScreenMenu_StartSlide(struct EwramData_EntityData *entity)
{
    if ((entity->ext.titleMenu.count != 0) && (entity->ext.titleMenu.confirmed == 0))
    {
        if (entity->ext.titleMenu.sliding == 0)
        {
            entity->ext.titleMenu.slideOffset = entity->ext.titleMenu.sliding;
            entity->ext.titleMenu.slidePhase = 1;
            entity->ext.titleMenu.sliding = 1;
        }
    }
}

/**
 * @brief Unload the title graphics, reset objects and delete the logo, menu
 * and frame entities.
 */
void TitleScreen_FreeGraphics(struct EwramData_EntityData *entity)
{
    ObjGfx_Unload(ObjGfx_Find((u8 *)&sTitleScreenGfx));
    ObjGfx_Unload(ObjGfx_Find((u8 *)&sTitleFlashGfx));
    ObjGfx_Unload(ObjGfx_Find((u8 *)&sTitleMenuGfx));
    Obj_Init();
    EntityDelete(entity);
    EntityDelete(&gEwramData->entityData[0x14]);
    EntityDelete(&gEwramData->entityData[0x15]);
}

/**
 * @brief Empty update of the effect group entity.
 */
void TitleScreenEffects_Update(struct EwramData_EntityData *entity)
{
    return;
}

/**
 * @brief Draw the four logo quadrants around the logo position.
 */
void TitleScreenLogo_Draw(struct EwramData_EntityData *entity)
{
    s32 oldX;
    s32 oldY;
    s32 i;
    struct TitleSpritePart *part;

    oldX = entity->posX.whole;
    oldY = entity->posY.whole;
    part = &sTitleSpriteParts[entity->stage];
    
    // for (i = 0; i < 4; i++)
    for (i = 3; i >= 0; i--)
    {
        entity->posX.whole = (part->offsetX << 0x10) + oldX;
        entity->posY.whole = (part->offsetY << 0x10) + oldY;
        entity->animFrame = part->animFrame;
        entity->palette = part->palette;
        Sprite_Draw(entity);
        part += 1;
    }
    entity->posX.whole = oldX;
    entity->posY.whole = oldY;
}

// End of title screen functions

/**
 * @brief Unreferenced: H-blank sine wave on BG2 horizontal offsets for
 * alternating lines, flipping every 16 frames.
 */
void BgEffect_Bg2InterlacedWave(void)
{
    s32 angle;
    s16 *hBlankBuf;
    s32 y;
    s32 scanline;
    s32 zeroIndex;
    u16 vOfs;

    hBlankBuf = gEwramData->hBlankEffect.hBlankBuffer[1 - gEwramData->hBlankEffect.currentBuffer];
    angle = gEwramData->frameCounter;
    angle <<= 0xA;
    vOfs = gDisplayRegisters.bgOfs[2].vOfs;
    HBlankEffectSetupWide(0, SCREEN_SIZE_Y, 2, REG_BG2HOFS);

    for (scanline = 0, zeroIndex = 2; scanline < SCREEN_SIZE_Y; scanline++, zeroIndex += 3)
    {
        y = scanline + vOfs;
        if (y > 0x120 || (y & 1) == (gEwramData->frameCounter & 0x10) / 0x10)
            hBlankBuf[zeroIndex] = 0;
        else
            *hBlankBuf = Sin(angle) >> 0xF;

        hBlankBuf++;
        angle -= 0x2000;
    }

    gEwramData->hBlankEffect.requestStart = 1;
}

/**
 * @brief Set up the BG0 text layer and window of the main menu and print
 * message 0x354 + index.
 */
void MainMenu_SetupMessageWindow(s32 choice)
{
    s16 *tilemap;
    s32 i;

    tilemap = VRAM_BASE + 0xE3C4;

    DMA_FILL_32(3, 0, VRAM_BASE + 0xE000, 0x800);
    DMA_FILL_32(3, 0, VRAM_BASE + 0x8000, 0x2000);

    gDisplayRegisters.bgCnt[0] = CREATE_BGCNT(2, 28, BGCNT_HIGH_PRIORITY, BGCNT_SIZE_256x256);
    BgPalette_Load((u8 *)sMainMenuPalette);
    BgPalette_LoadRange((u8 *)sTextBgPalette, 0, 1, 0xC);
    Text_SetTileBuffer(VRAM_BASE + 0x8000);
    Text_SetForegroundColor(8);
    Text_SetBackgroundColor(0);
    BgPalette_LoadRange((u8 *)sSaveSelectBgPalette, 3, 1, 0xD);

    switch (gEwramData->gameState.language)
    {
        case 2:
            Gfx_CopyResource((u8 *)sLocalizedWindowGfxLanguage2, VRAM_BASE + 0xA000, 0);
            break;

        case 3:
            Gfx_CopyResource((u8 *)sLocalizedWindowGfxLanguage3, VRAM_BASE + 0xA000, 0);
            break;

        default:
            Gfx_CopyResource((u8 *)sLocalizedWindowGfx, VRAM_BASE + 0xA000, 0);
            break;
    }

    Text12_SetRegion(0, 0, 0x29, 3);
    Text12_ClearRegion();
    Text12_SetRegion(0, 1, 0x25, 3);
    Text12_SetCursor(0, 1);

    for (i = 0; i < 0x80; i++)
    {
        *tilemap++ = i - 0x3FE0;
    }

    Text12_Print(Text_GetLocalizedEntry(choice + 0x354), 0);
}

/**
 * @brief Return 1 on A/Start and -1 on B. When any slot holds data, move the
 * choice (subState 0..2, wrapping) with up/down and print the entry
 * description text 0x354 + choice.
 */
s32 MainMenu_HandleChoiceInput(struct EwramData_EntityData *entity)
{
    s32 result;
    u16 repeatedInput;

    repeatedInput = gEwramData->inputData.repeatedInput;
    result = 0;

    if (repeatedInput & (KEY_START | KEY_A))
    {
        PlaySong(SE_243);
        result = 1;
    }
    else if (repeatedInput & KEY_B)
    {
        PlaySong(SE_240);
        result = -1;
    }

    if ((entity->ext.mainMenu.savedSlots != 0) && (result == 0))
    {
        if (repeatedInput & KEY_DOWN)
        {
            entity->subState += 1;
        }
        else if (repeatedInput & KEY_UP)
        {
            entity->subState -= 1;
        }

        if (repeatedInput & (KEY_DOWN | KEY_UP))
        {
            if (entity->subState == 3)
            {
                entity->subState = 0;
            }
            if (entity->subState > 3)
            {
                entity->subState = 2;
            }
            PlaySong(SE_241);
            Text12_ClearRegion();
            Text12_Print(Text_GetLocalizedEntry(entity->subState + 0x354), 0);
        }
    }

    return result;
}

/**
 * @brief Blank the display, clear two screen blocks, free the menu entity,
 * stop the menu music and clear gameState.subMode/subModeStep.
 */
void MainMenu_Close(struct EwramData_EntityData *entity)
{
    gUnk_03002CB0.dispCnt &= ~(DCNT_OBJ | DCNT_BG3 | DCNT_BG2 | DCNT_BG1 | DCNT_BG0 | DCNT_BLANK | DCNT_OBJ_1D | DCNT_OAM_HBL | DCNT_PAGE | DCNT_CGB | 7);
    gDisplayRegisters.bldCnt = BLDCNT_SCREEN_FIRST_TARGET | BLDCNT_BRIGHTNESS_DECREASE_EFFECT;
    gDisplayRegisters.bldY = BLDY_MAX_VALUE;
    DMA_FILL_32(3, 0, VRAM_BASE + 0xF000, 0x800);
    DMA_FILL_32(3, 0, VRAM_BASE + 0xF800, 0x800);
    MainMenu_FreeEntity(entity);
    PlaySong(STOP_SONG | MUS_14);
    gEwramData->gameState.subMode = gEwramData->gameState.subModeStep = 0;
}

typedef void (*MainMenuSubmenuInitFunc)(struct EwramData_EntityData *);

MainMenuSubmenuInitFunc sMainMenuSubmenuInitFuncs[3] = {
    MainMenu_InitGameStart,
    MainMenu_InitDataCopy,
    MainMenu_InitDataDelete,
};

typedef s32 (*MainMenuSubmenuUpdateFunc)(struct EwramData_EntityData *);

MainMenuSubmenuUpdateFunc sMainMenuSubmenuUpdateFuncs[3] = {
    MainMenu_UpdateGameStart,
    MainMenu_UpdateDataCopy,
    MainMenu_UpdateDataDelete
};

// Unused data, corresponds to .sourcePalette in sMainMenuPaletteCycleSteps
u8 sUnk_084F0D4C[8] = {
    0, 8, 9, 10, 11, 10, 9, 8
};

struct MainMenuPaletteCycleStep {
    u8 sourcePalette;
    u8 duration;
    u8 pad_2[0x4 - 0x2];
};

struct MainMenuPaletteCycleStep sMainMenuPaletteCycleSteps[8] = {
    [0] = {
        .sourcePalette = 0,
        .duration = 0x10
    },
    [1] = {
        .sourcePalette = 8,
        .duration = 0x10
    },
    [2] = {
        .sourcePalette = 9,
        .duration = 0x10
    },
    [3] = {
        .sourcePalette = 10,
        .duration = 0x10
    },
    [4] = {
        .sourcePalette = 11,
        .duration = 0x40
    },
    [5] = {
        .sourcePalette = 10,
        .duration = 0x10
    },
    [6] = {
        .sourcePalette = 9,
        .duration = 0x10
    },
    [7] = {
        .sourcePalette = 8,
        .duration = 0x10
    },
};

// Unused data
u8 sUnk_084F0D74[] = {
    0x29, 0x2a, 0x2c, 0x35, 0x2e, 0x2f, 0x32, 0x31, 0x33, 0x34, 0x2b, 0x2d, 0x30,
    0, 0, 0
};

static inline void GameModeMainMenuUpdate_inline(s32 alpha)
{
    gDisplayRegisters.bldAlpha = C_16_2_8(alpha, BLDALPHA_MAX_VALUE - alpha);
}

/**
 * @brief 4A48 | Handle main menu game mode
 * 
 * @return s32 Game mode, -1 is no change, -2 is next game mode, else the specified game mode
 */
s32 GameModeMainMenuUpdate(void)
{
    s32 nameResult;
    s32 choice;
    s32 submenuResult;
    s32 nextSlot;
    s32 gameMode;
    struct EwramData_EntityData *menu;
    struct GameState *gameState;
    struct GameState *nameGameState;
    struct MainMenuPaletteCycleStep *paletteCycle;
    s32 slotOffset;

    menu = gEwramData->entityData;
    gameMode = GAME_MODE_SAME_MODE;

    switch (gEwramData->gameModeUpdateStage)
    {
        case 0x0:
            gUnk_03002CB0.dispCnt = DCNT_OBJ | DCNT_BG3 | DCNT_BG2 | DCNT_BG1 | DCNT_BG0;
            gDisplayRegisters.bldCnt = BLDCNT_SCREEN_SECOND_TARGET | BLDCNT_BRIGHTNESS_DECREASE_EFFECT | BLDCNT_SCREEN_FIRST_TARGET;
            gDisplayRegisters.bldY = BLDY_MAX_VALUE;
            PlaySong(MUS_14);
            gEwramData->gameState.bossRushMode = 0;
            gEwramData->gameModeUpdateStage += 1;
            gEwramData->gameModeSubStage = 0;
            break;
    
        case 0x1:
            DMA_FILL_32(3, 0, VRAM_BASE + 0xF000, 0x800);
            Gfx_CopyResource((u8 *)sSaveSelectGfx, VRAM_BASE, 0);
            Gfx_CopyResource((u8 *)sSaveSelectGfx2, VRAM_BASE + 0x2000, 0);
    
            switch (gEwramData->gameState.language)
            {
                case 2:
                    Gfx_CopyResource((u8 *)sSaveMenuSpriteGfx9, VRAM_BASE + 0x4000, 0);
                    break;
                case 3:
                    Gfx_CopyResource((u8 *)sSaveMenuSpriteGfx15, VRAM_BASE + 0x4000, 0);
                    break;
                default:
                    Gfx_CopyResource((u8 *)sSaveMenuSpriteGfx3, VRAM_BASE + 0x4000, 0);
                    break;
            }

            BgPalette_Load((u8 *)sMainMenuPalette);
            BgMap_Setup(3, (struct BgMapMetadata *)sSaveSelectBgMap, 0, 0);
            BgMap_Setup(2, (struct BgMapMetadata *)sMainMenuBgMap6, 0, 0);
            BgMap_Setup(1, (struct BgMapMetadata *)sMainMenuBgMap5, 0, 0);
            MainMenu_InitEntity(menu);
            MainMenu_SetupMessageWindow(0);
            gEwramData->gameModeTimer = 0;
            gEwramData->gameModeUpdateStage += 1;
            gEwramData->gameModeSubStage = 0;
            break;
    
        case 0x2:
            BgPalette_StartColorFade(0, 0, 0, 0, 0x10000, 0x20);
            ObjPalette_StartColorFade(0, 0, 0, 0, 0x10000, 0x20);
            gUnk_03002CB0.dispCnt &= ~(DCNT_WINOBJ | DCNT_WIN1 | DCNT_WIN0); 
            gUnk_03002CB0.dispCnt |= DCNT_BG3 | DCNT_BG2 | DCNT_BG1 | DCNT_BG0;
            gDisplayRegisters.bldAlpha = C_16_2_8(10, 6);
            gDisplayRegisters.bldY = 0;
            gEwramData->gameModeUpdateStage += 1;
            gEwramData->gameModeSubStage = 0;
            gDisplayRegisters.bldCnt = BLDCNT_BG3_SECOND_TARGET_PIXEL | BLDCNT_ALPHA_BLENDING_EFFECT | BLDCNT_BG2_FIRST_TARGET_PIXEL;
            gEwramData->gameModeTimer = 0;
            break;
    
        case 0x3:
            choice = MainMenu_HandleChoiceInput(menu);
            if (choice < 0)
            {
                PlaySong(SONG_MUTE);
                gUnk_03002CB0.dispCnt &= ~(DCNT_WIN1 | DCNT_WIN0);
                gDisplayRegisters.bldY = BLDY_MAX_VALUE;
                BgPalette_StartColorFade(0, 0, 0, 0x10000, 0, 0x20);
                ObjPalette_StartColorFade(0, 0, 0, 0x10000, 0, 0x20);
                menu->state = 0;
                gEwramData->gameModeTimer = 0;
                gEwramData->gameModeUpdateStage = 0x63;
                gEwramData->gameModeSubStage = 0;
            }
            else if (choice != 0)
            {
                menu->state = 0;
                gDisplayRegisters.bldY = 0;
                gEwramData->gameModeTimer = 0;
                gEwramData->gameModeUpdateStage = 0x28;
                gEwramData->gameModeSubStage = 0;
            }
            break;
    
        case 0x28:
            gUnk_03002CB0.dispCnt &= ~DCNT_BG2;
            gDisplayRegisters.bldAlpha = C_16_2_8(BLDALPHA_MAX_VALUE, 0);
            gEwramData->gameModeTimer = 0x10;
            gEwramData->gameModeUpdateStage = 0x29;
            gEwramData->gameModeSubStage = 0;
            gUnk_03002CB0.dispCnt &= ~DCNT_BG2;
            gDisplayRegisters.bldCnt = BLDCNT_BG3_SECOND_TARGET_PIXEL | BLDCNT_ALPHA_BLENDING_EFFECT | BLDCNT_OBJ_FIRST_TARGET_PIXEL | BLDCNT_BG1_FIRST_TARGET_PIXEL | BLDCNT_BG0_FIRST_TARGET_PIXEL;
            gDisplayRegisters.bldAlpha = C_16_2_8(0, BLDALPHA_MAX_VALUE);
            gEwramData->gameModeTimer = 0;
            gEwramData->gameModeUpdateStage = 0x2A;
            gEwramData->gameModeSubStage = 0;
            break;
    
        case 0x29:
            if (gDisplayRegisters.bldAlpha < C_16_2_8(BLDALPHA_MAX_VALUE, 0))
            {
                gEwramData->gameModeTimer += 2;
                GameModeMainMenuUpdate_inline(gEwramData->gameModeTimer);
            }
            else
            {
                gUnk_03002CB0.dispCnt &= ~DCNT_BG2;
                gDisplayRegisters.bldCnt = BLDCNT_BG3_SECOND_TARGET_PIXEL | BLDCNT_ALPHA_BLENDING_EFFECT | BLDCNT_OBJ_FIRST_TARGET_PIXEL | BLDCNT_BG1_FIRST_TARGET_PIXEL | BLDCNT_BG0_FIRST_TARGET_PIXEL;
                gDisplayRegisters.bldAlpha = C_16_2_8(0, BLDALPHA_MAX_VALUE);
                gEwramData->gameModeTimer = 0;
                gEwramData->gameModeUpdateStage += 1;
                gEwramData->gameModeSubStage = 0;
            }
            break;
    
        case 0x2A:
            if (gDisplayRegisters.bldAlpha < C_16_2_8(BLDALPHA_MAX_VALUE, 0))
            {
                gEwramData->gameModeTimer += 2;
                GameModeMainMenuUpdate_inline(gEwramData->gameModeTimer);
                if (gEwramData->gameModeTimer > 0xC)
                {
                    menu->state = 0;
                }
            }
            else
            {
                gUnk_03002CB0.dispCnt |= DCNT_BG0;
                menu->state = 1;
                gEwramData->hBlankEffect.requestStop = 1;
                sMainMenuSubmenuInitFuncs[menu->subState](menu);
                menu->stage = 0;
                gEwramData->gameModeUpdateStage += 1;
                gEwramData->gameModeSubStage = 0;
            }
            break;
    
        case 0x2B:
            if (gEwramData->gameModeTimer != 0)
            {
                gEwramData->gameModeTimer -= 2;
                GameModeMainMenuUpdate_inline(gEwramData->gameModeTimer);
            }
            else
            {
                gUnk_03002CB0.dispCnt |= DCNT_BG3 | DCNT_BG2 | DCNT_BG1 | DCNT_BG0;
                gDisplayRegisters.bldCnt = BLDCNT_BG3_SECOND_TARGET_PIXEL | BLDCNT_ALPHA_BLENDING_EFFECT | BLDCNT_BG2_FIRST_TARGET_PIXEL;
                gDisplayRegisters.bldAlpha = C_16_2_8(10, 6);
                gEwramData->gameModeUpdateStage = 6;
                gEwramData->gameModeSubStage = 0;
            }
            break;
    
        case 0x2C:
            if (gEwramData->gameModeTimer < 0xB)
            {
                gDisplayRegisters.bldAlpha = C_16_2_8(10, 6);
                gEwramData->gameModeUpdateStage = 6;
                gEwramData->gameModeSubStage = 0;
            }
            else
            {
                gEwramData->gameModeTimer -= 2;
                GameModeMainMenuUpdate_inline(gEwramData->gameModeTimer);
            }
            break;
    
        case 0x33:
            if (gDisplayRegisters.bldAlpha < C_16_2_8(BLDALPHA_MAX_VALUE, 0))
            {
                gEwramData->gameModeTimer += 2;
                GameModeMainMenuUpdate_inline(gEwramData->gameModeTimer);
                break;
            }
            gEwramData->hBlankEffect.requestStop = 1;
            sMainMenuSubmenuInitFuncs[menu->subState](menu);
            menu->stage = 0;
            gDisplayRegisters.bldAlpha = C_16_2_8(BLDALPHA_MAX_VALUE, 0);
            gEwramData->gameModeTimer = 0x10;
            gDisplayRegisters.bldCnt = BLDCNT_BG3_SECOND_TARGET_PIXEL | BLDCNT_ALPHA_BLENDING_EFFECT | BLDCNT_OBJ_FIRST_TARGET_PIXEL | BLDCNT_BG2_FIRST_TARGET_PIXEL | BLDCNT_BG1_FIRST_TARGET_PIXEL | BLDCNT_BG0_FIRST_TARGET_PIXEL;
            gEwramData->gameModeUpdateStage += 1;
            gEwramData->gameModeSubStage = 0;
            break;
    
        case 0x34:
            if (gEwramData->gameModeTimer > 0xA)
            {
                gEwramData->gameModeTimer -= 2;
                GameModeMainMenuUpdate_inline(gEwramData->gameModeTimer);
                break;
            }
            gDisplayRegisters.bldCnt = BLDCNT_BG3_SECOND_TARGET_PIXEL | BLDCNT_ALPHA_BLENDING_EFFECT | BLDCNT_BG2_FIRST_TARGET_PIXEL;
            gDisplayRegisters.bldAlpha = C_16_2_8(10, 6);
            gEwramData->gameModeUpdateStage = 6;
            gEwramData->gameModeSubStage = 0;
            break;
    
        case 0x4:
            if (gDisplayRegisters.bldAlpha < C_16_2_8(BLDALPHA_MAX_VALUE, 0))
            {
                gEwramData->gameModeTimer += 2;
                GameModeMainMenuUpdate_inline(gEwramData->gameModeTimer);
                if (gEwramData->gameModeTimer > 0xC)
                {
                    menu->state = 0;
                }
                break;
            }
            gUnk_03002CB0.dispCnt |= DCNT_BG0;
            gEwramData->hBlankEffect.requestStop = 1;
            sMainMenuSubmenuInitFuncs[menu->subState](menu);
            menu->stage = 0;
            gEwramData->gameModeUpdateStage += 1;
            gEwramData->gameModeSubStage = 0;
            break;
    
        case 0x5:
            if (gEwramData->gameModeTimer != 0)
            {
                gEwramData->gameModeTimer -= 2;
                GameModeMainMenuUpdate_inline(gEwramData->gameModeTimer);
                break;
            }
            gDisplayRegisters.bldAlpha = C_16_2_8(10, 6);
            gDisplayRegisters.bldCnt &= ~(BLDCNT_OBJ_FIRST_TARGET_PIXEL | BLDCNT_BG1_FIRST_TARGET_PIXEL | BLDCNT_BG0_FIRST_TARGET_PIXEL);
            gEwramData->gameModeUpdateStage += 1;
            gEwramData->gameModeSubStage = 0;
            break;
    
        case 0x6:
            submenuResult = sMainMenuSubmenuUpdateFuncs[menu->subState](menu);
    
            if (submenuResult == -1)
            {
                PlaySong(SE_240);
                gEwramData->gameModeUpdateStage = 0x46;
                gEwramData->gameModeSubStage = 0;
            }
            else if (submenuResult == 1)
            {
                PlaySong(SE_244);
                gUnk_03002CB0.dispCnt &= ~DCNT_BG2;
                gDisplayRegisters.bldCnt |= (BLDCNT_OBJ_FIRST_TARGET_PIXEL | BLDCNT_BG1_FIRST_TARGET_PIXEL | BLDCNT_BG0_FIRST_TARGET_PIXEL);
                gEwramData->gameModeUpdateStage += 1;
                gEwramData->gameModeSubStage = 0;
            }
            else if (submenuResult == -2)
            {
                PlaySong(SE_244);
                gUnk_03002CB0.dispCnt &= ~DCNT_BG2;
                gDisplayRegisters.bldCnt = BLDCNT_SCREEN_SECOND_TARGET | BLDCNT_BRIGHTNESS_DECREASE_EFFECT | BLDCNT_SCREEN_FIRST_TARGET;
                gEwramData->gameModeUpdateStage = 0x14;
                gEwramData->gameModeSubStage = 0;
            }
            else if (submenuResult == -3)
            {
                PlaySong(SE_244);
                gDisplayRegisters.bldCnt = BLDCNT_SCREEN_FIRST_TARGET | BLDCNT_BRIGHTNESS_DECREASE_EFFECT;
                gDisplayRegisters.winIn_H = (WIN1_ALL >> 8) & ~WIN0_BG1;
                gDisplayRegisters.winIn_L = WIN0_ALL & ~WIN0_BG1;
                gDisplayRegisters.winOut_L = WIN0_ALL;
                gameState = &gEwramData->gameState;
                ButtonConfig_ResetDefaults();
                gameState->currentRoomDefinition = (struct RoomDefinition *)sDebug4531Room;
                gEwramData->gameModeUpdateStage = 0x1E;
                gEwramData->gameModeSubStage = 0;
            }
            break;
    
        case 0x7:
            if (gDisplayRegisters.bldAlpha < C_16_2_8(BLDALPHA_MAX_VALUE, 0))
            {
                gEwramData->gameModeTimer += 2;
                GameModeMainMenuUpdate_inline(gEwramData->gameModeTimer);
                if (gEwramData->gameModeTimer > 0xC)
                {
                    menu->state = 0;
                }
                if (gDisplayRegisters.bldAlpha == C_16_2_8(BLDALPHA_MAX_VALUE, 0))
                {
                    gUnk_03002CB0.dispCnt &= ~(DCNT_BG2 | DCNT_BG0);
                }
            }
            else
            {
                gEwramData->hBlankEffect.requestStop = 1;
                MainMenu_OpenNameEntry(menu);
                gEwramData->gameModeUpdateStage += 1;
                gEwramData->gameModeSubStage = 0;
            }
            break;
    
        case 0x8:
            if (gEwramData->gameModeTimer != 0)
            {
                gEwramData->gameModeTimer -= 2;
                GameModeMainMenuUpdate_inline(gEwramData->gameModeTimer);
                break;
            }
            gUnk_03002CB0.dispCnt |= DCNT_BG2;
            gDisplayRegisters.bldAlpha = C_16_2_8(10, 6);
            gDisplayRegisters.bldCnt = BLDCNT_BG3_SECOND_TARGET_PIXEL | BLDCNT_ALPHA_BLENDING_EFFECT | BLDCNT_BACKDROP_FIRST_TARGET_PIXEL | BLDCNT_BG3_FIRST_TARGET_PIXEL | BLDCNT_BG2_FIRST_TARGET_PIXEL | BLDCNT_BG0_FIRST_TARGET_PIXEL;
            gEwramData->hBlankEffect.requestStop = 1;
            gEwramData->gameModeUpdateStage += 1;
            gEwramData->gameModeSubStage = 0;
            break;
    
        case 0x9:
            nameResult = MainMenu_UpdateNameEntry(menu);
            if (nameResult < 0)
            {
                PlaySong(SE_240);
                gEwramData->gameModeUpdateStage += 1;
                gEwramData->gameModeSubStage = 0;
            }
            else if (nameResult > 0)
            {
                if (menu->subState != 0)
                {
                    SaveData_SaveSlotToSram(gEwramData->gameState.currentSaveSlot);
                    gEwramData->gameModeUpdateStage += 1;
                    gEwramData->gameModeSubStage = 0;
                    nextSlot = gEwramData->gameState.currentSaveSlot + 1;
                    if (SaveData_ReadSlotHeader(nextSlot, 1) != 0 && gEwramData->saveHeaders[1].playTime != 0)
                    {
                        slotOffset = nextSlot * 0x47C;
                        SramLockSaveSlot(nextSlot);
                        WriteAndVerifySramFast(&gEwramData->gameState.playerName, SRAM_BASE + 0x10 + 0x190 + slotOffset, 8);
                        SramUnlockSaveSlot(nextSlot);
                    }
                }
                else
                {
                    gDisplayRegisters.bldCnt = BLDCNT_SCREEN_FIRST_TARGET | BLDCNT_BRIGHTNESS_DECREASE_EFFECT;
                    gDisplayRegisters.winIn_H = (WIN1_ALL >> 8) & ~WIN0_BG1;
                    gDisplayRegisters.winIn_L = WIN0_ALL & ~WIN0_BG1;
                    gDisplayRegisters.winOut_L = WIN0_ALL;
                    nameGameState = &gEwramData->gameState;
                    ButtonConfig_ResetDefaults();
                    nameGameState->currentRoomDefinition = (struct RoomDefinition *)sDebug4531Room;
                    gEwramData->gameModeUpdateStage = 0x1E;
                    gEwramData->gameModeSubStage = 0;
                }
            }
            break;
    
        case 0xA:
            gUnk_03002CB0.dispCnt &= ~(DCNT_WINOBJ | DCNT_WIN1 | DCNT_WIN0);
            gUnk_03002CB0.dispCnt |= DCNT_BG3 | DCNT_BG2 | DCNT_BG1 | DCNT_BG0;
            gUnk_03002CB0.dispCnt &= ~DCNT_BG0;
            gDisplayRegisters.bldCnt = BLDCNT_BG3_SECOND_TARGET_PIXEL | BLDCNT_ALPHA_BLENDING_EFFECT | BLDCNT_BG2_FIRST_TARGET_PIXEL;
            gDisplayRegisters.bldAlpha = C_16_2_8(10, 6);
            gEwramData->gameModeTimer = 0xA;
            gEwramData->gameModeUpdateStage = 0x29;
            gEwramData->gameModeSubStage = 0;
            break;
    
        case 0xF:
            if (gDisplayRegisters.bldAlpha < C_16_2_8(BLDALPHA_MAX_VALUE, 0))
            {
                gEwramData->gameModeTimer += 2;
                GameModeMainMenuUpdate_inline(gEwramData->gameModeTimer);
                if (gEwramData->gameModeTimer > 0xC)
                {
                    menu->state = 0;
                }
                break;
            }
            gUnk_03002CB0.dispCnt &= ~(DCNT_WINOBJ | DCNT_WIN1 | DCNT_WIN0);
            gUnk_03002CB0.dispCnt |= DCNT_BG0;
            gEwramData->hBlankEffect.requestStop = 1;
            BgMap_Setup(1, (struct BgMapMetadata *)sMainMenuBgMap5, 0, 0);
            BgMap_Setup(2, (struct BgMapMetadata *)sMainMenuBgMap6, 0, 0);
            MainMenu_SetupMessageWindow(menu->subState);
            menu->state = 1;
            menu->stage = 0;
            gEwramData->gameModeUpdateStage += 1;
            gEwramData->gameModeSubStage = 0;
            if (menu->ext.mainMenu.savedSlots == 0)
            {
                menu->subState = 0;
            }
            break;
    
        case 0x10:
            if (gEwramData->gameModeTimer != 0)
            {
                gEwramData->gameModeTimer -= 2;
                GameModeMainMenuUpdate_inline(gEwramData->gameModeTimer);
                break;
            }
            gUnk_03002CB0.dispCnt |= DCNT_BG2;
            gDisplayRegisters.bldAlpha = C_16_2_8(10, 6);
            gDisplayRegisters.bldCnt &= ~(BLDCNT_OBJ_FIRST_TARGET_PIXEL | BLDCNT_BG1_FIRST_TARGET_PIXEL | BLDCNT_BG0_FIRST_TARGET_PIXEL);
            gEwramData->gameModeUpdateStage = 3;
            gEwramData->gameModeSubStage = 0;
            break;
    
        case 0x32:
        case 0x3C:
            gUnk_03002CB0.dispCnt = (gUnk_03002CB0.dispCnt & 0x1FFF) | 0xF00;
            gDisplayRegisters.bldCnt = BLDCNT_BG3_SECOND_TARGET_PIXEL | BLDCNT_ALPHA_BLENDING_EFFECT | BLDCNT_OBJ_FIRST_TARGET_PIXEL | BLDCNT_BG2_FIRST_TARGET_PIXEL | BLDCNT_BG1_FIRST_TARGET_PIXEL | BLDCNT_BG0_FIRST_TARGET_PIXEL;
            gDisplayRegisters.bldAlpha = C_16_2_8(10, 6);
            gEwramData->gameModeTimer = 0xA;
            gEwramData->gameModeUpdateStage += 1;
            gEwramData->gameModeSubStage = 0;
            break;
    
        case 0x3D:
            if (gDisplayRegisters.bldAlpha < C_16_2_8(BLDALPHA_MAX_VALUE, 0))
            {
                gEwramData->gameModeTimer += 2;
                GameModeMainMenuUpdate_inline(gEwramData->gameModeTimer);
                if (gEwramData->gameModeTimer > 0xC)
                {
                    menu->state = 0;
                }
                break;
            }
            gEwramData->hBlankEffect.requestStop = 1;
            BgMap_Setup(1, (struct BgMapMetadata *)sMainMenuBgMap5, 0, 0);
            BgMap_Setup(2, (struct BgMapMetadata *)sMainMenuBgMap6, 0, 0);
            MainMenu_SetupMessageWindow(menu->subState);
            menu->state = 1;
            menu->stage = 0;
            gDisplayRegisters.bldAlpha = C_16_2_8(BLDALPHA_MAX_VALUE, 0);
            gEwramData->gameModeTimer = 0x10;
            gDisplayRegisters.bldCnt = BLDCNT_BG3_SECOND_TARGET_PIXEL | BLDCNT_ALPHA_BLENDING_EFFECT | BLDCNT_OBJ_FIRST_TARGET_PIXEL | BLDCNT_BG2_FIRST_TARGET_PIXEL | BLDCNT_BG1_FIRST_TARGET_PIXEL | BLDCNT_BG0_FIRST_TARGET_PIXEL;
            gEwramData->gameModeUpdateStage += 1;
            gEwramData->gameModeSubStage = 0;
            break;
    
        case 0x3E:
            if (gEwramData->gameModeTimer > 0xA)
            {
                gEwramData->gameModeTimer -= 2;
                GameModeMainMenuUpdate_inline(gEwramData->gameModeTimer);
                break;
            }
            gDisplayRegisters.bldCnt = BLDCNT_BG3_SECOND_TARGET_PIXEL | BLDCNT_ALPHA_BLENDING_EFFECT | BLDCNT_BG2_FIRST_TARGET_PIXEL;
            gDisplayRegisters.bldAlpha = C_16_2_8(10, 6);
            gEwramData->gameModeUpdateStage = 3;
            gEwramData->gameModeSubStage = 0;
            break;
    
        case 0x46:
            gUnk_03002CB0.dispCnt &= ~DCNT_BG2;
            gDisplayRegisters.bldAlpha = C_16_2_8(BLDALPHA_MAX_VALUE, 0);
            gEwramData->gameModeTimer = 0x10;
            gEwramData->gameModeUpdateStage = 0x29;
            gEwramData->gameModeSubStage = 0;
            gUnk_03002CB0.dispCnt &= ~DCNT_BG2;
            gDisplayRegisters.bldCnt = BLDCNT_BG3_SECOND_TARGET_PIXEL | BLDCNT_ALPHA_BLENDING_EFFECT | BLDCNT_OBJ_FIRST_TARGET_PIXEL | BLDCNT_BG1_FIRST_TARGET_PIXEL | BLDCNT_BG0_FIRST_TARGET_PIXEL;
            gDisplayRegisters.bldAlpha = C_16_2_8(0, BLDALPHA_MAX_VALUE);
            gEwramData->gameModeTimer = 0;
            gEwramData->gameModeUpdateStage = 0x48;
            gEwramData->gameModeSubStage = 0;
            break;
    
        case 0x47:
            if (gDisplayRegisters.bldAlpha < C_16_2_8(BLDALPHA_MAX_VALUE, 0))
            {
                gEwramData->gameModeTimer += 2;
                GameModeMainMenuUpdate_inline(gEwramData->gameModeTimer);
                break;
            }
            gUnk_03002CB0.dispCnt &= ~DCNT_BG2;
            gDisplayRegisters.bldCnt = BLDCNT_BG3_SECOND_TARGET_PIXEL | BLDCNT_ALPHA_BLENDING_EFFECT | BLDCNT_OBJ_FIRST_TARGET_PIXEL | BLDCNT_BG1_FIRST_TARGET_PIXEL | BLDCNT_BG0_FIRST_TARGET_PIXEL;
            gDisplayRegisters.bldAlpha = C_16_2_8(0, BLDALPHA_MAX_VALUE);
            gEwramData->gameModeTimer = 0;
            gEwramData->gameModeUpdateStage += 1;
            gEwramData->gameModeSubStage = 0;
            break;
    
        case 0x48:
            if (gDisplayRegisters.bldAlpha < C_16_2_8(BLDALPHA_MAX_VALUE, 0))
            {
                gEwramData->gameModeTimer += 2;
                GameModeMainMenuUpdate_inline(gEwramData->gameModeTimer);
                if (gEwramData->gameModeTimer > 0xC)
                {
                    menu->state = 0;
                }
                break;
            }
            gEwramData->hBlankEffect.requestStop = 1;
            MainMenu_SetupMessageWindow((s32) menu->subState);
            BgMap_Setup(1, (struct BgMapMetadata *)sMainMenuBgMap5, 0, 0);
            BgMap_Setup(2, (struct BgMapMetadata *)sMainMenuBgMap6, 0, 0);
            if (menu->ext.mainMenu.savedSlots == 0)
            {
                menu->subState = 0;
            }
            menu->state = 1;
            gEwramData->gameModeUpdateStage += 1;
            gEwramData->gameModeSubStage = 0;
            break;
    
        case 0x49:
            if (gEwramData->gameModeTimer != 0)
            {
                gEwramData->gameModeTimer -= 2;
                GameModeMainMenuUpdate_inline(gEwramData->gameModeTimer);
                break;
            }
            gUnk_03002CB0.dispCnt |= DCNT_BG3 | DCNT_BG2 | DCNT_BG1 | DCNT_BG0;
            gDisplayRegisters.bldCnt = BLDCNT_BG3_SECOND_TARGET_PIXEL | BLDCNT_ALPHA_BLENDING_EFFECT | BLDCNT_BG2_FIRST_TARGET_PIXEL;
            gDisplayRegisters.bldAlpha = C_16_2_8(10, 6);
            gEwramData->gameModeUpdateStage = 3;
            gEwramData->gameModeSubStage = 0;
            break;
    
        case 0x4A:
            if (gEwramData->gameModeTimer > 0xA)
            {
                gEwramData->gameModeTimer -= 2;
                GameModeMainMenuUpdate_inline(gEwramData->gameModeTimer);
                break;
            }
            gDisplayRegisters.bldAlpha = C_16_2_8(10, 6);
            gEwramData->gameModeUpdateStage = 3;
            gEwramData->gameModeSubStage = 0;
            break;
    
        case 0x14:
            gEwramData->gameModeTimer += 1;
            gDisplayRegisters.bldY = gEwramData->gameModeTimer;
            if (gDisplayRegisters.bldY >= BLDY_MAX_VALUE)
            {
                gEwramData->hBlankEffect.requestStop = 1;
                MainMenu_Close(menu);
                gameMode = GAME_MODE_IN_GAME;
            }
            break;
    
        case 0x1E:
            gEwramData->gameModeTimer += 1;
            gDisplayRegisters.bldY = gEwramData->gameModeTimer;
            if (gDisplayRegisters.bldY >= BLDY_MAX_VALUE)
            {
                MainMenu_Close(menu);
                gameMode = GAME_MODE_INTRO_CUTSCENE;
            }
            break;
    
        case 0x63:
            if (ObjPalette_GetPendingFades() == 0)
            {
                BgPalette_StopColorFade();
                ObjPalette_StopColorFade();
                MainMenu_Close(menu);
                gameMode = GAME_MODE_TITLE_SCREEN;
            }
            break;
    }

    paletteCycle = sMainMenuPaletteCycleSteps;
    if (gEwramData->gameModeUpdateStage > 1)
    {
        if (menu->ext.mainMenu.paletteTimer++ >= paletteCycle[(s8)menu->ext.mainMenu.paletteStep].duration)
        {
            menu->ext.mainMenu.paletteTimer = 0;
            menu->ext.mainMenu.paletteStep += 1;
            if ((s8)menu->ext.mainMenu.paletteStep == 8)
            {
                menu->ext.mainMenu.paletteStep = 0;
            }
        }
        BgPalette_LoadRange((u8 *)sMainMenuPalette, paletteCycle[(s8)menu->ext.mainMenu.paletteStep].sourcePalette, 1, 0);
    }
    Entity_UpdateAll();
    return gameMode;
}
// #include
// #include
