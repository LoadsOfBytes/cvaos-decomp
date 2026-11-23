/* Enemy walker update (Manticore_UpdateWalk): approach, turn and attack decisions. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Target_IsInRect(s16, s16, s16, s16);
extern s32 Enemy_IsDamaged(struct EwramData_EntityData *);
extern s32 Tile_Classify(s16 x, s16 y);
extern s32 Entity_MoveWithCollision(struct EwramData_EntityData *, s32, s32, s32, s32);
extern s32 Anim_Start(struct EwramData_EntityData *entity, struct AnimScript *script, u8 mode, u8 flags);
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);
extern struct AnimScript sManticoreWalkAnimScript;

/* Walking step; one copy per animStep value so that every copy shares the 0x40
 * mask of the first flipX test (the copies are merged again by cross-jumping). */
#define STEP() do { s32 v = 0x18000; entity->velX.whole = v; if (!entity->flipX) entity->velX.whole = -v; } while (0)

/**
 * @brief Walk state machine (state 0): wake when the target is in the 240 x 80 box
 * or the Manticore is damaged, then walk at 1.5 px per frame and turn;
 * start the attack (state 1) at random when the target is in front within
 * 0x40 / 0x60 / 0x80 px.
 */
void Manticore_UpdateWalk(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg;
    s32 dx;
    s32 res;
    s32 found;
    s32 speed;
    do { /* FAKEMATCH: weights the refs of bg (allocated before the shared 0x40 mask) */
        bg = gEwramData->bgInfo + 1;
    } while (0);

    switch (entity->subState)
    {
    case 0:
        entity->flipX = Entity_IsTargetRight(entity);
        if (Target_IsInRect(entity->posX.part16.integer - 0x78, entity->posY.part16.integer - 0x28, 0xF0, 0x50) || Enemy_IsDamaged(entity))
        {
            entity->subState++;
            Anim_Start(entity, &sManticoreWalkAnimScript, 0, 1);
            entity->animStep = 9;
            entity->timer = 0x64;
            entity->velY.whole = 0x20000;
            entity->velX.whole = 0;
        }
        speed = 0x20000;
        entity->velY.whole = speed;
        Entity_MoveWithCollision(entity, speed, -0x200000, 0x80000, 0x10);
        break;
    case 1:
        if (entity->flipX)
            dx = 0x18;
        else
            dx = -0x18;
        if (Tile_Classify((s16)(bg->xPos.part16.integer + entity->posX.part16.integer + dx), (s16)(bg->yPos.part16.integer + entity->posY.part16.integer)))
        {
            entity->subState = 5;
            Anim_StartIndexed(entity, 8, 0, 0);
            entity->animDone = 0;
            entity->timer = 0;
            break;
        }
        if (entity->animTimer == 0)
        {
            if (entity->animStep == 2) STEP();
            else if (entity->animStep == 5) STEP();
            else if (entity->animStep == 8) STEP();
            else if (entity->animStep == 0xB) STEP();
        }
        res = Entity_MoveWithCollision(entity, 0x20000, -0x200000, 0x80000, 0x10);
        EntityDampVelocityX(entity, 4);
        if ((res & 4) && *(u16 *)&entity->animStep == 10)
        {
            if (entity->flipX != Entity_IsTargetRight(entity))
                goto turn;
            if (entity->flipX)
                dx = 0;
            else
                dx = -0x40;
            found = Target_IsInRect(entity->posX.part16.integer + dx, entity->posY.part16.integer - 0x3C, 0x40, 0x46);
            if (found)
            {
                if ((RandomNumberGenerator() & 0xFF) <= 0x7F)
                {
                    entity->state = 1;
                    entity->subState = 0;
                    break;
                }
            }
            else
            {
                do { /* FAKEMATCH: weights the refs of the flipX byte pointer (global-alloc priority) */
                    if (entity->flipX)
                        dx = 0;
                    else
                        dx = -0x60;
                } while (0);
                if (Target_IsInRect(entity->posX.part16.integer + dx, entity->posY.part16.integer - 0x3C, 0x60, 0x46) && (RandomNumberGenerator() & 0xFF) <= 0x3F)
                {
                    entity->state = 1;
                    do { /* FAKEMATCH: weights the refs of found (r4 before the 0x40 mask) */
                        entity->subState = found;
                    } while (0);
                    break;
                }
            }
            do { /* FAKEMATCH: weights the refs of the flipX byte pointer (global-alloc priority) */
                if (entity->flipX)
                    dx = 0;
                else
                    dx = -0x80;
            } while (0);
            if (Target_IsInRect(entity->posX.part16.integer + dx, entity->posY.part16.integer - 0x3C, 0x80, 0x46) && (RandomNumberGenerator() & 0xFF) <= 0x7F)
            {
                entity->state = 1;
                entity->subState = 10;
                break;
            }
        }
        if (EntityIsOutsideScreen(entity, 4))
        {
            entity->subState = 2;
            Anim_StartIndexed(entity, 0, 0, 1);
        }
        break;
    case 2:
        if (entity->flipX != Entity_IsTargetRight(entity))
        {
        turn:
            entity->subState = 3;
            Anim_StartIndexed(entity, 8, 0, 0);
            entity->animDone = 0;
            entity->timer = 0;
            break;
        }
        entity->subState = 1;
        Anim_Start(entity, &sManticoreWalkAnimScript, 0, 1);
        entity->animStep = 9;
        entity->timer = 0x64;
        speed = 0x20000;
        entity->velX.whole = 0;
        entity->velY.whole = speed;
        Entity_MoveWithCollision(entity, speed, -0x200000, 0x80000, 0x10);
        break;
    case 3:
        entity->timer++;
        if (entity->animDone)
        {
            entity->flipX ^= 1;
            entity->subState = 2;
            Anim_StartIndexed(entity, 0, 0, 1);
        }
        break;
    case 5:
        entity->timer++;
        if (entity->animDone)
        {
            entity->flipX ^= 1;
            entity->subState = 1;
            Anim_Start(entity, &sManticoreWalkAnimScript, 0, 1);
            entity->velY.whole = 0x20000;
            entity->velX.whole = 0;
        }
        break;
    }
}
