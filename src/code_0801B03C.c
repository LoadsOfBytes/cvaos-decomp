/* Player_UpdateDeath; sign-change
 * stop as in Julius_UpdateAbilityDash. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08040A38.h"

/**
 * @brief Shared Soma/Julius death state 16: decelerate X/Y without
 * crossing zero and, while visible, emit randomized shared particles
 * every fourth update. Preserve visibility-dependent timer advance.
 */
void Player_UpdateDeath(struct EwramData_EntityData *entity)
{
    s32 old, new, x, accel;

    old = entity->velX.whole;
    accel = (old < 0) ? 0x200 : -0x200;
    entity->accelX.whole = accel;
    new = old + accel;
    entity->velX.whole = new;
    if (old < 0)
        goto check_pos_x;
    if (new < 0)
        goto clear_x;
    if (old > 0)
        goto done_x;
check_pos_x:
    if (new <= 0)
        goto done_x;
clear_x:
    entity->velX.whole = 0;
    entity->accelX.whole = 0;
done_x:
    old = entity->velY.whole;
    new = old + entity->accelY.whole;
    entity->velY.whole = new;
    if (old < 0)
        goto check_pos_y;
    if (new < 0)
        goto clear_y;
    if (old > 0)
        goto done_y;
check_pos_y:
    if (new <= 0)
        goto done_y;
clear_y:
    entity->velY.whole = 0;
    entity->accelY.whole = 0;
done_y:
    if (!entity->hidden)
    {
        s32 timer = entity->timer + 1;
        entity->timer = timer;
        if (timer & 3)
            return;
        x = entity->posX.whole + RandomSigned(0x80000);
        Effect_SpawnShared(x, entity->posY.whole + RandomSigned(0x80000) - 0x100000, 0xD, 8);
    }
}
