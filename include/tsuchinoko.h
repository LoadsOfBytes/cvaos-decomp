#ifndef TSUCHINOKO_H
#define TSUCHINOKO_H

/* Functions of enemy/tsuchinoko.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void Tsuchinoko_SpawnDroplet(struct EwramData_EntityData *parent);
extern u8 Tsuchinoko_LoadGraphics(struct EwramData_EntityData *entity);

#endif /* TSUCHINOKO_H */
