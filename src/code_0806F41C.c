/* Werewolf jump: face the player, launch, then land into animation 1. */
#include "code/code_08039340.h"

extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Entity_MoveWithCollision(struct EwramData_EntityData *, s32, s32, s32, s32);

/**
 * @brief Recoil state machine (state 3): hop back from the target and resume on
 * landing.
 */
void Werewolf_UpdateRecoil(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        entity->flipX = Entity_IsTargetRight(entity);
        if (entity->flipX)
            entity->velX.whole = -0x18000;
        else
            entity->velX.whole = 0x18000;
        entity->accelX.whole = 0;
        entity->velY.whole = -0x24000;
        entity->accelY.whole = 0x3000;
        entity->subState++;
        /* fall through */
    case 1:
        if (Entity_MoveWithCollision(entity, 0x40000, -0x200000, 0x80000, 0xA) & 4)
        {
            entity->velX.whole = 0;
            entity->state = 0;
            entity->subState = 1;
            Anim_StartIndexed(entity, 1, 0, 1);
        }
        break;
    }
}
