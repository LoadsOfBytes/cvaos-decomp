/* Expanding orbit, parent-triggered release, drift, then affine fade-out. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 Entity_StepTowardPoint(struct EwramData_EntityData *,s32,s32,s32);
/**
 * @brief Place a ring point on its ellipse around its centre and follow the
 * owner phases.
 */
void EventScene30OrbitRing_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner=&gEwramData->entityData[entity->unk_4F4.part8.unk_4F5];
    s32 stage=entity->state;
    if((u8)stage==0)
    {
        u32 angle=(u16)entity->unk_508.part16.unk_508;
        s32 quarter=0x4000;
        Sin(angle+quarter);
        Sin((u16)entity->unk_508.part16.unk_508);
        entity->posX.part16.integer=entity->unk_504.part16.unk_504;
        entity->posY.part16.integer=entity->unk_504.part16.unk_506[0];
        entity->posX.part16.integer+=(Sin((u16)entity->unk_508.part16.unk_508+quarter)*entity->unk_50C.part8.unk_50C)>>13;
        entity->posY.part16.integer+=(Sin((u16)entity->unk_508.part16.unk_508)*entity->unk_50C.part8.unk_50D)>>13;
        entity->velX.whole=Sin((u16)entity->unk_508.part16.unk_508+quarter)>>1;
        entity->velY.whole=Sin((u16)entity->unk_508.part16.unk_508)>>1;
        entity->posX.whole+=entity->unk_50C.part16.unk_50E*entity->velX.whole;
        entity->posY.whole+=entity->unk_50C.part16.unk_50E*entity->velY.whole;
        if(entity->timer--%2==0)entity->unk_50C.part16.unk_50E++;
        entity->unk_508.part16.unk_508+=32;
        if(owner->attack&2)
        {
            u32 index;
            entity->velX.whole=Sin((u16)entity->unk_508.part16.unk_508+quarter)>>2;
            entity->velY.whole=Sin((u16)entity->unk_508.part16.unk_508)>>2;
            entity->state++;
            index=entity->unk_4F8.part8.unk_4FB;
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
        { if(entity->unk_500.whole)entity->unk_500.whole-=0x1000; }
        else
        { ObjAffine_Free(entity->affineIndex);entity->deleteRequested=1; }
    }
    ObjAffine_Set(entity->unk_51C,entity->unk_500.whole,entity->unk_500.whole,0,0,entity->affineIndex);
}
