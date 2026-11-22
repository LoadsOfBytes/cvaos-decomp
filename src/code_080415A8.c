/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   HitBox_SetHitRect2 = HitBox_SetHitRect */
#include "code_08033CAC.h"
#include "code_08009A0.h"
#include "code_08001004.h"
#include "code_08001194.h"
#include "code_08008750.h"
#include "code_0800CB00.h"
#include "code_0800F1FC.h"
#include "code/code_08014548.h"
#include "code/code_080211F0.h"
#include "code_08032444.h"
#include "code_08032E4C.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "code_080D73B8.h"
#include "gba.h"
#include "macros.h"
#include "agb_sram.h"
#include "agb_multi_sio_sync.h"
#include "syscalls.h"
#include "constants/main.h"
#include "data/data_0E0334.h"
#include "data/data_0E3464.h"
#include "structs/agb_sram.h"
#include "structs/ewram.h"
#include "structs/main.h"

/**
 * @brief 415A8 | Same as HitBox_SetHitRect.
 */
void HitBox_SetHitRect2(struct EwramData_EntityData *entity, struct HitBoxDef *box)
{
    s32 tmp;

    // TODO: is tmp necessary? or perhaps inline?
    if (!entity->flipX)
    {
        gEwramData->hitRectLeft = box->x + entity->posX.part16.integer;
        tmp = box->width;
        gEwramData->hitRectRight = gEwramData->hitRectLeft - 1 + tmp;
    }
    else
    {
        gEwramData->hitRectRight = entity->posX.part16.integer - box->x;
        tmp =  box->width;
        gEwramData->hitRectLeft = gEwramData->hitRectRight + 1 - tmp;
    }

    if (!entity->flipY)
    {
        gEwramData->hitRectTop = box->y + entity->posY.part16.integer;
        tmp = box->height;
        gEwramData->hitRectBottom = gEwramData->hitRectTop - 1 + tmp;
    }
    else
    {
        gEwramData->hitRectBottom = entity->posY.part16.integer - box->y;
        tmp = box->height;
        gEwramData->hitRectTop = gEwramData->hitRectBottom + 1 - tmp;
    }
}
