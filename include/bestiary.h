#ifndef BESTIARY_H
#define BESTIARY_H

/* Functions of menu/bestiary.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"



extern void BestiaryMenu_ReleaseResources(void);
extern void BestiaryMenu_DrawList(struct EwramData_EntityData *page);
extern s32 Bestiary_IsEnemyDefeated(s32 id);

extern void BestiaryMenu_UpdateDetailScreen(struct EwramData_EntityData *entity);
extern void BestiaryMenu_UpdateDetail(struct EwramData_EntityData *page);
extern void BestiaryMenu_Close(struct EwramData_EntityData *page);

#endif /* BESTIARY_H */
