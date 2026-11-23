#ifndef DAMAGE_NUMBERS_H
#define DAMAGE_NUMBERS_H

/* Functions of engine/damage_numbers.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void Hit_SetTimerOverride(s32 index, s32 value);
extern void Player_ShowDamage(s32 value, u8 kind);
extern void HudHPNumber_Create(void);

#endif /* DAMAGE_NUMBERS_H */
