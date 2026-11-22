/* Julius_HandleJumpInput. */
#include "code_08009A0.h"
#include "player_entity.h"
#include "code_080D73B8.h"

extern void Afterimage_Create(struct EwramData_EntityData *, s32, s32, s32);
void Julius_TryStartDiveAttack(struct EwramData_EntityData *entity);

/**
 * @brief Process the ability-button high jump (state 5, requires flag 2),
 * the ordinary jump and both additional-jump variants (flag 4 or
 * flag 0x2000000 marks use), then fall through to the dive-attack
 * start check unless the 0x800000 variant was taken.
 */
void Julius_HandleJumpInput(struct EwramData_EntityData *entity)
{
    u32 flags = PLAYER_ACTION_FLAGS(entity);

    if (flags & 0x20000000)
        return;
    if (flags & 0x160)
        return;
    if (gEwramData->inputData.playerNewInput & gEwramData->playerStatus.abilityButton)
    if (flags & 2)
    if (!(flags & 0x10))
    {
        PLAYER_REQUESTED_ANIM(entity) = 0xFF;
        entity->state = 5;
        PlaySong(0xBA);
        PLAYER_ACTION_FLAGS(entity) = (PLAYER_ACTION_FLAGS(entity) | 0x12) & 0xFFEFFBFB;
        entity->accelY.whole = 0x2000;
        if (!(PLAYER_ACTION_FLAGS(entity) & 0x4000000))
            entity->velY.whole = -0x90000;
        else
            entity->velY.whole = -0x24000;
        Afterimage_Create(entity, 0, 8, 0xD5);
        Afterimage_Create(entity, 0, 4, 0xD4);
    }
    if ((u16)PLAYER_DROP_THROUGH_TIMER(entity) != 0)
        return;
    flags = PLAYER_ACTION_FLAGS(entity);
    if (flags & 0x10)
        return;
    if (!(flags & 0x800000))
    {
        if (!(flags & 4) && (gEwramData->inputData.playerNewInput & gEwramData->playerStatus.jumpButton))
        {
            PlaySong(0xB9);
            if (PLAYER_ACTION_FLAGS(entity) & 2)
                PLAYER_ACTION_FLAGS(entity) |= 4;
            PLAYER_ACTION_FLAGS(entity) = (PLAYER_ACTION_FLAGS(entity) | 2) & 0xFFEFFBFF;
            PLAYER_AIR_TIME(entity) = 0x10;
            entity->accelY.whole = 0;
            if (!(PLAYER_ACTION_FLAGS(entity) & 0x4000000))
                entity->velY.whole = -0x50000;
            else
                entity->velY.whole = -0x13C00;
        }
    }
    else if (!(flags & 0x2000000) && (gEwramData->inputData.playerNewInput & gEwramData->playerStatus.jumpButton))
    {
        entity->accelY.whole = 0;
        PLAYER_ACTION_FLAGS(entity) = (flags | 0x02000002) & 0xFFEFFBF7;
        if (entity->velY.whole > 0x10000)
            entity->velY.whole += -0x4E000;
        else
            entity->velY.whole = -0x4E000;
        return;
    }
    Julius_TryStartDiveAttack(entity);
}
