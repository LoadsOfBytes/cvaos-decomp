#ifndef SHOP_H
#define SHOP_H

/* Functions of menu/shop.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"


extern void Shop_OpenList(struct GameState *shop, struct EwramData_EntityData *page);
extern s32 Shop_UpdateList(struct GameState *owner, struct EwramData_EntityData *menu);

#endif /* SHOP_H */
