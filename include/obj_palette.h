#ifndef OBJ_PALETTE_H
#define OBJ_PALETTE_H

/* Functions of engine/obj_palette.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void SharedObjPalette_RestoreAll(void);
extern void SharedObjPalette_Release(void *palette, u8 variant);
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);

#endif /* OBJ_PALETTE_H */
