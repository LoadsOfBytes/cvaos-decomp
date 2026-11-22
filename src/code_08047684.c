#include "code/code_08039340.h"

extern s32 GameFlag_Is3ASet(void);
extern void MenuCursor_LoadGraphics(struct EwramData_EntityData *entity);
extern void Cursor_StartAnimation(void);

/**
 * @brief 47684 | Load the menu background graphics for the current
 * language (2 = French, 3 = German) and set up BG0/BG1.
 */
void PauseMenu_LoadGraphics(void)
{
    void *a;
    void *b;
    void *c;

    a = (void *)0x0815E02C;
    b = (void *)0x08264398;
    c = (void *)0x0827B200;
    switch (gEwramData->gameState.language)
    {
    case 2:
        a = (void *)0x0827397C;
        b = (void *)0x08275980;
        c = (void *)0x08277984;
        break;
    case 3:
        a = (void *)0x08268838;
        b = (void *)0x0826A83C;
        c = (void *)0x0826C848;
        break;
    }
    if (GameFlag_Is3ASet())
    {
        BgPalette_Load((u8 *)0x0826664C);
        Gfx_CopyResource(b, (void *)0x06000000, 0);
    }
    else
    {
        BgPalette_Load((u8 *)0x080E5C20);
        Gfx_CopyResource(a, (void *)0x06000000, 0);
    }
    Gfx_CopyResource(c, (void *)0x0600A000, 0);
    gDisplayRegisters.bgCnt[0] = 0x1C04;
    gDisplayRegisters.bgCnt[1] = 0x1D00;
    gUnk_03002CB0.dispCnt &= 0x13FF;
    MenuCursor_LoadGraphics(&gEwramData->entityData[18]);
    Cursor_StartAnimation();
}
