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
        OBJECT0D_SPEED(entity) = impulse;
        OBJECT0D_AMPLITUDE(entity) = 0x100;
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
        OBJECT0D_SPEED(entity) = impulse;
        OBJECT0D_AMPLITUDE(entity) = 0x100;
    }
    angle = OBJECT0D_ANGLE(entity);
    if (angle > 0)
    {
        OBJECT0D_SPEED(entity) -= 0x10;
        if ((s32)(OBJECT0D_ANGLE(entity) = angle + OBJECT0D_SPEED(entity)) > 0)
            goto clamp;
        goto slow;
    }
    else if (angle < 0)
    {
        OBJECT0D_SPEED(entity) += 0x10;
        if ((s32)(OBJECT0D_ANGLE(entity) = angle + OBJECT0D_SPEED(entity)) < 0)
            goto clamp;
    slow:
        if (OBJECT0D_AMPLITUDE(entity) != 0)
            OBJECT0D_AMPLITUDE(entity) -= 0x20;
    }
    else
        OBJECT0D_ANGLE(entity) = angle + OBJECT0D_SPEED(entity);
clamp:
    {
        s32 v = OBJECT0D_SPEED(entity);
        s32 lim = OBJECT0D_AMPLITUDE(entity);
        if (v > lim || v < (lim = -lim))
            OBJECT0D_SPEED(entity) = lim;
    }
    {
        if ((s32)OBJECT0D_ANGLE(entity) > 0x1000)
            OBJECT0D_ANGLE(entity) = 0x1000;
        else if ((s32)OBJECT0D_ANGLE(entity) < -0x1000)
            OBJECT0D_ANGLE(entity) = -0x1000;
    }
    ObjAffine_Set(OBJECT0D_ANGLE(entity), 0x10000, 0x10000, 0, 0, entity->affineIndex);
}
