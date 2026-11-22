#include "structs/ewram.h"
#include "macros.h"
#include "code/code_08039340.h"

/* Select paired text entries for the observed button masks.
 * Keep the data/table register lifetimes and short offset binding: agbcc
 * otherwise changes the initial address calculation. Matching bytes do
 * not establish the original source spelling. */
/**
 * @brief Print paired glyphs for the configured action button index and return the selected A/B/L/R mask; retain the default-mask branch.
 */
u16 MenuText_PrintActionButton(s32 action)
{
    register struct EwramData *data asm("r2") = gEwramData; /* FAKEMATCH: pinned register */
    register u16 *buttons asm("r1"); /* FAKEMATCH: pinned register */
    u32 button;
    u16 ret;

    action <<= 1;
    {
        u32 offset = OFFSET_OF(struct EwramData, playerStatus.attackButton);
        buttons = (u16 *)((u8 *)data + offset);
    }
    button = *(u16 *)((u8 *)buttons + action);

    if (button == 1)
    {
        Text12_PutChar(0x80);
        Text12_PutChar(0x81);
        ret = 1;
    }
    else if (button == 2)
    {
        Text12_PutChar(0x82);
        Text12_PutChar(0x83);
        ret = 2;
    }
    else if (button == 0x200)
    {
        Text12_PutChar(0x84);
        Text12_PutChar(0x85);
        ret = button;
    }
    else
    {
        Text12_PutChar(0x86);
        Text12_PutChar(0x87);
        ret = 0x100;
    }
    return ret;
}
