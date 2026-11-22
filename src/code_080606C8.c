#include "gba.h"
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 GameFlag_GetMask(s32 flag);
extern s32 Entity_SetupFromObjectTable(struct EwramData_EntityData *entity, s32 kind);

/**
 * @brief Init of event scene variant 10 (Object 0x20 unk_514 = 10,
 * sEventSceneCreateFuncs[10]). Set up a cutscene object: only when event flag
 * 0x27 is not set, bossEventFlags bit 6 is set and the player is left of x =
 * 0x80; clears the HBlank buffer and faces the object towards the
 * player.
 */
void EventScene10_Init(struct EwramData_EntityData *entity)
{
    s32 r;
    struct BgLayerState *cam;

    r = GameFlag_GetMask(0x27);
    if (r != 0)
    {
        entity->deleteRequested = 1;
        return;
    }
    if (!(gEwramData->gameState.bossEventFlags & 0x40))
    {
        entity->deleteRequested = 1;
        return;
    }
    if ((s16)gEwramData->playerState.player->posX.part16.integer > 0x80)
    {
        entity->deleteRequested = 1;
        return;
    }
    gEwramData->hBlankEffect.requestStop = 1;
    DMA_FILL_32(3, r, gEwramData->hBlankEffect.hBlankBuffer, 0xA00);
    if (!Entity_SetupFromObjectTable(entity, 7))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 4, 1, 1);
    Anim_Update(entity);
    cam = &gEwramData->bgInfo[1];
    entity->posX.part16.integer = 0x118 - cam->xPos.part16.integer;
    entity->posY.part16.integer = 0xC0 - cam->yPos.part16.integer;
    entity->flipX = entity->posX.whole < gEwramData->playerState.player->posX.whole;
    gEwramData->playerState.player->flipX = 0;
}
