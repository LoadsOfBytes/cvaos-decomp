/* Soma_TryAirJump. */
#include "code_08009A0.h"
#include "player_entity.h"
#include "code/code_08040A38.h"
#include "code_080D73B8.h"

struct PlayerBodyBox {
    u8 x;
    u8 y;
    u8 width;
    u8 height;
};

extern struct PlayerBodyBox sSomaAirborneBodyBox[];

/**
 * @brief Used by Soma_HandleJumpInput: when the jump button is pressed in the
 * air state (flag 0x800000 or not 0x04000004) start the extra jump with
 * its hit box, sound 0xB9 and upward velocity. Return 1 when started.
 */
s32 Soma_TryAirJump(struct EwramData_EntityData *entity)
{
    struct PlayerBodyBox box;
    u16 zero;

    if ((PLAYER_ACTION_FLAGS(entity) & 0x800000) || !(PLAYER_ACTION_FLAGS(entity) & 0x04000004))
    {
        if ((gEwramData->inputData.playerNewInput & gEwramData->playerStatus.jumpButton)
            && (zero = PLAYER_DROP_THROUGH_TIMER(entity)) == 0)
        {
            u16 time = 0x14;
            box = sSomaAirborneBodyBox[0];
            PLAYER_REQUESTED_ANIM(entity) = time;
            HitBox_SetBody(entity, &box);
            PlaySong(0xB9);
            PLAYER_ACTION_FLAGS(entity) = (PLAYER_ACTION_FLAGS(entity) | 4) & ~0x10;
            if (!(PLAYER_ACTION_FLAGS(entity) & 0x400000))
            {
                entity->velY.whole = -0x42000;
                entity->accelY.whole = 0x1400;
            }
            else if (entity->velY.whole >= 0)
            {
                entity->velY.whole = -0x48000;
                entity->accelY.whole = zero;
            }
            return 1;
        }
    }
    return 0;
}
