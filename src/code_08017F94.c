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

static inline void SetBox(struct EwramData_EntityData *entity, s16 n)
{
    struct PlayerBodyBox box = sSomaAirborneBodyBox[0];
    PLAYER_REQUESTED_ANIM(entity) = n;
    HitBox_SetBody(entity, &box);
}

/* Handle the observed flag-dependent state, box, sound and motion changes.
 * Keep the zero assignment after the flag update and its r4 lifetime across
 * the calls: moving or folding it changes agbcc instruction ordering.
 * Matching output does not establish original source spelling. */
/**
 * @brief Update state 7: end on flag 0x400000; on contact flag 0x80000
 * rebound upward (velY -0x38000), set flag 8, clear flag 4, play
 * sound 0xB9 and return to state 0; otherwise set actionLockFlags |= 4.
 */
void Soma_UpdateDiveAttack(struct EwramData_EntityData *entity)
{
    if (PLAYER_ACTION_FLAGS(entity) & 0x400000)
    {
        entity->state = 0;
        return;
    }
    entity->hit.flags &= ~2;
    if (PLAYER_ACTION_FLAGS(entity) & 0x80000)
    {
        register s32 zero asm("r4"); /* FAKEMATCH: pinned register */
        PLAYER_ACTION_FLAGS(entity) = (PLAYER_ACTION_FLAGS(entity) | 8) & ~4;
        zero = 0;
        SetBox(entity, 0x14);
        PlaySong(0xB9);
        entity->velY.whole = -0x38000;
        entity->accelY.whole = -0x2000;
        entity->state = zero;
    }
    else
        gEwramData->playerState.actionLockFlags |= 4;
}
