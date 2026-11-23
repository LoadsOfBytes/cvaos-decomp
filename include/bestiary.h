#ifndef BESTIARY_H
#define BESTIARY_H

/* Functions of menu/bestiary.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

struct MenuPage4;

extern void BestiaryMenu_ReleaseResources(void);
extern void BestiaryMenu_DrawList(struct MenuPage4 *page);
extern s32 Bestiary_IsEnemyDefeated(s32 id);

#endif /* BESTIARY_H */
