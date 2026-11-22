#ifndef STOLAS_ENTITY_H
#define STOLAS_ENTITY_H

/*
 * Stolas (EnemyStolasUpdate; joint slots in joint_entity.h): per-frame flags
 * (cleared every update; bit 0 = show the held frame instead of the
 * animation) and the held frame.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define STOLAS_FLAGS(e) e->unk_4FC.part8.unk_4FC
#define STOLAS_HELD_FRAME(e) e->unk_4FC.part8.unk_4FD

/*
 * Draw offset in whole pixels (Stolas_Draw adds it to the position; set to
 * (0, -29) by EnemyStolasCreate) and the beam length of the beam attack
 * (TU alias DISTANCE, passed to Stolas_SpawnBeamSegment).
 */
#define STOLAS_DRAW_OFFSET_X(e) e->unk_4F8.part16.unk_4F8
#define STOLAS_DRAW_OFFSET_Y(e) e->unk_4F8.part16.unk_4FA
#define STOLAS_BEAM_LENGTH(e) e->unk_508.part16.unk_508

#endif /* STOLAS_ENTITY_H */
