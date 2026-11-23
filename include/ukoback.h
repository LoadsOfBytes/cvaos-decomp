#ifndef UKOBACK_H
#define UKOBACK_H

/* Functions of enemy/ukoback.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void Ukoback_OnContact(struct EwramData_EntityData *entity);
extern void Ukoback_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);

#endif /* UKOBACK_H */
