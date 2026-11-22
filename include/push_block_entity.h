#ifndef PUSH_BLOCK_ENTITY_H
#define PUSH_BLOCK_ENTITY_H

/*
 * Push block (PushBlock_Update, object 01 group): flags - bit 0 / 1 pushed
 * from the left / right this frame (set by the group's wall callbacks), bit 2
 * carrying a block, bit 3 falling, bit 4 scraping sound playing.
 * Group (PushBlockGroup_*): entity slots of its blocks (byte array).
 */
#define PUSH_BLOCK_FLAGS(e) e->unk_4FC.whole
#define PUSH_BLOCK_GROUP_SLOTS(e) e->unk_4F8.bytes

/*
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */

#endif /* PUSH_BLOCK_ENTITY_H */
