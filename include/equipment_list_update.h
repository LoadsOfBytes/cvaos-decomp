#ifndef EQUIPMENT_LIST_UPDATE_H
#define EQUIPMENT_LIST_UPDATE_H

/* Functions of menu/equipment_list_update.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

struct MenuPage;

extern void EquipmentList_UpdateForEquip(struct MenuPage *page);

#endif /* EQUIPMENT_LIST_UPDATE_H */
