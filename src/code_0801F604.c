/* Julius_UpdateVerticalMovement. */
#include "code_08009A0.h"
#include "player_entity.h"
#include "code_08001194.h"
#include "code_08011DD0.h"
#include "code_080D73B8.h"

void Anim_StartIndexed(struct EwramData_EntityData *entity, s32 script, s32 mode, s32 flags);

/* Function table of the object in solidObject (pointer at its unk_4F4). */
struct SolidObjectCallbacks {
    void *flags;
    s16 (*floorCorrection)(struct EwramData_EntityData *object, s32 x, s32 y);
};

/* Animation switch without hitbox, as in src/code_0801F00C.c (flags 1, 1). */
static inline void SetPlayerAnim(struct EwramData_EntityData *entity, s32 anim)
{
    if (!(gEwramData->playerState.actionLockFlags & 0x800) && entity->animScriptIndex != anim)
    {
        Anim_StartIndexed(entity, anim, 1, 1);
        entity->ext.player.requestedAnim = 0xFF;
        entity->ext.player.actionFlags &= 0xFFDFFFFF;
    }
}

/* Floor correction at room pixel (x, y): the tile map value (Collision_GetPushDown),
 * or the larger one reported by the object in solidObject. */
static inline s16 FloorCorrection(s16 x, s16 y)
{
    u16 c = Collision_GetPushDown(x, y);
    struct EwramData_EntityData *object = gEwramData->playerState.solidObject;

    if (object != NULL)
    {
        s16 d = ((struct SolidObjectCallbacks *)object->ext.solidObject.callbacks.words)->floorCorrection(object, x, y);
        return ((s16)c < d) ? d : c;
    }
    return c;
}

/**
 * @brief Update Julius airborne gravity, floor correction, landing
 * feedback and air animation through the existing flag guards.
 * Preserve the original asymmetric probe/velocity conditions.
 */
void Julius_UpdateVerticalMovement(struct EwramData_EntityData *entity)
{
    s16 corr = 0;
    u16 y = GetEntityRoomYPositionInteger(entity);
    u16 x = GetEntityRoomXPositionInteger(entity);
    u32 flags;

    if (!(entity->ext.player.actionFlags & 0x100002))
    {
        entity->ext.player.actionFlags |= 2;
        entity->velY.whole = corr;
        entity->accelY.whole = -0x2000;
    }
    if (!(entity->ext.player.actionFlags & 0x1E))
        return;
    entity->ext.player.actionFlags &= 0xEFEFFBFF;
    if (entity->ext.player.actionFlags & 0x4000000)
    {
        entity->velY.whole += 0x6000;
        entity->accelY.whole = corr;
    }
    if (entity->ext.player.airTime <= 0xF)
        entity->ext.player.airTime++;
    if (entity->velY.whole <= 0 || entity->velX.whole != 0)
    {
        /* (x - 5, y - 0x20) and (x + 5, y - 0x20), written in 16.16 as in the
         * original arithmetic. */
        corr = FloorCorrection(((x << 16) - 0x50000) >> 16, ((y << 16) - 0x200000) >> 16);
        if (corr == 0)
            corr = FloorCorrection(((x << 16) + 0x50000) >> 16, ((y << 16) - 0x200000) >> 16);
        if (corr != 0)
        {
            entity->posY.part16.integer += corr;
            if (entity->velY.whole <= -0x4F000 && (entity->ext.player.actionFlags & 0x10))
            {
                PlaySong(0xB8);
                entity->state = 0;
                ScreenShake_Request(1, 0x10, 0, 7);
            }
            if (entity->velY.whole < 0)
            {
                entity->velY.whole = 0x1000;
                entity->accelY.whole = -0x2000;
            }
        }
    }
    if (entity->velY.whole <= 0x1FFF)
        entity->velY.whole += 0x2000;
    entity->velY.whole += 0x1800;
    if (entity->velY.whole > 0)
    {
        entity->velY.whole += entity->accelY.whole;
        entity->accelY.whole += 0x400;
        if (entity->accelY.whole > 0x1000)
            entity->accelY.whole = 0x1000;
    }
    flags = entity->ext.player.actionFlags;
    if (flags & 0x10)
    {
        if (!(flags & 0x400000))
        {
            if (entity->velY.whole > 0x12000)
                entity->ext.player.actionFlags = flags & ~0x10;
        }
        else if (entity->velY.whole > 0x2000)
            entity->ext.player.actionFlags = flags & ~0x10;
    }
    flags = entity->ext.player.actionFlags;
    if (flags & 0x1E0)
        return;
    if (gEwramData->playerState.actionLockFlags & 4)
        return;
    if (flags & 0x10)
        SetPlayerAnim(entity, 0x16);
    else if (entity->velY.whole < -0x8000)
        SetPlayerAnim(entity, 3);
    else if (entity->velY.whole <= 0x7FFF)
        SetPlayerAnim(entity, 0x10);
    else
        SetPlayerAnim(entity, 0x11);
}
