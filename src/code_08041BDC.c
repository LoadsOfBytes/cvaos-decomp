#include "gba.h"
#include "structs/ewram.h"
#include "structs/entity_boxes.h"

extern u32 HitBox_OverlapsRect(void *rect, u8 *e, void *hb);
extern const u8 sHitTimers[];

static inline u8 GetVal(s32 type)
{
    switch (type)
    {
    case 3:
        return gEwramData->hitTimerOverride[0];
    case 5:
        return gEwramData->hitTimerOverride[1];
    case 6:
        return gEwramData->hitTimerOverride[2];
    case 9:
        return gEwramData->hitTimerOverride[3];
    default:
        return sHitTimers[type];
    }
}

u32 Hit_CheckPlayerAttack(u8 *e)
{
    u8 *src;
    u8 *dst;
    void *hb;


    src = (u8 *)gEwramData->playerState.player + 0x70;
    dst = e + 0x70;
    hb = &gEntityBoxes[e[0x73]].body;
    if (HitBox_OverlapsRect((void *)0x03001A48, e, hb))
    {
        if (dst[4] == 0)
        {
            dst[4] = src[0];
            dst[8] = GetVal(src[0]) + 1;
        }
        else if (dst[5] == 0)
        {
            dst[5] = src[0];
            dst[9] = GetVal(src[0]) + 1;
        }
        else if (dst[6] == 0)
        {
            dst[6] = src[0];
            dst[10] = GetVal(src[0]) + 1;
        }
        return 1;
    }
    return 0;
}
