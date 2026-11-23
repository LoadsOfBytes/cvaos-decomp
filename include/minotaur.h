#ifndef MINOTAUR_H
#define MINOTAUR_H

/* Functions of enemy/minotaur.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void AxeArmorAxe_Update(struct EwramData_EntityData *entity);
extern void AxeArmor_UpdateWalking(struct EwramData_EntityData *entity);

#endif /* MINOTAUR_H */
