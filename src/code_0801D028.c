/* Player_StartScriptedAnimation: start a scripted player animation (Soma or other character) with velocity. */
#include "code_08009A0.h"
#include "player_entity.h"
#include "code/code_080211F0.h"
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

/**
 * @brief Return one if the player is dead; otherwise select the character-dependent animation under existing guards and set scripted state, facing flags and velocity, returning zero.
 */
s32 Player_StartScriptedAnimation(u16 anim, u16 flags, s32 velX, s32 velY)
{
    u8 mode = 0;
    u16 left, right;

    if (gEwramData->playerStatus.currentHP <= 0)
        return 1;
    if (flags & 1)
        mode = 1;
    if (gEwramData->playerStatus.currentCharacter == 0)
    {
        u8 a = anim;
        struct EwramData_EntityData *player = gEwramData->playerState.player;
        struct PlayerBodyBox box = sSomaStandingBodyBox[0];
        s32 pal;

        if (!(gEwramData->playerState.actionLockFlags & 0x800) && player->animScriptIndex != a)
        {
            Anim_StartIndexed(player, a, 3, mode);
            HitBox_SetBody(player, &box);
            PLAYER_REQUESTED_ANIM(player) = 0xFF;
            PLAYER_ACTION_FLAGS(player) &= ~0x200000;
            pal = sSomaAnimPalettes[a];
            if (PLAYER_ANIM_PALETTE(player) != pal)
            {
                PLAYER_ANIM_PALETTE(player) = pal;
                ObjPalette_LoadRange(sSomaPalette, pal, 1, 0);
            }
        }
    }
    else
    {
        switch (anim)
        {
        case 0xE:
        case 0xF:
        case 0x10:
        case 0x11:
            anim = 0xC;
            break;
        case 0x20:
            anim = 0xB;
            break;
        default:
            anim = 0;
            break;
        }
        Player_TryStartAnimation(gEwramData->playerState.player, anim, mode);
    }
    if ((left = flags & 2))
        gEwramData->playerState.player->flipX = 1;
    right = flags & 4;
    if (right)
        gEwramData->playerState.player->flipX = 0;
    gEwramData->playerState.player->state = 0x11;
    PLAYER_ACTION_FLAGS(gEwramData->playerState.player) |= 0x40000000;
    if (left)
        gEwramData->playerState.player->flipX = 1;
    else if (right)
        gEwramData->playerState.player->flipX = 0;
    gEwramData->playerState.player->velX.whole = velX;
    gEwramData->playerState.player->velY.whole = velY;
    return 0;
}
