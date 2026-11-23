/* Buer accelerated charge, rotation, and sparks. */
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 Entity_GetTargetDistanceX(struct EwramData_EntityData *entity);
extern s32 Entity_Walk(struct EwramData_EntityData *entity,s32 speed,void *profile,s32 flags);
extern void Buer_RestoreXAtProbe(struct EwramData_EntityData *entity,s32 work);
extern void Buer_SpawnChargeEffect(s32 x,s32 y,s32 index,s32 facing);
extern s32 Buer_OffsetBySign(s32 value,s32 step);
extern s16 sBuerWalkProfile[];
#define SPEED(e) (e->ext.wheel.speed)
#define ANGLE(e) (e->ext.wheel.angle)
#define DIRECTION(e) (e->ext.wheel.direction)

/**
 * @brief Accelerate toward the sampled side, rotate and emit paired effects; then run a 32-tick collision-adjusted movement phase.
 */
void Buer_UpdateCharge(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *part = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    s32 work = entity->stage;
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
            work = entity->posX.whole;
            Entity_Walk(entity,SPEED(entity),sBuerWalkProfile,6);
            Buer_RestoreXAtProbe(entity,work);
            if (DIRECTION(entity))
            {
                SPEED(entity) += 0xCCC;
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
                SPEED(entity) -= 0xCCC;
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
                Buer_SpawnChargeEffect(initialX,(s16)(*y+24),0,DIRECTION(entity)==0);
                Buer_SpawnChargeEffect(*x,(s16)(*y+24),1,DIRECTION(entity)==0);
            }
            if (--entity->timer == 0xFF)
            {
                s32 direction = DIRECTION(entity);
                if (direction != 0)
                    direction = 1;
                DIRECTION(entity) = direction;
                entity->stage++;
                entity->timer = 32;
            }
            break;
        }
        case 2:
        {
            s32 collision;
            work = entity->posX.whole;
            collision = Entity_Walk(entity,SPEED(entity),sBuerWalkProfile,6);
            Buer_RestoreXAtProbe(entity,work);
            if (collision & 0x40)
                SPEED(entity) = Buer_OffsetBySign(SPEED(entity),-0x333);
            else if (collision & 0x20)
                SPEED(entity) = Buer_OffsetBySign(SPEED(entity),0x333);
            ANGLE(entity) += (s32)SPEED(entity) / 64;
            ObjAffine_Set(ANGLE(entity),0x10000,0x10000,0,(work = 0),entity->affineIndex);
            if (--entity->timer == 0xFF)
                entity->stage = work;
            break;
        }
    }
}
