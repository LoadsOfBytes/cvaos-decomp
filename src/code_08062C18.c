/* Delayed fragments spiral away, alternating their direction of rotation. */
#include "code_08009A0.h"
#include "event_scene30.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern u8 sEventScene30Gfx[],sEventScene30Palette[],sEventScene30SpriteSheet[];
extern void EventScene30Flash_Update(struct EwramData_EntityData *);
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *,u8 *,void *,u8,void *);
static inline void flashAt(s16 x,s16 y)
{
    struct EwramData_EntityData *effect=EntityCreateInRange(0x1B,0x48,(u32 *)EventScene30Flash_Update);
    if(effect)
    {
        if(!Entity_LoadGraphics(effect,sEventScene30Gfx,sEventScene30Palette,0,sEventScene30SpriteSheet))EntityDelete(effect);
        else
        {
            effect->posX.part16.integer=x;effect->posY.part16.integer=y;effect->spriteLayer=0;
            Anim_StartIndexed(effect,1,0,0);Anim_Update(effect);
        }
    }
}
/**
 * @brief After its delay, flash in and move along its random angle with a
 * trail, then fade out.
 */
void EventScene30OrbitFragment_Update(struct EwramData_EntityData *entity)
{
    s32 stage=entity->state;
    switch(stage)
    {
    case 0:
        if(--entity->timer==255)
        {
            entity->timer=stage;EVENT30_EFFECT_ANGLE(entity)=0x10000;entity->state++;
            entity->stage=stage;entity->subState=stage;entity->hidden=0;
            flashAt(entity->posX.part16.integer,entity->posY.part16.integer);
        }
        break;
    case 1:
    {
        u32 angle=(u16)((u16)entity->ext.event30Fragment.phase<<11);
        entity->velX.whole=(Sin(angle+0x4000)>>8)<<10;
        entity->velY.whole=(Sin(angle)>>8)<<10;
        entity->posX.whole+=entity->velX.whole;entity->posY.whole+=entity->velY.whole;
        if(++entity->timer>32)entity->spriteLayer=17;else entity->spriteLayer=3;
        if(entity->timer%2==0)
        {
            if(entity->ext.event30Fragment.dir)
            {entity->ext.event30Fragment.phase++;EVENT30_EFFECT_ANGLE(entity)+=0x200;}
            else {entity->ext.event30Fragment.phase--;EVENT30_EFFECT_ANGLE(entity)-=0x200;}
        }
        ObjAffine_Set(EVENT30_EFFECT_ANGLE(entity),0x12000,0x12000,0,0,entity->affineIndex);
        if(entity->timer>63){ObjAffine_Free(entity->affineIndex);entity->deleteRequested=1;}
        break;
    }
    }
}
