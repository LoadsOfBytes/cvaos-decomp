#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "gba.h"

extern s32 GameFlag_GetMask(s32);
extern s32 Entity_SetupFromObjectTable(struct EwramData_EntityData *, s32);
extern void Entity_SnapToGround(struct EwramData_EntityData *);

/* Scene setup, flag 0x24: stop HBlank effect and place actor at (190,160). */
/**
 * @brief Init of event scene variant 7 (Object 0x20 unk_514 = 7,
 * sEventSceneCreateFuncs[7]).
 */
void EventScene07_Init(struct EwramData_EntityData *entity)
{
    s32 flag = GameFlag_GetMask(0x24);
    struct BgLayerState *bg;
    if (flag != 0 || (s16)gEwramData->playerState.player->posX.part16.integer <= 31)
    {
        entity->deleteRequested = 1;
        return;
    }
    gEwramData->hBlankEffect.requestStop = 1;
    DMA_FILL_32(3, flag, gEwramData->hBlankEffect.hBlankBuffer, 0xA00);
    if (Entity_SetupFromObjectTable(entity, 5) == 0)
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    bg = &gEwramData->bgInfo[1];
    entity->posX.part16.integer = 190 - bg->xPos.part16.integer;
    entity->posY.part16.integer = 160 - bg->yPos.part16.integer;
    entity->flipX = 1;
    Entity_SnapToGround(entity);
    gEwramData->playerState.player->flipX = 1;
    gEwramData->gameState.stateFlags |= 1;
}

