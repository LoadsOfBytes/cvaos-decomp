#ifndef WEREJAGUAR_ENTITY_H
#define WEREJAGUAR_ENTITY_H

/*
 * Werejaguar work slots:
 * - manual anim: nonzero = the attack code steps the animation itself
 *   (EnemyWerejaguarUpdate skips Anim_Update);
 * - contact kind: which attack the contact damage belongs to (0 = body,
 *   1 = claw 3/4 power, 2 = knock back, 3 = strong knock back);
 * - contact hit: set by Werejaguar_DamagePlayerOnContact after a hit.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define WEREJAGUAR_MANUAL_ANIM(e) e->unk_4F8.part8.unk_4F8
#define WEREJAGUAR_CONTACT_KIND(e) e->unk_4F8.part8.unk_4F9
#define WEREJAGUAR_CONTACT_HIT(e) e->unk_4F8.part8.unk_4FB

/*
 * Death effect (WerejaguarDeathEffect_Update): sway angle, screen x / y and
 * the camera x / y at spawn (keeps it fixed in the room), base sprite layer
 * (drawn one layer in front of / behind it).
 */
#define WEREJAGUAR_EFFECT_ANGLE(e) e->unk_4F8.whole
#define WEREJAGUAR_EFFECT_X(e) e->unk_500.part16.unk_500
#define WEREJAGUAR_EFFECT_CAMERA_X(e) e->unk_500.part16.unk_502
#define WEREJAGUAR_EFFECT_Y(e) e->unk_504.part16.unk_504
#define WEREJAGUAR_EFFECT_CAMERA_Y(e) e->unk_504.part16.unk_506[0]
#define WEREJAGUAR_EFFECT_LAYER(e) e->unk_4FC.whole

/* Cleared when the attack starts; never read. */
#define WEREJAGUAR_UNUSED_FLAG(e) e->unk_4F8.part8.unk_4FA

#endif /* WEREJAGUAR_ENTITY_H */
