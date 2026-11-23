#ifndef SKULL_ARCHER_H
#define SKULL_ARCHER_H

/* Functions of enemy/skull_archer.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void SkullArcherArrow_OnHit(struct EwramData_EntityData *entity);

#endif /* SKULL_ARCHER_H */
