#ifndef HIT_FLAGS_CONSTANTS_H
#define HIT_FLAGS_CONSTANTS_H

/*
 * Bits of EntityHit.flags (entity->hit.flags). HitBox_Check* test them:
 * A is hurt by B only when A lacks HIT_NO_DAMAGE and B lacks HIT_NO_ATTACK.
 */
#define HIT_ACTIVE      0x1 /* the hit block has boxes (HitBox_GetBoxes) */
#define HIT_NO_ATTACK   0x2 /* the attack box does not hurt others */
#define HIT_NO_DAMAGE   0x4 /* the body box cannot be hurt */

#endif /* HIT_FLAGS_CONSTANTS_H */
