/* Soma_UpdateKnockbackFromImpulse; animation
 * block as in Soma_TryCeilingImpact, velocity stop as in Julius_UpdateKnockback. */
#include "code_08009A0.h"
#include "player_entity.h"
#include "code_08001194.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"

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
 * @brief Update damage-reaction state 14, entered with stored external
 * impulse velocities. Select animation 0x11, damp horizontal motion,
 * recover on landing/completion and update vertical motion.
 */
void Soma_UpdateKnockbackFromImpulse(struct EwramData_EntityData *entity)
{
    struct PlayerBodyBox box;
    s32 pal;
    s32 old, new;
    s32 hit;
    u16 x;
    s16 y;

    PLAYER_ACTION_FLAGS(entity) &= 0xEFFFFE9F;
    box = sSomaStandingBodyBox[0];
    if (!(gEwramData->playerState.actionLockFlags & 0x800) && entity->animScriptIndex != 0x11)
    {
        Anim_StartIndexed(entity, 0x11, 3, 0);
        HitBox_SetBody(entity, &box);
        PLAYER_REQUESTED_ANIM(entity) = 0xFF;
        PLAYER_ACTION_FLAGS(entity) &= ~0x200000;
        pal = sSomaAnimPalettes[0x11];
        if (PLAYER_ANIM_PALETTE(entity) != pal)
        {
            PLAYER_ANIM_PALETTE(entity) = pal;
            ObjPalette_LoadRange(sSomaPalette, pal, 1, 0);
        }
    }
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
    if (!(PLAYER_ACTION_FLAGS(entity) & 2) || (hit && (PLAYER_ACTION_FLAGS(entity) & 0x200000)))
    {
        PLAYER_ACTION_FLAGS(entity) &= 0xFFFEFF7F;
        entity->state = 0;
    }
    Soma_UpdateVerticalMovement(entity);
    Soma_HandleBlueSoulInput(entity);
}
