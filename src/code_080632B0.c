/* Initial flash followed by three delayed, randomly varied fragments. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern u8 sEventScene30Gfx[],sEventScene30Palette[],sEventScene30SpriteSheet[];
struct BurstFrames {u32 frame[3];};
extern struct BurstFrames sEventScene30BurstFrames;
struct BurstAnimations {u32 frame[5];};
extern struct BurstAnimations sEventScene30BurstAnimations;
extern struct EwramData_EntityData *EventScene30_CreateEffectSprite(s32);
extern void EventScene30Flash_Update(struct EwramData_EntityData *);
extern void EventScene30Spark_Update(struct EwramData_EntityData *);
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
 * @brief Flash at (x, y) with a random animation and spawn two delayed sparks
 * around it.
 */
void EventScene30_SpawnSparkBurstSmall(s16 x,s16 y)
{
    struct BurstFrames frames=sEventScene30BurstFrames;
    struct BurstAnimations animations=sEventScene30BurstAnimations;
    s32 count;
    flashAt(x,y,animations.frame[RandomNumberGenerator()%5]);
    for(count=1;count>=0;count--)
    {
        struct EwramData_EntityData *effect=EventScene30_CreateEffectSprite(0);
        if(effect)
        {
            effect->updateFunc=(u32 *)EventScene30Spark_Update;
            effect->posX.part16.integer=(u16)((u16)x+4)-(RandomNumberGenerator()%8);
            effect->posY.part16.integer=(u16)y+4-(RandomNumberGenerator()%8);
            effect->velY.whole=0x14000-(RandomNumberGenerator()&0x7FFF);
            effect->accelY.whole=(RandomNumberGenerator()&0x7FF)+0x800;
            effect->velY.whole/=8;
            effect->accelY.whole/=8;
            effect->spriteLayer=3;
            effect->animFrame=frames.frame[RandomNumberGenerator()%3];
            effect->group=128;effect->hidden=1;
            effect->timer=RandomNumberGenerator()%8;
        }
    }
}
