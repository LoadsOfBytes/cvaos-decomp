/* Soma_TryStartDiveAttack. */
#include "code_08009A0.h"
#include "player_entity.h"
#include "code_08032444.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code_080D73B8.h"

struct PlayerBodyBox {
    u8 x;
    u8 y;
    u8 width;
    u8 height;
};

extern struct PlayerBodyBox sSomaAirborneBodyBox[];
extern u8 sSomaAnimPalettes[];
extern u8 sSomaPalette[];
extern u8 sSomaDiveAttackBoxFast[];
extern u8 sSomaDiveAttackBoxSlow[];

/* Animation switch as in Soma_UpdateStatusReaction, with the 0x080E12FC hitbox and flag 1. */
static inline void SetSlideAnim(struct EwramData_EntityData *entity, s32 anim)
{
    struct PlayerBodyBox box = sSomaAirborneBodyBox[0];
    s32 pal;

    if (!(gEwramData->playerState.actionLockFlags & 0x800) && entity->animScriptIndex != anim)
    {
        Anim_StartIndexed(entity, anim, 3, 1);
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
 * @brief When ability check 3 passes, the entity is airborne (flag 2)
 * with flag 4 but not 0x800000, and down + new jump is pressed,
 * enter state 7 with downward velY 0x4E000, optional +-0x40000 X,
 * animation 0x26/0x27 and attack box by speed, and sound 0xA9.
 * The old slide description was incorrect.
 */
void Soma_TryStartDiveAttack(struct EwramData_EntityData *entity)
{
    struct EwramData *ew;
    u16 key;
    s32 speed;

    if (!Player_HasAbility(3))
        return;
    if (!(entity->ext.player.actionFlags & 2))
        return;
    if ((entity->ext.player.actionFlags & 0x800004) != 4)
        return;
    ew = gEwramData;
    if (!(ew->inputData.playerHeldInput & 0x80))
        return;
    if (!(ew->inputData.playerNewInput & ew->playerStatus.jumpButton))
        return;
    entity->hit.type = 2;
    entity->state = 7;
    entity->velY.whole = 0x4E000;
    entity->accelY.whole = 0x2000;
    if ((key = ew->inputData.playerHeldInput & 0x20))
    {
        entity->flipX = 1;
        entity->velX.whole = -0x40000;
        entity->accelX.whole = 0;
    }
    else if (ew->inputData.playerHeldInput & 0x10)
    {
        entity->flipX = 0;
        entity->velX.whole = 0x40000;
        entity->accelX.whole = key;
    }
    else
        entity->accelX.whole = (entity->velX.whole < 0) ? 0x4000 : -0x4000;
    speed = entity->velX.whole;
    if (speed < 0)
        speed = -speed;
    if (speed > 0x10000)
    {
        SetSlideAnim(entity, 0x26);
        HitBox_SetAttack(entity, sSomaDiveAttackBoxFast);
    }
    else
    {
        SetSlideAnim(entity, 0x27);
        HitBox_SetAttack(entity, sSomaDiveAttackBoxSlow);
    }
    gEwramData->playerState.actionLockFlags |= 4;
    PlaySong(0xA9);
}
