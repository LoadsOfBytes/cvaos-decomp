/* Julius_UpdateDiveAttack. */
#include "code_08009A0.h"
#include "player_entity.h"
#include "code_080D73B8.h"

/**
 * @brief Update state 7: end on flag 0x400000; on contact flag 0x80000
 * rebound upward (velY -0x3C000), set flag 8, clear flag 4, play
 * sound 0xB9 and return to state 0; otherwise set actionLockFlags |= 4.
 */
void Julius_UpdateDiveAttack(struct EwramData_EntityData *entity)
{
    if (entity->ext.player.actionFlags & 0x400000)
    {
        entity->state = 0;
        return;
    }
    {
        u8 two = 2;
        entity->hit.type = two;
    }
    entity->hit.flags &= ~2;
    if (entity->ext.player.actionFlags & 0x80000)
    {
        entity->ext.player.actionFlags = (entity->ext.player.actionFlags | 8) & ~4;
        entity->ext.player.requestedAnim = 3;
        PlaySong(0xB9);
        entity->velY.whole = -0x3C000;
        entity->accelY.whole = -0x2000;
        entity->state = 0;
    }
    else
        gEwramData->playerState.actionLockFlags |= 4;
}
