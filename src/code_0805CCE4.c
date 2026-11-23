#include "code_08009A0.h"
#include "event_scene03.h"
#include "code_0800CB00.h"
#include "structs/ewram.h"

/**
 * @brief Scroll the camera for camera step EVENT_SCENE03_CAMERA_STEP of event scene 3 until its
 * target x is reached; return 1 then.
 */
s32 EventScene03_ScrollCamera(struct EwramData_EntityData *entity)
{
    s32 done = 0;
    struct BgLayerState *bg;

    gEwramData->gameState.cameraScrollMinPriority = 1;
    switch (EVENT_SCENE03_CAMERA_STEP(entity))
    {
    case 0:
        Camera_ScrollWithPriority(1, 0x80000, 0);
        bg = &gEwramData->bgInfo[1];
        if ((s16)bg->xPos.part16.integer > 0x1F)
            done = 1;
        break;
    case 1:
        Camera_ScrollWithPriority(1, 0x10000, 0);
        bg = &gEwramData->bgInfo[1];
        if ((s16)bg->xPos.part16.integer > 0x5F)
            done = 1;
        break;
    case 2:
        Camera_ScrollWithPriority(1, 0x38000, 0);
        bg = &gEwramData->bgInfo[1];
        if ((s16)bg->xPos.part16.integer > 0xDD)
            done = 1;
        break;
    case 3:
        Camera_ScrollWithPriority(1, -0x40000, 0);
        bg = &gEwramData->bgInfo[1];
        if ((s16)bg->xPos.part16.integer <= 0x60)
            done = 1;
        break;
    case 4:
        Camera_ScrollWithPriority(1, -0x10000, 0);
        if ((s16)gEwramData->playerState.player->posX.part16.integer == 0x78)
            done = 1;
        break;
    case 5:
        Camera_ScrollWithPriority(1, -0x80000, 0);
        bg = &gEwramData->bgInfo[1];
        if ((s16)bg->xPos.part16.integer <= 0x20)
            done = 1;
        break;
    }
    return done;
}
