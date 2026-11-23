#ifndef GRAHAM_PROJECTILE_H
#define GRAHAM_PROJECTILE_H

/*
 * Graham's projectiles keep their work slots in ext.grahamShot,
 * ext.grahamSpiral and ext.grahamSpread. Views that do not fit a member:
 * - spiral spin: the spin added to the ring angle per frame (outside the
 *   work area);
 * - spiral radius pixels: the integer half of the 16.16 ring radius
 *   (ext.grahamSpiral.radius), read directly.
 * The macros expand to member chains or to the ENTITY_* views of the common
 * block (structs/ewram.h) without added parentheses; use them with a plain
 * base expression.
 */
#define GRAHAM_SPIRAL_SPIN(e) ENTITY_WORD_510(e)
#define GRAHAM_SPIRAL_RADIUS_PIXELS(e) e->ext.raw.unk_508.part16.unk_50A

#endif /* GRAHAM_PROJECTILE_H */
