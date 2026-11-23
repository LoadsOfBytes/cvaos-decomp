/* Select one of five emitters and add a small random pixel offset. */
#include "code_08009A0.h"
#include "graham_entity.h"
#include "structs/ewram.h"
struct GrahamSpawnTable { s32 coordinate[10]; };
extern struct GrahamSpawnTable sGrahamSpawnTable;
extern struct EwramData_EntityData *Graham_CreateSceneProjectile(s32,s32,s32);
/**
 * @brief Cycle through five table emitters, add randomized pixel offsets
 * and create one scene projectile. Select homing from the owner timer
 * modulo six; advance the emitter counter.
 */
void Graham_SpawnSceneProjectile(struct EwramData_EntityData *entity)
{
    struct GrahamSpawnTable positions=sGrahamSpawnTable;
    u16 counter=entity->ext.grahamEmitter.counter;
    u16 index=counter%5;
    u16 x=positions.coordinate[index*2];
    s32 *ys=&positions.coordinate[1];
    u16 y;
    ys+=index*2;
    y=*ys;
    entity->ext.grahamEmitter.counter=counter+1;
    if((u8)(entity->timer%6)==0)
    {
        s32 random=RandomNumberGenerator();
        s32 finalX=x+16;
        s32 finalY;
        s32 mask=31;
        finalX-=(random&mask);
        finalX=(u32)finalX<<16;
        finalX>>=16;
        random=RandomNumberGenerator();
        finalY=y+16;
        finalY-=(random&mask);
        finalY=(u32)finalY<<16;
        finalY>>=16;
        Graham_CreateSceneProjectile(finalX,finalY,1);
    }
    else
    {
        s32 random=RandomNumberGenerator();
        s32 finalX=x+16;
        s32 finalY;
        s32 mask=31;
        finalX-=(random&mask);
        finalX=(u32)finalX<<16;
        finalX>>=16;
        random=RandomNumberGenerator();
        finalY=y+16;
        finalY-=(random&mask);
        finalY=(u32)finalY<<16;
        finalY>>=16;
        Graham_CreateSceneProjectile(finalX,finalY,0);
    }
}
