/* Button configuration page update. */
#include "code_08009A0.h"
#include "code/code_08040A38.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

struct ButtonConfigPageView
{
    u8 pad_0[0x12];
    u8 active;
    u8 pad_13;
    u8 error;
    s8 selection;
    u8 pad_16[2];
    u8 choices[4];
};

struct Settings
{
    u8 pad_0[0x13C];
    u16 buttons[4];
};

extern const u16 sButtonConfigButtons[];
extern const u8 sButtonConfigDefaultChoices[4];
extern const u8 sButtonConfigChoiceSlots[4];
extern void ButtonConfigMenu_Redraw(struct ButtonConfigPageView *page);
extern void ButtonConfigMenu_DrawSprites(struct ButtonConfigPageView *page);
extern void MenuPageTitle_Delete(struct EwramData_EntityData *ent);

/**
 * @brief 4C05C | Button configuration page update: B or start (0xA) confirm the
 * four assignments (sButtonConfigButtons[choice]) and reject duplicates with an
 * error message (start keeps the page open), up/down moves the cursor,
 * right/A (0x11) and left cycle the button of the selected action, select (4) restores the
 * defaults. Redraws the page and closes it after a valid confirm.
 * Matching notes: do-while confirm loop (loop-weighted refs make the
 * "used" mask a caller-saved pseudo), the duplicate-error block placed in
 * the final else after `if (0)`, defaults copied as `*(u8 *)(i + t)`.
 *
 * @return -1 when the page is left, 0 otherwise
 */

/**
 * @brief Button-configuration page update: up/down selects a row, A/right and left
 * change its choice, Select restores the default choices. B or Start
 * stores the four bindings; a duplicate binding plays the error sound, shows
 * text 0x353 and keeps the page open. Start applies without leaving.
 *
 * @return -1 when the page is left, 0 otherwise
 */
s32 ButtonConfigMenu_Update(struct ButtonConfigPageView *page)
{
    struct Settings *settings;
    u16 repeated;
    u16 new;
    s32 ret;
    s32 used;
    s32 i;
    u8 *choices;

    settings = (struct Settings *)&gEwramData->playerStatus;
    repeated = gEwramData->inputData.repeatedInput;
    new = gEwramData->inputData.newInput;
    ret = (new & 0xA) ? -1 : 0;
    if (page->error && (new & ~0xA))
    {
        MenuText_ClearAndPrint(0x352);
        page->error = 0;
    }
    if (ret)
    {
        used = 0;
        i = 0;
        choices = page->choices;
        do
        {
            settings->buttons[i] = sButtonConfigButtons[choices[i]];
            if (settings->buttons[i] & used)
                goto error;
            PlaySong(0xF0);
            used |= settings->buttons[i];
        } while (++i <= 3);
        if (ret)
        {
            page->active = 0;
            if (new & 8)
                ret = 0;
        }
    }
    else if (repeated & 0x40)
    {
        PlaySong(0xF1);
        page->selection--;
        choices = page->choices;
    }
    else if (repeated & 0x80)
    {
        PlaySong(0xF1);
        page->selection++;
        choices = page->choices;
    }
    else if (repeated & 0x11)
    {
        PlaySong(0xF4);
        page->choices[sButtonConfigChoiceSlots[page->selection]]++;
        choices = page->choices;
    }
    else if (repeated & 0x20)
    {
        PlaySong(0xF4);
        page->choices[sButtonConfigChoiceSlots[page->selection]]--;
        choices = page->choices;
    }
    else
    {
        if (0)
        {
        error:
            PlaySong(0xEF);
            MenuText_ClearAndPrint(0x353);
            page->error = 1;
            ret = 0;
            goto end;
        }
        {
            s32 f = new & 4;
            choices = page->choices;
            if (f)
            {
                PlaySong(0xF4);
                i = 0;
                {
                    u8 *c = choices;
                    const u8 *t = sButtonConfigDefaultChoices;
                    for (; i <= 3; i++)
                        c[i] = *(u8 *)((u32)i + (u32)t);
                }
            }
        }
    }
end:
    page->selection &= 3;
    choices[sButtonConfigChoiceSlots[page->selection]] &= 3;
    if (repeated || new)
        ButtonConfigMenu_Redraw(page);
    ButtonConfigMenu_DrawSprites(page);
    if (ret)
        MenuPageTitle_Delete((struct EwramData_EntityData *)((u8 *)gEwramData + 0xF34));
    return ret;
}
