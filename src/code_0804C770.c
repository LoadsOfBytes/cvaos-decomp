#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"

struct MenuPage
{
    u8 pad_0[0xA];
    u8 state;
    u8 subState;
    u8 stage;
    u8 timer;
    u8 pad_E[0x14 - 0xE];
    s8 count;
    s8 cursor;
    s8 top;
};

extern s32 BestiaryMenu_MoveCursor(struct MenuPage *page, s32 repeated);
extern void BestiaryMenu_DrawList(struct MenuPage *page);
extern s32 Bestiary_IsEnemyDefeated(s32 index);
extern void Cursor_SetTilePosition(s32 a, s32 b);
extern void Cursor_Draw(void);

/**
 * @brief Enemy list update: up/down via BestiaryMenu_MoveCursor, R / L page by 10
 * entries (wrapping), B leaves, A opens the detail view when
 * Bestiary_IsEnemyDefeated allows it.
 */
void BestiaryMenu_UpdateList(struct MenuPage *page)
{
    s32 repeated;
    s32 new;
    s32 top;
    s32 count;
    s32 last;
    s32 rest;

    repeated = gEwramData->inputData.repeatedInput;
    new = gEwramData->inputData.newInput;
    if (BestiaryMenu_MoveCursor(page, repeated) != 0)
    {
        PlaySong(0xF1);
    }
    else if (repeated & 0x100)
    {
        top = page->top;
        count = page->count;
        last = count - 1;
        rest = last % 10 + 1;
        if (top != count - rest)
        {
            if (top + 10 > last)
                page->top = page->count - rest;
            else
                page->top = page->top + 10;
            page->cursor = page->top;
            BestiaryMenu_DrawList(page);
        }
        else
        {
            page->top = 0;
            page->cursor = 0;
            BestiaryMenu_DrawList(page);
        }
        PlaySong(0xF1);
    }
    else if (repeated & 0x200)
    {
        if (page->top != 0)
        {
            if (page->top - 10 < 0)
                page->top = 0;
            else
                page->top = page->top - 10;
            page->cursor = page->top;
            BestiaryMenu_DrawList(page);
        }
        else
        {
            page->cursor = page->top = page->count - 1 - (page->count - 1) % 10;
            BestiaryMenu_DrawList(page);
        }
        PlaySong(0xF1);
    }
    else if (new & 2)
    {
        PlaySong(0xF0);
        page->subState = 0xFF;
        page->stage = 0;
    }
    else if (new & 1)
    {
        if (Bestiary_IsEnemyDefeated(page->cursor))
        {
            PlaySong(0xF4);
            gDisplayRegisters.bldY = 0;
            page->subState = 5;
            page->stage = 0;
        }
        else
        {
            PlaySong(0xEF);
        }
    }
    Cursor_SetTilePosition((page->cursor & 1) ? 0xF : 1, ((page->cursor - page->top) >> 1) * 2 + 5);
    Cursor_Draw();
}
