#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern s32 Anim_StartIndexed(struct EwramData_EntityData *entity, u16 script, u8 mode, u8 flags);
/* Bg_SetControlIfLevelAllows(s32 level, s32 reg, u16 value): no prototype
 * here - its only caller (src/object/save_point_prompt.c, save point prompt)
 * declares the value as s32 (no zero extension at the calls). */

#endif /* GRAPHICS_H */
