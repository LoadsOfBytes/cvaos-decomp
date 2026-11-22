#include "code_08036670.h"
#include "code_08009A0.h"
#include "code_08001004.h"
#include "code_08001194.h"
#include "code_08008750.h"
#include "code_0800B700.h"
#include "code_0800CB00.h"
#include "code_0800F1FC.h"
#include "code_080109F4.h"
#include "code_08011DD0.h"
#include "code/code_08014548.h"
#include "code/code_080211F0.h"
#include "code_08032444.h"
#include "code_08032E4C.h"
#include "code_08033CAC.h"
#include "code_08035930.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "code_080D73B8.h"
#include "gba.h"
#include "macros.h"
#include "agb_sram.h"
#include "agb_multi_sio_sync.h"
#include "syscalls.h"

#include "constants/main.h"

#include "data/data_0E0334.h"
#include "data/data_0E3464.h"

#include "structs/agb_sram.h"
#include "structs/ewram.h"
#include "structs/main.h"

/**
 * @brief 36670 | Handle debug 2229 game mode
 * 
 * @return s32 Game mode, -1 is no change, -2 is next game mode, else the specified game mode
 */
s32 GameModeDebug2229Update(void)
{
    switch (gEwramData->gameModeSubStage)
    {
        case 0:
            gEwramData->gameModeSubStage = 1;
            gEwramData->gameState.currentRoomDefinition = (struct RoomDefinition *)0x08518710;
            InGame_InitScene();
            ScreenShake_Clear();
            break;

        case 1:
            Debug2229_FrameHook();
            if (Room_IsPlayerInside(gEwramData->playerState.player) == 0)
            {
                RoomTransition_Begin(&gEwramData->gameState);
                gEwramData->gameModeSubStage = 2;
            }
            else
            {
                RoomEntity_SpawnOnScroll();
            }
            Entity_UpdateAll();
            break;

        case 2:
            if (RoomTransition_Update(&gEwramData->gameState) != 0)
            {
                gEwramData->gameModeSubStage = 1;
            }
            break;

        case 3:
            EntityFreeAll();
            DMA_FILL_32(3, 0, VRAM_BASE + 0xE000, 0x2000);
            gEwramData->gameModeSubStage = 0;
            return GAME_MODE_KONAMI_LOGO;
    }

    ScreenShake_Update();
    if (!(gEwramData->paused))
    {
        InGame_UpdateTimerAndHud();
    }
    ItemIconCache_InvalidateUpload();
    return GAME_MODE_SAME_MODE;
}

/**
 * @brief Unreferenced debug entity: show the test sprite at (0x96, 0x50) and
 * move it randomly by one pixel per axis each frame.
 */
void DebugJitterSprite_Update(struct EwramData_EntityData *entity)
{
    s32 gfx;

    switch (entity->state)
    {
        case 0:
            gfx = ObjGfx_LoadResource((u8 *)0x081CBF24, 0);
            if (gfx >= 0)
            {
                ObjPalette_LoadRange((u8 *)0x0820A3D8, 0, 1, 2);
                Sprite_Attach(entity, (void *)0x082121F0, gfx, 2);
                Anim_SelectScript(entity, 0, 1);
                entity->drawFunc = (u32 *) Sprite_DrawEntity;
                entity->posX.part16.integer = 0x96;
                entity->posY.part16.integer = 0x50;
                entity->scrollWithBg = 1;
                entity->state = 1;
                return;
            }
            return;
        case 1:
            if ((RandomNumberGenerator() >> 4) & 1)
            {
                entity->posX.part16.integer += 1;
            }
            else
            {
                entity->posX.part16.integer -= 1;
            }

            if ((RandomNumberGenerator() >> 4) & 1)
            {
                entity->posY.part16.integer += 1;
            }
            else
            {
                entity->posY.part16.integer -= 1;
            }

            Anim_Update(entity);
            break;
    }
}
