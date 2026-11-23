#include "code/code_08039340.h"
#include "code/code_08040A38.h"

struct MenuPage
{
    u8 pad_0[0xA];
    u8 state;
    u8 subState;
    u8 stage;
    u8 timer;
    u8 pad_E[0x15 - 0xE];
    s8 cursor;
    s8 top;
};

extern void HudMap_WriteTiles(s32 x, s32 y, u16 tile, s32 count);
extern void HudMap_ClearAll(void);
extern void MenuCursor_LoadGraphics(struct EwramData_EntityData *entity);
extern void Cursor_StartAnimation(void);
extern void Cursor_SetTilePosition(s32 a, s32 b);
extern void BestiaryMenu_DrawList(struct MenuPage *page);
extern void BestiaryMenu_ReleaseResources(void);

/**
 * @brief Load the language specific frame graphics, draw the enemy list and the
 * header tiles and place the cursor.
 */
void BestiaryMenu_Setup(struct MenuPage *page)
{
    void *gfx;

    switch (gEwramData->gameState.language)
    {
    case 2:
        gfx = (void *)0x08277984;
        break;
    case 3:
        gfx = (void *)0x0826C848;
        break;
    default:
        gfx = (void *)0x0827B200;
        break;
    }
    Gfx_CopyResource(gfx, (void *)0x0600A000, 0);
    BgMap_Setup(1, (struct BgMapMetadata *)0x0815EA60, 0, 0);
    HudMap_ClearAll();
    Text8_SetRegion(0, 0xF, 0x29, 1);
    Text8_ClearRegion();
    BestiaryMenu_DrawList(page);
    BestiaryMenu_ReleaseResources();
    MenuCursor_LoadGraphics(&gEwramData->entityData[18]);
    Cursor_StartAnimation();
    HudMap_WriteTiles(0xB, 3, 0xF3D0, 0xE);
    HudMap_WriteTiles(0x11, 0xF, 0xF3E0, 4);
    HudMap_WriteTiles(0x15, 0xF, 0x13E4, 1);
    HudMap_WriteTiles(0x19, 0xF, 0x13E8, 1);
    HudMap_WriteTiles(0x1B, 0xF, 0xF3EA, 1);
    Cursor_SetTilePosition((page->cursor & 1) ? 0xF : 1, ((page->cursor - page->top) >> 1) * 2 + 5);
    {
        // writing through a pointer keeps the store order of the original
        u8 *p = &page->subState;
        p[0] = 1;
        p[1] = 0;
    }
}
