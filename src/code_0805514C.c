/* Orbiting child: spiral outward around a fixed point (state 0), then orbit and shrink around the owner (state 1). */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 Target_GetPixelX(struct EwramData_EntityData *);
extern s32 Target_GetPixelY(struct EwramData_EntityData *);
extern void SaveSparkle_SpawnAround(struct EwramData_EntityData *, s32, s32);

/**
 * @brief Spiral outwards, then orbit inwards onto the player; tell the owner
 * (unk_517, unk_516) when each phase ends. Emits sparkles.
 */
void SavePoint_UpdateOrbitLight(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner;
    s32 state;
    s32 s;
    s32 c;
    s32 r;
    u32 step;
    s32 rad;

    owner = entity->unk_500.unk_500_entity;
    state = entity->state;
    switch (state)
    {
    case 0:
        s = Sin(entity->unk_504.part8.unk_504 << 13);
        c = Sin((entity->unk_504.part8.unk_504 << 13) + 0x4000);
        rad = entity->unk_508.whole;
        r = rad >> 16;
        s *= r;
        c *= r;
        entity->posY.whole = s + 0x500000;
        entity->posX.whole = c + 0x760000;
        if ((u32)entity->unk_50C.whole <= 0x1DFFFF)
            step = (u32)entity->unk_50C.whole >> 16;
        else
            step = ((u32)entity->unk_50C.whole >> 16) * 3;
        entity->unk_508.whole = rad + (step << 13);
        entity->unk_50C.whole += 0x10000;
        entity->unk_510.whole += 0x800;
        if (*(vs32 *)&entity->unk_508.whole > 0xFFFFFF)
        {
            entity->state++;
            entity->timer = 2;
            entity->spriteLayer = 9;
            entity->unk_508.whole = 0x1000000;
            owner->unk_514.part8.unk_517 = 1;
            entity->unk_50C.whole = 0;
        }
        break;
    case 1:
        s = Sin((entity->unk_504.part8.unk_504 << 13) + entity->unk_50C.whole);
        c = Sin((entity->unk_504.part8.unk_504 << 13) + ({ s32 t_ = entity->unk_50C.whole; t_ + 0x4000; }));
        r = entity->unk_508.part16.unk_50A;
        s *= r;
        c *= r;
        entity->posX.whole = c + 0x780000;
        entity->posY.whole = s + 0x500000;
        entity->posX.whole += (Target_GetPixelX(entity) << 16) - owner->posX.whole;
        entity->posY.whole = entity->posY.whole + ((Target_GetPixelY(entity) << 16) - owner->posY.whole) + 0x200000;
        entity->unk_508.whole -= 0x30000;
        entity->unk_50C.whole += 0x200;
        if (entity->unk_508.whole <= 0)
        {
            entity->unk_500.unk_500_entity->unk_514.part8.unk_516 = state;
            entity->deleteRequested = 1;
        }
        break;
    }
    if ((++entity->timer & 3) == 0)
        SaveSparkle_SpawnAround(entity, 0x10, 0x10);
    Anim_Update(entity);
}
