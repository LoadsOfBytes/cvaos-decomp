/* Soma_HandleBatFlightInput. */
#include "code_08009A0.h"
#include "player_entity.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"

struct PlayerBodyBox {
    u8 x;
    u8 y;
    u8 width;
    u8 height;
};

extern struct PlayerBodyBox sSomaBatFormBodyBoxes[];
extern u8 sSomaAnimPalettes[];
extern u8 sSomaPalette[];

/* Animation switch as in Soma_UpdateStatusReaction, with the 0x080E1308 hitbox, flag 1 and
 * the saved animStep restored afterwards. */
static inline void SetFlyAnim(struct EwramData_EntityData *entity, s32 anim, u8 saved)
{
    struct PlayerBodyBox box = sSomaBatFormBodyBoxes[0];
    s32 pal;

    if (!(gEwramData->playerState.actionLockFlags & 0x800) && entity->animScriptIndex != anim)
    {
        Anim_StartIndexed(entity, anim, 3, 1);
        HitBox_SetBody(entity, &box);
        entity->unk_504.part16.unk_504 = 0xFF;
        entity->unk_4F4.whole &= ~0x200000;
        pal = sSomaAnimPalettes[anim];
        if (PLAYER_ANIM_PALETTE(entity) != pal)
        {
            PLAYER_ANIM_PALETTE(entity) = pal;
            ObjPalette_LoadRange(sSomaPalette, pal, 1, 0);
        }
        entity->animStep = saved;
    }
}

/**
 * @brief Bat-form flight input: each held d-pad axis sets +-0x20000
 * velocity, otherwise decelerate by 0x4000 without crossing zero.
 * Select animation 0x1B idle or 0x1C moving, restoring animStep.
 */
void Soma_HandleBatFlightInput(struct EwramData_EntityData *entity)
{
    u8 saved = entity->animStep;
    u16 key;
    u16 key2;
    s32 old, new;

    entity->unk_4FC.whole = 0x080E12EA;
    if ((key = gEwramData->inputData.playerHeldInput & 0x20))
    {
        entity->flipX = 1;
        entity->velX.whole = -0x20000;
        entity->accelX.whole = 0;
    }
    else if (gEwramData->inputData.playerHeldInput & 0x10)
    {
        entity->flipX = 0;
        entity->velX.whole = 0x20000;
        entity->accelX.whole = key;
    }
    else
        entity->accelX.whole = (entity->velX.whole < 0) ? 0x4000 : -0x4000;
    old = entity->velX.whole;
    new = old + entity->accelX.whole;
    entity->velX.whole = new;
    if (old < 0)
        goto check_x;
    if (new < 0)
        goto clear_x;
    if (old > 0)
        goto done_x;
check_x:
    if (new <= 0)
        goto done_x;
clear_x:
    entity->velX.whole = 0;
    entity->accelX.whole = 0;
done_x:
    if ((key2 = gEwramData->inputData.playerHeldInput & 0x40))
    {
        entity->velY.whole = -0x20000;
        entity->accelY.whole = 0;
    }
    else if (gEwramData->inputData.playerHeldInput & 0x80)
    {
        entity->velY.whole = 0x20000;
        entity->accelY.whole = key2;
    }
    else
        entity->accelY.whole = (entity->velY.whole < 0) ? 0x4000 : -0x4000;
    old = entity->velY.whole;
    new = old + entity->accelY.whole;
    entity->velY.whole = new;
    if (old < 0)
        goto check_y;
    if (new < 0)
        goto clear_y;
    if (old > 0)
        goto done_y;
check_y:
    if (new <= 0)
        goto done_y;
clear_y:
    entity->velY.whole = 0;
    entity->accelY.whole = 0;
done_y:
    if ((u16)entity->unk_504.part16.unk_504 == 0xFF)
    {
        if (!(gEwramData->inputData.playerHeldInput & 0xF0))
            SetFlyAnim(entity, 0x1B, saved);
        else
            SetFlyAnim(entity, 0x1C, saved);
    }
}
