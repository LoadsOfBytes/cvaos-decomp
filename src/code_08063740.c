/* Timed sequence of five animated effect entities. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
struct EffectStep { u16 x,y; u8 delay; u8 padding[3]; };
struct EffectSequence { struct EffectStep steps[5]; };
extern struct EffectSequence sEventScene30EmitterESequence;
extern u8 sEventScene30Gfx[],sEventScene30Palette[],sEventScene30SpriteSheet[];
extern void EventScene30Flash_Update(struct EwramData_EntityData *);
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *,u8 *,void *,u8,void *);
/**
 * @brief Emitter callback 4: step through sEventScene30EmitterESequence spawning flashes.
 */
void EventScene30EmitterE_Update(struct EwramData_EntityData *entity)
{
    struct EffectSequence sequence=sEventScene30EmitterESequence;
    s32 stage=entity->state;
    if((u8)stage==0)
    {
        entity->timer=sequence.steps[entity->stage].delay;
        entity->state=stage+1;
    }
    else if((u8)stage==1)
    {
        if(--entity->timer==255) entity->state=stage+1;
    }
    else
    {
        struct EffectStep *step=&sequence.steps[entity->stage];
        s32 x=(s16)(step->x+entity->posX.part16.integer);
        s32 y=(s16)(step->y+(u16)entity->posY.part16.integer);
        u32 animation=RandomNumberGenerator()&3;
        struct EwramData_EntityData *effect=EntityCreateInRange(0x1B,0x48,(u32 *)EventScene30Flash_Update);
        if(effect)
        {
            if(!Entity_LoadGraphics(effect,sEventScene30Gfx,sEventScene30Palette,0,sEventScene30SpriteSheet))EntityDelete(effect);
            else
            {
                effect->posX.part16.integer=x;
                effect->posY.part16.integer=y;
                effect->spriteLayer=0;
                Anim_StartIndexed(effect,animation,0,0);
                Anim_Update(effect);
            }
        }
        entity->state=0;
        entity->stage++;
    }
    if(entity->stage==5)entity->deleteRequested=1;
}

