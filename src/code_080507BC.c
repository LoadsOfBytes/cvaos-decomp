#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
extern u16 Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);

/**
 * @brief 507BC | Swinging object (object 0x0D, hangs from the ceiling):
 * hidden below y 0xA8; when the player lands on it (standingPlatform) or hits it
 * (Hit_CheckWithPlayer flags 0x1A) it gets a swing impulse of 0x100 away from the
 * player (sound 0x112, amplitude unk_504 = 0x100). The angle unk_4FC swings
 * back towards 0 with acceleration 0x10, losing 0x20 of amplitude at each
 * zero crossing; speed and angle (+-0x1000) are clamped and drawn with
 * ObjAffine_Set.
 */
void Object0DUpdate(struct EwramData_EntityData *entity)
{
    u16 box[8];
    s32 angle;
    s32 impulse;

    if ((s16)entity->posY.part16.integer > 0xA8)
        entity->hidden = 1;
    else
        entity->hidden = 0;
    Player_QueueEntity(entity, 0);
    if (entity->subState == 0)
    {
        if (gEwramData->playerState.standingPlatform != entity)
            goto hit;
        impulse = 0x100;
        PlaySong(0x112);
        if (gEwramData->playerState.player->posX.whole < entity->posX.whole)
            impulse = -impulse;
        entity->unk_500.whole = impulse;
        entity->unk_504.whole = 0x100;
        entity->subState = 1;
    }
    else if (gEwramData->playerState.standingPlatform != entity)
        entity->subState = 0;
hit:
    if (Hit_CheckWithPlayer(box, entity) && (box[0] & 0x1A))
    {
        impulse = 0x100;
        PlaySong(0x112);
        if (gEwramData->playerState.player->posX.whole < entity->posX.whole)
            impulse = -impulse;
        entity->unk_500.whole = impulse;
        entity->unk_504.whole = 0x100;
    }
    angle = entity->unk_4FC.whole;
    if (angle > 0)
    {
        entity->unk_500.whole -= 0x10;
        if ((s32)(entity->unk_4FC.whole = angle + entity->unk_500.whole) > 0)
            goto clamp;
        goto slow;
    }
    else if (angle < 0)
    {
        entity->unk_500.whole += 0x10;
        if ((s32)(entity->unk_4FC.whole = angle + entity->unk_500.whole) < 0)
            goto clamp;
    slow:
        if (entity->unk_504.whole != 0)
            entity->unk_504.whole -= 0x20;
    }
    else
        entity->unk_4FC.whole = angle + entity->unk_500.whole;
clamp:
    {
        s32 v = entity->unk_500.whole;
        s32 lim = entity->unk_504.whole;
        if (v > lim || v < (lim = -lim))
            entity->unk_500.whole = lim;
    }
    {
        if ((s32)entity->unk_4FC.whole > 0x1000)
            entity->unk_4FC.whole = 0x1000;
        else if ((s32)entity->unk_4FC.whole < -0x1000)
            entity->unk_4FC.whole = -0x1000;
    }
    ObjAffine_Set(entity->unk_4FC.whole, 0x10000, 0x10000, 0, 0, entity->affineIndex);
}
