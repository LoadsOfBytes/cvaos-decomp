#ifndef GRAHAM_PROJECTILE_H
#define GRAHAM_PROJECTILE_H

/*
 * Graham's homing scene projectile (Graham_CreateSceneProjectile,
 * GrahamSceneProjectile_Update): homing flag (steer towards the player), the
 * flight angle, the turn rate per frame (grows to 0x4000) and the 16.16 speed
 * (grows to 0x80000).
 * Spiral projectiles (Graham_SpawnSpiralProjectiles,
 * GrahamSpiralProjectile_Update): index and count of the ring, the spin added
 * to the ring angle per frame, and the sprite rotation with its speed.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define GRAHAM_SHOT_HOMING(e) e->unk_508.part8.unk_508
#define GRAHAM_SHOT_ANGLE(e) e->unk_4FC.whole
#define GRAHAM_SHOT_TURN(e) e->unk_500.whole
#define GRAHAM_SHOT_SPEED(e) e->unk_504.whole
#define GRAHAM_SPIRAL_INDEX(e) e->unk_4F8.part8.unk_4F8
#define GRAHAM_SPIRAL_COUNT(e) e->unk_4F8.part8.unk_4F9
#define GRAHAM_SPIRAL_SPIN(e) e->unk_510.whole
#define GRAHAM_SPIRAL_ROTATION(e) e->unk_500.part16.unk_500
#define GRAHAM_SPIRAL_ROTATION_SPEED(e) e->unk_500.part16.unk_502

/*
 * Spiral projectile geometry (GrahamSpiralProjectile_Update - TU aliases
 * SCALE / RADIUS / ANGLE): affine scale, 16.16 ring radius (its integer half
 * is read directly) and ring angle.
 */
#define GRAHAM_SPIRAL_SCALE(e) e->unk_504.whole
#define GRAHAM_SPIRAL_RADIUS(e) e->unk_508.whole
#define GRAHAM_SPIRAL_RADIUS_PIXELS(e) e->unk_508.part16.unk_50A
#define GRAHAM_SPIRAL_ANGLE(e) e->unk_50C.whole

#endif /* GRAHAM_PROJECTILE_H */
