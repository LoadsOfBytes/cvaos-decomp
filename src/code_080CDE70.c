/* Alastor horizontal rush, braking turn, rising slash and recovery. */
#include "code_08009A0.h"
#include "alastor_entity.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 Entity_GetTargetDistanceX(struct EwramData_EntityData *);
extern s32 Alastor_IsTargetInUpperBox(struct EwramData_EntityData *,u16,u16);
extern void Afterimage_Create(struct EwramData_EntityData *,s32,s32,s32);
#define ANGLE(e) (ALASTOR_ANGLE((e)))
#define DX(e) ((s16)Entity_GetTargetDistanceX(e))
#define ABS(x) ((x)<0?-(x):(x))
static inline u16 SubtractMargin(s32 distance,s32 margin) { return distance-margin; }
static inline u16 Clamp(u16 d) { register u32 r_ asm("r1"); /* FAKEMATCH: result in r1 */ r_ = d; asm("" :: "r"(d)); /* FAKEMATCH: narrowed value in its own register */ if((s16)r_<0) r_ = 0; return r_; }
/**
 * @brief Attack state (3): rush horizontally, brake and rotate; if the
 * target is in the upper test box, reverse into a rising attack with
 * afterimages, then recover to approach.
 */
void Alastor_UpdateRushAttack(struct EwramData_EntityData *entity)
{
    switch(entity->subState)
    {
        case 0:
        {
            s32 speed;
            s32 acceleration;
            s32 margin;
            s16 distance;
            entity->velX.whole=(speed=0x40000);
            entity->accelX.whole=(acceleration=-0x4000);
            entity->velY.whole=0xA000;
            if(!entity->flipX)
            {
                entity->velX.whole=-speed;
                entity->accelX.whole=-acceleration;
            }
            entity->subState++;
            margin=RandomNumberGenerator()%64+16;
            distance=Clamp(SubtractMargin(ABS(DX(entity)),margin));
            entity->timer=ABS(distance/(s16)entity->velX.part16.integer);
            PlaySong(0x86);
            break;
        }
        case 1:
            entity->posX.whole+=entity->velX.whole;
            entity->posY.whole+=entity->velY.whole;
            ObjAffine_Set(ANGLE(entity),0x10000,0x10000,entity->flipX,0,entity->affineIndex);
            if(--entity->timer==0xFF)
                entity->subState++;
            break;
        case 2:
        {
            if(entity->flipX)
                ANGLE(entity)=(u16)ANGLE(entity)-0x150;
            else
                ANGLE(entity)=0x150+(u16)ANGLE(entity);
            ObjAffine_Set(ANGLE(entity),0x10000,0x10000,entity->flipX,0,entity->affineIndex);
            entity->posX.whole+=entity->velX.whole;
            entity->velX.whole+=entity->accelX.whole;
            if(entity->velX.whole==0)
                entity->subState++;
            break;
        }
        case 3:
            switch(entity->stage)
            {
                case 0:
                {
                    s32 hit=Alastor_IsTargetInUpperBox(entity,128,256);
                    if(hit)
                    {
                        entity->stage++;
                        entity->flipX=1-entity->flipX;
                        ObjAffine_Set(ANGLE(entity),0x10000,0x10000,entity->flipX,0,entity->affineIndex);
                        entity->velY.whole=-0x500;
                        entity->accelY.whole=-0x5000;
                        entity->timer=40;
                        Afterimage_Create(entity,96,10,2);
                        Afterimage_Create(entity,96,6,2);
                        Afterimage_Create(entity,96,3,2);
                    }
                    else
                    {
                        entity->subState=0xFF;
                        entity->stage=hit;
                    }
                }
                case 1:
                    if(entity->flipX)
                        ANGLE(entity)=(u16)ANGLE(entity)+0x80;
                    else
                        ANGLE(entity)=(u16)ANGLE(entity)-0x80;
                    ObjAffine_Set(ANGLE(entity),0x10000,0x10000,entity->flipX,0,entity->affineIndex);
                    entity->posY.whole+=entity->velY.whole;
                    if(entity->timer==15)
                        PlaySong(0x86);
                    if(entity->timer<=14)
                        entity->velY.whole+=entity->accelY.whole;
                    if(--entity->timer==0xFF)
                    {
                        entity->accelY.whole=0x4000;
                        entity->stage++;
                    }
                    break;
                case 2:
                    entity->posY.whole+=entity->velY.whole;
                    entity->velY.whole+=entity->accelY.whole;
                    if(entity->velY.whole>0)
                    {
                        entity->subState=0xFF;
                        entity->stage=0;
                    }
                    break;
            }
            break;
        case 0xFF:
            if(entity->stage==0)
            {
                entity->timer=40;
                entity->stage++;
            }
            else if(--entity->timer==0xFF)
            {
                entity->timer=0;
                entity->state=1;
                entity->stage=0;
                entity->subState=0;
            }
            break;
    }
}

