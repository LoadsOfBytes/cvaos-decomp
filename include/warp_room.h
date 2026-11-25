#ifndef WARP_ROOM_H
#define WARP_ROOM_H

/* Functions of object/warp_room.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void WarpRoom_UpdateFadeExit(struct EwramData_EntityData *entity);
extern void WarpRoomCreate(struct EwramData_EntityData *entity);
extern void CrusherCreate(struct EwramData_EntityData *entity);
extern void CrusherUpdate(struct EwramData_EntityData *entity);

#endif /* WARP_ROOM_H */
