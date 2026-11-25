#ifndef OBJECTS_10_13_H
#define OBJECTS_10_13_H

/* Functions of object/objects_10_13.c used by other files. */

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

#endif /* OBJECTS_10_13_H */
