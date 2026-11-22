/* Alastor readies its blade and selects the next table-driven attack. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void Entity_FaceTarget(struct EwramData_EntityData *);
extern u8 Angle_TurnTowards(s16 *,u16,u16);
extern s32 sUnk_08528C2C[][9];
#define SET(e) ((e)->unk_4F8.part8.unk_4FA)
#define INDEX(e) ((e)->unk_4F8.part8.unk_4FB)
#define ATTACK(e) (sUnk_08528C2C[SET(e)][INDEX(e)])
#define ANGLE(e) ((e)->unk_500.part16.unk_500)
#define STEP(e) ((e)->unk_500.part16.unk_502)
#define ABS(x) ((x)<0?-(x):(x))
/**
 * @brief Selection state (2): coordinate linked-part poses and rotate
 * toward preparatory angles, then choose states 3 through 6 from the
 * selected attack table and advance its index.
 */
void Alastor_UpdateAttackSelection(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *part=&gEwramData->entityData[entity->unk_4F4.part8.unk_4F6];
    s32 speed=0x20000;
    s32 target=0xC000;
    switch(entity->subState)
    {
        case 0:
            Entity_FaceTarget(entity);
            if(INDEX(entity)==9)
            {
                SET(entity)=RandomNumberGenerator() % 4;
                INDEX(entity)=0;
            }
            if(ATTACK(entity)==2)
            {
                entity->state=5;
                entity->stage=0;
                entity->subState=0;
                INDEX(entity)++;
                break;
            }
            if(!(entity->unk_4FC.part16.unk_4FC & 2))
            {
                speed=-speed;
                target=0x10000-target;
            }
            entity->posX.whole+=speed;
            if(Angle_TurnTowards(&ANGLE(entity),(u16)ABS(STEP(entity)<<3),(u16)target))
            {
                part->state=2;
                part->stage=0;
                part->subState=0;
                entity->subState++;
            }
            goto transform;
        case 1:
            if(part->state==0)
                entity->subState++;
            break;
        case 2:
            target=0xAC00;
            if(!(entity->unk_4FC.part16.unk_4FC & 2))
                target=0x5400;
            if(Angle_TurnTowards(&ANGLE(entity),(u16)ABS(STEP(entity)<<4),(u16)target))
            {
                if(ATTACK(entity)==0)
                    entity->subState+=2;
                else
                    entity->subState++;
            }
            goto transform;
        case 3:
            target=0x4000;
            if(!(entity->unk_4FC.part16.unk_4FC & 2))
                target=0xC000;
            if(Angle_TurnTowards(&ANGLE(entity),(u16)ABS(STEP(entity)<<5),(u16)target))
                entity->subState++;
transform:
            ObjAffine_Set(ANGLE(entity),0x10000,0x10000,entity->flipX,0,entity->affineIndex);
            break;
        case 4:
        {
            s32 state=part->state;
            if(state!=0)
                break;
            if(INDEX(entity)==9)
            {
                SET(entity)=RandomNumberGenerator() % 4;
                INDEX(entity)=state;
            }
            switch(ATTACK(entity))
            {
                case 0: entity->state=3; entity->stage=state; entity->subState=state; break;
                case 1: entity->state=4; entity->stage=state; entity->subState=state; break;
                case 2: entity->state=5; entity->stage=state; entity->subState=state; break;
                case 3: entity->state=6; entity->stage=state; entity->subState=state; break;
            }
            INDEX(entity)++;
            break;
        }
    }
}

