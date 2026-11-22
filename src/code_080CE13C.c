/* Alastor movement/rotation attack driven by timed segments. */
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s16 sAlastorAttackSegments[][4];
#define INDEX(e) ((e)->unk_4F4.part8.unk_4F7)
#define ANGLE(e) ((e)->unk_500.part16.unk_500)
#define STEP(e) ((e)->unk_500.part16.unk_502)
/**
 * @brief Attack state (4): execute table-defined position, rotation and
 * duration segments, stop at the sentinel and recover to approach.
 */
void Alastor_UpdatePatternAttack(struct EwramData_EntityData *entity)
{
    s16 (*segments)[4]=sAlastorAttackSegments;
    s16 *segment=segments[INDEX(entity)];
    switch(entity->subState)
    {
        case 0:
            INDEX(entity)=0;
            segment=segments[0];
            entity->subState++;
            PlaySong(0x86);
        case 1:
        {
            s32 timer;
            s32 x;
            s32 facing;
            timer=(entity->timer=segment[3]);
            x=segment[0];
            facing=(s16)(entity->unk_4FC.part16.unk_4FC & 2);
            if(facing)
                x=-x;
            x<<=16;
            entity->velX.whole=x/timer;
            entity->velY.whole=(segment[1]<<16)/timer;
            STEP(entity)=segment[2]/timer;
            if(!facing)
                STEP(entity)=-STEP(entity);
            entity->subState++;
        }
        case 2:
        {
            s32 zero;
            u16 step;
            entity->posX.whole+=entity->velX.whole;
            entity->posY.whole+=entity->velY.whole;
            ANGLE(entity)+=STEP(entity);
            ObjAffine_Set(ANGLE(entity),0x10000,0x10000,entity->flipX,(zero=0),entity->affineIndex);
            if(--entity->timer==0xFF)
            {
                INDEX(entity)++;
                if(segments[INDEX(entity)][0]==0x7FFF)
                    entity->subState=5;
                else
                    entity->subState=1;
                entity->timer=0;
            }
            break;
        }
        case 5:
            if(entity->timer++>40)
            {
                entity->timer=0;
                entity->state=1;
                entity->stage=0;
                entity->subState=0;
                INDEX(entity)=0;
            }
            break;
    }
}

