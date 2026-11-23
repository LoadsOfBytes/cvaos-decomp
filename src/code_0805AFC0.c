#include "code_08009A0.h"
#include "menu_page.h"
#include "code_0800B700.h"
#include "code_0800CB00.h"
#include "code_0800F1FC.h"
#include "code_08011DD0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "gba/dma.h"
#include "structs/ewram.h"
#include "structs/main.h"
extern void Entity_UpdateAll(void);
extern void Shop_SetupBackgrounds(void);
extern void MenuCursor_LoadGraphics(struct EwramData_EntityData *);
extern void Cursor_StartAnimation(void);
extern void Shop_DrawCursors(struct EwramData_EntityData *);
extern void MenuItemIcon_Delete(void);
extern void HudMap_WriteTiles(s32, s32, u16, s32);
extern void Text12_PrintTwoLineEntryAt(s32, s32);
extern void Shop_OpenList(void *, void *);
extern s32 Shop_UpdateList(void *, void *);
extern s32 Mod(s32, s32);
extern struct BgMapMetadata *sShopBgMaps[];

/* Shop screen entry, category selection and return to the room. */
/**
 * @brief In-game sub-mode 9: shop entry, category selection (buy/sell), the
 * item list and the return to the room. Return nonzero when done.
 */
s32 Shop_Update(struct GameState *context)
{
    struct EwramData_EntityData *menu = &gEwramData->entityData[18];
    s32 repeated = gEwramData->inputData.repeatedInput;
    s32 pressed = gEwramData->inputData.newInput;
    s32 result = 0;
    switch (context->subModeStep) {
    case 0:
        Entity_UpdateAll();
        if (++gDisplayRegisters.bldY == 16) {
            s32 i;
            void *bg;
            ItemIconCache_Save();
            ObjGfx_SaveState();
            ObjAffine_SaveState();
            Shop_SetupBackgrounds();
            MenuCursor_LoadGraphics(menu);
            Cursor_StartAnimation();
            menu->state = 0;
            menu->subState = 0;
            DMA_FILL_32(3, 0, 0x0600E000, 0x800);
            Text12_SetRegion(0, 2, 37, 3);
            Text12_ClearRegion();
            for (i = 0; i < 4; i++)
                HudMap_WriteTiles(2, i + 16, 0xF060 + i * 32, 29);
            Text12_PrintTwoLineEntryAt(menu->subState + 948, 2);
            switch (gEwramData->gameState.language) {
            case 2: bg = (void *)0x0815F480; break;
            case 3: bg = (void *)0x0815F2D0; break;
            default: bg = (void *)0x0815F120; break;
            }
            BgMap_Setup(1, bg, 0, 0);
            context->subModeStep++;
            PlaySong(48);
        }
        break;
    case 1:
        if (--gDisplayRegisters.bldY == 0)
            context->subModeStep++;
        break;
    case 2:
        if (pressed & 3) {
            if ((pressed & 2) || menu->subState == 2) {
                PlaySong(240);
                context->subModeStep++;
            } else {
                MENU_PAGE_CURSOR(menu) = 0;
                PlaySong(244);
                context->subModeStep = 10;
            }
        } else if (repeated & 0xC0) {
            s32 choice = menu->subState;
            s32 direction = repeated & 0x40;
            s32 next = choice + 1;
            if (direction)
                next = choice + 2;
            menu->subState = Mod(next, 3);
            Text12_ClearRegion();
            Text12_PrintTwoLineEntryAt(menu->subState + 948, 2);
            PlaySong(241);
        }
        break;
    case 3:
        if (++gDisplayRegisters.bldY == 16) {
            u8 *gfx;
            struct RoomDefinition *room;
            EntityDelete(menu);
            MenuItemIcon_Delete();
            switch (gEwramData->gameState.language) {
            case 2: gfx = (void *)0x0827798C; break;
            case 3: gfx = (void *)0x0826C840; break;
            default: gfx = (void *)0x082052D8; break;
            }
            ObjGfx_Unload(ObjGfx_Find(gfx));
            ItemIconCache_Restore();
            room = context->currentRoomDefinition;
            Room_LoadGraphics(room);
            ObjGfx_RestoreState();
            ObjAffine_RestoreState();
            Room_LoadPalettes(room);
            Palette_RestoreState();
            Hud_Init();
            BgMap_Setup(1, gEwramData->savedBgMetadata, gEwramData->savedBgScrollX, gEwramData->savedBgScrollY);
            context->subModeStep++;
        }
        break;
    case 4:
        Entity_UpdateAll();
        Display_RestoreState();
        if (--gDisplayRegisters.bldY == 0) {
            gDisplayRegisters.bldCnt |= 0x3F00;
            gEwramData->paused = 0;
            { struct EwramData *updated = gEwramData;
              updated->unusedWriteLevel = 0; updated->bgControlMinWriteLevel = 0; }
            result = 1;
        }
        break;
    case 10:
        if (++gDisplayRegisters.bldY == 16) {
            /* agbcc otherwise swaps the table/index registers. This binding
             * preserves ROM identity; it does not establish original spelling. */
            register struct BgMapMetadata **table asm("r0"); /* FAKEMATCH: pinned register */
            s32 index;
            menu->state = menu->subState + 1;
            switch (gEwramData->gameState.language) {
            case 2: table = sShopBgMaps; index = menu->subState + 2; break;
            case 3: table = sShopBgMaps; index = menu->subState + 4; break;
            default: table = sShopBgMaps; index = menu->subState; break;
            }
            BgMap_Setup(1, table[index], 0, 0);
            gDisplayRegisters.bgOfs[0].hOfs = 0;
            menu->subState = 0;
            menu->stage = 0;
            context->subModeStep++;
        }
        break;
    case 11:
        if (gDisplayRegisters.bldY == 16)
            Shop_OpenList(context, menu);
        if (--gDisplayRegisters.bldY == 0)
            context->subModeStep++;
        break;
    case 12:
        if (Shop_UpdateList(context, menu))
            context->subModeStep++;
        break;
    case 13:
        if (++gDisplayRegisters.bldY == 16) {
            s32 i;
            void *bg;
            menu->subState = menu->state - 1;
            menu->state = 0;
            DMA_FILL_32(3, 0, 0x0600E000, 0x800);
            Text12_SetRegion(0, 2, 37, 3);
            Text12_ClearRegion();
            for (i = 0; i < 4; i++)
                HudMap_WriteTiles(2, i + 16, 0xF060 + i * 32, 29);
            Text12_PrintTwoLineEntryAt(menu->subState + 948, 2);
            switch (gEwramData->gameState.language) {
            case 2: bg = (void *)0x0815F480; break;
            case 3: bg = (void *)0x0815F2D0; break;
            default: bg = (void *)0x0815F120; break;
            }
            BgMap_Setup(1, bg, 0, 0);
            context->subModeStep = 1;
        }
        break;
    }
    if (context->subModeStep != 0)
        Shop_DrawCursors(menu);
    UpdateInGameTimer();
    return result;
}
