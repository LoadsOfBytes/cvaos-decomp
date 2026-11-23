#ifndef WITCH_H
#define WITCH_H

/* Functions of enemy/witch.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void StudentWitch_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern void WitchGroundProjectile_OnHit(struct EwramData_EntityData *entity);

#endif /* WITCH_H */
