/* Julius_UpdateKnockbackFromImpulse (variant of Julius_UpdateKnockback with an animation switch). */
#include "code_08009A0.h"
#include "player_entity.h"
#include "code_08001194.h"

void Julius_UpdateVerticalMovement(struct EwramData_EntityData *entity);
void Anim_StartIndexed(struct EwramData_EntityData *entity, s32 script, s32 mode, s32 flags);

/**
 * @brief Update damage-reaction state 14, entered with stored external
 * impulse velocities: select animation 0xC, then damp and recover as
 * Julius_UpdateKnockback and update vertical movement.
 */
void Julius_UpdateKnockbackFromImpulse(struct EwramData_EntityData *entity)
{
    s32 old, new;
    u16 x;
    s16 y;

    PLAYER_ACTION_FLAGS(entity) &= 0xEFFFFE9F;
    if (!(gEwramData->playerState.actionLockFlags & 0x800) && entity->animScriptIndex != 0xC)
    {
        Anim_StartIndexed(entity, 0xC, 1, 0);
        PLAYER_REQUESTED_ANIM(entity) = 0xFF;
        PLAYER_ACTION_FLAGS(entity) &= 0xFFDFFFFF;
    }
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
