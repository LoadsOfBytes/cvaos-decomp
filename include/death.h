#ifndef DEATH_H
#define DEATH_H

/* Functions of enemy/death.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void EvilButcherDeathParticle_Update(struct EwramData_EntityData *entity);

#endif /* DEATH_H */
