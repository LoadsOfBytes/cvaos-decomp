/* Shop menu page setup (Shop_OpenList). The flag loop has its own index
 * (j) so it stays in a caller-saved register. */
#include "gba.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

struct MenuPage
{
    u8 pad_0[0xA];
    u8 state;
    u8 pad_B[0x14 - 0xB];
    u8 count;
    u8 cursor;
    u8 top;
    u8 item;
};

extern u8 *const sShopLists[];
extern void HudMap_PrintNumber(s32 a, s32 b, s32 c, s32 d, s32 e, s32 f);
extern void Menu_DrawAttackDefense(s32 x, s32 y);
extern void Menu_DrawCurrentStats(s32 x, s32 y);
extern void HudMap_WriteTiles(s32 x, s32 y, u16 tile, s32 count);
extern void MenuPage_SetMode(struct MenuPage *entity, u8 mode);
extern void Shop_BuildSellList(u8 *shop, struct MenuPage *page, struct PlayerStatus *stats);
extern void Shop_DrawList(u8 *shop, struct MenuPage *page, struct PlayerStatus *stats, u8 *list, s32 mode);

static inline u8 ShopMode(u8 *s) { return s[0x3D0]; }
static inline u8 *ShopFlags(u8 *s) { return s + 0x3D1; }

/**
 * @brief Set up the item list page for buying or selling (building the sell
 * list) and draw it.
 */
void Shop_OpenList(u8 *shop, struct MenuPage *page)
{
    struct PlayerStatus *stats;
    s32 i;
    s32 j;
    u8 *mode;
    u8 *list;

    stats = &gEwramData->playerStatus;
    DMA_FILL_32(3, 0, 0x0600E000, 0x800);
    HudMap_PrintNumber(0x11, 9, stats->currentGold, 6, 6, 0);
    Menu_DrawAttackDefense(0x16, 2);
    Menu_DrawCurrentStats(0x16, 4);
    for (i = 0; i <= 3; i++)
    {
        HudMap_WriteTiles(5, i + 0x10, (i << 5) - 0xFA0, 0x18);
        HudMap_WriteTiles(2, i + 0xB, (i << 5) - 0xE80, 0xA);
    }
    Text12_SetRegion(0, 2, 0x23, 5);
    Text8_SetRegion(0, 0xC, 0x20, 4);
    Text8_ClearRegion();
    MenuPage_SetMode(page, 0);
    page->top = 0;
    page->count = 0;
    page->item = 0;
    page->cursor = 0;
    mode = &shop[0x3D0];
    *mode = 0;
    if (gEwramData->gameState.bossEventFlags & 0x10)
        *mode = 1;
    if (gEwramData->gameState.bossEventFlags & 2)
        *mode = 2;
    if (page->state == 2)
    {
        Shop_BuildSellList(shop, page, stats);
        Shop_DrawList(shop, page, stats, (u8 *)gEwramData + 0x135F4, 3);
    }
    else
    {
        list = sShopLists[ShopMode(shop)];
        for (j = 0; j < list[0]; j++)
        {
            u8 *p = ShopFlags(shop);
            p[j] = 1;
        }
        Shop_DrawList(shop, page, stats, list, 3);
    }
}
