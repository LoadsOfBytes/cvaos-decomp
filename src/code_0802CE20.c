#include "code_08009A0.h"
#include "soul_skill_entity.h"
#include "code_08001194.h"
#include "code_080D73B8.h"
#include "structs/main.h"
#include "structs/ewram.h"

/**
 * @brief Move the SkillSlimeUse projectile by its velocity and resolve floor,
 * ceiling and wall contacts using its half sizes; reflect the angle
 * ENTITY_HALF_510 and add the floor slope on floor or ceiling hits.
 */
void SkillSlimeMoveAndBounce(struct EwramData_EntityData *entity)
{
    u8 wall = 0;
    u32 slope = 0;
    u8 tile;
    u16 x, y;
    u16 c;
    s16 r;

    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
    x = GetEntityRoomXPositionInteger(entity);
    y = GetEntityRoomYPositionInteger(entity);
    if (entity->velY.whole >= 0)
    {
        r = Collision_GetPushUp((s16)x, (s16)y + entity->accelY.part16.integer);
        c = r;
        if (r != 0)
        {
            tile = gUnk_03002CB0.collisionProbeFlags;
            entity->posY.part16.integer += ({ s32 t_ = (s16)c; t_; });
            wall = 2;
        }
    }
    else
    {
        r = Collision_GetPushDown((s16)x, (s16)y - entity->accelY.part16.integer);
        c = r;
        if (r != 0)
        {
            tile = gUnk_03002CB0.collisionProbeFlags;
            entity->posY.part16.integer += ({ s32 t_ = (s16)c; t_; });
            wall = 2;
        }
    }
    y = GetEntityRoomYPositionInteger(entity);
    if (entity->velX.whole < 0)
    {
        r = Collision_GetPushRight((s16)x - entity->accelX.part16.integer, (s16)y);
        c = r;
        if (r != 0)
        {
            entity->posX.part16.integer += ({ s32 t_ = (s16)c; t_; });
            if (wall == 0)
                wall = 1;
        }
    }
    else if (entity->velX.whole > 0)
    {
        r = Collision_GetPushLeft((s16)x + entity->accelX.part16.integer, (s16)y);
        c = r;
        if (r != 0)
        {
            entity->posX.part16.integer += ({ s32 t_ = (s16)c; t_; });
            if (wall == 0)
                wall = 1;
        }
    }
    if (wall == 0)
        return;
    if (wall == 2 && (tile & 0xC0))
    {
        switch ((tile & 0xC0) >> 6)
        {
        case 1:
            slope = 0x4000;
            break;
        case 2:
            slope = 0x5556;
            break;
        case 3:
            slope = 0x63D8;
            break;
        }
        if (tile & 4)
            slope = (u32)(-(slope << 16)) >> 16;
    }
    if ((s16)slope != 0)
        PlaySong(0x16A);
    else
        PlaySong(0x16B);
    if (wall == 1)
        SKILL_SLIME_ANGLE(entity) = -0x8000 - SKILL_SLIME_ANGLE(entity);
    else
        SKILL_SLIME_ANGLE(entity) = -SKILL_SLIME_ANGLE(entity);
    SKILL_SLIME_ANGLE(entity) += ({ s32 t_ = (s16)slope; t_; });
}
