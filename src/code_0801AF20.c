/* Soma_StartDeath (same as
 * Julius_StartDeath with animation 0x33). */
#include "code_08009A0.h"
#include "player_entity.h"
#include "code_080D73B8.h"
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


void BlueSoul_StopAll(void);

/**
 * @brief Enter death state 16 from the main update when HP is nonpositive:
 * set the scene flags, animation 0x33, stop blue souls and apply
 * the existing recoil velocity, hit flags and sound.
 */
void Soma_StartDeath(struct EwramData_EntityData *entity)
{
    gEwramData->gameState.subMode = 2;
    gEwramData->gameState.subModeStep = 0;
    SetPlayerAnim(entity, 0x33);
    entity->hit.flags |= 4;
    BlueSoul_StopAll();
    entity->ext.player.actionFlags &= 0xEFFFFE9F;
    gEwramData->playerState.actionLockFlags |= 0xE;
    gEwramData->playerState.pendingHitDamage = 0;
    if (!entity->flipX)
        entity->velX.whole = -0x10000;
    else
        entity->velX.whole = 0x10000;
    entity->velY.whole = -0x18000;
    entity->accelY.whole = 0x2000;
    entity->state = 0x10;
    {
        u8 two = 2;
        gEwramData->playerState.player->ext.player.invulnerableTimer = two;
    }
    PlaySong(0x197);
}
