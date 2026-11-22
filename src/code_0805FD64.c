/* EventScene08_Init. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 GameFlag_GetMask(s32 flag);
extern s32 Entity_SetupFromObjectTable(struct EwramData_EntityData *entity, s32 kind);

/**
 * @brief Init of event scene variant 8 (Object 0x20 unk_514 = 8,
 * sEventSceneCreateFuncs[8]). Create step of a scene object: deleted when flag
 * 0x25 is set; else animation 0 at room (0x50, 0xA0) facing left and
 * gameState.stateFlags bit 0.
 */
void EventScene08_Init(struct EwramData_EntityData *entity)
{
    struct EwramData *ewram;
    struct BgLayerState *bg;

    if (GameFlag_GetMask(0x25) != 0)
    {
        entity->deleteRequested = 1;
        return;
    }
    if (Entity_SetupFromObjectTable(entity, 2) == 0)
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    ewram = gEwramData;
    bg = &ewram->bgInfo[1];
    entity->posX.part16.integer = 0x50 - bg->xPos.part16.integer;
    entity->posY.part16.integer = 0xA0 - bg->yPos.part16.integer;
    entity->flipX = 1;
    ewram->gameState.stateFlags |= 1;
}
