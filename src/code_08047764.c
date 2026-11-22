/* PauseMenu_Update, decompiled from the original assembly: menu page selection
 * (fade in/out, cursor over seven entries, page callbacks). The cursor read
 * through the inline CurSel() keeps the base and reloads the 0x25553 offset
 * as in the target. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code_08013960.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"

typedef s32 (*MenuPageFunc)(struct EwramData_EntityData *ent);

extern void HudMap_WriteTiles(s32 x, s32 y, u16 tile, s32 count);
extern void HudMap_ClearAll(void);
extern void PauseMenu_DrawStatusPage(struct GameState *gameState);
extern void PauseMenu_LoadGraphics(void);
extern void PauseMenu_Exit(void);
extern void Cursor_Draw(void);
extern void SoulTypeSelector_Update(struct EwramData_EntityData *ent);
extern void Cursor_SetTilePosition(s32 a, s32 b);
extern void MenuItemIcon_Draw(void);
extern void MenuSlotIcons_Draw(void);

extern struct BgMapMetadata *const sPauseMenuBgMaps[];
extern const MenuPageFunc sPauseMenuPageOpenFuncs[];
extern const MenuPageFunc sPauseMenuPageUpdateFuncs[];

static inline void DrawItem(s32 x, s32 y, s32 tile, s32 hl)
{
    tile += 0x300;
    if (hl)
        tile |= 0x3000;
    else
        tile |= 0x7000;
    HudMap_WriteTiles(x, y, tile, 6);
}

static inline s32 CurSel(void)
{
    return gEwramData->pauseMenuPage;
}

/**
 * @brief Pause menu state machine (state subModeStep): fade in and draw the status page,
 * move the cursor over the seven page entries (pauseMenuPage, wrapping), open a
 * page through the open/update tables sPauseMenuPageOpenFuncs/sPauseMenuPageUpdateFuncs (entry 4,
 * save, opens without the fade), and on B fade out and leave via
 * PauseMenu_Exit. Item icons are drawn while entry 1 or 2 is selected.
 *
 * @return 1 once the menu has closed, 0 otherwise
 */
s32 PauseMenu_Update(struct GameState *gameState)
{
    struct EwramData_EntityData *ent;
    s32 ret;
    s32 repeated;
    s32 new;
    s32 i;
    s32 sel;
    s32 tile;
    s32 x;
    s32 y;

    ent = &gEwramData->entityData[18];
    ret = 0;
    repeated = gEwramData->inputData.repeatedInput;
    new = gEwramData->inputData.newInput;
    switch (gameState->subModeStep)
    {
    case 0:
    case 1:
        gEwramData->bgControlMinWriteLevel = gEwramData->unk_A074_4 = 3;
        if ((u16)(gDisplayRegisters.bldY += 3) <= 0xF)
            break;
        gDisplayRegisters.bldY = 0x10;
        PauseMenu_LoadGraphics();
        PauseMenu_DrawStatusPage(gameState);
        gameState->subModeStep = 2;
        ent->stage = 0;
        ent->subState = 0;
        ent->state = 0;
        Cursor_SetTilePosition(1, gEwramData->pauseMenuPage + 0xC);
        Cursor_Draw();
        break;
    case 2:
        if ((s16)(gDisplayRegisters.bldY -= 3) <= 0)
        {
            gameState->subModeStep = 3;
            gDisplayRegisters.bldY = 0;
        }
        Cursor_Draw();
        break;
    case 3:
        if (repeated & 2)
        {
            PlaySong(0xF0);
            gameState->subModeStep = 7;
        }
        else if (new & 1)
        {
            PlaySong(0xF3);
            if (gEwramData->pauseMenuPage == 4)
            {
                gameState->subModeStep = 6;
                sPauseMenuPageOpenFuncs[gEwramData->pauseMenuPage](ent);
            }
            else
            {
                gameState->subModeStep = 4;
            }
        }
        else if (repeated & 0xC0)
        {
            PlaySong(0xF1);
            if (repeated & 0x40)
            {
                gEwramData->pauseMenuPage--;
                if (gEwramData->pauseMenuPage < 0)
                    gEwramData->pauseMenuPage = 6;
            }
            else
            {
                gEwramData->pauseMenuPage++;
                if (gEwramData->pauseMenuPage > 6)
                    gEwramData->pauseMenuPage = 0;
            }
            Cursor_SetTilePosition(1, gEwramData->pauseMenuPage + 0xC);
            x = 2;
            y = 0xC;
            sel = CurSel();
            for (i = 0, tile = 0x40; i <= 6; tile += 0x10, i++)
            {
                s32 t = tile + 0x300;
                if (i == sel)
                    t |= 0x3000;
                else
                    t |= 0x7000;
                HudMap_WriteTiles(x, y + i, t, 6);
            }
            MenuText_ClearAndPrint(gEwramData->pauseMenuPage + 0x362);
        }
        Cursor_Draw();
        break;
    case 4:
        if ((u16)(gDisplayRegisters.bldY += 3) <= 0xF)
            break;
        if (gEwramData->pauseMenuPage != 6)
        {
            BgMap_Setup(1, sPauseMenuBgMaps[gEwramData->pauseMenuPage + 1], 0, 0);
            HudMap_ClearAll();
        }
        sPauseMenuPageOpenFuncs[gEwramData->pauseMenuPage](ent);
        gameState->subModeStep = 5;
        gDisplayRegisters.bldY = 0x10;
        break;
    case 5:
        if ((s16)(gDisplayRegisters.bldY -= 3) <= 0)
        {
            gameState->subModeStep = 6;
            gDisplayRegisters.bldY = 0;
        }
        switch (gEwramData->pauseMenuPage)
        {
        case 0:
            SoulTypeSelector_Update(&gEwramData->entityData[21]);
            break;
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
            Cursor_Draw();
            break;
        }
        break;
    case 6:
        if (sPauseMenuPageUpdateFuncs[gEwramData->pauseMenuPage](ent))
        {
            if (gEwramData->pauseMenuPage == 4)
                gameState->subModeStep = 3;
            else
                gameState->subModeStep = 1;
        }
        break;
    case 7:
        if ((u16)(gDisplayRegisters.bldY += 3) <= 0xF)
            break;
        PauseMenu_Exit();
        Entity_UpdateAll();
        gDisplayRegisters.bldY = 0;
        gDisplayRegisters.bldCnt |= 0x3F00;
        Sound_ReplayTrackedWithVolume();
        gEwramData->startPauseEnabled = 1;
        gEwramData->paused = 0;
        gEwramData->bgControlMinWriteLevel = gEwramData->unk_A074_4 = 0;
        ret = 1;
        break;
    }
    if ((u8)(gEwramData->pauseMenuPage - 1) <= 1)
    {
        MenuItemIcon_Draw();
        MenuSlotIcons_Draw();
    }
    return ret;
}
