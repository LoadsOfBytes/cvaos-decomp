#ifndef MATH_ANIM_H
#define MATH_ANIM_H

/* Functions of engine/math_anim.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

struct Matrix2;

extern s16 Fixed8Multiply(s16 a, s16 b);
extern s16 Fixed8Divide(s16 a, s16 den);
extern s16 Fixed8Reciprocal(s16 x);
extern s32 FixedMultiply(s32 a, s32 b);
extern s32 FixedDivide(s32 num, s32 den);
extern s32 FixedReciprocal(s32 x);
extern s32 FixedMultiplyFast(s32 a, s32 b);
extern s32 FixedDivideFast(s32 num, s32 den);
extern s32 FixedReciprocalFast(s32 x);
extern void Matrix_SetIdentity(struct Matrix2 *m);
extern void Matrix_SetRotationScale(struct Matrix2 *m, s32 angle, s32 scaleX, s32 scaleY);
extern void Vec2_FromPosition(s32 *v, struct WhipChainNode *pos);
extern void Matrix_Copy(struct Matrix2 *dst, struct Matrix2 *src);
extern void Matrix_Add(s32 *dst, s32 *m1, s32 *m2);
extern void Matrix_Subtract(s32 *dst, s32 *m1, s32 *m2);
extern void Vec2_Add(s32 *dst, s32 *v1, s32 *v2);
extern void Vec2_Subtract(s32 *dst, s32 *v1, s32 *v2);
extern void Vec2_Scale(s32 *dst, s32 *v, s32 s);
extern void Matrix_MultiplyVec2(s32 *dst, s32 *m, s32 *v);
extern void Matrix_Multiply(s32 *dst, s32 *m1, s32 *m2);
extern s32 Anim_StepList(struct AnimScript *script, struct EwramData_EntityData *entity);
extern s32 Anim_StepInline(struct AnimScript *script, struct EwramData_EntityData *entity);
extern s32 Anim_StepInlineBox(struct AnimScript *script, struct EwramData_EntityData *entity);
extern s32 Anim_StepInlineBoxes(struct AnimScript *script, struct EwramData_EntityData *entity);
extern s32 Anim_UpdateGfxFrame0(struct EwramData_EntityData *entity);
extern s32 Anim_UpdateGfxFrame1(struct EwramData_EntityData *entity);
extern s32 Anim_UpdateStreamed(struct EwramData_EntityData *entity);
extern s32 Anim_Start(struct EwramData_EntityData *entity, struct AnimScript *script, u8 mode, u8 flags);
extern s32 Anim_Update(struct EwramData_EntityData *entity);
extern s32 Anim_GetStepDuration(struct EwramData_EntityData *entity);
extern s32 Anim_SelectScript(struct EwramData_EntityData *entity, u8 script, u8 flags);
extern s32 Anim_SwitchScript(struct EwramData_EntityData *entity, struct AnimScript *script, u8 flags);
extern s32 BgMap_ScrollRight(s32 bgIndex, struct BgLayerState *bg, s32 deltaX);
extern s32 BgMap_ScrollLeft(s32 bgIndex, struct BgLayerState *bg, s32 deltaX);
extern s32 BgMap_ScrollDownWithMargin(s32 bgIndex, struct BgLayerState *bg, s32 deltaY);
extern s32 BgMap_ScrollUpWithMargin(s32 bgIndex, struct BgLayerState *bg, s32 deltaY);
extern void BgMap_FillText(s32 bgIndex);
extern void BgMap_FillAffineBG2(s32 bgIndex);
extern void BgMap_Setup(s32 bgIndex, struct BgMapMetadata *metadata, u16 scrollX, u16 scrollY);
extern void BgMap_SetupPrimary(s32 bgIndex, struct BgMapMetadata *metadata, u16 scrollX, u16 scrollY);
extern s32 BgMap_ScrollDown(s32 bgIndex, struct BgLayerState *bg, s32 deltaY);
extern s32 BgMap_ScrollUp(s32 bgIndex, struct BgLayerState *bg, s32 deltaY);
extern s32 BgMap_GetVramEntryAddress(s32 bgIndex, u16 tileX, u16 tileY);
extern void BgMap_ScrollWithMargin(s32 bgIndex, s32 deltaX, s32 deltaY);
extern void BgMap_Scroll(s32 bgIndex, s32 deltaX, s32 deltaY);
extern void BgMap_SaveSelection(u8 bgIndex);
extern void Bg_SyncPriorities(void);
extern void Gfx_CopyResource(void *resource, void *dest, u16 queued);
extern void Gfx_CopyResourceBlocks(void *resource, s32 destinationIndex, s32 sourceIndex, s32 blockCount);
extern s32 Anim_StartIndexed(struct EwramData_EntityData *entity, u16 script, u8 mode, u8 flags);
extern s32 Bg_SetControlIfLevelAllows(s32 level, s32 reg, s32 value_);

#endif /* MATH_ANIM_H */
