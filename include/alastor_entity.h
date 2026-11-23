#ifndef ALASTOR_ENTITY_H
#define ALASTOR_ENTITY_H

/*
 * Work slots of Alastor (same layout as Nemesis, see nemesis_entity.h):
 * - angle: heading / affine rotation, turn: rotation added per frame;
 * - flags: state bits shared by the attack routines (2, 0x20, ...);
 * - part index: entity index of its linked part (Alastor_CreateLinkedPart);
 * - hits before react: hits it absorbs before the hit reaction (reset to 3);
 * - wait timer: random wait while positioning;
 * - attack set / index: attack table selection (Alastor_UpdateAttackSelection).
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define ALASTOR_ANGLE(e) e->unk_500.part16.unk_500
#define ALASTOR_TURN(e) e->unk_500.part16.unk_502
#define ALASTOR_FLAGS(e) e->unk_4FC.part16.unk_4FC
#define ALASTOR_PART_INDEX(e) e->unk_4F4.part8.unk_4F6
#define ALASTOR_HITS_BEFORE_REACT(e) e->unk_508.part8.unk_508
#define ALASTOR_WAIT_TIMER(e) e->unk_4F8.part16.unk_4F8
#define ALASTOR_ATTACK_SET(e) e->unk_4F8.part8.unk_4FA
#define ALASTOR_ATTACK_INDEX(e) e->unk_4F8.part8.unk_4FB

/*
 * Phase / timer of the current movement (TU aliases PHASE / TIMER): rocking
 * phase while approaching, arc timer, recoil phase.
 */
#define ALASTOR_PHASE(e) e->unk_4FC.part16.unk_4FE[0]

/* Pattern attack (Alastor_UpdatePatternAttack, TU alias INDEX): segment of
 * the movement pattern; reset by Alastor_UpdateSpinningLeap. */
#define ALASTOR_PATTERN_STEP(e) e->unk_4F4.part8.unk_4F7

#endif /* ALASTOR_ENTITY_H */
