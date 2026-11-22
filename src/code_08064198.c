/* Follow the parent, release, accelerate and retire the affine effect. */
#include "code_08009A0.h"
#include "event_scene30.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#define ATTACH_X(e,p) ((e)->posX.part16.integer=(*(s8 *)&EVENT30_JITTER_X((p))-48)+EVENT30_CENTER_X((p)))
#define ATTACH_Y(e,p) ((e)->posY.part16.integer=(s16)((s8)EVENT30_JITTER_Y((p))+8)+EVENT30_CENTER_Y((p)))
/**
 * @brief Halo: follow the owner, switch frame and scale with the owner radius.
 */
void EventScene30Halo_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner=&gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    s32 stage=entity->state;
    s32 scale;
    u32 radius;
    switch(stage)
    {
    case 0:
        if(EVENT30_TIMER(owner)==160)entity->animFrame=1;
        if(EVENT30_TIMER(owner)==0) { entity->timer=stage;entity->state++; }
        else entity->timer--;
        ATTACH_X(entity,owner); ATTACH_Y(entity,owner);
        radius=(u16)EVENT30_SCALE(owner);
        scale=0x100;
        scale-=radius;
        scale<<=8;
        scale+=0x10000;
        EVENT30_EFFECT_SCALE(entity)=scale;
        ObjAffine_Set(EVENT30_EFFECT_ANGLE(entity),scale,scale,0,0,entity->affineIndex);
        break;
    case 1:
        ATTACH_X(entity,owner); ATTACH_Y(entity,owner);
        if(EVENT30_EFFECT_SCALE(entity)!=0x12000)EVENT30_EFFECT_SCALE(entity)-=0x400;
        if(++entity->timer==81)
        { entity->animFrame=stage;entity->state++;entity->stage=0;entity->subState=0; }
        radius=(u16)EVENT30_SCALE(owner);
        scale=0x100;
        scale-=radius;
        scale<<=8;
        scale+=0x10000;
        EVENT30_EFFECT_SCALE(entity)=scale;
        ObjAffine_Set(EVENT30_EFFECT_ANGLE(entity),scale,scale,0,0,entity->affineIndex);
        break;
    case 2:
        entity->posX.whole+=entity->velX.whole;
        entity->posY.whole+=entity->velY.whole;
        entity->velX.whole+=entity->accelX.whole;
        entity->velY.whole+=entity->accelY.whole;
        radius=(u16)EVENT30_SCALE(owner);
        scale=0x100;
        scale-=radius;
        scale<<=8;
        scale+=0x10000;
        EVENT30_EFFECT_SCALE(entity)=scale;
        ObjAffine_Set(EVENT30_EFFECT_ANGLE(entity),scale,scale,0,0,entity->affineIndex);
        if(entity->timer==104)
        {
            entity->velX.whole=-0x40000;entity->velY.whole=0x10000;
            entity->accelX.whole=-0x10000;entity->accelY.whole=0x800;
            EVENT30_EFFECT_SPIN(entity)=0x400;
        }
        else entity->timer++;
        EVENT30_EFFECT_ANGLE(entity)+=EVENT30_EFFECT_SPIN(entity);
        if((s16)entity->posX.part16.integer< -32)
        { ObjAffine_Free(entity->affineIndex);entity->deleteRequested=1; }
        break;
    }
}
