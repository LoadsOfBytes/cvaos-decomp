/* Hit callback that knocks the entity away. */
#include "code/code_08039340.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern u16 Player_CalculateAttackPower(u16);
extern s8 Entity_GetTargetDirection(struct EwramData_EntityData *);

/**
 * @brief Clear health and affine mode; fall or reverse vertical velocity and bounce sideways according to projectile variant.
 */
void SkeletonProjectile_OnHit(struct EwramData_EntityData *entity, s32 hit, u16 *data)
{
    s32 speed;

    Player_CalculateAttackPower(data[2]);
    entity->unk_518.part16.hp = 0;
    entity->affine = 0;
    entity->unk_510.part8.hitFlashTimer = 8;
    if ((u8)entity->ext.skeletonBone.kind == 0)
    {
        entity->timer = 0x32;
        entity->velX.whole = 0;
        entity->accelX.whole = 0;
        entity->velY.whole = 0x4000;
        entity->accelY.whole = 0x2000;
        PlaySong(0x151);
    }
    else
    {
        if (Entity_GetTargetDirection(entity) > 0)
        {
            speed = entity->velX.whole;
            if (speed < 0)
                speed = -speed;
            entity->velX.whole = -(speed * 12);
        }
        else
        {
            speed = entity->velX.whole;
            if (speed < 0)
                speed = -speed;
            entity->velX.whole = speed * 12;
        }
        entity->velY.whole = -entity->velY.whole;
        PlaySong(0x15D);
    }
    entity->palette = entity->unk_510.part8.basePalette;
    entity->state = 3;
}
