/* Soma_UpdateStatusReaction. */
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

/* Switch the player animation (with hitbox and palette from the per-animation
 * tables) unless actionLockFlags bit 11 or it is already active. */
static inline void SetPlayerAnim(struct EwramData_EntityData *entity, s32 anim)
{
    struct PlayerBodyBox box = sSomaStandingBodyBox[0];
    s32 pal;

    if (!(gEwramData->playerState.actionLockFlags & 0x800) && entity->animScriptIndex != anim)
    {
        Anim_StartIndexed(entity, anim, 3, 0);
        HitBox_SetBody(entity, &box);
        entity->ext.player.requestedAnim = 0xFF;
        entity->ext.player.actionFlags &= ~0x200000;
        pal = sSomaAnimPalettes[anim];
        if (entity->ext.player.animPalette != pal)
        {
            entity->ext.player.animPalette = pal;
            ObjPalette_LoadRange(sSomaPalette, pal, 1, 0);
        }
    }
}

/**
 * @brief Update state 15 while status bit statusEffects & 4 remains set.
 * Select animation 0x49/0x4A from the existing status flag, stop
 * grounded horizontal motion and update vertical movement.
 */
void Soma_UpdateStatusReaction(struct EwramData_EntityData *entity)
{
    u16 x;
    s16 y;
    u16 flag = gEwramData->playerStatus.statusEffects & 4;

    if (!flag)
    {
        entity->ext.player.actionFlags &= 0xFFFEFF7F;
        entity->ext.player.actionFlags &= 0xEFFFFE9F;
        entity->state = flag;
        return;
    }
    x = GetEntityRoomXPositionInteger(entity);
    y = GetEntityRoomYPositionInteger(entity);
    Collision_IsPointSolid((s16)x, y);
    if (gEwramData->playerStatus.petrifyCounter & 2)
        SetPlayerAnim(entity, 0x49);
    else
        SetPlayerAnim(entity, 0x4A);
    if (!(entity->ext.player.actionFlags & 2))
    {
        entity->velX.whole = 0;
        entity->accelX.whole = 0;
    }
    Soma_UpdateVerticalMovement(entity);
}
