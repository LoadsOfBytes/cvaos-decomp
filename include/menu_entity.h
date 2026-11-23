#ifndef MENU_ENTITY_H
#define MENU_ENTITY_H

#include "structs/ewram.h"

/* Byte 0x51D of a pause-menu list entity: list top saved while a slot
 * list is open (SoulMenu_Update), cleared on setup (ItemMenu_Open). */
#define MENU_SAVED_TOP(e) (((u8 *)&(e)->enemyStatus)[1])

/* Menu cursor entity (entityData[18]): screen x / y in pixels, copied into
 * posX / posY by Cursor_Draw (lists, config and title menus). */
#define MENU_CURSOR_X(e) (e)->hp
#define MENU_CURSOR_Y(e) ENTITY_HALF_51A(e)

#endif // MENU_ENTITY_H
