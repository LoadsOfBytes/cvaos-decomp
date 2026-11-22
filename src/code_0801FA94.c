/* Julius_UpdateKnockback. */
#include "code_08009A0.h"
#include "player_entity.h"
#include "code_08001194.h"

void Julius_UpdateVerticalMovement(struct EwramData_EntityData *entity);

/**
 * @brief Update damage-reaction state 13: damp horizontal motion by
 * +-0x1000 on the tile check without crossing zero, return to state 0
 * when the airborne flag clears, then update vertical movement.
 */
void Julius_UpdateKnockback(struct EwramData_EntityData *entity)
{
    s32 old, new;
    u16 x;
    s16 y;

    PLAYER_ACTION_FLAGS(entity) &= 0xEFFFFE9F;
    x = GetEntityRoomXPositionInteger(entity);
    y = GetEntityRoomYPositionInteger(entity);
    if (Collision_IsPointSolid((s16)x, y))
        entity->accelX.whole = (entity->velX.whole < 0) ? 0x1000 : -0x1000;
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
    if (!(PLAYER_ACTION_FLAGS(entity) & 2))
    {
        PLAYER_ACTION_FLAGS(entity) &= ~0x80;
        entity->state = 0;
    }
    Julius_UpdateVerticalMovement(entity);
}
