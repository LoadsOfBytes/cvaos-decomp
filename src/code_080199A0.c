/* Soma_UpdateKnockback (Soma_UpdateKnockbackFromImpulse
 * without the animation switch). */
#include "code_08009A0.h"
#include "code_08001194.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code_080D73B8.h"

struct PlayerBodyBox {
    u8 x;
    u8 y;
    u8 width;
    u8 height;
};

extern struct PlayerBodyBox sSomaStandingBodyBox[];
extern u8 sSomaAnimPalettes[];
extern u8 sSomaPalette[];
void Soma_UpdateVerticalMovement(struct EwramData_EntityData *entity);
void Soma_HandleBlueSoulInput(struct EwramData_EntityData *entity);

/**
 * @brief Update damage-reaction state 13: damp horizontal motion without
 * crossing zero, recover on landing or tile/animation completion and
 * allow the existing ability-gated jump cancel. Update vertical motion.
 */
void Soma_UpdateKnockback(struct EwramData_EntityData *entity)
{
    s32 old, new;
    s32 hit;
    u16 x;
    s16 y;

    entity->unk_4F4.whole &= 0xEFFFFE9F;
    x = GetEntityRoomXPositionInteger(entity);
    y = GetEntityRoomYPositionInteger(entity);
    hit = 0;
    if (Collision_IsPointSolid((s16)x, y))
    {
        hit = 1;
        entity->accelX.whole = (entity->velX.whole < 0) ? 0x1000 : -0x1000;
    }
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
    if (!(entity->unk_4F4.whole & 2) || (hit && (entity->unk_4F4.whole & 0x200000)))
    {
        entity->unk_4F4.whole = entity->unk_4F4.whole & 0xFFFEFF7F;
        entity->state = 0;
    }
    else if ((gEwramData->playerStatus.soulStateFlags & 0x400000) && (gEwramData->inputData.playerNewInput & gEwramData->playerStatus.jumpButton))
    {
        entity->accelY.whole = 0;
        entity->unk_4F4.whole = entity->unk_4F4.whole & 0xFFEFFB7F;
        PlaySong(0xB9);
        entity->unk_4F4.whole |= 6;
        entity->velY.whole = 0;
        entity->state = 0;
    }
    Soma_UpdateVerticalMovement(entity);
    Soma_HandleBlueSoulInput(entity);
}
