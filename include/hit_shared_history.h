#ifndef HIT_SHARED_HISTORY_H
#define HIT_SHARED_HISTORY_H

/* Functions of engine/hit_shared_history.c used by other files. */

#include "types.h"
#include "structs/ewram.h"

struct HitResult;
extern u16 Hit_CheckWithPlayerSharedHistory(struct HitResult *res, struct EwramData_EntityData *entity, struct EwramData_EntityData *owner);

#endif // HIT_SHARED_HISTORY_H
