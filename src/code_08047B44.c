#include "code_08009A0.h"
#include "code_0800CB00.h"
#include "code_0800F1FC.h"
#include "code_08011DD0.h"
#include "code/code_08039340.h"
#include "code/code_080211F0.h"

extern void MenuItemIcon_Delete(void);
extern void SharedObjPalette_RestoreAll(void);

/**
 * @brief 47B44 | Leave the menu: delete the menu entity, restore the room
 * graphics and BG layers and refresh soul effects whose equipped blue /
 * yellow soul changed while the menu was open.
 */
void PauseMenu_Exit(void)
{
    struct RoomDefinition *room;
    struct RoomBgLayerDefinition *layer;
    s32 i;

    {
        struct EwramData_EntityData *ent = &gEwramData->entityData[18];
        struct GameState *g = &gEwramData->gameState;
        room = g->currentRoomDefinition;
        EntityDelete(ent);
    }
    MenuItemIcon_Delete();
    MenuItemIcon_Delete();
    BgMap_Setup(1, gEwramData->savedBgMetadata, gEwramData->savedBgScrollX, gEwramData->savedBgScrollY);
    Hud_Init();
    SharedObjPalette_RestoreAll();
    Palette_RestoreState();
    ItemIconCache_Restore();
    ObjGfx_RestoreState();
    ObjAffine_RestoreState();
    gUnk_03002CB0.dispCnt = 0;
    Room_LoadGraphics(room);
    Room_LoadPalettes(room);
    Display_RestoreState();
    layer = gEwramData->gameState.currentRoomDefinition->bgLayers;
    for (i = 1; i <= 3; i++)
    {
        Obj_SetBgLayer(layer->layer, i);
        layer++;
    }
    Bg_SyncPriorities();
    if (gEwramData->sharedScratchBuffer[0x14] != gEwramData->playerStatus.equippedBlueSoul)
        BlueSoul_StopAll();
    if (gEwramData->sharedScratchBuffer[0x15] != gEwramData->playerStatus.equippedYellowSoul)
        YellowSoul_Stop();
}
