/* Werewolf ground movement and transition into an attack. */
#include "code/code_08039340.h"
#include "code/code_08060B98.h"

extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Entity_MoveWithCollision(struct EwramData_EntityData *, s32, s32, s32, s32);
extern s32 Target_IsInRect(s32, s32, s32, s32);
extern s32 Enemy_IsDamaged(struct EwramData_EntityData *);

/**
 * @brief Walk state machine (state 0): land, face the target and wait until it is
 * in the 160 x 160 box, then approach and start the attack (state 1).
 */
void Werewolf_UpdateWalk(struct EwramData_EntityData *entity)
{
    s32 flags;
    switch (entity->subState)
    {
    case 0:
        entity->flipX = Entity_IsTargetRight(entity);
        entity->velX.whole = 0;
        entity->accelX.whole = 0;
        entity->velY.whole = 0x20000;
        entity->accelY.whole = 0x2800;
        flags = Entity_MoveWithCollision(entity, 0x20000, -0x200000, 0x80000, 0xA);
        if (flags & 4)
        {
            if (Target_IsInRect((s16)(entity->posX.part16.integer - 0x50),
                            (s16)(entity->posY.part16.integer - 0x50), 0xA0, 0xA0)
                || Enemy_IsDamaged(entity))
            {
                entity->subState = 1;
                Anim_StartIndexed(entity, 1, 0, 1);
            }
        }
        break;
    case 1:
        entity->flipX = Entity_IsTargetRight(entity);
        if (!EntityIsOutsideScreen(entity, 3))
        {
            if (entity->animTimer == 0)
            {
                if (entity->flipX)
                    entity->velX.whole = 0x10000;
                else
                    entity->velX.whole = -0x10000;
                if (entity->animStep == 3)
                    entity->velX.whole *= 4;
            }
            else
                entity->velX.whole = 0;
            flags = Entity_MoveWithCollision(entity, 0x20000, -0x200000, 0x80000, 0xA);
        if (flags & 4)
            {
                if (Target_IsInRect((s16)(entity->posX.part16.integer - 0x3C),
                                (s16)(entity->posY.part16.integer - 0x28), 0x78, 0x50))
                {
                    entity->state = 1;
                    entity->subState = 0;
                    entity->velX.whole = 0;
                    entity->timer = 0;
                    Anim_StartIndexed(entity, 0, 0, 1);
                }
            }
        }
        else
        {
            entity->subState = 0;
            Anim_StartIndexed(entity, 0, 0, 1);
        }
        break;
    }
}
