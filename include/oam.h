#ifndef OAM_H
#define OAM_H

/* Functions of engine/oam.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

struct Matrix2;
struct ObjEntry;

extern void Obj_ResetBuffers(void);
extern void Obj_SetBgLayers(s32 bg0, s32 bg1, s32 bg2, s32 bg3);
extern void Obj_SetBgLayer(s32 layer, s16 bg_);
extern void Obj_BuildOam(void);
extern void Obj_BuildOamFast(void);
extern void ObjAffine_SetParams(u16 pa, u16 pb, u16 pc, u16 pd, u16 index);
extern void ObjAffine_Set(s32 angle, s32 scaleX, s32 scaleY, u16 flipX, u16 flipY, u16 index);
extern void ObjAffine_SetBiased(s32 angle, s32 scaleX, s32 scaleY, u16 flipX, u16 flipY, u16 index);
extern void Obj_Init(void);
extern void ObjAffine_ResetAll(void);
extern struct ObjEntry *Obj_Alloc(void);
extern struct ObjEntry *Obj_AllocMany(u32 count);
extern void Obj_AddToLayer(struct ObjEntry *obj, u32 layer);
extern s32 Obj_GetBgPriority(s16 bg);
extern void Obj_TransferToOam(void);
extern void Obj_EnableTransfer(void);
extern void Obj_DisableTransfer(void);
extern void ObjAffine_InitGroups(s32 split0, s32 split1);
extern s32 ObjAffine_Alloc(u32 group);
extern void ObjAffine_Free(s32 index);
extern s32 ObjAffine_GetAngle(s32 index);
extern s32 ObjAffine_GetScaleX(s32 index);
extern s32 ObjAffine_GetScaleY(s32 index);
extern s32 ObjAffine_GetFlipX(s32 index);
extern s32 ObjAffine_GetFlipY(s32 index);
extern void ObjAffine_GetMatrix(struct Matrix2 *dst, s32 index);
extern s32 Obj_GetOamCount(void);
extern s32 ObjAffine_SaveState(void);
extern s32 ObjAffine_RestoreState(void);

#endif /* OAM_H */
