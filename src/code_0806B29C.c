/* Animation viewer entity (AnimationViewer_Update); matched with a permuter
 * (the s32 temporary for the frame byte fixes the register allocation). */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

/*
 * Animation viewer (debug): frame mode (0 = play animation scripts, 1 = step
 * single frames) and the selected frame. Token-identical expansion.
 */
#define ANIM_VIEWER_FRAME_MODE(e) e->unk_504.part8.unk_504
#define ANIM_VIEWER_FRAME(e) e->unk_504.part8.unk_505

struct AnimViewerFrame
{
    u32 scriptHeader;
    u8 frame;
    u8 pad_5[3];
};

extern const struct AnimViewerFrame sAnimViewerInitialFrame;

/**
 * @brief Handle sequence selection/restart and switching to individual
 * frame selection with the existing input masks. Preserve the copied
 * 64-bit animation object, frame-byte temporary and animation mode 0.
 */
void AnimationViewer_Update(struct EwramData_EntityData *entity)
{
    union { u64 raw; struct AnimViewerFrame f; } anim;
    struct EwramData *ewram;

    anim.raw = *(const u64 *)&sAnimViewerInitialFrame;

    if ((s8)ANIM_VIEWER_FRAME_MODE(entity) == 0)
    {
        if ((gEwramData->inputData.playerNewInput & 0x40) && entity->animScriptIndex != 0)
            Anim_StartIndexed(entity, entity->animScriptIndex - 1, 0, 0);
        if (gEwramData->inputData.playerNewInput & 0x80)
            Anim_StartIndexed(entity, entity->animScriptIndex + 1, 0, 0);
        if (gEwramData->inputData.playerNewInput & 1)
            Anim_StartIndexed(entity, entity->animScriptIndex, 0, 0);
        Anim_Update(entity);
        if (gEwramData->inputData.playerNewInput & 0x104)
        {
            ANIM_VIEWER_FRAME_MODE(entity) = 1;
            ANIM_VIEWER_FRAME(entity) = 0;
            Anim_Start(entity, (struct AnimScript *)&anim, 0, 1);
        }
    }
    else
    {
        entity->animTimer = 0;
        if (gEwramData->inputData.playerNewInput & 0x40)
        {
            if ((s8)ANIM_VIEWER_FRAME(entity) != 0)
                ANIM_VIEWER_FRAME(entity)--;
            entity->animTimer = 100;
        }
        if (gEwramData->inputData.playerNewInput & 0x80)
        {
            ANIM_VIEWER_FRAME(entity)++;
            entity->animTimer = 100;
        }
        ewram = gEwramData;
        if (ewram->inputData.playerNewInput & 1)
        {
            ANIM_VIEWER_FRAME(entity) = 0;
            entity->animTimer = 100;
        }
        if (ewram->inputData.playerNewInput & 0x208)
        {
            ANIM_VIEWER_FRAME_MODE(entity) = 0;
            Anim_StartIndexed(entity, 0, 0, 0);
        }
        {
            s32 index = ANIM_VIEWER_FRAME(entity);
            u8 frame = index;
            anim.f.frame = frame;
        }
        Anim_Update(entity);
    }
}
