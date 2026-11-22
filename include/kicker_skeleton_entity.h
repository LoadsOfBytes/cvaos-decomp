#ifndef KICKER_SKELETON_ENTITY_H
#define KICKER_SKELETON_ENTITY_H

/*
 * Kicker Skeleton (KickerSkeleton_*):
 * - flags: bit 0 while kicking (stronger contact damage with knockback);
 * - speed: 16.16 walk speed of the kick dash;
 * - hit side: +1 / -1, side the killing hit came from (scatters fragments).
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define KICKER_SKELETON_FLAGS(e) e->unk_4F8.whole
#define KICKER_SKELETON_SPEED(e) e->unk_4FC.whole
#define KICKER_SKELETON_HIT_SIDE(e) e->unk_504.whole

#endif /* KICKER_SKELETON_ENTITY_H */
