/* EventScene16_Init. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 Entity_SetupFromObjectTable(struct EwramData_EntityData *entity, s32 kind);

/**
 * @brief Init of event scene variant 16 (Object 0x20 unk_514 = 16,
 * sEventSceneCreateFuncs[16]). Create step: deleted once stateFlags bit 16 is set;
 * else animation 4 at room (0x50, 0xC0) facing left with enemyFlags bit 2.
 */
void EventScene16_Init(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg;

    if (gEwramData->gameState.stateFlags & 0x10000)
    {
        entity->deleteRequested = 1;
        return;
    }
    if (Entity_SetupFromObjectTable(entity, 1) == 0)
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 4, 0, 1);
    Anim_Update(entity);
    bg = &gEwramData->bgInfo[1];
    entity->posX.part16.integer = 0x50 - bg->xPos.part16.integer;
    entity->posY.part16.integer = 0xC0 - bg->yPos.part16.integer;
    entity->flipX = 1;
    entity->enemyFlags |= 4;
}
