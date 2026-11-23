#ifndef BESTIARY_CURSOR_H
#define BESTIARY_CURSOR_H

/* Functions of menu/bestiary_cursor.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"


extern s32 BestiaryMenu_MoveCursor(struct EwramData_EntityData *page, s32 input);

#endif /* BESTIARY_CURSOR_H */
