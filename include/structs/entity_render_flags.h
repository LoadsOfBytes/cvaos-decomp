#ifndef STRUCTS_ENTITY_RENDER_FLAGS_H
#define STRUCTS_ENTITY_RENDER_FLAGS_H

#include "types.h"

/* In EwramData_EntityData, the bitfields named unk_53C_* and unk_53D_*
 * occupy the two bytes at entity-relative offsets 0x58 and 0x59. Some
 * rendering/allocation checks intentionally read both bytes as one halfword.
 *
 * Keep this raw view separate from the existing bitfield declarations:
 * agbcc does not support the anonymous union needed to expose both views
 * under the current field names. A macro also preserves the caller's
 * address/result temporaries; an inline getter changes their allocation.
 * This view documents the observed layout, not the original source spelling.
 */
enum {
    ENTITY_RENDER_FLAGS_OFFSET = 0x58
};

/* Byte and address views retain the caller's original access width, including
 * the locally defined bitfield views used by some matching routines. */
#define ENTITY_RENDER_FLAGS_ADDRESS(entity) \
    ((u8 *)(entity) + ENTITY_RENDER_FLAGS_OFFSET)
#define ENTITY_RENDER_FLAGS_HIGH_ADDRESS(entity) \
    ((u8 *)(entity) + ENTITY_RENDER_FLAGS_OFFSET + 1)

#define ENTITY_RENDER_FLAGS16(entity) \
    (*(u16 *)((u8 *)(entity) + ENTITY_RENDER_FLAGS_OFFSET))

#endif /* STRUCTS_ENTITY_RENDER_FLAGS_H */
