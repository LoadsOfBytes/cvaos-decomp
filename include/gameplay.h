#ifndef GAMEPLAY_H
#define GAMEPLAY_H

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern u8* Text_GetLocalizedEntry(s32 index);


extern s32 Hit_Enable(struct EwramData_EntityData *entity, s32 type, void *onHit, s32 onHurt);
extern s32 HitBox_SetAttackRaw(struct EwramData_EntityData *entity, s32 box); // TODO: is param_1 a pointer or value?
extern void Sprite_DrawPartsArm(void); // TODO: arm code, figure out types
extern void Sprite_DrawPartsAffineArm(void); // TODO: arm code, figure out types

#endif /* GAMEPLAY_H */
