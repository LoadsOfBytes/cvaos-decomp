#ifndef BUER_H
#define BUER_H

/* Functions of enemy/buer.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void BuerChargeEffect_Update(struct EwramData_EntityData *entity);
extern void Buer_MoveAndRotate(struct EwramData_EntityData *entity, s32 mode);

#endif /* BUER_H */
