#include "debug_menu.h"
#include "core.h"
#include "bg_and_collision.h"
#include "debug_tests.h"
#include "debug_sio.h"
#include "debug_obj_disp.h"
#include "sound.h"
#include "gba.h"

#include "constants/main.h"

#include "data/data_0E0334.h"
#include "data/data_0E3464.h"

#include "structs/agb_sram.h"
#include "structs/ewram.h"
#include "structs/main.h"

static struct NestedDebugMenuState sDebugNestedMenuMenuState;

struct DebugNestedMenuTestDispatch {
    s32 (*runFunc)(s32 testIndex);
    s32 testIndex;
}; /* size = 0x8 */
struct DebugNestedMenuTestDispatch sDebugNestedMenuTestDispatch[] = {
    [0] = {
        .runFunc = DebugObjDispTest_Run,
        .testIndex = 0
    },
    [1] = {
        .runFunc = DebugDispTest_Run,
        .testIndex = 0
    },
    [2] = {
        .runFunc = DebugDispTest_Run,
        .testIndex = 1
    },
    [3] = {
        .runFunc = DebugDispTest_Run,
        .testIndex = 2
    },
    [4] = {
        .runFunc = DebugSioTest_Run,
        .testIndex = 0
    },
    [5] = {
        .runFunc = DebugSioTest_Run,
        .testIndex = 1
    },
    [6] = {
        .runFunc = NULL,
        .testIndex = 0
    }
};

const u8 sUnk_080e1fd4[] = { 0xF6, 0xE0, 0x14, 0x20 }; // TODO: what is this data?

struct NestedDebugMenuEntry sUnk_084F149C[] = {
    [0] = {
        .label = "DISP TEST",
        .submenu = 0,
        .selectionId = 1
    },
    [1] = {
        .label = "LINK TEST",
        .submenu = 0,
        .selectionId = 2
    },
    [2] = {
        .label = "VRAM HIT TEST",
        .submenu = 0,
        .selectionId = 3
    },
    [3] = {
        .label = 0,
        .submenu = 0,
        .selectionId = 0
    }
};

struct NestedDebugMenuEntry sUnk_084F14CC[] = {
    [0] = {
        .label = "DISP TEST",
        .submenu = 0,
        .selectionId = 0
    },
    [1] = {
        .label = 0,
        .submenu = 0,
        .selectionId = 0
    }
};

struct NestedDebugMenuEntry sUnk_084F14E4[] = {
    [0] = {
        .label = "SIO TEST 0",
        .submenu = 0,
        .selectionId = 4
    },
    [1] = {
        .label = "SIO TEST 1",
        .submenu = 0,
        .selectionId = 5
    },
    [2] = {
        .label = 0,
        .submenu = 0,
        .selectionId = 0
    }
};

struct NestedDebugMenuEntry sNestedDebugMenuEntries[] = {
    [0] = {
        .label = "BG",
        .submenu = sUnk_084F149C,
        .selectionId = 0xFE
    },
    [1] = {
        .label = "OBJ",
        .submenu = sUnk_084F14CC,
        .selectionId = 0xFE
    },
    [2] = {
        .label = "ETC",
        .submenu = sUnk_084F14E4,
        .selectionId = 0xFE
    },
    [3] = {
        .label = "EXIT",
        .submenu = NULL,
        .selectionId = 6
    },
    [4] = {
        .label = NULL,
        .submenu = NULL,
        .selectionId = 0
    }
};

/**
 * @brief 3681C | Nested debug test menu (debug menu entry "2739"): BG, OBJ and ETC tests
 * 
 * @return s32 Game mode, -1 is no change, -2 is next game mode, else the specified game mode
 */
s32 GameModeDebugNestedMenuUpdate(void)
{
    s32 gameMode;
    struct DebugNestedMenuTestDispatch *entry;

    gameMode = GAME_MODE_SAME_MODE;

    switch (gEwramData->gameModeSubStage)
    {
        case 0:
            DMA_FILL_32(3, 0, &sDebugNestedMenuMenuState, sizeof(sDebugNestedMenuMenuState));
            DebugMenu_Init(&sDebugNestedMenuMenuState, sNestedDebugMenuEntries);
            gEwramData->gameModeSubStage = 1;
            break;

        case 1:
            sDebugNestedMenuMenuState.selectionResult = DebugMenu_Update(&sDebugNestedMenuMenuState, 4, 4);
            switch (sDebugNestedMenuMenuState.selectionResult)
            {
                case 6:
                    gEwramData->gameModeSubStage = 4;
                    break;

                default:
                    gEwramData->gameModeSubStage = 2;
                    break;
                
                case 0xFE:
                case 0xFF:
                    break;
            }
            break;

        case 2:
            DMA_FILL_32(3, 0, VRAM_BASE + 0xE000, 0x800);
            gEwramData->gameModeSubStage = 3;
            /* fallthrough */
        case 3:
            entry = &sDebugNestedMenuTestDispatch[sDebugNestedMenuMenuState.selectionResult];
            if (entry->runFunc(entry->testIndex) != 0)
            {
                EntityFreeAll();
                DMA_COPY_32(3, &sDebugPalette, PALRAM_BASE, sizeof(sDebugPalette));
                sDebugNestedMenuMenuState.testStage = 0;
                gEwramData->gameModeSubStage = 1;
            }
            break;

        case 4:
            gEwramData->gameModeSubStage = 0;
            gameMode = GAME_MODE_DEBUG;
            ResetAudio();
            break;

        default:
            gEwramData->gameModeSubStage = 0;
            break;
    }

    return gameMode;
}

/**
 * @brief Return the current debug test stage (0 init, 1 run, 2 exit).
 */
s32 DebugTest_GetStage(void)
{
    return sDebugNestedMenuMenuState.testStage;
}

/**
 * @brief Set the debug test stage.
 */
void DebugTest_SetStage(u8 stage)
{
    sDebugNestedMenuMenuState.testStage = stage;
}

/**
 * @brief Draw and navigate the current list of the nested debug menu: Up/Down
 * move, A returns the entry id and enters a submenu, B goes back.
 * Return 0xFF when nothing was chosen.
 */
s32 DebugMenu_Update(struct NestedDebugMenuState *menu, u16 tileIndex, u16 rowOffset)
{
    s32 chosen;
    s32 i;
    s32 count;
    s8 *cursor;
    struct NestedDebugMenuEntry *list;
    u8 tileInfo;

    list = DebugMenu_GetCurrentList(menu);
    count = DebugMenu_CountEntries(list);
    chosen = 0xFF;

    cursor = &menu->cursors[menu->depth];
    if (gEwramData->inputData.playerNewInput & 0x40)
    {
        if (--*cursor < 0)
        {
            *cursor = count - 1;
        }
    }
    if (gEwramData->inputData.playerNewInput & 0x80)
    {
        if (++*cursor > (count - 1))
        {
            *cursor = 0;
        }
    }

    if (gEwramData->inputData.playerNewInput & 1)
    {
        chosen = list[*cursor].selectionId;
        if (list[*cursor].submenu != NULL)
        {
            menu->depth += 1;
            list = DebugMenu_GetCurrentList(menu);
            count = DebugMenu_CountEntries(list);
            cursor = &menu->cursors[menu->depth];
            DMA_FILL_32(3, 0, VRAM_BASE + 0xE000, 0x800);
        }
    }
    if ((gEwramData->inputData.playerNewInput & 2) && (menu->depth > 0))
    {
        menu->depth -= 1;
        list = DebugMenu_GetCurrentList(menu);
        count = DebugMenu_CountEntries(list);
        cursor = &menu->cursors[menu->depth];
        DMA_FILL_32(3, 0, VRAM_BASE + 0xE000, 0x800);
    }

    for (i = 0; i < count; i++)
    {
        tileInfo = 0;
        if (i == *cursor)
        {
            tileInfo = 1;
        }
        BgCmdBuffer_WriteString(tileIndex, rowOffset + i, tileInfo, list[i].label);
    }

    return chosen;
}

/**
 * @brief Clear the debug menu state and set its root list.
 */
void DebugMenu_Init(struct NestedDebugMenuState *menu, struct NestedDebugMenuEntry *root)
{
    DMA_FILL_32(3, 0, menu, 0xC);
    menu->rootList = root;
}

/**
 * @brief Count the entries of a NULL-terminated debug menu list.
 */
s32 DebugMenu_CountEntries(struct NestedDebugMenuEntry *entries)
{
    s32 i;

    for (i = 0; entries[i].label != NULL; i++);
    return i;
}

/**
 * @brief Follow the chosen entries from the root to the current list.
 */
struct NestedDebugMenuEntry* DebugMenu_GetCurrentList(struct NestedDebugMenuState *menu)
{
    struct NestedDebugMenuEntry *list;
    s32 i;

    list = menu->rootList;
    for (i = 0; i < menu->depth; i++)
    {
        list = list[menu->cursors[i]].submenu;
    }
    return list;
}
