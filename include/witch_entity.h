#ifndef WITCH_ENTITY_H
#define WITCH_ENTITY_H

/*
 * Witch (Witch_*): flags (bit 0 = invulnerable while charging, bit 1 =
 * casting, bit 2 = wait set up; Witch_UpdateCast / _UpdateCharge write the
 * byte through a cast pointer) and the 16.16 point it flies to.
 */
#define WITCH_FLAGS(e) e->unk_4F8.part8.unk_4F8
#define WITCH_MOVE_X(e) e->unk_500.whole
#define WITCH_MOVE_Y(e) e->unk_504.whole

/*
 * Spell (WitchSpell_*): affine scale, angle and spin, homing delay, the room
 * point it flies to and the entity index of the casting witch.
 */
#define WITCH_SPELL_SCALE(e) e->unk_4F4.whole
#define WITCH_SPELL_ANGLE(e) e->unk_4F8.part16.unk_4F8
#define WITCH_SPELL_SPIN(e) e->unk_4F8.part16.unk_4FA
#define WITCH_SPELL_DELAY(e) e->unk_4FC.part16.unk_4FC
#define WITCH_SPELL_TARGET_X(e) e->unk_500.whole
#define WITCH_SPELL_TARGET_Y(e) e->unk_504.whole
#define WITCH_SPELL_OWNER_INDEX(e) e->unk_50C.part8.unk_50C

/* Orbit particle (WitchOrbitParticle_Update): radius, angle and spin. */
#define WITCH_ORBIT_RADIUS(e) e->unk_4F4.whole
#define WITCH_ORBIT_ANGLE(e) e->unk_4F8.part16.unk_4F8
#define WITCH_ORBIT_SPIN(e) e->unk_4F8.part16.unk_4FA

/* Ground projectile (WitchGroundProjectile_Update): walk speed. */
#define WITCH_GROUND_SPEED(e) e->unk_4F8.whole

/*
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */

/* Cast (Witch_UpdateCast - TU alias TIMER): frames of the current cast step. */
#define WITCH_CAST_TIMER(e) e->unk_4FC.part16.unk_4FC

#endif /* WITCH_ENTITY_H */
