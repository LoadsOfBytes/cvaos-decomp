#ifndef HARPY_ENTITY_H
#define HARPY_ENTITY_H

/*
 * Harpy (Harpy_UpdateAttack): room point of the dive (0x18 px above the
 * target) and the y it started from. Feathers (Harpy_ThrowFeather /
 * _ThrowWavyFeather, HarpyWavyFeather_Update): flight angle.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define HARPY_DIVE_X(e) e->unk_4F8.whole
#define HARPY_DIVE_Y(e) e->unk_4FC.whole
#define HARPY_START_Y(e) e->unk_500.whole
#define HARPY_FEATHER_ANGLE(e) e->unk_4F8.whole

#endif /* HARPY_ENTITY_H */
