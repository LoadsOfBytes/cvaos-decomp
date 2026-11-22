/* Play fifteen timed burst positions; the state selects the burst variant. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include <string.h>
struct BurstPosition {s16 x,y;u8 delay;u8 pad[3];};
extern struct BurstPosition sEventScene30BurstPositions[15];
extern u8 sEventScene30Gfx[],sEventScene30Palette[],sEventScene30SpriteSheet[];
extern void EventScene30Flash_Update(struct EwramData_EntityData *);
extern void EventScene30_SpawnSparkBurst(s16,s16);
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *,u8 *,void *,u8,void *);
static inline void flashAt(s16 x,s16 y,s32 animation)
{
    struct EwramData_EntityData *effect=EntityCreateInRange(0x1B,0x48,(u32 *)EventScene30Flash_Update);
    if(effect)
    {
        if(!Entity_LoadGraphics(effect,sEventScene30Gfx,sEventScene30Palette,0,sEventScene30SpriteSheet))EntityDelete(effect);
        else
        {
            effect->posX.part16.integer=x;effect->posY.part16.integer=y;effect->spriteLayer=0;
            Anim_StartIndexed(effect,animation,0,0);Anim_Update(effect);
        }
    }
}
/**
 * @brief Emitter callback 0 of EventScene30_UpdateTransformation: after the
 * step delays of sEventScene30BurstPositions spawn spark bursts at the listed
 * positions.
 */
void EventScene30EmitterA_Update(struct EwramData_EntityData *entity)
{
    struct BurstPosition positions[15];
    s32 stage;
    memcpy(positions,sEventScene30BurstPositions,sizeof(positions));
    stage=entity->state;
    if((u8)stage==0)
    {
        entity->timer=positions[entity->stage].delay;
        entity->state=stage+1;
    }
    else if((u8)stage==1)
    {
        if(--entity->timer==255)entity->state=stage+1;
    }
    else
    {
        s32 parity=1;
        parity&=stage;
        if(parity)
        {
            EventScene30_SpawnSparkBurst((s16)(positions[entity->stage].x+entity->posX.part16.integer),
                (s16)(positions[entity->stage].y+entity->posY.part16.integer));
        }
        else
        {
            flashAt((s16)(positions[entity->stage].x+entity->posX.part16.integer),
                (s16)(positions[entity->stage].y+entity->posY.part16.integer),RandomNumberGenerator()%4);
        }
        entity->state=0;entity->stage++;
    }
    if(entity->stage==15)entity->deleteRequested=1;
}
