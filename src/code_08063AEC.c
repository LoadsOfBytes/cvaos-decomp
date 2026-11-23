/* Scatter twenty expanding orbit particles around a saved origin. */
#include "code_08009A0.h"
#include "event_scene30.h"
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
            ENTITY_PARENT_INDEX(effect)=ENTITY_INDEX(owner);
            effect->ext.event30Ring.index=index;
            effect->ext.event30Ring.centerX=x;
            effect->ext.event30Ring.centerY=y;
            radius=point->radiusX;
            radiusX=&effect->ext.event30Ring.radiusX;
            *radiusX=radius;
            radius=point->radiusY>>1;
            radiusY=&effect->ext.event30Ring.radiusY;
            *radiusY=radius;
            random=(RandomNumberGenerator()&0xFFF)+640;
            effect->ext.event30Ring.angle=point->angle-random;
            EVENT30_EFFECT_ANGLE(effect)=0;
            effect->ext.event30Ring.size=1;
            effect->posX.part16.integer=effect->ext.event30Ring.centerX;
            effect->posY.part16.integer=effect->ext.event30Ring.centerY;
            effect->posX.part16.integer+=(Sin((u16)effect->ext.event30Ring.angle+0x4000)* *radiusX)>>13;
            effect->posY.part16.integer+=(Sin((u16)effect->ext.event30Ring.angle)* *radiusY)>>13;
            effect->ext.event30Ring.spread=0;
            effect->velX.whole=Sin((u16)effect->ext.event30Ring.angle+0x4000)>>8;
            effect->velY.whole=Sin((u16)effect->ext.event30Ring.angle)>>8;
            effect->ext.event30Effect.scale=0x10000;
            effect->ext.event30Ring.size=point->scale*2;
            effect->timer=index-128-index/5;
            effect->spriteLayer=17;
            effect->animFrame=RandomNumberGenerator()%3+10;
        }
        point++;index++;
    }while(index<=19);
}
