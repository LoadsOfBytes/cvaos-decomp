#ifndef WAITER_SKELETON_H
#define WAITER_SKELETON_H

/* Functions of enemy/waiter_skeleton.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void SkullArcher_UpdateIdle(struct EwramData_EntityData *entity);
extern void SkullArcher_UpdateShoot(struct EwramData_EntityData *entity);
extern void SkullArcher_UpdatePatrol(struct EwramData_EntityData *entity);

#endif /* WAITER_SKELETON_H */
