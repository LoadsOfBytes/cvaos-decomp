/* Nemesis_UpdatePositioning: twin of Alastor_UpdateApproach (approach a point near the player and rock);
 * deeper vertical target (+48/16), palette call in state 0; matched. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
extern s32 Entity_GetTargetDistanceX(struct EwramData_EntityData *);
extern s32 Entity_GetTargetDistanceY(struct EwramData_EntityData *);
extern void Entity_FaceTarget(struct EwramData_EntityData *);
extern u8 Angle_TurnTowards(s16 *,s32,s32);
#define DX(e) ((s16)Entity_GetTargetDistanceX(e))
#define DY(e) ((s16)Entity_GetTargetDistanceY(e))
#define ABS(x) ((x)<0?-(x):(x))
#define ANGLE(e) ((e)->unk_500.part16.unk_500)
#define PHASE(e) ((e)->unk_4FC.part16.unk_4FE[0])
#define FLAGS(e) ((e)->unk_4FC.part16.unk_4FC)
static inline s32 Both(s32 x,s32 y) { return (x&y)==1; }
/**
 * @brief Turn and approach a point near the target, rock the affine angle and enter attack selection when both axis tolerances are reached.
 */
void Nemesis_UpdatePositioning(struct EwramData_EntityData *entity)
{
    s8 reachedX;
    s32 reachedY;
    reachedX=0;
    reachedY=0;
    if (DX(entity)>0)
        FLAGS(entity)|=2;
    else
        FLAGS(entity)&=~2;
    switch(entity->subState)
    {
        case 0:
            entity->objMode=0;
            HitBox_SetAttackRaw(entity,0x1010F8F8);
            entity->velX.whole=0x8000;
            entity->velY.whole=0x8000;
            if (DX(entity)>0)
                entity->velX.whole=-entity->velX.whole;
            if (DY(entity)>0)
                entity->velY.whole=-entity->velY.whole;
            PHASE(entity)=0;
            entity->unk_500.part16.unk_502=-100;
            entity->subState=2;
            break;
        case 1:
        case 2:
            if (DX(entity)>0)
            {
                if (Angle_TurnTowards(&ANGLE(entity),0x180,0x1000))
                    entity->subState++;
            }
            else
            {
                if (Angle_TurnTowards(&ANGLE(entity),0x180,0xF000))
                    entity->subState++;
            }
            ObjAffine_Set(ANGLE(entity),0x10000,0x10000,0,0,entity->affineIndex);
            break;
        case 3:
            entity->velY.whole=0x8000;
            entity->velX.whole=0x8000;
            if (DX(entity)>0)
                entity->velX.whole=-ABS(entity->velX.whole);
            if (DY(entity)+32>0)
                entity->velY.whole=-ABS(entity->velY.whole);
            if (ABS(DX(entity))>72)
                entity->posX.whole+=entity->velX.whole;
            else
                reachedX=1;
            if (ABS(DY(entity)+48)>16)
                entity->posY.whole+=entity->velY.whole;
            else
                reachedY=1;
            {
                s32 both;
                do { both=reachedY&reachedX; } while (0);
            if (both==1 && entity->state!=2)
            {
                entity->state=2;
                entity->stage=0;
                entity->subState=0;
            }
            }
            if ((DX(entity)>=0 && entity->velX.whole>0) || (DX(entity)<=0 && entity->velX.whole<0))
            {
                entity->subState=0;
                break;
            }
            ANGLE(entity)=Sin((u16)PHASE(entity))/32;
            if (DX(entity)>0)
                ANGLE(entity)+=0x1000;
            else
                ANGLE(entity)+=-0x1000;
            {
                u32 phase=(u16)PHASE(entity);
                PHASE(entity)=phase+0x200;
            }
            ObjAffine_Set(ANGLE(entity),0x10000,0x10000,0,0,entity->affineIndex);
            if ((s16)entity->unk_4F8.part16.unk_4F8!=0)
                entity->unk_4F8.part16.unk_4F8--;
            else
                entity->unk_4F8.part16.unk_4F8=RandomNumberGenerator()%180+10;
            break;
    }
}

