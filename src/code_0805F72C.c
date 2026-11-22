/* EventScene06_Init. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 GameFlag_GetMask(s32 flag);
extern s32 Entity_SetupFromObjectTable(struct EwramData_EntityData *entity, s32 kind);

/**
 * @brief Init of event scene variant 6 (Object 0x20 unk_514 = 6,
 * sEventSceneCreateFuncs[6]). Create step of a scene object: deleted when flag
 * 0x23 is set; else animation 0 at room (0x50, 0xC0) facing left,
 * gameState.stateFlags bit 0 and hBlankEffect.requestStop.
 */
void EventScene06_Init(struct EwramData_EntityData *entity)
{
    struct EwramData *ewram;
    struct BgLayerState *bg;

    if (GameFlag_GetMask(0x23) != 0)
    {
        entity->deleteRequested = 1;
        return;
    }
    if (Entity_SetupFromObjectTable(entity, 6) == 0)
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    ewram = gEwramData;
    bg = &ewram->bgInfo[1];
    entity->posX.part16.integer = 0x50 - bg->xPos.part16.integer;
    entity->posY.part16.integer = 0xC0 - bg->yPos.part16.integer;
    entity->flipX = 1;
    ewram->gameState.stateFlags |= 1;
    ewram->hBlankEffect.requestStop = 1;
}
