#ifndef ROOM_OBJECT_ENTITY_H
#define ROOM_OBJECT_ENTITY_H

/*
 * Object 0x0D (swinging object hanging from the ceiling, Object0DUpdate):
 * affine angle (clamped to +-0x1000), angular speed and the swing amplitude
 * (0x100 after an impulse, loses 0x20 per swing).
 */
#define OBJECT0D_ANGLE(e) e->unk_4FC.whole
#define OBJECT0D_SPEED(e) e->unk_500.whole
#define OBJECT0D_AMPLITUDE(e) e->unk_504.whole

/*
 * Object 0x16 (affine zoom-out cutscene object, Object16Update): zoom timer
 * (0x100 frames), BG affine scale, rotation angle and its accelerating step.
 */
#define OBJECT16_TIMER(e) e->unk_4F8.whole
#define OBJECT16_SCALE(e) e->unk_500.whole
#define OBJECT16_ANGLE(e) e->unk_504.whole
#define OBJECT16_SPIN(e) e->unk_508.whole

/*
 * Object 0x37 (Object37_*): affine angle (0..0x3000) and the closed flag (1
 * while its collision blocks are set); its child (Object37Child_Update)
 * turns between 0 and -0x2000.
 */
#define OBJECT37_ANGLE(e) e->unk_4FC.whole
#define OBJECT37_CLOSED(e) e->unk_500.whole
#define OBJECT37_CHILD_ANGLE(e) e->unk_4FC.whole

/*
 * Object 0x14 (rolling affine sprite, Object14Variant0/1_Update): room pixel
 * position and the position of the previous frame, affine angle; variant 0
 * also grows its scale, variant 1 is pushed by the player (flags bit 0, set
 * by Object14_WallLeft/RightCorrection).
 */
#define OBJECT14_ROOM_X(e) e->unk_4FC.part16.unk_4FC
#define OBJECT14_ROOM_Y(e) e->unk_4FC.part16.unk_4FE[0]
#define OBJECT14_PREV_X(e) e->unk_500.part16.unk_500
#define OBJECT14_PREV_Y(e) e->unk_500.part16.unk_502
#define OBJECT14_ANGLE(e) e->unk_504.part16.unk_504
#define OBJECT14_SCALE(e) e->unk_508.whole
#define OBJECT14_FLAGS(e) e->unk_4F8.part16.unk_4F8

/*
 * Object 0x31 (Object31_UpdateState): start delay, then an affine angle
 * turning with a speed that accelerates by the step (4) up to 0x60.
 */
#define OBJECT31_DELAY(e) e->unk_4F8.whole
#define OBJECT31_ANGLE(e) e->unk_4FC.whole
#define OBJECT31_SPEED(e) e->unk_500.whole
#define OBJECT31_STEP(e) e->unk_504.whole

/*
 * Object 0x35 (floor switch, Object35_*): 16.16 home position (its integer
 * halves are also read through unk_4F4.part16.unk_4F6 / unk_4F8.part16.unk_4FA),
 * frames the player has stood on it, and the pressed flag (drawn 0x30 px
 * lower). Its child links back through child owner.
 */
#define OBJECT35_HOME_X(e) e->unk_4F4.whole
#define OBJECT35_HOME_Y(e) e->unk_4F8.whole
#define OBJECT35_STAND_FRAMES(e) e->unk_4FC.whole
#define OBJECT35_PRESSED(e) e->unk_500.whole
#define OBJECT35_CHILD_OWNER(e) e->unk_504.unk_504_entity

/*
 * Object 0x36 (rotating BG-affine platform, Object36*): rotation angle, its
 * step, radius (half the shape width) and the BG affine scale.
 */
#define OBJECT36_ANGLE(e) e->unk_4F8.whole
#define OBJECT36_SPIN(e) e->unk_4FC.part16.unk_4FE[0]
#define OBJECT36_RADIUS(e) e->unk_4FC.part16.unk_4FC
#define OBJECT36_SCALE(e) e->unk_500.part16.unk_500

/*
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */

/*
 * Object 0x14 variant 2 (Object14Variant2_Update; room / previous position as
 * above): swing angle and its random speed.
 */
#define OBJECT14_SWING_ANGLE(e) e->unk_508.part16.unk_508
#define OBJECT14_SWING_SPEED(e) e->unk_508.part16.unk_50A

/*
 * Object 0x30 falling leaf (Object30_SpawnFallingLeaf, Object30Leaf_Update):
 * kind 0..2 (animation, fall speed, parallax) and the BG 1 x scroll of the
 * previous frame (for the parallax).
 */
#define OBJECT30_LEAF_KIND(e) e->unk_4F8.whole
#define OBJECT30_LEAF_SCROLL_X(e) e->unk_4FC.whole

/* Object 0x19 (Object19Create / Object19Update): affine angle and its step. */
#define OBJECT19_ANGLE(e) e->unk_4F8.part16.unk_4F8
#define OBJECT19_SPIN(e) e->unk_4F8.part16.unk_4FA

/*
 * Object 0x15 (Object15Update, its orbiters read the owner): orbit angle
 * (+0x40 per frame) and the 16.16 orbit radius (shrinks from 0x100000).
 */
#define OBJECT15_ANGLE(e) e->unk_4FC.part16.unk_4FC
#define OBJECT15_RADIUS(e) e->unk_500.whole

/*
 * Object 0x08 (breakable block, Object08Update) and its dust child
 * (BreakableBlock_SpawnDust): block kind / orientation byte (copied to the
 * child, selects the room flag and the dust spread); the child keeps the dust
 * ObjGfx slot. Object 0x1B (conveyor, Object1BUpdate): scroll position and the
 * colour of the current frame. Object 0x25 spawner (Object25Update): live
 * children (each links back through spawner and decrements it).
 */
#define OBJECT08_KIND(e) e->unk_4F8.part8.unk_4F8
#define OBJECT08_DUST_GFX(e) e->unk_4FC.whole
#define OBJECT1B_SCROLL(e) e->unk_4F8.whole
#define OBJECT1B_COLOR(e) e->unk_4FC.part8.unk_4FC
#define OBJECT25_CHILD_COUNT(e) e->unk_4FC.whole
#define OBJECT25_SPAWNER(e) e->unk_500.unk_500_entity

/*
 * Elevator (object 0x17, Object17Update): frames up / down has been held on
 * it (starts moving after 3). Warp room (object 0x1F, WarpRoom_Update): bottom
 * line of window 0 (0x74 on arrival, closes to 0x24).
 */
#define OBJECT17_HOLD_FRAMES(e) e->unk_4FC.part8.unk_4FC
#define WARP_ROOM_WINDOW_BOTTOM(e) e->unk_4F8.part16.unk_4F8

/*
 * Object 0x26 (Object26Create / _UpdateState): frame clocks of its two traps,
 * counted while the target is in its area; object 0x27 fires when the first
 * reaches 0x78 (starts at 0x1E), object 0x28 when the second does (starts at
 * 0x5A); each trap resets its clock.
 * Warp room gate parts (WarpRoom_UpdateGatePart - TU aliases DIST / SPEED /
 * ACCEL / POS / STEP / HOMEX / HOMEY): travelled distance, speed and
 * acceleration of the opening, position / step of the part and its home x / y.
 */
#define OBJECT26_CLOCK_27(e) e->unk_4FC.part16.unk_4FC
#define OBJECT26_CLOCK_28(e) e->unk_4FC.part16.unk_4FE[0]
#define WARP_GATE_DIST(e) e->unk_4F8.whole
#define WARP_GATE_SPEED(e) e->unk_4FC.part16.unk_4FE[0]
#define WARP_GATE_ACCEL(e) e->unk_4FC.part16.unk_4FC
#define WARP_GATE_POS(e) e->unk_504.part16.unk_504
#define WARP_GATE_STEP(e) e->unk_504.part16.unk_506[0]
#define WARP_GATE_HOME_X(e) e->unk_500.part16.unk_500
#define WARP_GATE_HOME_Y(e) e->unk_500.part16.unk_502

#endif /* ROOM_OBJECT_ENTITY_H */
