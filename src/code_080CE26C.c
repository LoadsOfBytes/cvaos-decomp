/* Alastor wind-up and aimed charge. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 Target_GetX(struct EwramData_EntityData *);
extern s32 Target_GetY(struct EwramData_EntityData *);
extern s32 GetEntityRoomXPositionWhole(struct EwramData_EntityData *);
extern s32 GetEntityRoomYPositionWhole(struct EwramData_EntityData *);
extern s32 Entity_GetTargetAngleAbove16(struct EwramData_EntityData *);
#define ANGLE(e) (e->ext.alastor.angle)
#define STEP(e) (e->ext.alastor.turn)
#define ABS(x) ((x)<0?-(x):(x))
/**
 * @brief Attack state (5): spin during wind-up, aim velocity toward the
 * target, travel for the computed duration and return to approach.
 * Keep existing target reads, division and angle arithmetic.
 */
void Alastor_UpdateAimedCharge(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg=&gEwramData->bgInfo[1];
    s32 targetX=Target_GetX(entity)+bg->xPos.whole;
    s32 targetY=Target_GetY(entity)+bg->yPos.whole-0x200000;
    (void)ABS(targetX-GetEntityRoomXPositionWhole(entity));
    (void)ABS(targetY-GetEntityRoomYPositionWhole(entity));
    switch(entity->subState)
    {
        case 0:
            entity->accelY.whole=0;
            entity->accelX.whole=0;
            entity->velY.whole=0;
            entity->velX.whole=0;
            STEP(entity)=0x1000;
            if(!entity->flipX)
                STEP(entity)=-STEP(entity);
            entity->timer=0;
            entity->subState++;
        case 1:
            if(((u32)entity->timer % 16)==0)
                PlaySong(0x85);
            if(entity->timer++>64)
            {
                STEP(entity)=0;
                ANGLE(entity)=Entity_GetTargetAngleAbove16(entity)+0x4000;
                entity->velX.whole=FixedMultiply(Sin(ANGLE(entity)+0x10000),0x20000);
                entity->velY.whole=FixedMultiply(Sin(ANGLE(entity)+0xC000),0x20000);
                entity->timer=ABS((Target_GetX(entity)-entity->posX.whole)/entity->velX.whole);
                entity->subState++;
                PlaySong(0x86);
            }
            break;
        case 2:
            if(--entity->timer==0)
            {
                entity->subState=3;
                entity->timer=0;
            }
            break;
        case 3:
        case 4:
            entity->state=1;
            entity->stage=0;
            entity->subState=0;
            break;
    }
    entity->posX.whole+=entity->velX.whole;
    entity->posY.whole+=entity->velY.whole;
    entity->velX.whole+=entity->accelX.whole;
    entity->velY.whole+=entity->accelY.whole;
    ANGLE(entity)+=STEP(entity);
    ObjAffine_Set(ANGLE(entity),0x10000,0x10000,entity->flipX,0,entity->affineIndex);
}

