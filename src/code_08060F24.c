/* EventScene15_Init. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 Entity_SetupFromObjectTable(struct EwramData_EntityData *entity, s32 kind);

/**
 * @brief Init of event scene variant 15 (Object 0x20 unk_514 = 15,
 * sEventSceneCreateFuncs[15]). Create step: animation 0 at room (0, 0xA0) with
 * hidden and facing left, stateFlags bit 0.
 */
void EventScene15_Init(struct EwramData_EntityData *entity)
{
    struct EwramData *ewram;
    struct BgLayerState *bg;

    if (Entity_SetupFromObjectTable(entity, 1) == 0)
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    ewram = gEwramData;
    bg = &ewram->bgInfo[1];
    entity->posX.part16.integer = -bg->xPos.part16.integer;
    entity->posY.part16.integer = 0xA0 - bg->yPos.part16.integer;
    entity->hidden = 1;
    entity->flipX = 1;
    ewram->gameState.stateFlags |= 1;
}
