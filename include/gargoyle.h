#ifndef GARGOYLE_H
#define GARGOYLE_H

/* Functions of enemy/gargoyle.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void EnemyGargoyleCreate(struct EwramData_EntityData *entity);
extern void EnemyGargoyleUpdate(struct EwramData_EntityData *entity);

#endif /* GARGOYLE_H */
