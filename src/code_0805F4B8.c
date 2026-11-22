/* EventScene05_Init. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 GameFlag_GetMask(s32 flag);
extern s32 Entity_SetupFromObjectTable(struct EwramData_EntityData *entity, s32 kind);
extern void Interaction_End(struct EwramData_EntityData *entity, s32 deleteEntity);

/**
 * @brief Init of event scene variant 5 (Object 0x20 unk_514 = 5,
 * sEventSceneCreateFuncs[5]). Create step of a scene object: when flag 0x22 is
 * set Interaction_End(entity, 1); else animation 0 at room (0xA0, 0xA0)
 * facing right and gameState.stateFlags bit 0.
 */
void EventScene05_Init(struct EwramData_EntityData *entity)
{
    struct EwramData *ewram;
    struct BgLayerState *bg;

    if (GameFlag_GetMask(0x22) != 0)
    {
        Interaction_End(entity, 1);
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
    entity->posX.part16.integer = 0xA0 - bg->xPos.part16.integer;
    entity->posY.part16.integer = 0xA0 - bg->yPos.part16.integer;
    entity->flipX = 0;
    ewram->gameState.stateFlags |= 1;
}
