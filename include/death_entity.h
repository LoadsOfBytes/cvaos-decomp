#ifndef DEATH_ENTITY_H
#define DEATH_ENTITY_H

/*
 * Death (boss) family:
 * - owner index: Death's helper entities (weapons, blades, pillars, effects)
 *   keep the entity index of their owner in this byte instead of the usual
 *   ENTITY_PARENT_INDEX byte;
 * - flags: Death's state bits (1 = floating weapon gone, 2 = hit, 4 = weapon
 *   attack running, 8 / 0x80 = throw attack steps, 0x10 = wall attack,
 *   0x20 = intro done, 0x40 = weapon damaged);
 * - weapon angle / timer: rotation of the floating weapon and the countdown of
 *   its current move.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define DEATH_OWNER_INDEX(e) e->unk_4F8.part8.unk_4F8
#define DEATH_FLAGS(e) e->unk_4F8.part16.unk_4F8
#define DEATH_WEAPON_ANGLE(e) e->unk_4FC.part16.unk_4FC
#define DEATH_WEAPON_TIMER(e) e->unk_4F8.part16.unk_4FA

/*
 * Death attacks: dash step (Death_UpdateDashAttack, 0..5) and the room pixel
 * point the dash / throw attack flies to.
 * Blade (DeathBlade_Update): heading, 16.16 speed (to 0x18000) and spin angle.
 * Pillar (DeathPillar_Update): affine x / y scale.
 */
#define DEATH_DASH_STEP(e) e->unk_4FC.part8.unk_4FC
#define DEATH_TARGET_X(e) e->unk_500.part16.unk_500
#define DEATH_TARGET_Y(e) e->unk_500.part16.unk_502
#define DEATH_BLADE_HEADING(e) e->unk_4FC.part16.unk_4FC
#define DEATH_BLADE_SPEED(e) e->unk_504.whole
#define DEATH_BLADE_SPIN(e) e->unk_4FC.part16.unk_4FE[0]
#define DEATH_PILLAR_SCALE_X(e) e->unk_500.whole
#define DEATH_PILLAR_SCALE_Y(e) e->unk_504.whole

/*
 * Weapon (DeathWeapon_Update - TU aliases SPIN / SPEED): spin speed of the
 * thrown weapon; the secondary (mirror) instance has the mirror flag set and
 * keeps the entity index of the primary weapon. The arm pose follows the
 * owner's DEATH_DASH_STEP (index into sDeathWeaponArmPoses).
 */
#define DEATH_WEAPON_SPIN(e) e->unk_4FC.part16.unk_4FE[0]
#define DEATH_WEAPON_MIRROR(e) e->unk_4F8.part8.unk_4FA
#define DEATH_WEAPON_PRIMARY_INDEX(e) e->unk_4F8.part8.unk_4F9

#endif /* DEATH_ENTITY_H */
