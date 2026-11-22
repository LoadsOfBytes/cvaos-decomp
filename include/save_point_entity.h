#ifndef SAVE_POINT_ENTITY_H
#define SAVE_POINT_ENTITY_H

/*
 * Work slots of the save point (Object 0x1C, Object1CCreate / Object1CUpdate):
 * - graphics: ObjGfx slot of sSaveWarpRoomSpriteSheet, shared with its parts;
 * - frame base: first sprite frame of the parts (they add 1..3);
 * - affine: affine slot of the growing / shrinking sprite, scale: its scale;
 * - yes selected: cursor of the save prompt (1 = Yes; A saves only then);
 * - flags: 2 = the game was saved;
 * - markers index: entity index of the child that draws the prompt markers
 *   (SavePoint_DrawPromptMarkers);
 * - blink: counter of the blinking light.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define SAVE_POINT_GRAPHICS(e) e->unk_4FC.part16.unk_4FC
#define SAVE_POINT_FRAME_BASE(e) e->unk_4FC.part16.unk_4FE[0]
#define SAVE_POINT_AFFINE(e) e->unk_4F8.part8.unk_4FB
#define SAVE_POINT_SCALE(e) e->unk_500.whole
#define SAVE_POINT_YES_SELECTED(e) e->unk_4F8.part8.unk_4FA
#define SAVE_POINT_FLAGS(e) e->unk_508.part16.unk_508
#define SAVE_POINT_MARKERS_INDEX(e) e->unk_518.part8.unk_519
#define SAVE_POINT_BLINK(e) e->unk_518.part8.unk_518

/*
 * Parts spawned by SavePoint_SpawnAnimParts / SavePoint_SpawnOrbitLights:
 * part owner is the save point. Orbit lights (SavePoint_UpdateOrbitLight):
 * light index 0..7 (angle base index << 13), 16.16 radius (its integer half
 * is also read as radius px), and the phase: the radius growth counter while
 * spreading, then the angle offset while circling in.
 * Prompt (SavePoint_StepPromptFade): fade level 0..0x10000.
 */
#define SAVE_POINT_PART_OWNER(e) e->unk_500.unk_500_entity
#define SAVE_POINT_LIGHT_INDEX(e) e->unk_504.part8.unk_504
#define SAVE_POINT_LIGHT_RADIUS(e) e->unk_508.whole
#define SAVE_POINT_LIGHT_RADIUS_PX(e) e->unk_508.part16.unk_50A
#define SAVE_POINT_LIGHT_PHASE(e) e->unk_50C.whole
#define SAVE_POINT_PROMPT_FADE(e) e->unk_4F8.whole

#endif /* SAVE_POINT_ENTITY_H */
