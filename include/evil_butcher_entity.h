#ifndef EVIL_BUTCHER_ENTITY_H
#define EVIL_BUTCHER_ENTITY_H

/*
 * Evil Butcher projectile (EvilButcherProjectile_Update): affine angle, spin
 * step and the x offset (+-6 px) of its ground probe.
 * Debris (EvilButcherDebris_Update): affine angle and spin step.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define EVIL_BUTCHER_PROJECTILE_ANGLE(e) e->unk_4FC.part16.unk_4FC
#define EVIL_BUTCHER_PROJECTILE_SPIN(e) e->unk_4FC.part16.unk_4FE[0]
#define EVIL_BUTCHER_PROJECTILE_PROBE_X(e) e->unk_500.part16.unk_500
#define EVIL_BUTCHER_DEBRIS_ANGLE(e) e->unk_4FC.part16.unk_4FC
#define EVIL_BUTCHER_DEBRIS_SPIN(e) e->unk_4FC.part16.unk_4FE[0]

#endif /* EVIL_BUTCHER_ENTITY_H */
