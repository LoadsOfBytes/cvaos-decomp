#include "types.h"

struct MenuGrid
{
    u8 pad_0[0x14];
    s8 count;
    s8 cursor;
    s8 top;
};

/**
 * @brief Move the cursor of a two-column menu grid with an eight-entry visible page:
 * left/right toggles the column, up/down moves one row, R/L moves one page.
 * Input priority and the signed byte fields are preserved.
 *
 * @return 0 nothing changed, 1 cursor moved, 2 the visible top changed
 */
s32 MenuGrid_MoveCursor(struct MenuGrid *page, s32 input)
{
    s32 oldTop = page->top;
    s32 result = 0;

    if (input & 0x30)
    {
        if (page->cursor + 1 < page->count || (page->cursor & 1))
        {
            page->cursor ^= 1;
            result = 1;
        }
    }
    else if (input & 0x40)
    {
        if (page->cursor > 1)
        {
            page->cursor -= 2;
            if (page->cursor < page->top)
                page->top -= 2;
            result = 1;
        }
    }
    else if (input & 0x80)
    {
        if (page->cursor < page->count - 1)
        {
            page->cursor += 2;
            if (page->cursor >= page->count)
                page->cursor = page->count - 1;
            if (page->cursor >= page->top + 8)
                page->top += 2;
            result = 1;
        }
    }
    else if (input & 0x100)
    {
        if (oldTop + 8 < page->count)
        {
            page->cursor += 8;
            page->top += 8;
            if (page->cursor > page->count - 1)
                page->cursor = page->count - 1;
            result = 1;
        }
        else if (page->cursor < page->count - 1)
        {
            page->cursor = page->count - 1;
            result = 1;
        }
    }
    else if (input & 0x200)
    {
        if (page->top > 7)
        {
            page->cursor -= 8;
            page->top -= 8;
            result = 1;
        }
        else if (page->cursor != 0)
        {
            if (page->cursor <= 7)
                page->cursor = 0;
            else
                page->cursor -= 8;
            page->top = 0;
            result = 1;
        }
    }

    if (page->top != oldTop)
        result = 2;
    return result;
}
