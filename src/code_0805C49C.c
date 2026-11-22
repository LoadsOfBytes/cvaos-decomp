#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "structs/main.h"
#include "gba/dma.h"

extern s32 Entity_SetupFromObjectTable(struct EwramData_EntityData *entity, s32 kind);
extern struct EwramData_EntityData *Entity_CreateFromObjectTable(s32 kind);
extern void EventScene31BlockHolder_Update(struct EwramData_EntityData *entity);

/**
 * @brief 5C49C | Scene setup (needs Entity_SetupFromObjectTable(entity, 7)): place the
 * object at room (0xFF, 0xBF) facing left, create helper 0xB at (0x78, 0xB0)
 * with update EventScene31BlockHolder_Update, move the player to (0xA0, 0xBF), set stateFlags
 * bits 0 and 16, fade palettes, clear BG VRAM and hide BG0.
 */
void EventScene31_Init(struct EwramData_EntityData *entity)
{
    struct EwramData *ewram;
    struct BgLayerState *bg;
    struct EwramData_EntityData *child;
    s32 zero;
    s32 fill;

    if (Entity_SetupFromObjectTable(entity, 7) == 0)
        goto fail;
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    {
    struct BgLayerState *bg1 = &gEwramData->bgInfo[1];
    entity->posX.part16.integer = 0xFF - bg1->xPos.part16.integer;
    zero = 0;
    fill = 0;
    entity->posY.part16.integer = 0xBF - bg1->yPos.part16.integer;
    }
    entity->flipX = 0;
    child = Entity_CreateFromObjectTable(0xB);
    entity->unk_4F8.unk_4F8_entity = child;
    if (child == NULL)
    {
    fail:
        entity->deleteRequested = 1;
        return;
    }
    {
    struct BgLayerState *bg2 = &gEwramData->bgInfo[1];
    child->posX.part16.integer = 0x78 - bg2->xPos.part16.integer;
    entity->unk_4F8.unk_4F8_entity->posY.part16.integer = 0xB0 - bg2->yPos.part16.integer;
    }
    Anim_StartIndexed(entity->unk_4F8.unk_4F8_entity, 0, 0, 1);
    Anim_Update(entity);
    entity->unk_4F8.unk_4F8_entity->updateFunc = (u32 *)EventScene31BlockHolder_Update;
    gEwramData->playerState.player->hidden = 1;
    ewram = gEwramData;
    ewram->playerState.player->posX.part16.integer = 0xA0 - (bg = &ewram->bgInfo[1])->xPos.part16.integer;
    ewram->playerState.player->posY.part16.integer = 0xBF - bg->yPos.part16.integer;

    ewram->gameState.stateFlags |= 1;
    ewram->gameState.stateFlags |= 0x10000;
    BgPalette_StartColorFade(0, 0, 0, 0, 0x10000, 0x80);
    ObjPalette_StartColorFade(0, 0, 0, 0, 0x10000, 0x80);
    entity->state = 1;
    entity->stage = zero;
    entity->subState = zero;
    DMA_FILL_32(3, fill, 0x06004000, 0x2000);
    DMA_FILL_16(3, fill, 0x0600E000, 0x800);
    gUnk_03002CB0.dispCnt &= ~0x100;
}
