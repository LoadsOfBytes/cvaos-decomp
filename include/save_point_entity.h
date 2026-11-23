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
 * The macros expand to member chains or to the ENTITY_* views of the common
 * block (structs/ewram.h) without added parentheses; use them with a plain
 * base expression.
 */
#define SAVE_POINT_MARKERS_INDEX(e) ENTITY_BYTE_519(e)
#define SAVE_POINT_BLINK(e) ENTITY_BYTE_518(e)
/* Save point state bytes in the common block (bytes of spawnParam0 /
 * spawnParam1): saving finished (1: the orbit lights start and the blinking
 * stops), the orbit lights finished their outward spiral / reached the
 * player. The lights set the last two on their owner. */
#define SAVE_POINT_SAVE_DONE(e) ENTITY_BYTE_515(e)
#define SAVE_POINT_LIGHTS_OUT_DONE(e) ENTITY_BYTE_517(e)
#define SAVE_POINT_LIGHTS_IN_DONE(e) ENTITY_BYTE_516(e)
/* Orbit light: affine spin angle, +0x800 per frame while spiralling out. */
#define SAVE_POINT_LIGHT_SPIN(e) ENTITY_WORD_510(e)
#define SAVE_POINT_LIGHT_RADIUS_PX(e) e->ext.raw.unk_508.part16.unk_50A

#endif /* SAVE_POINT_ENTITY_H */
