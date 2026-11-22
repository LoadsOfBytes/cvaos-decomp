/* EventScene12_Init. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 Entity_SetupFromObjectTable(struct EwramData_EntityData *entity, s32 kind);

/**
 * @brief Init of event scene variant 12 (Object 0x20 unk_514 = 12,
 * sEventSceneCreateFuncs[12]). Create step: animation 1 entering from x -16 at
 * room y 0xA0, moving right (0x4000), facing left; always sets stateFlags
 * bit 0.
 */
void EventScene12_Init(struct EwramData_EntityData *entity)
{
    if (Entity_SetupFromObjectTable(entity, 1) == 0)
    {
        EntityDelete(entity);
    }
    else
    {
        Anim_StartIndexed(entity, 1, 0, 1);
        Anim_Update(entity);
        entity->posX.part16.integer = -0x10;
        {
            struct BgLayerState *bg = &gEwramData->bgInfo[1];

            entity->posY.part16.integer = 0xA0 - bg->yPos.part16.integer;
        }
        entity->velX.whole = 0x4000;
        entity->flipX = 1;
    }
    gEwramData->gameState.stateFlags |= 1;
}
