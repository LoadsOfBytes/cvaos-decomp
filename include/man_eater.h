#ifndef MAN_EATER_H
#define MAN_EATER_H

/* Functions of enemy/man_eater.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void KillerMantleLandingEffect_Update(struct EwramData_EntityData *entity);
extern void KillerMantlePart_Update(struct EwramData_EntityData *entity);

#endif /* MAN_EATER_H */
