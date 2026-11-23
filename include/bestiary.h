#ifndef BESTIARY_H
#define BESTIARY_H

/* Functions of menu/bestiary.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

struct MenuPage;
struct MenuPage2;

struct MenuPage4;

extern void BestiaryMenu_ReleaseResources(void);
extern void BestiaryMenu_DrawList(struct MenuPage4 *page);
extern s32 Bestiary_IsEnemyDefeated(s32 id);

extern void BestiaryMenu_UpdateDetailScreen(struct EwramData_EntityData *entity);
extern void BestiaryMenu_UpdateDetail(struct MenuPage *page);
extern void BestiaryMenu_Close(struct MenuPage2 *page);

#endif /* BESTIARY_H */
