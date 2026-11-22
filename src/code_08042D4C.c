/* Number indicator movement: kind zero follows the player with a wobble;
 * the other kinds rise. All are removed after forty updates. */
#include "code_08009A0.h"
#include "structs/ewram.h"
/**
 * @brief Kind 0 follows the player with vertical wobble; kinds 1 and 2 rise. Request deletion when the timer reaches 40.
 */
void PlayerDamageIndicator_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
    case 0:
    {
        s32 wobble = ((40 - entity->timer) >> 4) << 16;
        struct EwramData_EntityData **player;
        s32 velocity;
        s32 y;
        if (entity->timer & 2) wobble = -wobble;
        velocity = entity->velY.whole;
        entity->velY.whole = velocity - 0x4000;
        player = &gEwramData->playerState.player;
        y = (*player)->posY.whole;
        velocity -= 0x1A4000;
        entity->posY.whole = y + velocity + wobble;
        entity->posX.whole = (*player)->posX.whole;
        break;
    }
    case 1: entity->posY.whole -= 0x8000; break;
    case 2: entity->posY.whole -= 0x8000; break;
    }
    if (++entity->timer == 40) entity->deleteRequested = 1;
}
