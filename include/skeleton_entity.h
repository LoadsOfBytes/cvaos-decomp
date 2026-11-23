#ifndef SKELETON_ENTITY_H
#define SKELETON_ENTITY_H

/*
 * Work slots of the Skeleton enemy (EnemySkeletonCreate / EnemySkeletonUpdate):
 * - home x / patrol left / patrol right: room x of the spawn point and the
 *   patrol bounds (home +- 0x20);
 * - affine / spin angle: its affine slot and the rotation angle advanced
 *   every frame;
 * - flags: part and state bits (1 = linked part created, 4 = attached part
 *   created, 2 / 8 / 0x20 combat variants);
 * - cooldown: frames before the next attack check;
 * - walk speed: 16.16 walking speed (damped every frame);
 * - throw urge: grows every frame in combat; a throw starts when a random
 *   value falls below it; throw count: bones thrown in the current attack.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define SKELETON_HOME_X(e) e->unk_504.part16.unk_506[0]
#define SKELETON_PATROL_RIGHT(e) e->unk_4F8.part16.unk_4F8
#define SKELETON_PATROL_LEFT(e) e->unk_4F8.part16.unk_4FA
#define SKELETON_AFFINE(e) e->unk_504.part16.unk_504
#define SKELETON_SPIN_ANGLE(e) e->unk_500.part16.unk_502
#define SKELETON_FLAGS(e) e->unk_4FC.part8.unk_4FE
#define SKELETON_COOLDOWN(e) e->unk_500.part16.unk_500
#define SKELETON_WALK_SPEED(e) e->unk_508.whole
#define SKELETON_THROW_URGE(e) e->unk_4FC.part8.unk_4FD
#define SKELETON_THROW_COUNT(e) e->unk_4FC.part8.unk_4FC

/*
 * Death pieces (Skeleton_SpawnDeathPieces, SkeletonDeathPiece_UpdateUnused):
 * the dead skeleton counts its live pieces in death pieces (the throw count
 * slot); a piece keeps the skeleton's entity index and its piece index (row
 * of sSkeletonDeathPieceOffsets, stored in the parent-index slot).
 */
#define SKELETON_DEATH_PIECES(e) e->unk_4FC.part8.unk_4FC
#define SKELETON_PIECE_OWNER_INDEX(e) e->unk_4F8.part8.unk_4F8
#define SKELETON_PIECE_INDEX(e) e->unk_4F4.part8.unk_4F5

/* Thrown bone (SkeletonProjectile_Update): kind (1 = the variant thrown with
 * flag 2: frame 0x1B, sound 0x89). */
#define SKELETON_BONE_KIND(e) e->unk_4FC.part8.unk_4FC

/* EnemySkeletonUpdate stores a random 40..119 here every frame; never read. */
#define SKELETON_UNUSED_DELAY(e) e->unk_4F4.part16.unk_4F6

#endif /* SKELETON_ENTITY_H */
