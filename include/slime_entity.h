#ifndef SLIME_ENTITY_H
#define SLIME_ENTITY_H

/*
 * Work slots of the Slime enemy (crawls along floors, walls and ceilings):
 * - speed: signed crawl speed along the surface (the sign is the direction);
 * - angle: surface angle (affine rotation), turned towards
 *   sSlimeSurfaceTurnTargets[turn surface] by the turn rate per frame;
 * - turn cooldown: frames after a turn before the next surface check;
 * - flags: 1 = was hit (start the hurt animation), 2 = hurt animation playing,
 *   4 = detached and falling;
 * - scale x / y: affine scale (squash while crawling and dying);
 * - front / back support: room coordinates of the two support probes
 *   (Slime_UpdateSurfaceSupport).
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define SLIME_SPEED(e) e->unk_4F8.whole
#define SLIME_ANGLE(e) e->unk_504.part16.unk_504
#define SLIME_TURN_RATE(e) e->unk_504.part16.unk_506[0]
#define SLIME_TURN_SURFACE(e) e->unk_4F4.part8.unk_4F7
#define SLIME_TURN_COOLDOWN(e) e->unk_4F4.part8.unk_4F6
#define SLIME_FLAGS(e) e->unk_4F4.part8.unk_4F5
#define SLIME_SCALE_X(e) e->unk_4FC.whole
#define SLIME_SCALE_Y(e) e->unk_500.whole
#define SLIME_FRONT_SUPPORT_X(e) e->unk_508.part16.unk_508
#define SLIME_FRONT_SUPPORT_Y(e) e->unk_508.part16.unk_50A
#define SLIME_BACK_SUPPORT_X(e) e->unk_50C.part16.unk_50C
#define SLIME_BACK_SUPPORT_Y(e) e->unk_50C.part16.unk_50E

#endif /* SLIME_ENTITY_H */
