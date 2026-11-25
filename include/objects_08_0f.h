#ifndef OBJECTS_08_0F_H
#define OBJECTS_08_0F_H

/* Functions of object/objects_08_0f.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void BreakableBlockCreate(struct EwramData_EntityData *entity);
extern void BreakableBlockUpdate(struct EwramData_EntityData *entity);
extern void CrumblingWallCreate(struct EwramData_EntityData *entity);
extern void CrumblingWallUpdate(struct EwramData_EntityData *entity);
extern void BossFlagEnemySpawnerCreate(struct EwramData_EntityData *entity);
extern void BossFlagEnemySpawnerUpdate(struct EwramData_EntityData *entity);
extern void GameFlagEnemySpawnerCreate(struct EwramData_EntityData *entity);
extern void GameFlagEnemySpawnerUpdate(struct EwramData_EntityData *entity);
extern void TriggerZoneUpdate(struct EwramData_EntityData *entity);
extern void TriggerZoneCreate(struct EwramData_EntityData *entity);
extern void SwingingPlatformCreate(struct EwramData_EntityData *entity);
extern void SwingingPlatformUpdate(struct EwramData_EntityData *entity);
extern void BreakablePropCreate(struct EwramData_EntityData *entity);
extern void BreakablePropUpdate(struct EwramData_EntityData *entity);
extern void CandleCreate(struct EwramData_EntityData *entity);
extern void CandleUpdate(struct EwramData_EntityData *entity);

#endif /* OBJECTS_08_0F_H */
