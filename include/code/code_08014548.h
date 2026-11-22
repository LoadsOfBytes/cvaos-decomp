#ifndef CODE_08014548_H
#define CODE_08014548_H

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void Player_InitNewGameIfNeeded(void);
extern void Soma_InitEntity(struct EwramData_EntityData *entity);
extern void Julius_InitEntity(struct EwramData_EntityData *entity);
extern void Player_Create(s32 unused); // param real or implicit call?
extern s16 Player_GetOverheadY(void);

#endif /* CODE_08014548_H */
