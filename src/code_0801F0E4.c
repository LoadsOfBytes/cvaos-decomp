/* Julius_TryStartDiveAttack
 * (dive attack as in Soma_TryStartDiveAttack without hitboxes). */
#include "code_08009A0.h"
#include "code_080D73B8.h"

void Anim_StartIndexed(struct EwramData_EntityData *entity, s32 script, s32 mode, s32 flags);
extern void Afterimage_Create(struct EwramData_EntityData *, s32, s32, s32);

static inline void SetPlayerAnim(struct EwramData_EntityData *entity, s32 anim)
{
    if (!(gEwramData->playerState.actionLockFlags & 0x800) && entity->animScriptIndex != anim)
    {
        Anim_StartIndexed(entity, anim, 1, 1);
        entity->unk_504.part16.unk_504 = 0xFF;
        entity->unk_4F4.whole &= 0xFFDFFFFF;
    }
}

/**
 * @brief When the entity is airborne (flag 2) without flags 0x400000 and
 * 0x800000 and down + new jump is pressed, enter state 7 with downward
 * velY 0x4E000, optional +-0x40000 X, animation 9/0x1E by speed,
 * sounds 0x1EC/0xA9 and two afterimages. Julius counterpart of
 * Soma_TryStartDiveAttack, without attack-box setup.
 */
void Julius_TryStartDiveAttack(struct EwramData_EntityData *entity)
{
    struct EwramData *ew;
    u16 key;
    s32 speed;

    if ((entity->unk_4F4.whole & 0xC00002) != 2)
        return;
    ew = gEwramData;
    if (!(ew->inputData.playerHeldInput & 0x80))
        return;
    if (!(ew->inputData.playerNewInput & ew->playerStatus.jumpButton))
        return;
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
        SetPlayerAnim(entity, 9);
    else
        SetPlayerAnim(entity, 0x1E);
    gEwramData->playerState.actionLockFlags |= 4;
    PlaySong(0x1EC);
    PlaySong(0xA9);
    Afterimage_Create(entity, 0, 6, 0xD5);
    Afterimage_Create(entity, 0, 3, 0xD4);
}
