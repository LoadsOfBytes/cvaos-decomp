/* Parent-linked affine effect: attachment, release, sparks and departure. */
#include "code_08009A0.h"
#include "event_scene30.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 sEventScene30CenterEffectOffsets[21];
extern void EventScene30_SpawnSparkBurst(s16,s16);
extern void EventScene30_SpawnDriftFragment(s16,s16,s32,s32);
/**
 * @brief Central effect: follow the owner timer, spawn spark bursts at set
 * times and scale with the owner radius.
 */
void EventScene30CenterEffect_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner=&gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    s32 offsets[21];
    s32 stage;
    u32 radius;
    s32 scale;
    memcpy(offsets,sEventScene30CenterEffectOffsets,sizeof(offsets));
    stage=entity->state;
    switch(stage)
    {
    case 0:
        if(EVENT30_TIMER(owner)==263)
            EventScene30_SpawnSparkBurst(entity->posX.part16.integer,(s16)(entity->posY.part16.integer+16));
        if(EVENT30_TIMER(owner)==255)
        {
            entity->timer=stage;
            entity->state+=2;
            EVENT30_EFFECT_ANGLE(entity)=-0x800;
            entity->velX.whole=0x600;
            entity->velY.whole=stage;
            entity->accelX.whole=24;
            entity->accelY.whole=-8;
            break;
        }
        if(entity->timer<30)entity->timer++;
        entity->posX.part16.integer=(*(s8 *)&EVENT30_JITTER_X(owner)-24)+EVENT30_CENTER_X(owner);
        entity->posY.part16.integer=(EVENT30_CENTER_Y(owner)-72)+(s8)EVENT30_JITTER_Y(owner)+(u8)(entity->timer/10);
        break;
    case 1:
        if(entity->timer<20)entity->timer++;
        if(EVENT30_TIMER(owner)==160)
        {entity->timer=0;entity->state++;break;}
        { s32 dx=*(s8 *)&EVENT30_JITTER_X(owner); dx-=24; entity->posX.part16.integer=dx+EVENT30_CENTER_X(owner)+offsets[entity->timer]; }
        entity->posY.part16.integer=(EVENT30_CENTER_Y(owner)-72)+(s8)EVENT30_JITTER_Y(owner)+(u8)(entity->timer/10);
        break;
    case 2:
        entity->posX.whole+=entity->velX.whole;
        entity->posY.whole+=entity->velY.whole;
        entity->velX.whole+=entity->accelX.whole;
        entity->velY.whole+=entity->accelY.whole;
        { u32 spin=0xFFF0; EVENT30_EFFECT_SPIN(entity)=spin-1;
        EVENT30_EFFECT_ANGLE(entity)+=(s16)spin; }
        if(EVENT30_TIMER(owner)==0)
        {entity->timer=0;entity->state++;}
        else entity->timer--;
        break;
    case 3:
        if((entity->timer&1)==0 && entity->timer<40 && entity->timer>24)
        {
            s32 x=(s16)((entity->posX.part16.integer+4)-(RandomNumberGenerator()&7));
            s32 y=(s16)((entity->posY.part16.integer+4)-(RandomNumberGenerator()&7));
            EventScene30_SpawnDriftFragment(x,y,0,0);
        }
        entity->posX.whole+=entity->velX.whole;
        entity->posY.whole+=entity->velY.whole;
        entity->velX.whole+=entity->accelX.whole;
        entity->velY.whole+=entity->accelY.whole;
        if((s8)entity->timer>=0)entity->timer++;
        if(entity->timer==72)
        {
            entity->velX.whole=0x10000;
            entity->velY.whole=-0x1200;
            entity->accelX.whole=0x2000;
            entity->accelY.whole=-0x200;
        }
        EVENT30_EFFECT_ANGLE(entity)+=EVENT30_EFFECT_SPIN(entity);
        if(EVENT30_EFFECT_SPIN(entity)> -576)EVENT30_EFFECT_SPIN(entity)--;
        if((s16)entity->posX.part16.integer>271)
        {ObjAffine_Free(entity->affineIndex);entity->deleteRequested=1;}
        break;
    }
    radius=(u16)EVENT30_SCALE(owner);
    scale=0x100;
    scale-=radius;
    scale<<=8;
    scale+=0x10000;
    EVENT30_EFFECT_SCALE(entity)=scale;
    ObjAffine_Set(EVENT30_EFFECT_ANGLE(entity),scale,scale,0,0,entity->affineIndex);
}
