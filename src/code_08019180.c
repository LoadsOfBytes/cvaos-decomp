/* Soma_HandleJumpInput. */
#include "code_08009A0.h"
#include "player_entity.h"
#include "code_08032444.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern void Afterimage_Create(struct EwramData_EntityData *, s32, s32, s32);
s32 Soma_TryAirJump(struct EwramData_EntityData *entity);
void Soma_TryStartDiveAttack(struct EwramData_EntityData *entity);

/**
 * @brief Process drop-through, ordinary jump, ability-gated high jump
 * and additional airborne jump input under the existing flags.
 * Retain the helper fallback, timing windows and movement variants.
 */
void Soma_HandleJumpInput(struct EwramData_EntityData *entity)
{
    u32 flags = entity->ext.player.actionFlags;
    u32 zero;

    if (flags & 0x20000000)
        return;
    if (flags & 0x160)
        return;
    if (!(zero = flags & 2) && (flags & 0x1000)
        && (gEwramData->inputData.playerHeldInput & 0x80)
        && (gEwramData->inputData.playerNewInput & gEwramData->playerStatus.jumpButton))
    {
        entity->ext.player.dropThroughTimer = 0x10;
        if (flags & 0x1000000)
            entity->ext.player.dropThroughTimer = 0x20;
        entity->ext.player.actionFlags = (entity->ext.player.actionFlags | 2) & ~0x11000;
        entity->velX.whole = zero;
        entity->velY.whole = zero;
        entity->accelY.whole = -0x1000;
    }
    if (Player_HasAbility(4) && (gEwramData->inputData.playerNewInput & gEwramData->playerStatus.abilityButton))
    {
        flags = entity->ext.player.actionFlags;
        if (((flags & 2) || (gEwramData->inputData.playerHeldInput & 0x40)) && !(zero = flags & 0x10))
        {
            entity->ext.player.requestedAnim = 0xFF;
            entity->state = 5;
            PlaySong(0xBA);
            entity->ext.player.actionFlags = (entity->ext.player.actionFlags | 0x12) & ~0x100404;
            entity->accelY.whole = zero;
            if (!(entity->ext.player.actionFlags & 0x4000000))
                entity->velY.whole = -0xA0000;
            else
                entity->velY.whole = -0x28000;
            Afterimage_Create(entity, 0, 8, 0x1D);
            Afterimage_Create(entity, 0, 6, 0x1C);
            return;
        }
    }
    if ((u16)entity->ext.player.dropThroughTimer != 0)
        return;
    flags = entity->ext.player.actionFlags;
    if (!(zero = flags & 0x800000))
    {
        if (!(flags & 2) || entity->ext.player.airTime <= 3)
        {
            if (gEwramData->inputData.playerNewInput & gEwramData->playerStatus.jumpButton)
            {
                PlaySong(0xB9);
                entity->ext.player.actionFlags = (entity->ext.player.actionFlags | 2) & ~0x100400;
                entity->ext.player.airTime = 0x10;
                entity->accelY.whole = zero;
                if (!(entity->ext.player.actionFlags & 0x4000000))
                    entity->velY.whole = -0x4F400;
                else
                    entity->velY.whole = -0x13C00;
            }
        }
        else if (Player_HasAbility(2) && Soma_TryAirJump(entity))
            return;
    }
    else if (!(zero = flags & 0x2000000))
    {
        if (gEwramData->inputData.playerNewInput & gEwramData->playerStatus.jumpButton)
        {
            entity->accelY.whole = zero;
            entity->ext.player.actionFlags = (flags | 0x02000002) & ~0x100408;
            if (entity->velY.whole > 0x10000)
                entity->velY.whole -= 0x4E000;
            else
                entity->velY.whole = -0x4E000;
            return;
        }
    }
    Soma_TryStartDiveAttack(entity);
}
