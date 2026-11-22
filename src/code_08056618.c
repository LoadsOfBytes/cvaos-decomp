#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

/**
 * @brief Main behaviour of object 0x21, called by Object21Update.
 */
void Object21_UpdateState(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    switch (entity->subState)
    {
    case 0:
        EntityAccelerateAndMoveY(entity, 0x70000);
        if (entity->animScriptIndex == 0 && (s16)entity->posY.part16.integer >= 0x24 - (s16)bg->yPos.part16.integer)
            Anim_StartIndexed(entity, 3, 0, 1);
        if ((s16)entity->posY.part16.integer >= 0x7F - (s16)bg->yPos.part16.integer)
        {
            entity->posY.part16.integer = 0x7F - bg->yPos.part16.integer;
            entity->velY.whole = -0x14000;
            entity->subState++;
            entity->timer = 0;
            PlaySong(0x7F);
        }
        break;
    case 1:
        EntityAccelerateAndMoveY(entity, 0x70000);
        if ((s16)entity->posY.part16.integer >= 0x7F - (s16)bg->yPos.part16.integer)
        {
            entity->posY.part16.integer = 0x7F - bg->yPos.part16.integer;
            entity->velY.whole = 0;
            entity->subState++;
            entity->timer = 0;
        }
        break;
    case 2:
        if (entity->timer++ > 0x3B)
        {
            entity->subState++;
            entity->timer = 0;
        }
        break;
    case 3:
        entity->velY.whole = -0x10000;
        entity->posY.whole += entity->velY.whole;
        if (entity->animScriptIndex == 3 && (s16)entity->posY.part16.integer <= 0x24 - (s16)bg->yPos.part16.integer)
            Anim_StartIndexed(entity, 0, 0, 1);
        if ((s16)entity->posY.part16.integer <= 0x18 - (s16)bg->yPos.part16.integer)
        {
            entity->posY.part16.integer = 0x18 - bg->yPos.part16.integer;
            entity->subState++;
            entity->timer = 0;
        }
        break;
    case 4:
        if (entity->timer++ > 0x3B)
        {
            entity->subState = 0;
            entity->timer = 0;
            entity->velY.whole = 0;
        }
        break;
    }
}
