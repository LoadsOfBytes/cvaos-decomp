#ifndef LEGION_ENTITY_H
#define LEGION_ENTITY_H

/*
 * Work slots of the Legion main entity (EnemyLegionCreate / EnemyLegionUpdate):
 * - data: address of its heap block (struct LegionState: room position,
 *   rotation, bob phase, shell states), freed when it dies;
 * - circle dir: 1 / 2 = which way it circles the room edges (chosen towards
 *   the target), retarget timer: frames until the direction is chosen again;
 * - flags: 1 = no contact damage, 2 = bob up and down, 4 = rotate the
 *   affine sprite, 8 / 0x10 / 0x20 = shell part ring 1..3 spawned;
 * - broken cores: bit per core quadrant that has broken off
 *   (LegionCorePart_Update);
 * - anim palette: palette used by the palette animations (corpses and
 *   fragments copy it); body palette: the base palette.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define LEGION_DATA(e) e->unk_4F8.whole
#define LEGION_CIRCLE_DIR(e) e->unk_500.part8.unk_500
#define LEGION_RETARGET_TIMER(e) e->unk_500.part8.unk_502
#define LEGION_FLAGS(e) e->unk_504.part8.unk_506
#define LEGION_BROKEN_CORES(e) e->unk_4FC.part8.unk_4FE
#define LEGION_ANIM_PALETTE(e) e->unk_504.part8.unk_504
#define LEGION_BODY_PALETTE(e) e->unk_504.part8.unk_505

/*
 * Legion core parts (Legion_CreateCorePart) and their details
 * (LegionCorePart_CreateDetail): the entity they hang on, their offset from it
 * (16.16), their kind / quadrant, a tick counter and the tile data passed to
 * the fragments they shed.
 */
#define LEGION_PART_OWNER(e) e->unk_4F4.unk_4F4_entity
#define LEGION_PART_OFFSET_X(e) e->unk_4F8.whole
#define LEGION_PART_OFFSET_Y(e) e->unk_4FC.whole
#define LEGION_PART_KIND(e) e->unk_504.part8.unk_504
#define LEGION_PART_TICK(e) e->unk_504.part8.unk_505
#define LEGION_PART_TILES(e) e->unk_500.whole

/*
 * Corpses (Legion_CreateCorpse, LegionCorpse_Update - TU aliases TARGET_X /
 * TARGET_Y / FRAME): room x it wanders to, the target y of its rise, and the
 * animation frame index. Shell parts (Legion_CreateShellPart,
 * LegionShellPart_Update - TU aliases KIND / ANGLE / RADIUS; owner in
 * LEGION_PART_OWNER): the centre entity they circle, kind, orbit angle,
 * orbit radius and release delay.
 */
#define LEGION_CORPSE_TARGET_X(e) e->unk_4F8.whole
#define LEGION_CORPSE_TARGET_Y(e) e->unk_4FC.whole
#define LEGION_CORPSE_FRAME(e) e->unk_504.part8.unk_504
#define LEGION_SHELL_CENTER(e) e->unk_504.unk_504_entity
#define LEGION_SHELL_KIND(e) e->unk_500.part8.unk_501
#define LEGION_SHELL_ANGLE(e) e->unk_500.part16.unk_502
#define LEGION_SHELL_RADIUS(e) e->unk_4F8.whole
#define LEGION_SHELL_DELAY(e) e->unk_500.part8.unk_500

/* Shell parts: which of the three shells (index into LegionState.shells) they belong to. */
#define LEGION_SHELL_INDEX(e) e->unk_4FC.part8.unk_4FC

/*
 * Intro (Legion_UpdateIntro - TU aliases STAGE / POSE / CYCLE): intro stage,
 * the step the corpses and core parts wait for (6, 7 ...) and the number of
 * corpses / parts that have reached their place in the current step.
 */
#define LEGION_INTRO_STAGE(e) e->unk_504.part8.unk_507
#define LEGION_INTRO_STEP(e) e->unk_4FC.part8.unk_4FC
#define LEGION_INTRO_ARRIVED(e) e->unk_4FC.part8.unk_4FD

/* LEGION_DATA read as a pointer (the same word through the entity member). */
#define LEGION_DATA_PTR(e) e->unk_4F8.unk_4F8_entity

#endif /* LEGION_ENTITY_H */
