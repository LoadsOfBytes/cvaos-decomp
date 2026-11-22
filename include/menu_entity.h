#ifndef MENU_ENTITY_H
#define MENU_ENTITY_H

#include "structs/ewram.h"

/* Byte 0x51D of a pause-menu list entity: list top saved while a slot
 * list is open (SoulMenu_Update), cleared on setup (ItemMenu_Open). */
#define MENU_SAVED_TOP(e) (((u8 *)&(e)->unk_51C)[1])

#endif // MENU_ENTITY_H
