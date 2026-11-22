/* Probabilistic gate followed by a direction-dependent room obstacle probe. */
#include "code_08009A0.h"
#include "minotaur_entity.h"
#include "code_08001194.h"
#include "structs/ewram.h"
extern s32 Target_IsNearEntity(struct EwramData_EntityData *, s32, s32);

s32 Minotaur_CheckAttackChance(struct EwramData_EntityData *entity, s32 mask)
{
    u16 x, y, result;
    if (!MINOTAUR_VARIANT(entity) || (RandomNumberGenerator2() & mask) != 0)
        return 1;
    if (!(MINOTAUR_FLAGS(entity) & 2))
        return 0;
    if (Target_IsNearEntity(entity, 160, 120))
        return 1;
    x = GetEntityRoomXPositionInteger(entity);
    y = GetEntityRoomYPositionInteger(entity);
    if (entity->flipX)
        result = Collision_GetPushLeft((s16)x + 80, (s16)y);
    else
        result = Collision_GetPushRight((s16)x - 80, (s16)y);
    return (s16)result;
}
