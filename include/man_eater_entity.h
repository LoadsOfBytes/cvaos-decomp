#ifndef MAN_EATER_ENTITY_H
#define MAN_EATER_ENTITY_H

/*
 * Man Eater heads (created by EnemyManEaterUpdate, ManEaterHead_Update):
 * - owner index: entity index of the Man Eater body;
 * - orbit slot: which sManEaterHeadOrbitPoints entry the head uses (also the
 *   index of a neck link);
 * - flags: 1 = lunging, 2 = may lunge again, 4 = neck links created;
 * - angle / home angle: current angle of the head (kept within +-0x2000 of
 *   the home angle); base angle: angle offset added to the body rotation;
 * - lunge speed: 16.16 extension speed (grows to 0x60000).
 * Neck links keep the head index (owner index), the previous link index, the
 * base entity index, their slot and their angle.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define MAN_EATER_OWNER_INDEX(e) e->unk_4F8.part8.unk_4F8
#define MAN_EATER_PREV_INDEX(e) e->unk_4F8.part8.unk_4F9
#define MAN_EATER_BASE_INDEX(e) e->unk_4F8.part8.unk_4FA
#define MAN_EATER_ORBIT_SLOT(e) e->unk_4F8.part8.unk_4FB
#define MAN_EATER_HEAD_FLAGS(e) e->unk_4F4.part8.unk_4F5
#define MAN_EATER_ANGLE(e) e->unk_4FC.part16.unk_4FC
#define MAN_EATER_HOME_ANGLE(e) e->unk_4FC.part16.unk_4FE[0]
#define MAN_EATER_BASE_ANGLE(e) e->unk_508.part16.unk_508
#define MAN_EATER_LUNGE_SPEED(e) e->unk_504.whole

/*
 * Body (EnemyManEaterCreate / EnemyManEaterUpdate): number of heads still
 * alive (3 at first; dies at 0), its affine angle and the sway phase it is
 * computed from.
 */
#define MAN_EATER_HEAD_COUNT(e) e->unk_500.part8.unk_500
#define MAN_EATER_BODY_ANGLE(e) e->unk_4FC.part16.unk_4FC
#define MAN_EATER_SWAY_PHASE(e) e->unk_500.part16.unk_502

/*
 * Neck links (ManEaterSegment_Update; head / orbit slot in OWNER_INDEX /
 * ORBIT_SLOT, the swing angle in MAN_EATER_ANGLE): swing speed, state saved
 * while the head lunges, and the flag of the tip link (slot 5) that drops
 * projectiles once the head is below 200 HP.
 */
#define MAN_EATER_LINK_SWING_SPEED(e) e->unk_4FC.part16.unk_4FE[0]
#define MAN_EATER_LINK_SAVED_STATE(e) e->unk_500.part8.unk_500
#define MAN_EATER_LINK_DROPS(e) e->unk_500.part8.unk_501

#endif /* MAN_EATER_ENTITY_H */
