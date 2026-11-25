#include "debug_room.h"
#include "core.h"
#include "in_game.h"
#include "room_entities.h"
#include "map_screen.h"
#include "map_flags.h"
#include "player_core.h"
#include "gba.h"

#include "constants/main.h"

#include "data/interrupt_table.h"
#include "data/debug_font.h"

#include "structs/agb_sram.h"
#include "structs/ewram.h"
#include "structs/main.h"
#include "math_anim.h"
#include "obj_gfx.h"
#include "palette.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"

/**
 * @brief 36670 | Debug room (debug menu entry "2229"): play in sDebugRoom with room transitions
 * 
 * @return s32 Game mode, -1 is no change, -2 is next game mode, else the specified game mode
 */
s32 GameModeDebugRoomUpdate(void)
{
    switch (gEwramData->gameModeSubStage)
    {
        case 0:
            gEwramData->gameModeSubStage = 1;
            gEwramData->gameState.currentRoomDefinition = (struct RoomDefinition *)sDebugRoomRoom;
            InGame_InitScene();
            ScreenShake_Clear();
            break;

        case 1:
            DebugRoom_FrameHook();
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
            gfx = ObjGfx_LoadResource((u8 *)sDebugJitterSpriteGfx, 0);
            if (gfx >= 0)
            {
                ObjPalette_LoadRange((u8 *)sDebugJitterSpritePalette, 0, 1, 2);
                Sprite_Attach(entity, (void *)sDebugJitterSpriteSheet, gfx, 2);
                Anim_SelectScript(entity, 0, 1);
                entity->drawFunc = Sprite_DrawEntity;
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
