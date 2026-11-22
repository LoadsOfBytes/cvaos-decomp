#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "structs/main.h"
#include "gba/dma.h"

extern s32 Entity_SetupFromObjectTable(struct EwramData_EntityData *entity, s32 kind);

/**
 * @brief Init of event scene variant 29 (Object 0x20 unk_514 = 29,
 * sEventSceneCreateFuncs[29]). Scene object setup (only while stateFlags bit 15 is
 * set): place it at room (0x40, 0xBF) facing right, put the player 0x18
 * px to its right, set stateFlags bit 0, fade palettes and clear BG VRAM,
 * hide BG0.
 */
void EventScene29_Init(struct EwramData_EntityData *entity)
{
    struct EwramData *ewram;
    struct BgLayerState *bg;

    if (!(gEwramData->gameState.stateFlags & 0x8000) || Entity_SetupFromObjectTable(entity, 7) == 0)
    {
        entity->deleteRequested = 1;
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    ewram = gEwramData;
    bg = &ewram->bgInfo[1];
    entity->posX.part16.integer = 0x40 - bg->xPos.part16.integer;
    entity->posY.part16.integer = 0xBF - bg->yPos.part16.integer;
    entity->flipX = 1;
    ewram->playerState.player->hidden = 1;
    gEwramData->playerState.player->posX.part16.integer = entity->posX.part16.integer + 0x18;
    gEwramData->playerState.player->posY.part16.integer = entity->posY.part16.integer;
    gEwramData->gameState.stateFlags |= 1;
    BgPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0, 0x10000, 0x80);
    ObjPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0, 0x10000, 0x80);
    DMA_FILL_32(3, 0, 0x06004000, 0x2000);
    DMA_FILL_16(3, 0, 0x0600E000, 0x800);
    gUnk_03002CB0.dispCnt &= ~0x100;
}
