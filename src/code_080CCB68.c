/* Bael accelerated charge, rotation, and sparks. */
#include "code_08009A0.h"
#include "wheel_enemy_entity.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 Entity_GetTargetDistanceX(struct EwramData_EntityData *entity);
extern s32 Entity_Walk(struct EwramData_EntityData *entity,s32 speed,void *profile,s32 flags);
extern void Bael_RestoreXAtProbe(struct EwramData_EntityData *entity,s32 work);
extern void Bael_SpawnChargeEffect(s32 x,s32 y,s32 index,s32 facing);
extern s32 Bael_OffsetBySign(s32 value,s32 step);
extern s16 sBaelWalkProfile[];
#define SPEED(e) (WHEEL_SPEED((e)))
#define ANGLE(e) (WHEEL_ANGLE((e)))
#define DIRECTION(e) (WHEEL_DIRECTION((e)))

/**
 * @brief Accelerate and rotate toward the sampled side, emit effects and count completed charges in the linked head.
 */
void Bael_UpdateCharge(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *part = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    s32 work = entity->stage;
    s32 reset;
    switch (work)
    {
        case 0:
            if ((s16)Entity_GetTargetDistanceX(entity) > 0)
                DIRECTION(entity) = work;
            else
                DIRECTION(entity) = 1;
            entity->timer = 80;
            entity->stage++;
        case 1:
        {
            u32 timer;
            reset = entity->posX.whole;
            /* Keep the pre-movement coordinate in r4 across collision handling.
             * Reusing reset below also preserves the later zero's lifetime. */
            {
                register s32 oldX asm("r4") = reset; /* FAKEMATCH: pinned register */
                Entity_Walk(entity, SPEED(entity), sBaelWalkProfile, 6);
                Bael_RestoreXAtProbe(entity, oldX);
            }
            if (DIRECTION(entity))
            {
                SPEED(entity) += 0x1998;
                if ((s32)SPEED(entity) > 0x20000)
                {
                    SPEED(entity) = 0x20000;
                    entity->timer = 0;
                }
                if ((s32)SPEED(entity) > 0)
                    part->flipX = 1;
                if (entity->timer > 50)
                    ANGLE(entity) += 0x400;
                else
                    ANGLE(entity) += 0x800;
            }
            else
            {
                SPEED(entity) -= 0x1998;
                if ((s32)SPEED(entity) < -0x20000)
                {
                    SPEED(entity) = -0x20000;
                    entity->timer = 0;
                }
                if ((s32)SPEED(entity) < 0)
                    part->flipX = 0;
                if (entity->timer > 50)
                    ANGLE(entity) -= 0x400;
                else
                    ANGLE(entity) -= 0x800;
            }
            ObjAffine_Set(ANGLE(entity),0x10000,0x10000,0,0,entity->affineIndex);
            timer = entity->timer;
            if ((u8)(timer % 3) == 0 && timer > 50)
            {
                s16 *x;
                u16 *y;
                s32 initialX;
                if ((u8)(timer % 9) == 0)
                    PlaySong(0x84);
                x = (s16 *)&entity->posX.part16.integer;
                initialX = *x;
                y = &entity->posY.part16.integer;
                Bael_SpawnChargeEffect(initialX,(s16)(*y+24),0,DIRECTION(entity)==0);
                Bael_SpawnChargeEffect(*x,(s16)(*y+24),1,DIRECTION(entity)==0);
            }
            if (--entity->timer == 0xFF)
            {
                s32 direction = DIRECTION(entity);
                if (direction != 0)
                    direction = 1;
                DIRECTION(entity) = direction;
                /* Initialize the reset value before the state store: agbcc must
                 * retain this ordering to reproduce the original ROM. */
                {
                    s32 next = entity->stage + 1;
                    reset = 0;
                    entity->stage = next;
                }
                entity->timer = 32;
                {
                    s32 count = (s8)part->unk_4FC.part8.unk_4FD;
                    if ((u32)count >= (u32)RandomNumberGenerator() % 5 + 1)
                    {
                        s32 zero = reset;
                        entity->subState = zero;
                        entity->stage = zero;
                        entity->accelX.whole = zero;
                        entity->velX.whole = zero;
                        entity->accelY.whole = zero;
                        entity->velY.whole = zero;
                        SPEED(entity) = (s32)SPEED(entity) / 2;
                        part->unk_4FC.part8.unk_4FD = zero;
                        entity->timer = zero;
                    }
                }
            }
            break;
        }
        case 2:
        {
            s32 collision;
            work = entity->posX.whole;
            collision = Entity_Walk(entity,SPEED(entity),sBaelWalkProfile,6);
            Bael_RestoreXAtProbe(entity,work);
            if (collision & 0x40)
                SPEED(entity) = Bael_OffsetBySign(SPEED(entity),-0x666);
            else if (collision & 0x20)
                SPEED(entity) = Bael_OffsetBySign(SPEED(entity),0x666);
            ANGLE(entity) += (s32)SPEED(entity) / 64;
            ObjAffine_Set(ANGLE(entity),0x10000,0x10000,0,(work = 0),entity->affineIndex);
            if (--entity->timer == 0xFF)
            {
                entity->stage = work;
                part->unk_4FC.part8.unk_4FD++;
            }
            break;
        }
    }
}
