#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern s32 Entity_IsInRect(struct EwramData_EntityData *, s16, s16, u16, u16);

/**
 * @brief Place the projectile relative to its owner and accelerate it
 * horizontally. Hit state 5 falls; contact state 6 counts down to removal.
 * Keep collision checks during stun/time stop and the timer-underflow test.
 */
void RipperProjectile_Update(struct EwramData_EntityData *entity)
{
    u32 buf[4];
    struct EwramData_EntityData *owner = entity->ext.ripperProjectile.owner;

    Enemy_UpdateHitFlash(entity);
    if (Entity_IsTimeStopped(entity) == 0 || entity->state == 5 || entity->state == 6)
    {
        if (entity->hitFlashTimer == 0 || entity->state == 2 || entity->state == 5
            || entity->state == 6)
            goto run;
    }
    Hit_CheckWithPlayer(buf, entity);
    return;
run:
    switch (entity->state)
    {
    case 0:
        entity->state++;
    case 1:
        entity->velX.whole = 0x30000;
        entity->velY.whole = 0;
        entity->accelX.whole = 0x50;
        entity->accelY.whole = 0;
        entity->state++;
    case 2:
        entity->flipX = owner->flipX;
        if (entity->flipX)
            entity->posX.whole += 0xC0000 + owner->posX.whole;
        else
            entity->posX.whole += -0xC0000 + owner->posX.whole;
        entity->posY.whole += -0xC0000 + owner->posY.whole;
        entity->state++;
    case 3:
        if (!entity->flipX)
        {
            entity->velX.whole = -entity->velX.whole;
            entity->accelX.whole = -entity->accelX.whole;
        }
        PlaySong(0x89);
        entity->state++;
    case 4:
        entity->velX.whole += entity->accelX.whole;
        entity->posX.whole += entity->velX.whole;
        break;
    case 5:
        if (entity->subState == 0)
        {
            entity->accelX.whole = 0;
            entity->velX.whole = 0;
            entity->velY.whole = 0x8000;
            entity->accelY.whole = 0x1000;
            entity->subState++;
            entity->stage = 0;
        }
        entity->velY.whole += entity->accelY.whole;
        entity->posY.whole += entity->velY.whole;
        break;
    case 6:
        if (entity->subState == 0)
        {
            entity->accelY.whole = 0;
            entity->accelX.whole = 0;
            entity->velY.whole = 0;
            entity->timer = 5;
            entity->subState++;
            entity->stage = 0;
        }
        if (--entity->timer == 0xFF)
            entity->deleteRequested = 1;
        break;
    }
    Hit_CheckWithPlayer(buf, entity);
    if (!Entity_IsInRect(entity, -0x80, -0x40, 0x1F0, 0x120))
        entity->deleteRequested = 1;
}
