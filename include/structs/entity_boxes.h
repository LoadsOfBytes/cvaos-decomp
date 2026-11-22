#ifndef STRUCTS_ENTITY_BOXES_H
#define STRUCTS_ENTITY_BOXES_H

#include "types.h"
#include "structs/ewram.h"

/* Per-entity-slot table in IWRAM at 0x03001A58 (0x680 bytes, cleared by
 * HitBox_ClearAll). Each slot holds two 4-byte records with the layout of
 * struct SpriteBox (x, y, width, height; attack box first, body box
 * second, e.g. 0x0810FCF8 = {-8, -4, 16, 8}). They are also written as one
 * packed 32-bit value by HitBox_SetAttackRaw / HitBox_SetBodyRaw. */
union EntityBox {
    struct SpriteBox box;
    s32 raw;
};

struct EntityBoxes {
    union EntityBox attack;
    union EntityBox body;
}; /* size = 0x8 */

extern struct EntityBoxes gEntityBoxes[0xD0];

#endif
