#ifndef DECORATION_BOSS_RUSH_H
#define DECORATION_BOSS_RUSH_H

/* Functions of object/decoration_boss_rush.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void BossRushResultCreate(struct EwramData_EntityData *entity);
extern void BossRushResultUpdate(struct EwramData_EntityData *entity);
extern void BossRushTimerStarterCreate(struct EwramData_EntityData *entity);
extern void BossRushTimerStarterUpdate(struct EwramData_EntityData *entity);
extern void RoomDecorationCreate(struct EwramData_EntityData *entity);
extern void RoomDecorationUpdate(struct EwramData_EntityData *entity);
extern void WreckingBallCreate(struct EwramData_EntityData *entity);
extern void WreckingBallUpdate(struct EwramData_EntityData *entity);

#endif /* DECORATION_BOSS_RUSH_H */
