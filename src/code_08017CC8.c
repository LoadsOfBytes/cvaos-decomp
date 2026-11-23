/* Soma_TryCeilingImpact; animation
 * block as in Soma_InitEntity (src/code/code_08014548.c). */
#include "code_08009A0.h"
#include "player_entity.h"
#include "code_08011DD0.h"
#include "code_08032E4C.h"
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

/**
 * @brief Called after a ceiling correction while rising. When velY is at
 * least 0x4F000 upward with flag 0x10 but not 0x80, play sound 0xB8,
 * animation 0x25, shake the screen, stop motion, enter state 6 and
 * return 1; otherwise return 0. It is not a hard-landing handler.
 */
s32 Soma_TryCeilingImpact(struct EwramData_EntityData *entity)
{
    struct PlayerBodyBox box;
    s32 pal;

    if (entity->velY.whole <= -0x4F000 && (entity->ext.player.actionFlags & 0x90) == 0x10)
    {
        PlaySong(0xB8);
        Effect_SpawnDebrisBurst(entity);
        box = sSomaStandingBodyBox[0];
        if (!(gEwramData->playerState.actionLockFlags & 0x800) && entity->animScriptIndex != 0x25)
        {
            Anim_StartIndexed(entity, 0x25, 3, 0);
            HitBox_SetBody(entity, &box);
            entity->ext.player.requestedAnim = 0xFF;
            entity->ext.player.actionFlags &= ~0x200000;
            pal = sSomaAnimPalettes[0x25];
            if (entity->ext.player.animPalette != pal)
            {
                entity->ext.player.animPalette = pal;
                ObjPalette_LoadRange(sSomaPalette, pal, 1, 0);
            }
        }
        entity->state = 6;
        ScreenShake_Request(1, 0x10, 0, 7);
        entity->velX.whole = 0;
        entity->velY.whole = 0;
        entity->accelY.whole = 0;
            return 1;
    }
    return 0;
}
