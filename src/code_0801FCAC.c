/* Julius_UpdateAbilityDash. */
#include "code_08009A0.h"
#include "player_entity.h"

/**
 * @brief Update state 11: set drawSkip, decelerate X without crossing zero,
 * stop vertical motion, flip facing at timer 8 and return to state 0
 * after 0x40 updates.
 */
void Julius_UpdateAbilityDash(struct EwramData_EntityData *entity)
{
    s32 old, new;

    {
        u8 two = 2;
        gEwramData->playerState.player->ext.player.invulnerableTimer = two;
    }
    entity->drawSkip = 1;
    old = entity->velX.whole;
    new = old + entity->accelX.whole;
    entity->velX.whole = new;
    if (old < 0)
        goto check_pos;
    if (new < 0)
        goto clear;
    if (old > 0)
        goto done;
check_pos:
    if (new <= 0)
        goto done;
clear:
    entity->velX.whole = 0;
    entity->accelX.whole = 0;
done:
    entity->velY.whole = 0;
    entity->accelY.whole = 0;
    if (entity->timer == 8)
        entity->flipX ^= 1;
    if (++entity->timer > 0x3F)
        entity->state = 0;
}
