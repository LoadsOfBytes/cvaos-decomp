/* Load the transition's room metadata, backgrounds and initial palette. */
#include "code_08009A0.h"
#include "code_08001194.h"
#include "code_0800CB00.h"
#include "code_0800F1FC.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "structs/main.h"
#include "gba.h"
extern struct RoomDefinition sEndingRoomDefinition;
extern u8 sTextBgPalette[];
/**
 * @brief Unreferenced: load the graphics, palettes and BG layers (with their
 * initial scroll) of room 0x08522C54, the room the debug menu uses for
 * the ending events, without its entities.
 */
void Room_LoadEndingRoomBackground(void)
{
    struct RoomDefinition *room=&sEndingRoomDefinition;
    struct GameState *world=&gEwramData->gameState;
    struct RoomBgLayerDefinition *layer;
    s32 index;
    u16 *bgControl;
    gUnk_03002CB0.dispCnt=room->displayControl;
    Room_LoadGraphics(room);Room_LoadPalettes(room);
    layer=room->bgLayers;
    bgControl=gDisplayRegisters.bgCnt;
    for(index=1;index<4;index++)
    {
        struct RoomBgDimensions *metadata;
        u16 x,y;
        bgControl[index]=layer->control;
        gEwramData->bgInfo[index].scrollMode=layer->scrollMode;
        gEwramData->bgInfo[index].pixelWidth=layer->pixelWidth;
        gEwramData->bgInfo[index].pixelHeight=layer->pixelHeight;
        Obj_SetBgLayer(layer->layer,index);
        metadata=layer->dimensions;
        x=BgLayer_GetInitialScrollX(0,layer->scrollMode,metadata->widthCells,layer->pixelWidth);
        y=BgLayer_GetInitialScrollY(0,layer->scrollMode,layer->dimensions->heightCells,layer->pixelHeight);
        BgMap_Setup(index,(struct BgMapMetadata *)metadata,x,y);
        layer++;
    }
    Obj_SetBgLayer(0,0);Bg_SyncPriorities();
    world->roomMapXPos=room->mapX;
    world->roomMapYPos=room->mapY;
    world->unk_8E_0=room->unk_23_0;
    gEwramData->collisionBlocksActive=room->collisionBlocksEnabled;
    Collision_ClearAllBlocks();
    BgPalette_LoadRange(sTextBgPalette,0,1,14);
    Text_SetTileBuffer((void *)0x06004000);
    Text_SetForegroundColor(8);Text_SetBackgroundColor(0);
}
