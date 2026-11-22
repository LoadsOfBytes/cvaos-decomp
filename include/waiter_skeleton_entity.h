#ifndef WAITER_SKELETON_ENTITY_H
#define WAITER_SKELETON_ENTITY_H

/*
 * Waiter Skeleton (WaiterSkeleton_*): flags - bit 0 the tray was thrown,
 * bit 1 the tray still carries its item, bit 2 set from the walk's third
 * animation step, bit 3 a thrown tray hit the player.
 */
#define WAITER_SKELETON_FLAGS(e) e->unk_504.whole

/*
 * Trays (WaiterSkeletonTray_Update, WaiterSkeletonThrownTray_*): owner,
 * animation script of the thrown object (17 = item, 9 = tray) and the affine
 * angle of the spinning thrown tray.
 */
#define WAITER_TRAY_OWNER(e) e->unk_4FC.unk_4FC_entity
#define WAITER_TRAY_ANIM(e) e->unk_4F8.part16.unk_4F8
#define WAITER_TRAY_ANGLE(e) e->unk_504.whole

/*
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */

#endif /* WAITER_SKELETON_ENTITY_H */
