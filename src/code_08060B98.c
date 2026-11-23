/* EventScene11_Init. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 Entity_SetupFromObjectTable(struct EwramData_EntityData *entity, s32 kind);

/**
 * @brief Init of event scene variant 11 (Object 0x20 spawnParam0 = 11,
 * sEventSceneCreateFuncs[11]). Create step of a scene object: deleted once
 * stateFlags bit 16 is set; else animation 0 facing like the player,
 * stateFlags bit 0.
 */
void EventScene11_Init(struct EwramData_EntityData *entity)
{
    struct EwramData *ewram;

    if (gEwramData->gameState.stateFlags & 0x10000)
    {
        entity->deleteRequested = 1;
        return;
    }
    if (Entity_SetupFromObjectTable(entity, 4) == 0)
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    ewram = gEwramData;
    entity->flipX = ewram->playerState.player->flipX;
    ewram->gameState.stateFlags |= 1;
}
