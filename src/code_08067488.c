#include "gba.h"
#include "structs/ewram.h"
#include "structs/main.h"

struct ShopList;
void PlaySong(u32 song);
extern void HudMap_ZeroTiles(s32 x, s32 y, s32 count);
extern s32 HudMap_PrintNumber(s32 x, s32 y, s32 value, s32 digits, s32 color, s32 flag);
extern s32 Shop_CanTrade(void *owner, void *menu, struct PlayerStatus *stats, s32 index);
extern void Shop_SellItem(void *owner, void *menu, struct PlayerStatus *stats, s32 index);
extern void Shop_BuyItem(void *owner, void *menu, struct PlayerStatus *stats, s32 index);
extern void Shop_ShowConfirmWindow(void);
extern s32 Shop_HandleListInput(void *owner, void *menu, struct PlayerStatus *stats, struct ShopList *list, s32 input);
extern void Shop_DrawList(void *owner, void *menu, struct PlayerStatus *stats, struct ShopList *list, s32 mode);

struct Owner {
    u8 pad_0[0x3D0];
    u8 shop;
};

struct ShopMenu {
    u8 pad_0[0xA];
    u8 state;
    u8 pad_B;
    u8 stage;
    u8 timer;
    u8 pad_E[0x17 - 0xE];
    u8 cursor;
};

extern struct ShopList *const sShopLists[];

struct ShopList {
    u8 count;
    u8 items[1];
};

#define SELL_LIST ((struct ShopList *)&gEwramData->sharedScratchBuffer[0x200])

static inline u8 ShopItem(u8 *items, struct ShopMenu *menu)
{
    return items[menu->cursor];
}

/**
 * @brief Handle the item list and the yes/no confirmation; sell or buy the
 * selected item on yes. Return nonzero when the list is closed.
 */
s32 Shop_UpdateList(struct Owner *owner, struct ShopMenu *menu)
{
    struct PlayerStatus *stats = &gEwramData->playerStatus;
    s32 input = gEwramData->inputData.repeatedInput;
    s32 ret = 0;
    struct ShopList *list;
    s32 result;

    if (menu->state == 2)
        list = SELL_LIST;
    else
        list = sShopLists[owner->shop];
    if (menu->stage == 0)
    {
        if (input & 2)
        {
            PlaySong(0xF0);
            ret = 1;
        }
        else if (input & 1)
        {
            if (Shop_CanTrade(owner, menu, stats, ShopItem(list->items, menu)))
            {
                Shop_ShowConfirmWindow();
                PlaySong(0xF3);
                menu->timer = 0;
                menu->stage++;
            }
            else
            {
                PlaySong(0xEF);
            }
        }
        else
        {
            result = Shop_HandleListInput(owner, menu, stats, list, input);
            if (result != 0)
                Shop_DrawList(owner, menu, stats, list, result);
        }
    }
    else if ((input & 1) && menu->timer == 0)
    {
        if (menu->state == 2)
            Shop_SellItem(owner, menu, stats, ShopItem(list->items, menu));
        else
            Shop_BuyItem(owner, menu, stats, ShopItem(list->items, menu));
        PlaySong(0xF5);
        Shop_DrawList(owner, menu, stats, list, 3);
        HudMap_ZeroTiles(0xA, 3, 9 - HudMap_PrintNumber(0x11, 9, stats->currentGold, 6, 6, 0));
        gEwramData->hBlankEffect.requestStop = 1;
        gUnk_03002CB0.dispCnt &= ~0x2000;
        gUnk_03002CB0.dispCnt &= ~0x4000;
        menu->stage = 0;
    }
    else if (input & 3)
    {
        PlaySong(0xF0);
        gEwramData->hBlankEffect.requestStop = 1;
        gUnk_03002CB0.dispCnt &= ~0x2000;
        gUnk_03002CB0.dispCnt &= ~0x4000;
        menu->stage = 0;
    }
    else if ((input & 0x10) && menu->timer == 0)
    {
        PlaySong(0xF1);
        menu->timer++;
    }
    else if ((input & 0x20) && menu->timer != 0)
    {
        PlaySong(0xF1);
        menu->timer = 0;
    }
    return ret;
}
