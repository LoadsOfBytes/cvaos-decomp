#ifndef SAVE_POINT_ENTITY_H
#define SAVE_POINT_ENTITY_H

/*
 * The save point (Object 0x1C) keeps its work slots in ext.savePoint,
 * ext.savePointPrompt, ext.savePointPart and ext.savePointLight. Views that
 * do not fit a member:
 * - markers index: entity index of the child that draws the prompt markers
 *   (SavePoint_DrawPromptMarkers), outside the work area;
 * - blink: counter of the blinking light, outside the work area;
 * - light radius px: the integer half of ext.savePointLight.radius.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define SAVE_POINT_MARKERS_INDEX(e) e->unk_518.part8.unk_519
#define SAVE_POINT_BLINK(e) e->unk_518.part8.unk_518
#define SAVE_POINT_LIGHT_RADIUS_PX(e) e->ext.raw.unk_508.part16.unk_50A

#endif /* SAVE_POINT_ENTITY_H */
