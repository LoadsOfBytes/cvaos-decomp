/* Scatter twenty expanding orbit particles around a saved origin. */
#include "code_08009A0.h"
#include "structs/ewram.h"
#include <string.h>
struct OrbitPoint { u16 angle; u8 radiusX,radiusY; u16 scale; u16 padding; };
extern struct OrbitPoint sEventScene30OrbitPoints[];
extern struct EwramData_EntityData *EventScene30_CreateEffectSprite(s32);
extern void EventScene30OrbitRing_Update(struct EwramData_EntityData *);
/**
 * @brief Spawn the twenty ring points of sEventScene30OrbitPoints
 * (EventScene30OrbitRing_Update) around (x, y).
 */
void EventScene30_SpawnOrbitRing(struct EwramData_EntityData *owner,u16 x,u16 y)
{
    struct OrbitPoint points[20];
    struct OrbitPoint *point;
    s32 index;
    memcpy(points,sEventScene30OrbitPoints,sizeof(points));
    index=0;point=points;
    do
    {
        struct EwramData_EntityData *effect=EventScene30_CreateEffectSprite(1);
        if(effect)
        {
            u32 random,radius;
            u8 *radiusX,*radiusY;
            Sin(point->angle+0x4000);
            Sin(point->angle);
            effect->updateFunc=(u32 *)EventScene30OrbitRing_Update;
            effect->unk_4F4.part8.unk_4F5=owner->unk_4F4.part8.unk_4F4;
            effect->unk_4F8.part8.unk_4FB=index;
            effect->unk_504.part16.unk_504=x;
            effect->unk_504.part16.unk_506[0]=y;
            radius=point->radiusX;
            radiusX=&effect->unk_50C.part8.unk_50C;
            *radiusX=radius;
            radius=point->radiusY>>1;
            radiusY=&effect->unk_50C.part8.unk_50D;
            *radiusY=radius;
            random=(RandomNumberGenerator()&0xFFF)+640;
            effect->unk_508.part16.unk_508=point->angle-random;
            effect->unk_51C=0;
            effect->unk_508.part16.unk_50A=1;
            effect->posX.part16.integer=effect->unk_504.part16.unk_504;
            effect->posY.part16.integer=effect->unk_504.part16.unk_506[0];
            effect->posX.part16.integer+=(Sin((u16)effect->unk_508.part16.unk_508+0x4000)* *radiusX)>>13;
            effect->posY.part16.integer+=(Sin((u16)effect->unk_508.part16.unk_508)* *radiusY)>>13;
            effect->unk_50C.part16.unk_50E=0;
            effect->velX.whole=Sin((u16)effect->unk_508.part16.unk_508+0x4000)>>8;
            effect->velY.whole=Sin((u16)effect->unk_508.part16.unk_508)>>8;
            effect->unk_500.whole=0x10000;
            effect->unk_508.part16.unk_50A=point->scale*2;
            effect->timer=index-128-index/5;
            effect->spriteLayer=17;
            effect->animFrame=RandomNumberGenerator()%3+10;
        }
        point++;index++;
    }while(index<=19);
}
