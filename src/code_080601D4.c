/* EventScene09_PlaceHelpers. */
#include "code/code_08039340.h"
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
    if (entity->ext.event09.actor5->updateFunc != NULL)
        entity->ext.event09.actor5->deleteRequested = 1;
    if (entity->ext.event09.actor1->updateFunc != NULL)
        entity->ext.event09.actor1->posX.whole = entity->posX.whole + 0x600000;
    Anim_StartIndexed(entity->ext.event09.actor1, 1, 0, 1);
    entity->flipX = 1;
}
