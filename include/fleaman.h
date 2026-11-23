#ifndef FLEAMAN_H
#define FLEAMAN_H

/* Functions of enemy/fleaman.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void Fleaman_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void Fleaman_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);

#endif /* FLEAMAN_H */
