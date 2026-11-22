/* Allocate a directional fragment with selectable frame families. */
#include "code_08009A0.h"
#include "structs/ewram.h"
struct FragmentFrames { u32 frame[3]; };
extern struct FragmentFrames sEventScene30FragmentFrames;
extern struct EwramData_EntityData *EventScene30_CreateEffectSprite(s32);
extern void EventScene30Effect_UpdateBallistic(struct EwramData_EntityData *);
/**
 * @brief Spawn a drifting fragment (EventScene30Effect_UpdateBallistic) at (x,
 * y), mirrored when reverse is set.
 */
struct EwramData_EntityData *EventScene30_SpawnDriftFragment(s16 x,s16 y,s32 reverse,s32 alternate)
{
    struct EwramData_EntityData *effect=EventScene30_CreateEffectSprite(0);
    struct FragmentFrames frames=sEventScene30FragmentFrames;
    if(effect)
    {
        effect->updateFunc=(u32 *)EventScene30Effect_UpdateBallistic;
        effect->posX.part16.integer=x;
        effect->posY.part16.integer=y;
        effect->velX.whole=0x30000;
        effect->velY.whole=-0x8000;
        effect->accelX.whole=0x800;
        effect->accelY.whole=-0x100;
        if(reverse)
        {
            effect->velX.whole=-effect->velX.whole;
            effect->velY.whole=-effect->velY.whole;
            effect->accelX.whole=-effect->accelX.whole;
            effect->accelY.whole=-effect->accelY.whole;
        }
        effect->timer=64;
        effect->spriteLayer=3;
        if(!alternate)effect->animFrame=frames.frame[RandomNumberGenerator()%3];
        else effect->animFrame=RandomNumberGenerator()%7+2;
        effect->group=128;
    }
    return effect;
}
