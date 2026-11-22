#include "code/code_08039340.h"
#include "structs/render_slots.h"

struct Matrix2;
extern void ObjAffine_GetMatrix(struct Matrix2 *dst, s32 index);
extern void (*gObjAffineGetMatrixFunc)(struct Matrix2 *, s32);
extern s32 (*gObjAffineGetFlipXFunc)(s32);
extern s32 (*gObjAffineGetFlipYFunc)(s32);
extern struct ObjEntry *(*gObjAllocManyFunc)(u32);
extern void (*gObjAddToLayerFunc)(struct ObjEntry *, u32);
extern void (*gMatrixMultiplyVec2Func)(s32 *, s32 *, s32 *);
extern s32 (*gObjTilesGetTileFunc)(s32, u32, u32);

/* Install the seven IWRAM rendering/transfer callbacks.
 * This routine uses the original ARM compiler, unlike most game code. */
void Obj_InstallCallbacks(void)
{
    gObjAffineGetMatrixFunc = ObjAffine_GetMatrix;
    gObjAffineGetFlipXFunc = ObjAffine_GetFlipX;
    gObjAffineGetFlipYFunc = ObjAffine_GetFlipY;
    gObjAllocManyFunc = Obj_AllocMany;
    gObjAddToLayerFunc = Obj_AddToLayer;
    gMatrixMultiplyVec2Func = Matrix_MultiplyVec2;
    gObjTilesGetTileFunc = ObjTiles_GetTile;
}
