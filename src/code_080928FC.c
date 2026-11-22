#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern s32 Entity_IsTargetRight(struct EwramData_EntityData *entity);
extern s32 Target_IsInRect(s16 x, s16 y, s32 w, s32 h);
extern void Entity_MoveWithCollision(struct EwramData_EntityData *entity, s32 landingVelocity, s32 ceilingOffset, s32 speedLimit, s32 wallOffset);

/**
 * @brief 928FC | Zombie Soldier state 0 (walk): while on the ground
 * (animStep 1 / 5, animTimer clear) it faces the player and, with the player
 * in an 80x70 area in front, attacks with 1/2 chance (state 1, animation
 * 1), or with 1/4 chance when within 220x70 (state 1 with sub-state 10,
 * animation 2); otherwise walks at 0x4000 / frame. Stopped in the air
 * (animStep 0 / 4); moves via Entity_MoveWithCollision.
 */
void ZombieSoldier_UpdateWalk(struct EwramData_EntityData *entity)
{
    if (entity->subState != 0)
        return;
    if ((entity->animStep == 1 || entity->animStep == 5) && entity->animTimer == 0)
    {
        entity->flipX = Entity_IsTargetRight(entity);
        if (Target_IsInRect(entity->posX.part16.integer - 0x28, entity->posY.part16.integer - 0x40, 0x50, 0x46)
            && (RandomNumberGenerator() & 0x7F) <= 0x3F)
        {
            entity->state = 1;
            entity->subState = 0;
            Anim_StartIndexed(entity, 1, 0, 0);
            return;
        }
        if ((RandomNumberGenerator() & 0x7F) <= 0x1F
            && Target_IsInRect(entity->posX.part16.integer - 0x6E, entity->posY.part16.integer - 0x40, 0xDC, 0x46))
        {
            entity->state = 1;
            entity->subState = 10;
            Anim_StartIndexed(entity, 2, 0, 0);
            return;
        }
        if (entity->flipX)
            entity->velX.whole = 0x4000;
        else
            entity->velX.whole = -0x4000;
        entity->velY.whole = 0x10000;
        entity->accelY.whole = 0x2800;
    }
    if ((entity->animStep == 0 || entity->animStep == 4) && entity->animTimer == 0)
        entity->velX.whole = 0;
    Entity_MoveWithCollision(entity, 0x10000, -0x200000, 0x80000, 8);
}
