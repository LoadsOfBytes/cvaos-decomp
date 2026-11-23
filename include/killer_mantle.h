#ifndef KILLER_MANTLE_H
#define KILLER_MANTLE_H

/* Functions of enemy/killer_mantle.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void DeathOverlay_Update(struct EwramData_EntityData *entity);
extern s32 DeathWeapon_UpdateWallTraversal(struct EwramData_EntityData *entity, struct EwramData_EntityData *target);

#endif /* KILLER_MANTLE_H */
