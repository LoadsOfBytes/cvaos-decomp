#ifndef QUEZLCOATL_ENTITY_H
#define QUEZLCOATL_ENTITY_H

/*
 * Quezlcoatl (EnemyQuezlcoatl*, Quezlcoatl_*, QuezlcoatlSegment_Update):
 * - history: heap buffer (60 bytes) of past head positions the segments
 *   follow (owner's buffer in the segments);
 * - bounce count: water bounces since the last turn (re-faces the target on
 *   even counts);
 * - camera x / y: camera position of the previous frame (scroll correction,
 *   head and segments).
 */
#define QUEZLCOATL_HISTORY(e) e->unk_504.whole
#define QUEZLCOATL_BOUNCE_COUNT(e) e->unk_4F8.part16.unk_4F8
#define QUEZLCOATL_CAMERA_X(e) e->unk_4FC.part16.unk_4FC
#define QUEZLCOATL_CAMERA_Y(e) e->unk_4FC.part16.unk_4FE[0]
#define QUEZLCOATL_SEGMENT_INDEX(e) e->unk_500.part8.unk_500

/*
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */

#endif /* QUEZLCOATL_ENTITY_H */
