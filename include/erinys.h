#ifndef ERINYS_H
#define ERINYS_H

/* Functions of enemy/erinys.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void Tsuchinoko_UpdateJump(struct EwramData_EntityData *entity);

#endif /* ERINYS_H */
