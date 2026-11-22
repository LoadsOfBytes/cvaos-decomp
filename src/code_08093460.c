#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 Entity_AdjustSpeedForSlope(struct EwramData_EntityData *entity, s32 speed);
extern void Entity_MoveWithCollision(struct EwramData_EntityData *entity, s32 landingVelocity, s32 ceilingOffset, s32 speedLimit, s32 wallOffset);

/**
 * @brief 93460 | Enemy attack sub-state (Zombie / Medusa family, file
 * 0x08093218): for 8 frames it lunges at 0x10000 / frame in its facing
 * (Entity_AdjustSpeedForSlope), then stops; moves via Entity_MoveWithCollision; at the end of the
 * animation it returns to state 0 walking at 0x8000; otherwise song 0x85
 * on frames 4, 7, 10 and 13.
 */
void Curly_UpdateAttack(struct EwramData_EntityData *entity)
{
    if (entity->subState != 0)
        return;
    if (entity->timer <= 7)
    {
        entity->timer++;
        if (entity->flipX)
            entity->velX.whole = Entity_AdjustSpeedForSlope(entity, 0x10000);
        else
            entity->velX.whole = Entity_AdjustSpeedForSlope(entity, -0x10000);
        entity->velY.whole = 0x10000;
    }
    else
        entity->velX.whole = 0;
    entity->accelY.whole = 0x2800;
    Entity_MoveWithCollision(entity, 0x10000, -0x200000, 0x80000, 8);
    if (entity->animDone)
    {
        Anim_StartIndexed(entity, 0, 0, 1);
        entity->state = 0;
        entity->subState = 0;
        if (entity->flipX)
            entity->velX.whole = Entity_AdjustSpeedForSlope(entity, 0x8000);
        else
            entity->velX.whole = Entity_AdjustSpeedForSlope(entity, -0x8000);
        entity->velY.whole = 0x10000;
        entity->accelY.whole = 0x2800;
        return;
    }
    if (entity->animTimer == 0
        && (entity->animStep == 4 || entity->animStep == 7 || entity->animStep == 10 || entity->animStep == 13))
        PlaySong(0x85);
}
