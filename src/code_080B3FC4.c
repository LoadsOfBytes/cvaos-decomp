/* Sprite-data setup. */
#include "balore_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void *Heap_Alloc(u16);

struct BaloreOverlayFrameView {
    u8 originX;
    u8 originY;
    u8 pad_2[2];
    u8 boxCount;
    u8 partCount;
    u8 pad_6[2];
    u32 boxes;
    void *parts;
};

struct BaloreOverlaySheetView {
    u16 frameCount;
    u16 scriptCount;
    struct BaloreOverlayFrameView *frames;
    u32 scripts;
    u32 scriptTable;
};

/**
 * @brief B3FC4 | Sprite-data setup used by BaloreOverlay_Update: allocates three
 * blocks (unk_4FC 4 words, unk_500 4 words, unk_504 0x20 words), links them
 * (A -> B -> C) and registers them with Sprite_Attach using byte 0x17 and
 * 0x2C of the template unk_4F8; then animation 0, draw callback
 * Sprite_DrawEntity.  Returns 1 if an allocation failed, else 0.
 */
s32 BaloreOverlay_SetupSprite(struct EwramData_EntityData *entity)
{
    u8 *info = (u8 *)BALORE_OVERLAY_INFO(entity);
    struct BaloreOverlaySheetView *a;
    struct BaloreOverlayFrameView *b;
    void *c;

    if ((BALORE_OVERLAY_SHEET(entity) = (s32)Heap_Alloc(4)) == 0)
        return 1;
    if ((BALORE_OVERLAY_FRAME(entity) = (s32)Heap_Alloc(4)) == 0)
        return 1;
    c = Heap_Alloc(0x20);
    BALORE_OVERLAY_PARTS(entity) = (s32)c;
    if (c == NULL)
        return 1;
    a = (struct BaloreOverlaySheetView *)BALORE_OVERLAY_SHEET(entity);
    b = (struct BaloreOverlayFrameView *)BALORE_OVERLAY_FRAME(entity);
    a->frameCount = 1;
    a->scriptCount = 0;
    a->frames = b;
    a->scripts = 0;
    a->scriptTable = 0;
    b->originX = 0;
    b->originY = 0;
    b->boxCount = 0;
    b->partCount = 0;
    b->boxes = 0;
    b->parts = c;
    Sprite_Attach(entity, a, (s8)info[0x17], info[0x2C]);
    Anim_StartIndexed(entity, 0, 2, 1);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    Anim_Update(entity);
    return 0;
}
