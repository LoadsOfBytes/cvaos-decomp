#ifndef SAVE_POINT_H
#define SAVE_POINT_H

/* Functions of object/save_point.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern s32 SavePoint_SpawnOrbitLights(struct EwramData_EntityData *parent);
extern void Object1DCreate(struct EwramData_EntityData *entity);
extern void Object1DUpdate(struct EwramData_EntityData *entity);
extern void Object1ECreate(struct EwramData_EntityData *entity);
extern void Object1EUpdate(struct EwramData_EntityData *entity);

#endif /* SAVE_POINT_H */
