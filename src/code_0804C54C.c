#include "code_08009A0.h"
#include "code/code_08039340.h"

struct MenuPage
{
    u8 pad_0[0xA];
    u8 state;
    u8 subState;
    u8 stage;
};

extern void BestiaryMenu_Setup(struct MenuPage *page);
extern void BestiaryMenu_UpdateList(struct MenuPage *page);
extern void BestiaryMenu_UpdateDetailScreen(struct MenuPage *page);
extern void BestiaryMenu_UpdateDetail(struct MenuPage *page);
extern void BestiaryMenu_Close(struct MenuPage *page);
extern void Cursor_Draw(void);
extern void Entity_UpdateReserved(void);

/**
 * @brief Bestiary page state machine (fades between the list,
 * BestiaryMenu_UpdateDetailScreen and BestiaryMenu_UpdateDetail).
 *
 * @return -1 when the page is left, 0 otherwise
 */
s32 BestiaryMenu_Update(struct MenuPage *page)
{
    s32 ret = 0;

    switch (page->subState)
    {
    case 0:
        BestiaryMenu_Setup(page);
        page->subState = 4;
        page->stage = ret;
        break;
    case 4:
        if ((s16)(gDisplayRegisters.bldY -= 4) != 0)
            break;
        gDisplayRegisters.bldY = ret;
        page->subState = 1;
        page->stage = 0;
        break;
    case 1:
        BestiaryMenu_UpdateList(page);
        break;
    case 5:
        if ((u16)(gDisplayRegisters.bldY += 4) <= 0xF)
            break;
        gDisplayRegisters.bldY = 0x10;
        page->subState = 2;
        page->stage = 0;
        break;
    case 2:
        BestiaryMenu_UpdateDetailScreen(page);
        break;
    case 3:
        if ((s16)(gDisplayRegisters.bldY -= 4) != 0)
            break;
        gDisplayRegisters.bldY = ret;
        page->subState = 7;
        break;
    case 7:
        BestiaryMenu_UpdateDetail(page);
        break;
    case 6:
        if ((u16)(gDisplayRegisters.bldY += 4) <= 0xF)
            break;
        gDisplayRegisters.bldY = 0x10;
        page->subState = 0;
        page->stage = 0;
        break;
    case 0xFF:
        BestiaryMenu_Close(page);
        ret = -1;
        break;
    }
    switch (page->subState)
    {
    case 1:
    case 4:
    case 5:
        Cursor_Draw();
        break;
    case 7:
        Entity_UpdateReserved();
        break;
    }
    return ret;
}
