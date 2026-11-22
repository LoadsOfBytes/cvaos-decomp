/* Open the button configuration page. */
#include "structs/ewram.h"

/* Button configuration menu state (see ButtonConfigMenu_Update). */
struct InputMenuState
{
    u8 pad_0[0xA];
    u8 state;
    u8 subState;
    u8 stage;
    u8 pad_D[0x12 - 0xD];
    u8 active;
    u8 pad_13;
    u8 error;
    s8 selection;
    u8 pad_16[2];
    u8 choices[4];
};

extern const u16 sButtonConfigButtons[];
extern void MenuPage_Reset(void *page);
extern void HudMap_WriteTiles(s32 x, s32 y, s32 tile, s32 count);
extern void Text12_SetRegion(s32 a, s32 b, s32 c, s32 d);
extern void MenuText_ClearAndPrint(s32 text);
extern void ButtonConfigMenu_Redraw(struct InputMenuState *menu);
extern void ButtonConfigMenu_DrawSprites(struct InputMenuState *menu);
extern void MenuPageTitle_Create(void *entity, s32 type);

/**
 * @brief 4BF8C | Open the button configuration menu: find the choice index
 * of each of the four bindings, draw the frame and help text.
 * Matching notes: do-while loops with `next = i + 1` computed early; the
 * loop-invariant bases (bindings, choices) are computed in the outer body
 * so loop.c hoists them after `i = 0`; the second loop reuses `j`.
 */
void ButtonConfigMenu_Open(struct InputMenuState *menu)
{
    struct EwramData *ew;
    u16 *bindings;
    s32 i;
    s32 j;

    ew = gEwramData;
    i = 0;
    bindings = (u16 *)((u8 *)ew + 0x13398);
    do
    {
        s32 next;
        const u16 *t;
        u16 *b;
        s32 o;
        u8 *c;

        u8 *choices = menu->choices;
        j = 0;
        o = i * 2;
        next = i + 1;
        b = (u16 *)((u8 *)bindings + o);
        c = &choices[i];
        t = sButtonConfigButtons;
        do
        {
            if (*b == *t)
            {
                *c = j;
                break;
            }
            t++;
            j++;
        } while (j <= 3);
        i = next;
    } while (i <= 3);
    menu->state = 0;
    menu->subState = 0;
    menu->stage = 0;
    MenuPage_Reset(menu);
    menu->error = 3;
    for (j = 0; j <= 2; j++)
        HudMap_WriteTiles(5, j + 0xF, j * 0x20 + 0x3200, 0x18);
    Text12_SetRegion(0, 0, 0x20, 2);
    MenuText_ClearAndPrint(0x352);
    menu->error = 0;
    menu->active = 1;
    ButtonConfigMenu_Redraw(menu);
    ButtonConfigMenu_DrawSprites(menu);
    MenuPageTitle_Create((u8 *)gEwramData + 0xF34, 0x24);
}
