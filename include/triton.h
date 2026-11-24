#ifndef TRITON_H
#define TRITON_H

/* Functions of enemy/triton.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void Triton_UpdateMove(struct EwramData_EntityData *entity);
extern void Triton_UpdateAttack(struct EwramData_EntityData *entity);
extern struct EwramData_EntityData *Triton_CreateSpear(struct EwramData_EntityData *owner);
extern u8 Triton_LoadGraphics(struct EwramData_EntityData *entity);
extern void Triton_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void Triton_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);

#endif /* TRITON_H */
