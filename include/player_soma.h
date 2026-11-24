#ifndef PLAYER_SOMA_H
#define PLAYER_SOMA_H

/* Functions of player/player_soma.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void Player_InitNewGameIfNeeded(void);
extern void Soma_InitEntity(struct EwramData_EntityData *entity);
extern void Julius_InitEntity(struct EwramData_EntityData *entity);
extern void Player_Create(s32 unused); // param real or implicit call?
extern void Player_UpdateDeath(struct EwramData_EntityData *entity);
extern s16 Player_GetOverheadY(void);
extern s32 Player_StartScriptedAnimation(u16 anim, u16 flags, s32 velX, s32 velY);

#endif /* PLAYER_SOMA_H */
