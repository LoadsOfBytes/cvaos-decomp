#ifndef BLUE_SOULS_H
#define BLUE_SOULS_H

/* Functions of soul/blue_souls.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void BlueSoul_StopAll(void);
extern void Soma_HandleBlueSoulInput(struct EwramData_EntityData *entity);
extern void YellowSoul_CreateEquippedEntity(struct EwramData_EntityData *entity);
extern void YellowSoul_Restart(u8 soul, s32 force);
extern void YellowSoul_Stop(void);

#endif /* BLUE_SOULS_H */
