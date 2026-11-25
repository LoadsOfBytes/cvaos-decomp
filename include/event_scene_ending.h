#ifndef EVENT_SCENE_ENDING_H
#define EVENT_SCENE_ENDING_H

/*
 * Event scene 30 keeps its work slots in ext.event30 (controller),
 * ext.event30Effect, ext.event30Ring, ext.event30Fragment and
 * ext.event30Driver. Views that do not fit a member:
 * - jitter y: vertical jitter offset of the controller, the high byte of
 *   ext.event30.jitterX (TU alias JITTER_Y);
 * - effect angle: affine rotation angle of the effect sprites (halo, centre
 *   effect, orbit ring), kept in the entity field enemyStatus.
 * The macros expand to member chains or to the ENTITY_* views of the common
 * block (structs/ewram.h) without added parentheses; use them with a plain
 * base expression.
 */
#define EVENT30_JITTER_Y(e) e->ext.raw.unk_508.part8.unk_50B
#define EVENT30_EFFECT_ANGLE(e) e->enemyStatus
/* Palette transition (EventSceneEnding_StepPaletteTransition): frame counter of
 * the current step and the step index 0..6 (byte 0 of hp). */
#define EVENT30_PALETTE_TIMER(e) ENTITY_HALF_51A(e)
#define EVENT30_PALETTE_STEP(e) ENTITY_BYTE_518(e)

#endif /* EVENT_SCENE_ENDING_H */
