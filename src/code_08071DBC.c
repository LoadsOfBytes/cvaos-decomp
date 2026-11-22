/* Death: scatter four pieces, wait for landing, then blink and delete. */
#include "code_08009A0.h"
#include "structs/ewram.h"
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern void Mudman_SpawnPiece(struct EwramData_EntityData *, s32, s32, s32, s32);
extern s32 Entity_MoveWithCollision(struct EwramData_EntityData *, s32, s32, s32, s16);
extern s32 Entity_GetVisibilityPattern(s32);

/**
 * @brief Death state machine (state 2): fall apart with splashing mud pieces, then
 * request deletion.
 */
void Mudman_UpdateDeath(struct EwramData_EntityData *entity)
{
    s32 substate = entity->subState;
    switch (substate)
    {
        case 0:
            entity->flipX = Entity_IsTargetRight(entity);
            entity->velX.whole = substate;
            entity->accelX.whole = substate;
            entity->velY.whole = 0x20000;
            entity->accelY.whole = 0x2800;
            entity->subState++;
            entity->timer = substate;
            {
                s32 vx = -0x1000 - (RandomNumberGenerator() & 0xFFFF);
                s32 vy = -0x10000 - (RandomNumberGenerator() & 0x7FFF);
                Mudman_SpawnPiece(entity, entity->posX.whole - 0x60000, entity->posY.whole - 0x100000, vx, vy);
            }
            {
                s32 vx = (RandomNumberGenerator() & 0xFFFF) + 0x1000;
                s32 vy = -0x10000 - (RandomNumberGenerator() & 0x7FFF);
                Mudman_SpawnPiece(entity, entity->posX.whole + 0x60000, entity->posY.whole - 0x100000, vx, vy);
            }
            {
                s32 vx = -0x1000 - (RandomNumberGenerator() & 0xFFFF);
                s32 vy = -0x18000 - (RandomNumberGenerator() & 0x7FFF);
                Mudman_SpawnPiece(entity, entity->posX.whole - 0x60000, entity->posY.whole - 0x100000, vx, vy);
            }
            {
                s32 vx = (RandomNumberGenerator() & 0xFFFF) + 0x1000;
                s32 vy = -0x18000 - (RandomNumberGenerator() & 0x7FFF);
                Mudman_SpawnPiece(entity, entity->posX.whole + 0x60000, entity->posY.whole - 0x100000, vx, vy);
            }
            /* Fall through. */
        case 1:
            if ((Entity_MoveWithCollision(entity, 0x40000, -0x200000, 0x80000, 8) & 4) && entity->animDone)
            {
                entity->subState++;
                entity->timer = 0x28;
            }
            break;
        case 2:
            entity->hidden = Entity_GetVisibilityPattern(entity->timer);
            if ((s8)--entity->timer <= 0)
                entity->deleteRequested = 1;
            break;
    }
}
