/* Draw the three equipped soul names in the soul menu and highlight the cursor row. */
#include "gba.h"
#include "bg_map.h"
#include "structs/ewram.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"

struct SoulMenu
{
    u8 pad[0x13];
    u8 type;
    u8 count;
    s8 cursor;
    u8 pad16;
    u8 selected;
};

/**
 * @brief Print the equipped red, blue and yellow soul names (or the none text
 * 0x34F) and recolor the three category rows, highlighting the cursor row.
 */
void SoulMenu_DrawEquippedNames(struct SoulMenu *menu)
{
    struct PlayerStatus *equip;
    s32 i;
    s32 text;
    u16 pal;
    s32 odd;
    s32 x;
    u16 y;
    u16 *dst;
    s32 j;
    s32 col;

    equip = &gEwramData->playerStatus;
    col = 6;
    for (i = 0; i < 3; i++)
    {
        text = 0x34F;
        switch (i)
        {
            case 0:
                if (equip->equippedRedSoul != 0)
                    text = equip->equippedRedSoul + 0xE2;
                pal = 0x8000;
                break;
            case 1:
                if (equip->equippedBlueSoul != 0)
                    text = 0x11A + equip->equippedBlueSoul;
                pal = 0x9000;
                break;
            case 2:
                if (equip->equippedYellowSoul != 0)
                    text = 0x132 + equip->equippedYellowSoul;
                pal = 0x6000;
                break;
        }
        odd = (i + 1) & 1;
        x = 0;
        if (odd)
            x = 0x15;
        y = ((i + 1) >> 1) + 10;
        Text8_SetRegion(x, y, 0xE, 1);
        Text8_ClearRegion();
        if (odd)
            Text8_SetPixelCursor(0x80, y);
        Text8_Print(Text_GetLocalizedEntry(text), 0);
        { s32 x_ = col << 1; s32 y_ = (i << 1) + 3; asm("" : "+r"(y_)); y_ <<= 6; y_ += BG_MAP_VRAM; dst = (u16 *)(x_ + y_); } /* FAKEMATCH: opaque row keeps (row*2+3)<<6 unfolded */
        if (menu->cursor != i)
            pal = 0x3000;
        { s32 m_ = 0xFFF; asm volatile("" : "+r"(m_)); for (j = 9; j >= 0; j--) /* FAKEMATCH: mask loaded per row (not hoisted) */
        {
            *dst = (*dst & m_) | pal; asm("" :: "r"(text)); /* FAKEMATCH: text outlives, lowers its priority (r9) */
            dst++;
        } }
    }
}
