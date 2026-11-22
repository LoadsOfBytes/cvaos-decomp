#include "code_08009A0.h"
#include "code_08032444.h"
#include "code/code_08039340.h"

/* Local 0x24-byte template-table view: retain this TU's types and padding. */
struct EnemyTemplateView
{
    u8 pad_0[0x17];
    u8 soulType;
    u8 soulIndex; /* 1-based, 0 = none */
    u8 pad_19[0x24 - 0x19];
};

struct MenuPage
{
    u8 pad_0[0xA];
    u8 state;
    u8 subState;
    u8 stage;
    u8 timer;
    u8 pad_E[0x13 - 0xE];
    u8 descriptionMode;
    s8 count;
    s8 cursor;
    s8 top;
};

extern const struct EnemyTemplateView sEnemyData[];
extern void BestiaryMenu_DrawDetailTitle(struct MenuPage *page);
extern s32 BestiaryMenu_DrawDescription(s32 id, s32 mode);
extern s32 Bestiary_IsEnemyDefeated(s32 index);

/**
 * @brief Detail view input: left/right toggles between the enemy description and
 * its soul name (only when the soul is owned), up/down (or L/R) moves to the
 * previous / next defeated enemy, B closes the view.
 */
void BestiaryMenu_UpdateDetail(struct MenuPage *page)
{
    const struct EnemyTemplateView *entry;
    s32 new;
    s32 repeated;
    s32 old;
    s32 oldCursor;

    entry = &sEnemyData[page->cursor];
    new = gEwramData->inputData.newInput;
    repeated = gEwramData->inputData.repeatedInput;
    if (page->timer == 0)
        gUnk_03002CB0.dispCnt |= 0x1000;
    else
        page->timer--;
    if (entry->soulIndex != 0 && (new & 0xC0))
    {
        old = page->descriptionMode;
        if (new & 0x80)
        {
            if (SoulInventory_GetSoulTotal(entry->soulType, entry->soulIndex - 1))
                page->descriptionMode = 1;
        }
        else
        {
            if (SoulInventory_GetSoulTotal(entry->soulType, entry->soulIndex - 1))
                page->descriptionMode = 0;
            else
                page->descriptionMode = old = 1;
        }
        if (page->descriptionMode != old)
        {
            BestiaryMenu_DrawDetailTitle(page);
            BestiaryMenu_DrawDescription(page->cursor, page->descriptionMode);
            PlaySong(0xF1);
        }
    }
    if (repeated & 0x110)
    {
        oldCursor = page->cursor;
        PlaySong(0xF1);
        do
        {
            if (page->cursor + 1 < page->count)
                page->cursor++;
            else
                page->cursor = 0;
        } while (!Bestiary_IsEnemyDefeated(page->cursor));
        goto moved;
    }
    else if (repeated & 0x220)
    {
        oldCursor = page->cursor;
        PlaySong(0xF1);
        do
        {
            if (page->cursor - 1 >= 0)
                page->cursor = page->cursor - 1;
            else
                page->cursor = page->count - 1;
        } while (!Bestiary_IsEnemyDefeated(page->cursor));
    moved:
        if (page->cursor != oldCursor)
        {
            page->subState = 2;
            gUnk_03002CB0.dispCnt &= 0xEFFF;
        }
    }
    else if ((gEwramData->inputData.newInput & 2) && page->timer == 0)
    {
        gUnk_03002CB0.dispCnt |= 0x1000;
        PlaySong(0xF0);
        page->subState = 6;
        page->stage = 0;
    }
}
