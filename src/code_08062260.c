/* Prepare the event actor, player and companion for the scene. */
#include "code_08009A0.h"
#include "code_08013960.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "gba.h"
extern s32 Entity_SetupFromObjectTable(struct EwramData_EntityData *,s32);
extern struct EwramData_EntityData *Entity_CreateFromObjectTable(s32);
/**
 * @brief Init of event scene variant 23 (Object 0x20 unk_514 = 23,
 * sEventSceneCreateFuncs[23]).
 */
void EventScene23_Init(struct EwramData_EntityData *entity)
{
    struct EwramData *ram;
    struct BgLayerState *background;
    if(!(gEwramData->gameState.stateFlags&0x2000)){entity->deleteRequested=1;return;}
    if(!Entity_SetupFromObjectTable(entity,0))goto fail;
    Anim_StartIndexed(entity,0,0,1);Anim_Update(entity);
    ram=gEwramData;background=&ram->bgInfo[1];
    entity->posX.part16.integer=64-background->xPos.part16.integer;
    entity->posY.part16.integer=192-background->yPos.part16.integer;
    entity->flipX=0;
    ram->playerState.player->posX.part16.integer=entity->posX.part16.integer+32;
    ram->playerState.player->posY.part16.integer=entity->posY.part16.integer;
    ram->playerState.player->flipX=0;
    entity->ext.event23.actor=Entity_CreateFromObjectTable(1);
    if(!entity->ext.event23.actor)
    {
fail:
        EntityDelete(entity);return;
    }
    entity->ext.event23.actor->posX.part16.integer=entity->posX.part16.integer+96;
    entity->ext.event23.actor->posY.part16.integer=entity->posY.part16.integer+1;
    Anim_StartIndexed(entity->ext.event23.actor,1,0,1);
    entity->ext.event23.actor->flipX=0;
    gEwramData->gameState.stateFlags|=1;
    BgPalette_StartColorFade(31,31,31,0,0x10000,128);
    ObjPalette_StartColorFade(31,31,31,0,0x10000,128);
    DMA_FILL_32(3,0,(void *)0x06004000,0x2000);
    DMA_FILL_16(3,0,(void *)0x0600E000,0x800);
    gEwramData->gameState.stateFlags|=32;
    gEwramData->gameState.stateFlags|=0x10000;
    Music_SetCurrent(25);
}
