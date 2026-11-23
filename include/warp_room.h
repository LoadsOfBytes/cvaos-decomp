#ifndef WARP_ROOM_H
#define WARP_ROOM_H

/* Functions of object/warp_room.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void Object1FUpdate(struct EwramData_EntityData *entity);
extern void Object1FCreate(struct EwramData_EntityData *entity);
extern void Object21Create(struct EwramData_EntityData *entity);
extern void Object21Update(struct EwramData_EntityData *entity);

#endif /* WARP_ROOM_H */
