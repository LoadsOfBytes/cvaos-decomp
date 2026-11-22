/* Walking motion with a delayed chance to begin the nearby-player action. */
#include "code_08009A0.h"
#include "structs/ewram.h"
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Entity_MoveWithCollision(struct EwramData_EntityData *, s32, s32, s32, s32);
extern s32 Target_IsInRect(s32, s32, s32, s32);
void Biphron_UpdateWalk(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        entity->flipX = Entity_IsTargetRight(entity);
        if (entity->flipX) entity->velX.whole = 0x4000;
        else entity->velX.whole = -0x4000;
        entity->velY.whole = 0x10000;
        entity->accelY.whole = 0x2800;
        entity->subState++;
        /* fall through */
    case 1:
        if (Entity_MoveWithCollision(entity, 0x10000, -0x200000, 0x80000, 8) & 4)
        {
            if (entity->timer++ > 59)
            {
                entity->timer = 0;
                entity->subState = 0;
                if (Target_IsInRect((s16)(entity->posX.part16.integer - 90), (s16)(entity->posY.part16.integer - 100), 180, 200))
                {
                    if ((RandomNumberGenerator() & 127) <= 47)
                    {
                        entity->state = 1;
                        entity->subState = 0;
                    }
                }
            }
        }
        break;
    }
}
