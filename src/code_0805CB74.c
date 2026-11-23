#include "code_08009A0.h"
#include "event_scene03.h"
#include "structs/ewram.h"
#include "structs/main.h"

#define SET_MOSAIC(t)                                                        \
    {                                                                        \
        v = (u32)((t) << 24) >> 26;                                              \
        gDisplayRegisters.mosaic = v | (v << 4) | (v << 8) | (v << 12);      \
    }

/**
 * @brief Mosaic step of event scene 3: fade the mosaic in and out over its
 * sub-states; return 0 when finished.
 */
s32 EventScene03_UpdateMosaic(struct EwramData_EntityData *entity)
{
    s32 ret = 1;
    s32 t;
    u32 v;

    switch (entity->subState)
    {
    case 0:
        gDisplayRegisters.mosaic = 0xFFFF;
        entity->timer = 0x40;
        entity->subState++;
        break;
    case 1:
        t = entity->timer;
        SET_MOSAIC(t);
        if ((u32)(t << 24) >> 24 == 0x20)
            entity->subState++;
        else
            entity->timer = t - 1;
        break;
    case 2:
        t = entity->timer;
        SET_MOSAIC(t);
        if ((u32)(t << 24) >> 24 == 0x30)
            entity->subState++;
        else
            entity->timer = t + 1;
        break;
    case 3:
        t = entity->timer;
        SET_MOSAIC(t);
        if ((u32)(t << 24) >> 24 == 0x10)
            entity->subState++;
        else
            entity->timer = t - 1;
        break;
    case 4:
        t = entity->timer;
        SET_MOSAIC(t);
        if ((u32)(t << 24) >> 24 == 0x20)
            entity->subState++;
        else
            entity->timer = t + 2;
        break;
    case 5:
        t = entity->timer;
        SET_MOSAIC(t);
        if ((u32)(t << 24) >> 24 == 0)
            entity->subState++;
        else
            entity->timer = t - 1;
        if (--entity->timer == 0xFF)
        {
            gDisplayRegisters.mosaic = 0;
            gDisplayRegisters.bldCnt = 0;
            gEwramData->playerState.player->mosaic = 0;
            entity->ext.event03.actor0->mosaic = 0;
            ret = 0;
        }
        break;
    }
    return ret;
}
