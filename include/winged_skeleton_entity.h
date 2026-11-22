#ifndef WINGED_SKELETON_ENTITY_H
#define WINGED_SKELETON_ENTITY_H

/*
 * Winged Skeleton spawner (EnemyWingedSkeletonUpdate): number of live
 * skeletons it created; each skeleton links back through spawner and
 * decrements the count when it is deleted.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define WINGED_SKELETON_CHILD_COUNT(e) e->unk_4FC.whole
#define WINGED_SKELETON_SPAWNER(e) e->unk_4FC.unk_4FC_entity

/*
 * Flight (WingedSkeleton_UpdateFly and its TU macros): set when the tile
 * ahead in the dive direction is solid; the skeleton then turns instead of
 * tracking the target.
 */
#define WINGED_SKELETON_BLOCKED(e) e->unk_500.part8.unk_500

#endif /* WINGED_SKELETON_ENTITY_H */
