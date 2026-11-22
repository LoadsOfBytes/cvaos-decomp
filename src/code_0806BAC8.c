/* Entity_GetTargetAngleAbove16: matched (s16 x/y locals: fold narrows to signed short subtraction,
   expand_binop loads posX before sign-extending x). */
/* Aim at the selected target, with a sixteen-pixel vertical offset. */
#include "structs/ewram.h"
#include "syscalls.h"
static inline s32 TargetX(struct EwramData_EntityData *entity)
{
    if (!gEwramData->gameState.linkPlay)
    {
        struct EwramData_EntityData *target = gEwramData->playerState.enemyTarget;
        if (target == NULL) return (s16)entity->posX.part16.integer;
        return (s16)target->posX.part16.integer;
    }
    return (s16)gEwramData->playerState.linkTargetProxy->posX.part16.integer;
}
static inline s16 TargetY(struct EwramData_EntityData *entity)
{
    if (!gEwramData->gameState.linkPlay)
    {
        struct EwramData_EntityData *target = gEwramData->playerState.enemyTarget;
        if (target == NULL) return entity->posY.part16.integer;
        return target->posY.part16.integer;
    }
    return gEwramData->playerState.linkTargetProxy->posY.part16.integer;
}
/**
 * @brief Return ArcTan2 toward the selected target with target Y reduced by 16 pixels; preserve signed-halfword coordinate subtraction and target fallback.
 */
u16 Entity_GetTargetAngleAbove16(struct EwramData_EntityData *entity)
{
    s16 x, y;
    if (entity->enemyFlags & 4)
    {
        struct EwramData_EntityData *player = gEwramData->playerState.player;
        x = player->posX.part16.integer;
        y = player->posY.part16.integer;
    }
    else
    {
        x = TargetX(entity);
        y = TargetY(entity);
    }
    return ArcTan2(x - (s16)entity->posX.part16.integer, y - entity->posY.part16.integer - 16);
}
