/* Witch charge, guard and recovery sequence. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern struct EwramData_EntityData *Entity_CreateObject(u32 *update);
extern s32 Target_GetPixelX(struct EwramData_EntityData *entity);
extern struct EwramData_EntityData *Afterimage_Create(struct EwramData_EntityData *entity, s32 delay, s32 count, s32 flags);
extern void EntityDampVelocityX(struct EwramData_EntityData *entity, s32 factor);
extern void EntityDampVelocityY(struct EwramData_EntityData *entity, s32 factor);
extern void EntityMoveX(struct EwramData_EntityData *entity);
extern void EntityMoveY(struct EwramData_EntityData *entity);
extern void WitchCastEffect_Update(struct EwramData_EntityData *entity);
extern void Witch_SpawnMovementParticle(struct EwramData_EntityData *entity);

static inline u8 PlayerIsRight(struct EwramData_EntityData *e)
{
    s16 *x = &e->posX.part16.integer;
    if (*x >= (s16)Target_GetPixelX(e))
        return 0;
    return 1;
}

/**
 * @brief Charge state (2): set guard/cast flags, spawn the attached effect,
 * slide and move diagonally with afterimages, then clear flags and recover.
 */
void Witch_UpdateCharge(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child;

    switch (entity->subState)
    {
        case 0:
            entity->timer = 0x40;
            entity->velX.whole = 0x18000;
            entity->accelX.whole = -0x600;
            if (entity->flipX)
            {
                entity->velX.whole = -entity->velX.whole;
                entity->accelX.whole = -entity->accelX.whole;
            }
            *(u8 *)&entity->unk_4F8 |= 3;
            child = Entity_CreateObject((u32 *)WitchCastEffect_Update);
            if (child != NULL)
            {
                child->unk_50C.part8.unk_50C = entity->unk_4F4.part8.unk_4F4;
                child->scrollWithBg = 1;
                child->flipX = entity->flipX;
                child->attack = entity->attack;
            }
            entity->subState = 1;
        case 1:
            EntityAccelerateAndMoveX(entity, 0x20000);
            if (entity->timer != 0)
            {
                entity->timer--;
                break;
            }
            entity->velX.whole = -0x38000;
            entity->accelX.whole = 0x600;
            entity->velY.whole = 0x1C000;
            entity->accelY.whole = -0xA00;
            if (entity->flipX)
            {
                entity->velX.whole = -entity->velX.whole;
                entity->accelX.whole = -entity->accelX.whole;
            }
            entity->timer = 0x40;
            entity->subState = 2;
            Anim_StartIndexed(entity, 0xC, 0, 1);
            Afterimage_Create(entity, 0x50, 0xC, 1);
            Afterimage_Create(entity, 0x50, 6, 0);
            break;
        case 2:
            EntityAccelerateAndMove(entity, 0x20000);
            if (entity->timer != 0)
            {
                if (entity->timer & 1)
                    Witch_SpawnMovementParticle(entity);
                entity->timer--;
                break;
            }
            *(u8 *)&entity->unk_4F8 &= 0xFC;
            entity->timer = 0x20;
            entity->subState = 3;
            break;
        case 3:
            if (entity->stage == 0)
            {
                if (entity->flipX != PlayerIsRight(entity))
                {
                    entity->flipX ^= 1;
                    Anim_StartIndexed(entity, 1, 0, 0);
                    entity->stage = 1;
                }
            }
            EntityDampVelocityX(entity, 0x20);
            EntityDampVelocityY(entity, 0x20);
            EntityMoveX(entity);
            EntityMoveY(entity);
            if (entity->timer != 0)
            {
                entity->timer--;
                break;
            }
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->state = 0;
            entity->stage = 0;
            entity->subState = 0;
            break;
    }
}
