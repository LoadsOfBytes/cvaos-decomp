/* Place the event actor and player relative to the scrolling background. */
#include "code_08009A0.h"
#include "code_08013960.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "gba.h"
extern s32 Entity_SetupFromObjectTable(struct EwramData_EntityData *,s32);
/**
 * @brief Set up event scene 24: place the object at room (64, 192) facing right
 * with the player 24 px to its right, set stateFlags bits 0 and 0x20, clear
 * the HUD graphics and map, play song 24.
 */
void EventScene24_Setup(struct EwramData_EntityData *entity)
{
    if(!Entity_SetupFromObjectTable(entity,0)){EntityDelete(entity);return;}
    Anim_StartIndexed(entity,4,0,1);Anim_Update(entity);
    {
    struct EwramData *ram=gEwramData;
    struct BgLayerState *background=&ram->bgInfo[1];
    entity->posX.part16.integer=64-background->xPos.part16.integer;
    entity->posY.part16.integer=192-background->yPos.part16.integer;
    entity->flipX=0;
    ram->playerState.player->posX.part16.integer=entity->posX.part16.integer+24;
    ram->playerState.player->posY.part16.integer=entity->posY.part16.integer;
    ram->playerState.player->flipX=0;
    }
    gEwramData->gameState.stateFlags|=1;
    gEwramData->gameState.stateFlags|=32;
    DMA_FILL_32(3,0,(void *)0x06004000,0x2000);
    DMA_FILL_16(3,0,(void *)0x0600E000,0x800);
    Music_SetCurrent(24);
}
