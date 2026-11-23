#include "code_08009A0.h"
#include "room_object_entity.h"
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
        entity->ext.object0D.speed = impulse;
        entity->ext.object0D.amplitude = 0x100;
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
        entity->ext.object0D.speed = impulse;
        entity->ext.object0D.amplitude = 0x100;
    }
    angle = entity->ext.object0D.angle;
    if (angle > 0)
    {
        entity->ext.object0D.speed -= 0x10;
        if ((s32)(entity->ext.object0D.angle = angle + entity->ext.object0D.speed) > 0)
            goto clamp;
        goto slow;
    }
    else if (angle < 0)
    {
        entity->ext.object0D.speed += 0x10;
        if ((s32)(entity->ext.object0D.angle = angle + entity->ext.object0D.speed) < 0)
            goto clamp;
    slow:
        if (entity->ext.object0D.amplitude != 0)
            entity->ext.object0D.amplitude -= 0x20;
    }
    else
        entity->ext.object0D.angle = angle + entity->ext.object0D.speed;
clamp:
    {
        s32 v = entity->ext.object0D.speed;
        s32 lim = entity->ext.object0D.amplitude;
        if (v > lim || v < (lim = -lim))
            entity->ext.object0D.speed = lim;
    }
    {
        if ((s32)entity->ext.object0D.angle > 0x1000)
            entity->ext.object0D.angle = 0x1000;
        else if ((s32)entity->ext.object0D.angle < -0x1000)
            entity->ext.object0D.angle = -0x1000;
    }
    ObjAffine_Set(entity->ext.object0D.angle, 0x10000, 0x10000, 0, 0, entity->affineIndex);
}
