/* Expanding orbit, parent-triggered release, drift, then affine fade-out. */
#include "code_08009A0.h"
#include "event_scene30.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 Entity_StepTowardPoint(struct EwramData_EntityData *,s32,s32,s32);
/**
 * @brief Place a ring point on its ellipse around its centre and follow the
 * owner phases.
 */
void EventScene30OrbitRing_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner=&gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    s32 stage=entity->state;
    if((u8)stage==0)
    {
        u32 angle=(u16)entity->ext.event30Ring.angle;
        s32 quarter=0x4000;
        Sin(angle+quarter);
        Sin((u16)entity->ext.event30Ring.angle);
        entity->posX.part16.integer=entity->ext.event30Ring.centerX;
        entity->posY.part16.integer=entity->ext.event30Ring.centerY;
        entity->posX.part16.integer+=(Sin((u16)entity->ext.event30Ring.angle+quarter)*entity->ext.event30Ring.radiusX)>>13;
        entity->posY.part16.integer+=(Sin((u16)entity->ext.event30Ring.angle)*entity->ext.event30Ring.radiusY)>>13;
        entity->velX.whole=Sin((u16)entity->ext.event30Ring.angle+quarter)>>1;
        entity->velY.whole=Sin((u16)entity->ext.event30Ring.angle)>>1;
        entity->posX.whole+=entity->ext.event30Ring.spread*entity->velX.whole;
        entity->posY.whole+=entity->ext.event30Ring.spread*entity->velY.whole;
        if(entity->timer--%2==0)entity->ext.event30Ring.spread++;
        entity->ext.event30Ring.angle+=32;
        if(owner->attack&2)
        {
            u32 index;
            entity->velX.whole=Sin((u16)entity->ext.event30Ring.angle+quarter)>>2;
            entity->velY.whole=Sin((u16)entity->ext.event30Ring.angle)>>2;
            entity->state++;
            index=entity->ext.event30Ring.index;
            entity->timer=(6-(index>>2))*index;
        }
    }
    else if((u8)stage==1)
    {
        if(--entity->timer==255) { entity->state=stage+1;entity->timer=0; }
        else { entity->posX.whole+=entity->velX.whole;entity->posY.whole+=entity->velY.whole; }
    }
    else
    {
        if(Entity_StepTowardPoint(entity,0x780000,0x500000,3))
        { if(entity->ext.event30Effect.scale)entity->ext.event30Effect.scale-=0x1000; }
        else
        { ObjAffine_Free(entity->affineIndex);entity->deleteRequested=1; }
    }
    ObjAffine_Set(EVENT30_EFFECT_ANGLE(entity),entity->ext.event30Effect.scale,entity->ext.event30Effect.scale,0,0,entity->affineIndex);
}
