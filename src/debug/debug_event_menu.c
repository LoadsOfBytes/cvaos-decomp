#include "debug_event_menu.h"
#include "bg_and_collision.h"
#include "in_game.h"
// #include "bg_effects.h" // TODO: Bg_UpdateScrollModes implicitly called with arguments (it should have none)
#include "map_flags.h"
#include "soul_inventory.h"
#include "player_level.h"
#include "gba.h"

#include "constants/main.h"

#include "data/interrupt_table.h"
#include "data/debug_font.h"

#include "structs/agb_sram.h"
#include "structs/ewram.h"
#include "structs/main.h"
#include "equipment_menu.h"
#include "item_menu.h"
#include "menu_common.h"
#include "player_soma.h"
#include "main_menu.h"
#include "data/data_4F1444.h"
#include "constants/game_flags.h"
extern struct RoomDefinition sEndingRoomDefinition; /* read as u8[] by graham.c */
extern int sprintf(char *, const char *, ...);

const u16 sDebugEventMenuPalette[] = {
    0x7FFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x7FFF, 0x001F, 0x001F, 0x001F, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x7FFF, 0x03E0, 0x03E0, 0x03E0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x7FFF, 0x7C00, 0x7C00, 0x7C00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
};

/**
 * @brief 38A38 | Debug event menu (debug menu entry "4531", "DEBUG MODE --usr04531--"): pick and start an event scene
 * 
 * @return s32 Game mode, -1 is no change, -2 is next game mode, else the specified game mode
 */
s32 GameModeDebugEventMenuUpdate(void)
{
    s32 gameMode;
    struct PlayerStatus *playerStatus;

    // 080E2668
    void *subroutine_arg0[] = {
        (void*)sDebugEventRoom0,
        (void*)sDebugEventRoom1,
        (void*)sDebugEventRoom2,
        (void*)sDebugEventRoom3,
        (void*)sDebugEventRoom4,
        (void*)sDebugEventRoom5,
        (void*)sDebugEventRoom6,
        (void*)sDebugEventRoom7,
        (void*)sDebugEventRoom8,
        (void*)sDebugEventRoom9,
        (void*)sDebugEventRoom10,
        (void*)sDebugEventRoom11,
        (void*)sDebugEventRoom12,
        (void*)&sEndingRoomDefinition,
        (void*)sDebugEventRoom14,
        (void*)&sEndingRoomDefinition,
        (void*)sDebugEventRoom0,
        (void*)sDebugEventRoom17,
    };

    gameMode = GAME_MODE_SAME_MODE;
    if (gEwramData->debugEventMenuState == 0)
    {
        gEwramData->debugEventMenuState = 1;
    }

    switch (gEwramData->debugEventMenuState)
    {
        case 1:
            gEwramData->debugEventMenuState = DebugEventMenu_UpdateEventMenu();
            if (gEwramData->debugEventMenuState != 1)
            {
                gEwramData->debugEventMenuSubStage = 0;
            }
            break;

        case 2:
            gameMode = DebugEventMenu_SelectSaveSlot();
            break;

        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 19:
            if (gEwramData->debugEventMenuState == 12)
            {
                gEwramData->gameState.bossEventFlags |= BOSS_DEFEATED_GRAHAM;
                GameFlag_Set(FLAG_SOMA_AWAKENED);
            }
            else if (gEwramData->debugEventMenuState == 10)
            {
                gEwramData->gameState.bossEventFlags |= BOSS_DEFEATED_BALORE;
            }
            else if (gEwramData->debugEventMenuState == 19)
            {
                GameFlag_Set(FLAG_GRAHAM_BATTLE_STARTED);
            }

            GameFlag_Set(FLAG_PROLOGUE_DONE);
            gameMode = DebugEventMenu_RunEventGame();
            if (gEwramData->debugEventMenuState == 19 && gEwramData->debugEventMenuSubStage == 1)
            {
                gEwramData->playerStatus.equippedRedSoul = 0x2C;
                gEwramData->playerStatus.equippedBlueSoul = 2;
                gEwramData->playerStatus.equippedYellowSoul = 7;
            }
            break;

        case 16:
            gEwramData->gameState.stateFlags &= ~STATE_ENDING_CHAOS;
            gEwramData->gameState.stateFlags &= ~STATE_ENDING_JULIUS;
            gEwramData->gameState.stateFlags |= STATE_ENDING_GRAHAM;
            gameMode = DebugEventMenu_RunEventGame();
            break;

        case 17:
            gEwramData->gameState.stateFlags &= ~STATE_ENDING_GRAHAM;
            gEwramData->gameState.stateFlags &= ~STATE_ENDING_JULIUS;
            gEwramData->gameState.stateFlags |= STATE_ENDING_CHAOS;
            gameMode = DebugEventMenu_RunEventGame();
            break;

        case 18:
            gEwramData->gameState.stateFlags &= ~STATE_ENDING_GRAHAM;
            gEwramData->gameState.stateFlags &= ~STATE_ENDING_CHAOS;
            gEwramData->gameState.stateFlags |= STATE_ENDING_JULIUS;
            gameMode = DebugEventMenu_RunEventGame();
            break;

        case 20:
            gEwramData->gameState.stateFlags |= STATE_ENDING_SEQUENCE;
            gameMode = DebugEventMenu_RunEventGame();
            break;

        case 21:
            gEwramData->debugEventMenuState = 0;
            gEwramData->debugEventMenuSubStage = 0;
            gEwramData->debugEventMenuState = 0;
            gameMode = GAME_MODE_DEBUG;
            break;
    }

    GameFlag_Set(FLAG_PROLOGUE_DONE);
    if (gEwramData->debugEventMenuSubStage == 1)
    {
        if (gEwramData->debugEventMenuState >= 3)
        {
            gEwramData->gameState.currentRoomDefinition = subroutine_arg0[gEwramData->debugEventMenuState - 3];

            playerStatus = &gEwramData->playerStatus;
            if (gEwramData->debugEventMenuState == 19)
            {
                Player_SetLevel(0x63);
            }
            else
            {
                Player_SetLevel(0x2D);
            }
            playerStatus->currentHP = playerStatus->maxHP;
            playerStatus->currentMP = playerStatus->maxMP;

            if (gEwramData->debugEventMenuState == 15)
            {
                GameFlag_Set(FLAG_YOKO_RESCUED);
            }
        }
    }

    if (gEwramData->debugEventMenuState == 1)
    {
        if (gEwramData->inputData.newInput & (4 | 2))
        {
            gEwramData->debugEventMenuState = 21;
        }

        if (gEwramData->inputData.newInput & 8)
        {
            gEwramData->gameState.currentRoomDefinition = (struct RoomDefinition *)sDebugEventMenuRoom;
            return 4;
        }
    }

    return gameMode;
}

struct DebugEventMenuEntry {
    u8 *label; // string ptr
    s32 eventState;
};

/**
 * @brief Draw the paged "DEBUG MODE --usr04531--" event list, move the cursor
 * and return the chosen event state on A (1 while choosing).
 */
s32 DebugEventMenu_UpdateEventMenu(void)
{
    s32 pageStart;
    s32 prevPageStart;
    s32 index;
    u16 input;
    s32 ret;
    s32 row;

    // 080E281C
    struct DebugEventMenuEntry subroutine_arg0[] = {
        [0] = {
            .label = "EVENT02:GRA :",
            .eventState = 4
        },
        [1] = {
            .label = "EVENT03:YOKO:",
            .eventState = 5
        },
        [2] = {
            .label = "EVENT04:JB  :",
            .eventState = 6
        },
        [3] = {
            .label = "EVENT05:GRA :HYOUHEN",
            .eventState = 7
        },
        [4] = {
            .label = "EVENT06:YOKO:TSUISEKI",
            .eventState = 8
        },
        [5] = {
            .label = "EVENT07:YOKO:SANGEKI",
            .eventState = 9
        },
        [6] = {
            .label = "EVENT08:JB  :KIOKU",
            .eventState = 10
        },
        [7] = {
            .label = "EVENT09:GRA :VS GRAHAM",
            .eventState = 11
        },
        [8] = {
            .label = "EVENT10:JB  :VS BELMONDO",
            .eventState = 12
        },
        [9] = {
            .label = "EVENT11:    :VS CHAOS",
            .eventState = 13
        },
        [10] = {
            .label = "EVENT12:ENDING_BAD",
            .eventState = 16
        },
        [11] = {
            .label = "EVENT13:ENDING_TRUE",
            .eventState = 17
        },
        [12] = {
            .label = "EVENT14:SYOUNIN",
            .eventState = 14
        },
        [13] = {
            .label = "EVENT16:YOKO HINSHI",
            .eventState = 15
        },
        [14] = {
            .label = "EVENTJB:ENDING_BELMONDO",
            .eventState = 18
        },
        [15] = {
            .label = "EVENT09_05:VS GRAHAM2",
            .eventState = 19
        },
        [16] = {
            .label = "EVENTED:ENDING CHAOS",
            .eventState = 20
        },
        [17] = {
            .label = NULL,
            .eventState = 0
        },
    };

    input = gEwramData->inputData.repeatedInput;
    ret = 1;

    DMA_COPY_16(3, &sDebugEventMenuPalette, PALRAM_BASE, 0x80);
    BgCmdBuffer_WriteString(1, 1, 1, "DEBUG MODE --usr04531--"); // "DEBUG MODE --usr04531--"

    prevPageStart = ((u32*)&gEwramData->debugMenuWork)[0];
    pageStart = (gEwramData->debugEventMenuSubStage / 17) * 17;
    ((u32*)&gEwramData->debugMenuWork)[0] = pageStart;
    if (prevPageStart != pageStart)
    {
        DMA_FILL_32(3, 0, VRAM_BASE + 0xE000, 0x800);
    }

    ((u32*)&gEwramData->debugMenuWork)[2] = 0;
    for (index = ((u32*)&gEwramData->debugMenuWork)[0]; subroutine_arg0[index].label != NULL; index++)
    {
        if (((s32*)&gEwramData->debugMenuWork)[2] <= 0x10)
        {
            ((u32*)&gEwramData->debugMenuWork)[2] += 1;
        }
    }
    ((u32*)&gEwramData->debugMenuWork)[1] = ((u32*)&gEwramData->debugMenuWork)[0] + ((u32*)&gEwramData->debugMenuWork)[2];

    if (gEwramData->inputData.newInput & 1)
    {
        DMA_FILL_32(3, 0, VRAM_BASE + 0xE000, 0x800);
        ret =  subroutine_arg0[gEwramData->debugEventMenuSubStage].eventState;
        return ret;
    }

    if (input & 0x40)
    {
        if ((gEwramData->debugEventMenuSubStage % 17) != 0)
        {
            gEwramData->debugEventMenuSubStage -= 1;
        }
        else
        {
            gEwramData->debugEventMenuSubStage = ((u32*)&gEwramData->debugMenuWork)[1] - 1;
        }
    }
    else if (input & 0x80)
    {
        if ((((gEwramData->debugEventMenuSubStage + 1) % 17) == 0) || ((gEwramData->debugEventMenuSubStage + 1) >= ((s32*)&gEwramData->debugMenuWork)[1]))
        {
            gEwramData->debugEventMenuSubStage = ((u32*)&gEwramData->debugMenuWork)[0];
        }
        else
        {
            gEwramData->debugEventMenuSubStage += 1;
        }
    }
    else if (input & 0x100)
    {
        if ((gEwramData->debugEventMenuSubStage + 0x11) < index)
        {
            gEwramData->debugEventMenuSubStage += 0x11;
        }
        else
        {
            gEwramData->debugEventMenuSubStage = index - 1;
        }
    }
    else if (input & 0x200)
    {
        if (gEwramData->debugEventMenuSubStage > 0x11)
        {
            gEwramData->debugEventMenuSubStage -= 0x11;
        }
        else
        {
            gEwramData->debugEventMenuSubStage = 0;
        }
    }

    index = ((u32*)&gEwramData->debugMenuWork)[0];
    row = 0;
    for (; index < (s32) (((u32*)&gEwramData->debugMenuWork)[0] + ((u32*)&gEwramData->debugMenuWork)[2]); )
    {
        BgCmdBuffer_WriteString(4, 2 + row, (index == gEwramData->debugEventMenuSubStage) ? 1 : 3, subroutine_arg0[index].label);
        index++;
        row++;
    }
    return ret;
}

s32 sUnk_084F158C[] = {
    0x37, 0x18, 0x23, 0x8
};

s32 sUnk_084F159C[] = {
    0x20, 0x3B, 0x2D
};

/**
 * @brief Set up a new game with all souls, items and abilities, then run
 * GameModeInGameUpdate with the debug HP/MP/position overlay and the
 * L+R level-up cheat.
 */
s32 DebugEventMenu_RunEventGame(void)
{
    s32 j;
    s32 i;
    s32 result;
    struct PlayerStatus *playerStatus;
    s16 roomY;
    s16 roomX;
    struct GameState *gameState;
    struct EwramData_EntityData *player;

    gameState = &gEwramData->gameState;
    playerStatus = &gEwramData->playerStatus;
    player = gEwramData->playerState.player;
    result = -1;

    switch (gEwramData->debugEventMenuSubStage)
    {
        case 0:
            gameState->subMode = 0;
            DMA_FILL_32(3, 0, VRAM_BASE + 0xE000, 0x2000);
            Player_InitNewGameIfNeeded();
            gEwramData->playerStatus.equippedRedSoul = 0x2D;
            gEwramData->playerStatus.equippedBlueSoul = 3;
            Player_EquipItem(2, 4);

            for (i = 0; i < 4; i++)
            {
                for (j = 0; j < sUnk_084F158C[i]; j++)
                {
                    SoulInventory_AddAmountToFirstGroupTotal(i, j, 1);
                }
            }
            SoulInventory_AddAmountToSoulTotal(0, 0, 7);

            for (i = 3; i < 6; i++)
            {
                for (j = 0; j < sUnk_084F159C[i - 3]; j++)
                {
                    switch (i)
                    {
                        case 3:
                            gEwramData->playerStatus.itemInventory[j] = 1;
                            break;

                        case 4:
                            gEwramData->playerStatus.weaponInventory[j] = 1;
                            break;

                        case 5:
                            gEwramData->playerStatus.armorInventory[j] = 1;
                            break;
                    }
                }
            }

            Player_SetAbility(0, 1);
            Player_SetAbility(1, 1);
            Player_SetAbility(2, 1);
            Player_SetAbility(3, 1);
            Player_SetAbility(4, 1);
            Player_SetAbility(5, 1);

            Player_ClearEquipmentAndSoulLists();
            gEwramData->gameState.currentRoomDefinition = (struct RoomDefinition *)sDebugEventRoom9;
            gEwramData->debugEventMenuSubStage = 1;
            break;

        case 1:
            gEwramData->debugEventMenuSubStage = 2;
            break;

        case 2:
            if ((gameState->subMode != 6) && (gEwramData->debugEventMenuFlags & 4))
            {
                DMA_COPY_32(3, &sDebugPalette, PALRAM_BASE + 0x160, 0x40);
                roomX = GetEntityRoomXPositionInteger(gEwramData->playerState.player);
                roomY = GetEntityRoomYPositionInteger(gEwramData->playerState.player);
                {
                    char text[29];

                    sprintf(text, "HP=%3d MP=%3d", gEwramData->playerStatus.currentHP, gEwramData->playerStatus.currentMP);
                    BgCmdBuffer_WriteString(0xE, 4, 0xC, (u8 *) text);

                    sprintf(text, "%004x:%004x", roomX, roomY);
                    BgCmdBuffer_WriteString(0xE, 5, 0xC, (u8 *) text);
                }
            }

            if (gEwramData->debugEventMenuFlags & 2)
            {
                playerStatus->equippedRedSoul = 0x37;
                playerStatus->equippedBlueSoul = 3;
                playerStatus->equippedYellowSoul = 4;
                Player_RefreshEquipmentStats();
                playerStatus->currentExperience = 0x86470;
                Player_ApplyEarnedLevels(playerStatus);
                playerStatus->currentHP = playerStatus->maxHP;
                playerStatus->currentMP = playerStatus->maxMP;
                gEwramData->debugEventMenuFlags &= ~0x2;
            }

            result = GameModeInGameUpdate();
            if (((gEwramData->inputData.heldInput & 0x300) == 0x300) && ((gEwramData->inputData.newInput & 0x8) == 0x8))
            {
                if (gEwramData->debugEventMenuFlags & 4)
                {
                    gEwramData->debugEventMenuFlags &= ~4;
                }
                else
                {
                    gEwramData->debugEventMenuFlags |= 4;
                }
            }

            if (((gEwramData->inputData.heldInput & 0x300) == 0x300) && ((gEwramData->inputData.newInput & 0x2) == 0x2))
            {
                if (gEwramData->paused)
                {
                    gEwramData->debugEventMenuFlags |= 2;
                }
            }
            break;
    }

    return result;
}

/**
 * @brief Debug event menu state 2: open the save select screen; on a choice
 * start the game at room sDebugEventMenuRoom2 (return 4), on cancel return 0.
 */
s32 DebugEventMenu_SelectSaveSlot(void)
{
    s32 result;
    struct EwramData_EntityData *entity;

    entity = &gEwramData->entityData[0x12];
    result = -1;

    switch (gEwramData->debugEventMenuSubStage)
    {
        case 0:
            SaveSelect_Open(entity);
            gEwramData->debugEventMenuSubStage += 1;
            break;

        case 1:
            switch (SaveSelect_Update(entity))
            {
                case -1:
                    result = 0;
                    break;

                case 0:
                    result = -1;
                    break;

                case 1:
                    gEwramData->gameState.currentRoomDefinition = (struct RoomDefinition *)sDebugEventMenuRoom2;
                    result = 4;
                    break;
            }

            if (result != -1)
            {
                gEwramData->debugEventMenuSubStage = 0;
                SaveSelect_Close(entity);
            }
            break;
    }

    return result;
}
