#ifndef DEVIL_ENTITY_H
#define DEVIL_ENTITY_H

/*
 * Devil main entity:
 * - target x / y: room position it flies to (beside the target, random
 *   height), speed: its approach speed (eased up to 0x20000);
 * - timer: frame counter of the current approach / attack step;
 * - intangible: nonzero = no contact damage check (parts also skip their
 *   hit checks unless it is 1);
 * - parts hidden: nonzero = the body parts are not drawn.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define DEVIL_TARGET_X(e) e->unk_4F4.whole
#define DEVIL_TARGET_Y(e) e->unk_4F8.whole
#define DEVIL_SPEED(e) e->unk_4FC.whole
#define DEVIL_TIMER(e) e->unk_508.part16.unk_508
#define DEVIL_INTANGIBLE(e) e->unk_508.part8.unk_50A
#define DEVIL_PARTS_HIDDEN(e) e->unk_508.part8.unk_50B

/* Parts (DevilPart_Update): affine angle (from the joint direction). */
#define DEVIL_PART_ANGLE(e) e->unk_4FC.whole

/*
 * Projectile (DevilProjectile_Spawn / _Update): owner, 16.16 offset from the
 * owner it is held at, and the frame (timer value) it is released on.
 */
#define DEVIL_PROJECTILE_OWNER(e) e->unk_500.unk_500_entity
#define DEVIL_PROJECTILE_OFFSET_X(e) e->unk_4F8.whole
#define DEVIL_PROJECTILE_OFFSET_Y(e) e->unk_4FC.whole
#define DEVIL_PROJECTILE_RELEASE(e) e->unk_504.part8.unk_504

/* Charge effect (DevilChargeEffect_Spawn / _Update): 16.16 offset from the Devil. */
#define DEVIL_CHARGE_OFFSET_X(e) e->unk_4F8.whole
#define DEVIL_CHARGE_OFFSET_Y(e) e->unk_4FC.whole

/* Charge effect (DevilChargeEffect_Spawn / _Update): the Devil it follows. */
#define DEVIL_CHARGE_OWNER(e) e->unk_500.unk_500_entity

#endif /* DEVIL_ENTITY_H */
