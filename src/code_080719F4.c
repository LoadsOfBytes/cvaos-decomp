/* Detect a nearby target, emerge and alternate movement with attacks. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Target_IsInRect(s32, s32, s32, s32);
extern s32 Entity_MoveWithCollision(struct EwramData_EntityData *, s32, s32, s32, s16);
static inline void Emerge(struct EwramData_EntityData *entity)
{
    entity->subState++;
    entity->animFlags &= ~2;
    entity->animDone = 0;
}
/**
 * @brief Walk state machine (state 0): wait under the ground until the target is
 * in the 120 x 80 box (or as configured by unk_516), emerge, then alternate
 * walking toward the target with the mud-throw attack (state 1).
 */
void Mudman_UpdateWalk(struct EwramData_EntityData *entity)
{
    s32 substate = entity->subState;
    switch (substate)
    {
    case 0:
        {
            s32 mode;
            entity->flipX = Entity_IsTargetRight(entity);
            mode = entity->unk_514.part16.spawnParam1;
            if (mode == 0)
            {
                if (Target_IsInRect((s16)(entity->posX.part16.integer - 60),
                                  (s16)(entity->posY.part16.integer - 40), 120, 80))
                    Emerge(entity);
            }
            else
            {
                s32 x;
                if (mode == 1) x = entity->posX.part16.integer + 16;
                else if (mode == 2) x = entity->posX.part16.integer - 76;
                else goto finished;
                if (Target_IsInRect((s16)x, (s16)(entity->posY.part16.integer - 40), 60, 80))
                {
                    Emerge(entity);
                    entity->unk_514.part16.spawnParam1 = substate;
                }
            }
        }
        break;
    case 1:
        if (entity->animDone)
        {
            entity->subState = 2;
            entity->animDone = 0;
            Anim_StartIndexed(entity, 2, 0, 1);
            entity->timer = 100;
            entity->velY.whole = 0x20000;
        }
        break;
    case 2:
        if ((Entity_MoveWithCollision(entity, 0x20000, -0x200000, 0x80000, 8) & 4) && entity->timer++ > 59)
        {
            entity->timer = 0;
            entity->flipX = Entity_IsTargetRight(entity);
            if ((RandomNumberGenerator() & 3) == 0)
            {
                entity->state = 1;
                entity->subState = 0;
                goto finished;
            }
            if (entity->flipX) entity->velX.whole = 0x4000;
            else entity->velX.whole = -0x4000;
        }
        if (EntityIsOutsideScreen(entity, 4))
        {
            Anim_StartIndexed(entity, 0, 0, 2);
            entity->subState = 0;
        }
        break;
    }
finished:
    return;
}
