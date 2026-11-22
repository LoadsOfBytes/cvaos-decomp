/* Collapse the body's oscillation while shifting its angle history. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"
extern void WhiteDragon_SpawnSpark(struct EwramData_EntityData *,s32,s32,s32);
/**
 * @brief Death motion (state 2): shrink the neck path and let the head sag,
 * throwing sparks (WhiteDragon_SpawnSpark).
 */
void WhiteDragon_UpdateDeath(struct EwramData_EntityData *entity)
{
    u16 *history=(u16 *)entity->unk_504.whole;
    s32 i;
    u32 angle;
    if(entity->velY.part16.integer<48)
        entity->velY.part16.integer++;
    else if(entity->velY.part16.integer>48)
        entity->velY.part16.integer--;
    if(entity->unk_500.part16.unk_500>256)
        entity->unk_500.part16.unk_500--;
    else if(entity->unk_500.part16.unk_500<256)
        entity->unk_500.part16.unk_500+=4;
    if((u16)entity->velY.part16.fraction<0x800)
        entity->velY.part16.fraction+=32;
    entity->velX.part16.fraction+=entity->velY.part16.fraction;
    if((u16)entity->unk_500.part16.unk_502>0x8000)
    {
        if((u16)entity->unk_500.part16.unk_502<0xFEB0)
            entity->unk_500.part16.unk_502+=320;
    }
    else if((u16)entity->unk_500.part16.unk_502<0x7EB0)
        entity->unk_500.part16.unk_502+=320;
    for(i=99;i>0;i--) history[i]=history[i-1];
    history[0]=entity->velX.part16.fraction;
    if(entity->flipX)
    {
        angle=(u16)entity->velX.part16.integer;
        if(angle<0x8000) angle+=0x10000;
        if(angle<0x10000)
        {
            angle+=64;
            if(angle>0x10000) angle=0x10000;
        }
        else
        {
            angle-=64;
            if(angle<0x10000) angle=0x10000;
        }
        entity->velX.part16.integer=angle;
    }
    else
    {
        angle=(u16)entity->velX.part16.integer;
        if(angle<0x8000)
        {
            angle+=64;
            if(angle>0x8000) angle=0x8000;
        }
        else
        {
            angle-=64;
            if(angle<0x8000) angle=0x8000;
        }
        entity->velX.part16.integer=angle;
    }
    if(!(entity->timer&3))
        WhiteDragon_SpawnSpark(entity,entity->posX.whole,entity->posY.whole,entity->spriteLayer-1);
    if(!(entity->timer&15)) PlaySong(0x6E);
    if((s8)--entity->timer<=0) entity->deleteRequested=1;
}
