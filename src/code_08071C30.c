/* Hit reaction: emit two pieces in the facing direction and recover on landing. */
#include "code_08009A0.h"
#include "structs/ewram.h"
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern void Mudman_SpawnPiece(struct EwramData_EntityData *, s32, s32, s32, s32);
extern s32 Entity_MoveWithCollision(struct EwramData_EntityData *, s32, s32, s32, s16);
extern s32 Anim_StartIndexed(struct EwramData_EntityData *, u16, u8, u8);

/* The two velocity locals, offset registers and coordinate-read order preserve
 * agbcc's original allocation. They do not establish the original source form.
 * Keep the first mask expressed as an unsigned remainder: it selects a different
 * destination from the subsequent random mask. */
static inline void EmitLeft(struct EwramData_EntityData *entity)
{
    s32 vx = RandomNumberGenerator() % 0x10000;
    s32 base = -0x1000;
    vx = base - vx;
    {
        s32 vy = -0x10000 - (RandomNumberGenerator() & 0x7FFF);
        register s32 xOffset asm("r9"); /* FAKEMATCH: pinned register */
        register s32 yOffset asm("r8"); /* FAKEMATCH: pinned register */
        s32 x, y;
        x = entity->posX.whole;
        xOffset = -0x60000;
        x += xOffset;
        y = entity->posY.whole;
        yOffset = -0x120000;
        y += yOffset;
        Mudman_SpawnPiece(entity, x, y, vx, vy);
        {
            s32 vx2 = base - (RandomNumberGenerator() & 0x7FFF);
            s32 vy2 = -0x18000 - (RandomNumberGenerator() & 0x7FFF);
            Mudman_SpawnPiece(entity, entity->posX.whole + xOffset, entity->posY.whole + yOffset, vx2, vy2);
        }
    }
}

static inline void EmitRight(struct EwramData_EntityData *entity)
{
    s32 vx = RandomNumberGenerator() % 0x10000;
    s16 base = 0x1000;
    vx += base;
    {
        s32 vy = -0x10000 - (RandomNumberGenerator() & 0x7FFF);
        register s32 xOffset asm("r8"); /* FAKEMATCH: pinned register */
        register s32 yOffset asm("r6"); /* FAKEMATCH: pinned register */
        s32 x, y;
        x = entity->posX.whole;
        xOffset = 0x60000;
        x += xOffset;
        y = entity->posY.whole;
        yOffset = -0x120000;
        y += yOffset;
        Mudman_SpawnPiece(entity, x, y, vx, vy);
        {
            s32 vx2 = RandomNumberGenerator() & 0x7FFF;
            vx2 += base;
            {
                s32 vy2 = -0x18000 - (RandomNumberGenerator() & 0x7FFF);
                Mudman_SpawnPiece(entity, entity->posX.whole + xOffset, entity->posY.whole + yOffset, vx2, vy2);
            }
        }
    }
}

/**
 * @brief Hurt state machine (state 3): face the target, hop with two mud pieces
 * thrown backwards and return to state 0 on landing.
 */
void Mudman_UpdateHurt(struct EwramData_EntityData *entity)
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
            if (entity->flipX)
                EmitLeft(entity);
            else
                EmitRight(entity);
            /* Fall through. */
        case 1:
            if ((Entity_MoveWithCollision(entity, 0x40000, -0x200000, 0x80000, 8) & 4) && entity->animDone)
            {
                entity->state = 0;
                entity->subState = 0;
                Anim_StartIndexed(entity, 0, 0, 2);
            }
            break;
    }
}
