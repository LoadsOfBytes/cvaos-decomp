#ifndef OBJ_GFX_H
#define OBJ_GFX_H

/* Functions of engine/obj_gfx.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

/* OBJ graphics cache: see the ObjGfxEntry comment in structs/ewram.h. */
struct ObjTileFrameSource {
    u8 kind;
    u8 pad_1[0x4 - 0x1];
    void *compressedData;
};

extern void ObjGfx_Init(void);
extern void ObjGfx_Release(s16 tileSlot);
extern u8 Sprite_AllocSlot(void *sheet, s32 tileSlot);
extern void ObjGfx_DecompressSet(struct EwramData_unk19404 *set);
extern void Sprite_Init(void);
extern s32 ObjGfx_Load(u8 *resource, s32 flags);
extern s32 ObjGfx_LoadFrames(u8 *list, s32 size, s32 count, s32 flags);
extern s32 ObjGfx_LoadSet(u8 *set, s32 size, s32 count, s32 flags);
extern s32 ObjGfx_SetFrame(s32 tileSlot, s32 index, s32 frame, s32 flags);
extern void Sprite_Free(struct EwramData_EntityData *entity, u32 freeGfx);
extern s32 Sprite_Draw(struct EwramData_EntityData *entity);
extern s32 Sprite_BuildSingle(struct EwramData_EntityData *entity);
extern s32 Sprite_DrawSingle(struct EwramData_EntityData *entity);
extern s32 ObjGfx_Find(u8 *resource);
extern s32 ObjGfx_SaveState(void);
extern s32 ObjGfx_RestoreState(void);
extern s32 ObjTiles_LoadFrame(s32 firstSlot, struct ObjTileFrameSource *src, u32 variant, s32 tileOffset, u32 count, s32 useDmaQueue);
extern s32 ObjGfx_LoadResource(u8 *resource, s32 flags);
extern s32 ObjGfx_GetFrame(s32 tileSlot, s32 index);
extern s32 Sprite_AttachDefault(struct EwramData_EntityData *entity, s32 tileSlot, s32 palette);
extern s32 Sprite_Attach(struct EwramData_EntityData *entity, void *sheet, s32 tileSlot, u32 palette);
extern void ObjGfx_Unload(s32 tileSlot);
extern s32 Sprite_GetTileSlot(struct EwramData_EntityData *entity);
extern void ObjGfx_Decompress(void *resource);
extern void Sprite_DrawEntity(struct EwramData_EntityData *entity);
extern s32 Palette_RestoreSaved(struct PaletteBuffer *buffer);
extern void Palette_BlendSlots(struct PaletteBuffer *buffer);
extern void Palette_BlendSlotsAndColor(struct PaletteBuffer *buffer);
extern void Palette_StepFades(struct PaletteBuffer *buffer);
extern void Palette_UpdateFades(void);
extern void Palette_Init(void);
extern void Palette_TransferToPalram(void);
extern s32 PaletteAnim_SaveSlot(u16 slot);
extern s32 PaletteAnim_RestoreSaved(void);
extern void PaletteAnim_Update(void);

#endif /* OBJ_GFX_H */
