/* Falling, orbiting, delayed release, then shrinking flight. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 Entity_StepTowardPoint(struct EwramData_EntityData *,s32,s32,s32);
/**
 * @brief Fall, then bob on a sine until the owner sets flag 2, then leave.
 */
void EventScene30FloatingFragment_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner=&gEwramData->entityData[entity->unk_4F4.part8.unk_4F5];
    s32 stage=entity->state;
    s32 state=(u8)stage;
    if(state==0)
    {
        entity->posY.whole+=entity->velY.whole;
        entity->velY.whole+=entity->accelY.whole;
        if(--entity->timer==255)
        {
            entity->state=stage+1;entity->stage=state;entity->subState=state;
            entity->accelY.whole=state;entity->velY.whole=state;
        }
    }
    else if(state==1)
    {
        entity->velY.whole=Sin((u16)entity->unk_4FC.part16.unk_4FC+0x4000)/4;
        entity->unk_4FC.part16.unk_4FC+=0x400;
        entity->posY.whole+=entity->velY.whole;
        if(owner->attack&2)
        {
            entity->state++;entity->stage=0;entity->subState=0;
            entity->timer=RandomNumberGenerator()%32;
        }
    }
    else if(state==2)
    {
        if(--entity->timer==255)
        {entity->timer=18;entity->state=stage+1;entity->stage=0;entity->subState=0;}
    }
    else if(state==3)
    {
        if(entity->stage==0)entity->stage++;
        if(!Entity_StepTowardPoint(entity,0x780000,0x500000,4))
        {ObjAffine_Free(entity->affineIndex);entity->deleteRequested=1;return;}
        if(entity->timer)
        {entity->unk_500.whole-=0x1000;entity->timer--;}
    }
    if(entity->state==1)
    {
        entity->velX.whole=Sin(*(u16 *)((u8 *)entity+0x3E)+0x4000)*2;
        entity->velY.whole=Sin(*(u16 *)((u8 *)entity+0x3E))*2;
        *(u16 *)((u8 *)entity+0x3E)+=0x155;
    }
    entity->posX.whole+=entity->velX.whole;
    entity->posY.whole+=entity->velY.whole;
    entity->velY.whole+=entity->accelY.whole;
    ObjAffine_Set(entity->unk_51C,entity->unk_500.whole,entity->unk_500.whole,0,0,entity->affineIndex);
    entity->unk_51C+=0x200;
}
