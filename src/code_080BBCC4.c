#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 Entity_Walk(struct EwramData_EntityData *, s32, void *, s32);
extern s32 Target_IsNearEntity(struct EwramData_EntityData *, s32, s32);
extern s32 Enemy_IsDamaged(struct EwramData_EntityData *);
extern u8 sCockatriceFamilyWalkProfile[];

static inline s32 FrameOffset(s32 frame)
{
    return (frame << 14) + 0x1000;
}

/* Patrol: turn at walls or after animation cycles, then check for attack. */
/**
 * @brief Patrol state (0): walk at animation-dependent speed, turn at walls
 * or after animation cycles, and activate attack state 1 near the target
 * or after damage.
 */
void CockatriceFamily_UpdatePatrol(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        entity->subState = 1;
    case 1:
    {
        u8 frame = (entity->animStep + 1) % 8;
        s32 offset = FrameOffset(frame);
        if (frame > 4) offset = 0;
        if ((Entity_Walk(entity, offset, sCockatriceFamilyWalkProfile, 20) & 0x1B) != 8)
        {
            if (entity->velX.part16.integer != 0)
                entity->flipX ^= 1;
        }
        else if (*(u16 *)&entity->animStep == 0)
        {
            if (entity->stage++ > 2)
            {
                entity->flipX ^= 1;
                entity->stage = 0;
            }
        }
        if (Target_IsNearEntity(entity, 240, 100) || Enemy_IsDamaged(entity))
        {
            entity->state = 1;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    }
    }
}
