#ifndef DAMAGE_NUMBERS_H
#define DAMAGE_NUMBERS_H

/* Functions of engine/damage_numbers.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

struct EntityBoxes;

extern void Hit_SetTimerOverride(s32 index, s32 value);
extern void Player_ShowDamage(s32 value, u8 kind);
extern void HudHPNumber_Create(void);

extern struct EntityBoxes *Hit_GetEntityBoxes(struct EwramData_EntityData *entity);
extern void Entity_ShowDamage(struct EwramData_EntityData *owner, s32 value, u8 kind);

#endif /* DAMAGE_NUMBERS_H */
