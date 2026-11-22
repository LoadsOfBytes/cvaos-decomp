/* Bat helpers 0x080AD6AC..0x080AD747. */
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern s32 Enemy_TakeDamage(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);

/**
 * @brief Start animation 1 once, then advance it. The gameplay meaning of state 4 is unresolved.
 */
void Bat_UpdateState4Animation(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            if (entity->animScriptIndex != 1)
                Anim_StartIndexed(entity, 1, 0, 1);
            entity->subState = 1;
            entity->stage = 0;
        case 1:
            Anim_Update(entity);
            break;
    }
}

/**
 * @brief Apply damage and enter death state 3 when Enemy_TakeDamage reports death.
 */
void Bat_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hitData)
{
    if (Enemy_TakeDamage(entity, attacker, hitData))
    {
        entity->state = 3;
        entity->stage = 0;
        entity->subState = 0;
    }
}

/**
 * @brief Test playerState.enemyTarget in a width-by-height rectangle below the bat, centered horizontally.
 */
s32 Bat_IsTargetBelow(struct EwramData_EntityData *entity, u16 width, u16 height)
{
    struct EwramData_EntityData *target = gEwramData->playerState.enemyTarget;
    s16 left = entity->posX.part16.integer - (width >> 1);
    s16 top = entity->posY.part16.integer;

    return Entity_IsInRect(target, left, top, width, height);
}
