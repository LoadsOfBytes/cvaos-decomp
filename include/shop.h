#ifndef SHOP_H
#define SHOP_H

/* Functions of menu/shop.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

struct MenuPage;
struct Owner3;
struct ShopMenu3;

extern void Shop_OpenList(u8 *shop, struct MenuPage *page);
extern s32 Shop_UpdateList(struct Owner3 *owner, struct ShopMenu3 *menu);

#endif /* SHOP_H */
