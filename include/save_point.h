#ifndef SAVE_POINT_H
#define SAVE_POINT_H

/* Functions of object/save_point.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern s32 SavePoint_SpawnOrbitLights(struct EwramData_EntityData *parent);
extern void SaveWarpRoomWallCreate(struct EwramData_EntityData *entity);
extern void SaveWarpRoomWallUpdate(struct EwramData_EntityData *entity);
extern void EmptyObject1ECreate(struct EwramData_EntityData *entity);
extern void EmptyObject1EUpdate(struct EwramData_EntityData *entity);
extern s32 SavePoint_SpawnAnimParts(struct EwramData_EntityData *parent);

#endif /* SAVE_POINT_H */
