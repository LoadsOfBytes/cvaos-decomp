/* Bestiary list cursor movement. */
#include "gba.h"
#include "bestiary_cursor.h"

struct MenuPage
{
    u8 pad_0[0x14];
    s8 count;
    s8 cursor;
    s8 top;
};

/**
 * @brief 4C8A4 | Move the cursor of a two-column list with ten visible
 * rows: left/right toggles the column, up/down move by a row and scroll
 * the window (top) by two entries.
 * Matching notes: the original keeps the loaded `top` in a short-lived
 * register (r1) and copies it into the long-lived `top` after computing
 * `top + 9`; only pinning the two short-lived values reproduces that.
 *
 * @return 1 when the cursor moved, 0 otherwise
 */
s32 BestiaryMenu_MoveCursor(struct MenuPage *page, s32 input)
{
    s32 moved = 0;
    s32 cursor;
    s32 top;

    if (input & 0x30)
    {
        if (page->cursor + 1 < page->count || (page->cursor & 1))
        {
            page->cursor ^= 1;
            moved = 1;
        }
    }
    else if (input & 0x40)
    {
        if (page->cursor - 2 >= page->top && page->cursor > 1)
        {
            page->cursor -= 2;
            if (page->cursor < page->top)
                page->top -= 2;
            moved = 1;
        }
    }
    else if (input & 0x80)
    {
        cursor = page->cursor;
        {
            s32 next = cursor + 2;
            s32 t = page->top;
            s32 limit = t + 9;

            top = t;
            if (next <= limit && cursor < page->count - 1)
            {
                if (cursor != page->count - 2)
                    page->cursor += 2;
                if (page->cursor >= page->count)
                    page->cursor = page->count - 1;
                if (page->cursor >= top + 10)
                    page->top += 2;
                moved = 1;
            }
        }
    }
    return moved;
}
