#ifndef WHEEL_ENEMY_ENTITY_H
#define WHEEL_ENEMY_ENTITY_H

/*
 * Work slots of Buer and Bael, the rolling wheel enemies (Bael's code is a copy
 * of Buer's with its own data):
 * - speed: signed 16.16 rolling speed passed to Entity_Walk (sign = direction,
 *   eased and bounced off walls);
 * - angle: affine rotation, advanced by speed / 64 while rolling;
 * - direction: turn direction while charging;
 * Their charge effect entities keep the animation index to play.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define WHEEL_SPEED(e) e->unk_4FC.whole
#define WHEEL_ANGLE(e) e->unk_4F8.whole
#define WHEEL_DIRECTION(e) e->unk_4F4.part8.unk_4F7
#define WHEEL_EFFECT_ANIM(e) e->unk_4F4.part8.unk_4F6

/*
 * Death (Buer_UpdateDeath / Bael_UpdateDeath - TU aliases SHAKE_X / SHAKE_Y):
 * shake offset of the head part, bytes 0 / 1 of the angle slot.
 */
#define WHEEL_SHAKE(e) e->unk_4F8.bytes

#endif /* WHEEL_ENEMY_ENTITY_H */
