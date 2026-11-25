#ifndef DEBUG_ROOM_H
#define DEBUG_ROOM_H

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

s32 GameModeDebugRoomUpdate(void);
void DebugJitterSprite_Update(struct EwramData_EntityData *entity);

#endif /* DEBUG_ROOM_H */
