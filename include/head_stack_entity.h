#ifndef HEAD_STACK_ENTITY_H
#define HEAD_STACK_ENTITY_H

/*
 * Head stack (HeadStack_Create / HeadStack_Update): entity indices of its
 * segments, bottom first (spawnParam0 = height 1..8; 0 = destroyed), and the
 * on-screen flag of the keeper. Segments (HeadStackSegment_*): position in the
 * column (0 = bottom; its parity picks the attack) and the variant index into
 * sUnk_081193DC (graphics, debris, attacks), which their effects and
 * projectiles read from the segment or keep themselves.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define HEAD_STACK_SLOTS(e) e->unk_4FC.bytes
#define HEAD_STACK_ON_SCREEN(e) e->unk_4F8.part8.unk_4F9
#define HEAD_STACK_LEVEL(e) e->unk_4F4.part8.unk_4F5
#define HEAD_STACK_TYPE(e) e->unk_4F4.part8.unk_4F6

#endif /* HEAD_STACK_ENTITY_H */
