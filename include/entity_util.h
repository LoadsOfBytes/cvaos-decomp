#ifndef ENTITY_UTIL_H
#define ENTITY_UTIL_H

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern s8 ObjPalette_FindOrLoad(void *palette); // TODO: types?
extern s32 Entity_IsInRect(struct EwramData_EntityData *entity, s32 left, s32 top, s32 width, s32 height);
extern void HitEffect_Spawn(s32, s32, s32);

#endif /* ENTITY_UTIL_H */
