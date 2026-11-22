#ifndef STRUCTS_RENDER_SLOTS_H
#define STRUCTS_RENDER_SLOTS_H

#include "types.h"

/*
 * Sprite object (OAM) manager, code_0803DB6C.c (Obj_*, ObjAffine_*), all in IWRAM.
 *
 * Each frame the game allocates OAM entries from gObjEntries (Obj_Alloc,
 * Obj_AllocMany), fills them and links them into one of 32 layers
 * (Obj_AddToLayer). Obj_BuildOamArm (ARM copy in gObjBuildOamArm) walks the
 * layers front to back into the shadow OAM gOamBuffer, sets each entry's
 * priority from gObjState.layerPriority (derived from the BG layer positions,
 * Obj_SetBgLayers) and scrolls layers 1-31 by the screen shake offset; the
 * 32 affine parameter sets of gObjAffine are written into the affine slots.
 * Obj_TransferToOam copies gOamBuffer to OAM in VBlank; with more than 128
 * linked objects it alternates between two 128-entry windows (flicker).
 */

/* 2x2 16.16 matrix (a b / c d), Matrix_* helpers in code_0803E820.c. */
struct Matrix2 {
    s32 a;
    s32 b;
    s32 c;
    s32 d;
}; /* size = 0x10 */

/* One OAM affine parameter set: the requested transform and the resulting
 * OAM parameters (8.8). */
struct ObjAffine {
    s32 angle; /* 0x10000 = full turn */
    s32 scaleX; /* 16.16 */
    s32 scaleY;
    s16 flipX;
    s16 flipY;
    struct Matrix2 matrix; /* forward rotation/scale matrix */
    u16 pa; /* inverse matrix written to OAM */
    u16 pb;
    u16 pc;
    u16 pd;
}; /* size = 0x28 */
extern struct ObjAffine gObjAffine[0x20];

struct ObjState {
    s16 objCount; /* gObjEntries allocated this frame */
    s16 layeredCount; /* entries linked into the layers this frame */
    u16 flags; /* bit 0: Obj_TransferToOam enabled, bit 1: flicker phase */
    s8 bgLayer[4]; /* layer index of BG0-BG3 (Obj_SetBgLayers) */
    s8 bgPriority[4]; /* resulting BG priority (rank of bgLayer) */
    u8 layerPriority[0x20]; /* OAM priority of each object layer */
    u16 affineSplit0; /* ObjAffine_Alloc groups, see ObjAffine_InitGroups */
    u16 affineSplit1;
    u8 pad_32[0x34 - 0x32];
    s32 affineUsedMask; /* bit N: gObjAffine[N] allocated */
    s32 oamCount; /* entries written by the last OAM build */
};
extern struct ObjState gObjState;

/* OAM entry being built plus its layer link. */
struct ObjEntry {
    u8 y;
    u8 pad_1[0x5 - 0x1];
    u8 tileNumHigh:2; /* attribute 2 bits 8-9 */
    u8 priority:2;
    u8 pad_6[0x8 - 0x6];
    s16 next; /* next entry in the same layer, -1 = end */
    u16 index; /* own index in gObjEntries */
}; /* size = 0xC */
extern struct ObjEntry gObjEntries[0x100];

/* Shadow OAM entry (only the fields accessed by name). */
struct OamShadow {
    u8 y;
    u8 pad_1[0x2 - 0x1];
    u16 x:9;
    u8 pad_2[0x6 - 0x4];
    u16 affineParam; /* every 4 entries hold one affine parameter set */
}; /* size = 0x8 */
extern struct OamShadow gOamBuffer[0x100];

struct ObjLayer {
    s16 head; /* first gObjEntries index, -1 = empty */
    s16 tail;
}; /* size = 0x4 */
extern struct ObjLayer gObjLayers[0x20];

typedef void (*ObjBuildOamArmFunc)(struct ObjEntry *, struct OamShadow *, struct ObjLayer *, struct ObjState *);
extern u8 gObjBuildOamArm[];

extern struct ObjEntry *Obj_Alloc(void);
extern struct ObjEntry *Obj_AllocMany(u32 count);
extern void Obj_AddToLayer(struct ObjEntry *obj, u32 layer);

#endif
