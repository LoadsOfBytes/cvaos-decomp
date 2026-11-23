#ifndef DEBUG_2229_H
#define DEBUG_2229_H

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

s32 GameModeDebug2229Update(void);
void DebugJitterSprite_Update(struct EwramData_EntityData *entity);

#endif /* DEBUG_2229_H */
