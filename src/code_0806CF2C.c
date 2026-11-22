/* Target-relative helpers 0x0806CF2C..0x0806D0B7. Target = playerState.linkTargetProxy when gameState.linkPlay is
 * set, else playerState.enemyTarget. */
#include "code_08009A0.h"
#include "structs/ewram.h"
#include "syscalls.h"

extern s32 Target_GetX(struct EwramData_EntityData *entity);
extern s32 Target_GetY(struct EwramData_EntityData *entity);
// Defined in src/code_08068A70.c with u16 parameters; the original caller
// sign-extends the first two coordinates, i.e. it saw an s16 prototype.
extern s32 Entity_IsInRect(struct EwramData_EntityData *entity, s16 left, s16 top, u16 width, u16 height);

/**
 * @brief Return 1 when signed entity X minus target X is negative, meaning the target is to the right; missing target returns entity flipX.
 */
s32 Entity_IsTargetRight(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *target = gEwramData->playerState.enemyTarget;

    if (gEwramData->gameState.linkPlay)
        target = gEwramData->playerState.linkTargetProxy;

    if (target == NULL)
        return entity->flipX;

    if ((s16)entity->posX.part16.integer - (s16)target->posX.part16.integer >= 0)
        return 0;
    return 1;
}

/**
 * @brief Return 1 when entity Y minus target Y is negative, meaning the target is below; missing target returns a random bit.
 */
s32 Entity_IsTargetBelow(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *target = gEwramData->playerState.enemyTarget;

    if (gEwramData->gameState.linkPlay)
        target = gEwramData->playerState.linkTargetProxy;

    if (target == NULL)
        return RandomNumberGenerator() & 1;

    if (entity->posY.part16.integer - target->posY.part16.integer >= 0)
        return 0;
    return 1;
}

/**
 * @brief 6CFFC | Whether enemyTarget exists and lies in the given rectangle
 * (see Entity_IsInRect).
 */
s32 Target_IsInRect(u16 left, u16 top, u16 width, u16 height)
{
    if (gEwramData->playerState.enemyTarget == NULL)
        return 0;
    return Entity_IsInRect(gEwramData->playerState.enemyTarget, left, top, width, height);
}

#define DIFF_INT(a, b) (((a) - (b) < 0) ? -(((b) - (a)) >> 16) : (((a) - (b)) >> 16))

/**
 * @brief Return Euclidean distance to Target_GetX/Y using truncated whole-pixel differences and a signed-halfword result.
 */
s16 Entity_GetTargetDistance(struct EwramData_EntityData *entity)
{
    s32 dx;
    s32 dy;

    dx = DIFF_INT(Target_GetX(entity), entity->posX.whole);
    dx *= dx;
    dy = DIFF_INT(Target_GetY(entity), entity->posY.whole);
    dy *= dy;
    return Sqrt(dx + dy);
}
