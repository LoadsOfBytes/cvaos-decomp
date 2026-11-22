#ifndef BEAM_SKELETON_ENTITY_H
#define BEAM_SKELETON_ENTITY_H

/*
 * Beam Skeleton (same patrol / approach logic as the Skeleton):
 * - home x / patrol left / patrol right: spawn x and patrol bounds (+-64);
 * - cooldown: frames before the next attack check; walk speed: 16.16 speed
 *   (damped every frame); fire urge: grows while approaching, firing starts
 *   when a random value falls below it;
 * - firing: bit 0 = the beam is active;
 * - beam level: growing beam scale (0xFFFF = full, the beam then deals
 *   damage);
 * Its beam entities keep the skeleton's entity index (owner index) and a
 * frame counter.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define BEAM_SKELETON_HOME_X(e) e->unk_504.part16.unk_506[0]
#define BEAM_SKELETON_PATROL_RIGHT(e) e->unk_4F8.part16.unk_4F8
#define BEAM_SKELETON_PATROL_LEFT(e) e->unk_4F8.part16.unk_4FA
#define BEAM_SKELETON_COOLDOWN(e) e->unk_500.part16.unk_500
#define BEAM_SKELETON_WALK_SPEED(e) e->unk_508.whole
#define BEAM_SKELETON_FIRE_URGE(e) e->unk_4FC.part8.unk_4FD
#define BEAM_SKELETON_FIRING(e) e->unk_4FC.part8.unk_4FE
#define BEAM_SKELETON_BEAM_LEVEL(e) e->unk_504.part16.unk_504
#define BEAM_OWNER_INDEX(e) e->unk_4F4.part8.unk_4F6
#define BEAM_COUNTER(e) e->unk_4F8.part8.unk_4F8

#endif /* BEAM_SKELETON_ENTITY_H */
