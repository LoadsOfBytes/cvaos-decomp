#include "gba.h"
#include "structs/ewram.h"
#include "structs/entity_boxes.h"

extern void HitBox_SetHitRect2(u8 *e, void *hb);
extern u32 HitBox_OverlapsHitRect(u8 *e, void *hb);
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

static inline void AddHit(u8 *dst, u8 *src)
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
}

u32 Hit_Check(u8 *a, u8 *b)
{
    u8 *src;
    u8 *dst;
    struct EntityBoxes *hb;
    struct EntityBoxes *hb2;

    src = a + 0x70;
    dst = b + 0x70;
    hb = &gEntityBoxes[a[0x73]];
    hb2 = &gEntityBoxes[b[0x73]];
    HitBox_SetHitRect2(b, &hb2->body);
    if (HitBox_OverlapsHitRect(a, &hb->attack))
    {
        AddHit(dst, src);
        return 1;
    }
    if (src[1] == 0 && HitBox_OverlapsHitRect(a, &hb->body))
    {
        AddHit(dst, src);
        return 2;
    }
    return 0;
}
