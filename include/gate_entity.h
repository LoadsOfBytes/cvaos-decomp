#ifndef GATE_ENTITY_H
#define GATE_ENTITY_H

/*
 * Gate (Gate_*, object 0): opening angle of the two wings (0 = closed,
 * 0x4000 = open, stepped by 0x400), the affine slots of the two wings
 * (Gate_Draw reads them as GATE_AFFINE_BYTES[0] / [1]) and the palette slot.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define GATE_OPEN_ANGLE(e) e->unk_4F8.whole
#define GATE_AFFINE_0(e) e->unk_4FC.part8.unk_4FC
#define GATE_AFFINE_1(e) e->unk_4FC.part8.unk_4FD
#define GATE_PALETTE(e) e->unk_4FC.part8.unk_4FF

/* Both wing affine slots as a byte array (Gate_Draw). */
#define GATE_AFFINE_BYTES(e) e->unk_4FC.bytes

#endif /* GATE_ENTITY_H */
