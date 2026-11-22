/* Follow the parent, release, accelerate and retire the affine effect. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#define ATTACH_X(e,p) ((e)->posX.part16.integer=(*(s8 *)&(p)->unk_508.part16.unk_50A-48)+(p)->unk_504.part16.unk_504)
#define ATTACH_Y(e,p) ((e)->posY.part16.integer=(s16)((s8)(p)->unk_508.part8.unk_50B+8)+(p)->unk_504.part16.unk_506[0])
/**
 * @brief Halo: follow the owner, switch frame and scale with the owner radius.
 */
void EventScene30Halo_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner=&gEwramData->entityData[entity->unk_4F4.part8.unk_4F5];
    s32 stage=entity->state;
    s32 scale;
    u32 radius;
    switch(stage)
    {
    case 0:
        if(owner->unk_508.part16.unk_508==160)entity->animFrame=1;
        if(owner->unk_508.part16.unk_508==0) { entity->timer=stage;entity->state++; }
        else entity->timer--;
        ATTACH_X(entity,owner); ATTACH_Y(entity,owner);
        radius=(u16)owner->unk_500.part16.unk_500;
        scale=0x100;
        scale-=radius;
        scale<<=8;
        scale+=0x10000;
        entity->unk_500.whole=scale;
        ObjAffine_Set(entity->unk_51C,scale,scale,0,0,entity->affineIndex);
        break;
    case 1:
        ATTACH_X(entity,owner); ATTACH_Y(entity,owner);
        if(entity->unk_500.whole!=0x12000)entity->unk_500.whole-=0x400;
        if(++entity->timer==81)
        { entity->animFrame=stage;entity->state++;entity->stage=0;entity->subState=0; }
        radius=(u16)owner->unk_500.part16.unk_500;
        scale=0x100;
        scale-=radius;
        scale<<=8;
        scale+=0x10000;
        entity->unk_500.whole=scale;
        ObjAffine_Set(entity->unk_51C,scale,scale,0,0,entity->affineIndex);
        break;
    case 2:
        entity->posX.whole+=entity->velX.whole;
        entity->posY.whole+=entity->velY.whole;
        entity->velX.whole+=entity->accelX.whole;
        entity->velY.whole+=entity->accelY.whole;
        radius=(u16)owner->unk_500.part16.unk_500;
        scale=0x100;
        scale-=radius;
        scale<<=8;
        scale+=0x10000;
        entity->unk_500.whole=scale;
        ObjAffine_Set(entity->unk_51C,scale,scale,0,0,entity->affineIndex);
        if(entity->timer==104)
        {
            entity->velX.whole=-0x40000;entity->velY.whole=0x10000;
            entity->accelX.whole=-0x10000;entity->accelY.whole=0x800;
            entity->unk_508.part16.unk_50A=0x400;
        }
        else entity->timer++;
        entity->unk_51C+=entity->unk_508.part16.unk_50A;
        if((s16)entity->posX.part16.integer< -32)
        { ObjAffine_Free(entity->affineIndex);entity->deleteRequested=1; }
        break;
    }
}
