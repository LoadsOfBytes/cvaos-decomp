/* EventScene09_PlaceHelpers. */
#include "code/code_08039340.h"
#include "event_scene09.h"
#include "structs/ewram.h"

/**
 * @brief 601D4 | Scene step: face left one layer above the player with
 * frame 0x12, end helper unk_4F8 (deleteRequested) and put helper unk_4FC 0x60
 * px to the right with animation 1.
 */
s32 EventScene09_PlaceHelpers(struct EwramData_EntityData *entity)
{
    entity->flipX = 1;
    entity->spriteLayer = gEwramData->playerState.player->spriteLayer + 1;
    entity->animFrame = 0x12;
    if (EVENT09_ACTOR5(entity)->updateFunc != NULL)
        EVENT09_ACTOR5(entity)->deleteRequested = 1;
    if (EVENT09_ACTOR1(entity)->updateFunc != NULL)
        EVENT09_ACTOR1(entity)->posX.whole = entity->posX.whole + 0x600000;
    Anim_StartIndexed(EVENT09_ACTOR1(entity), 1, 0, 1);
    entity->flipX = 1;
}
