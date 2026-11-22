/* Allocate a directional fragment with selectable frame families. */
#include "code_08009A0.h"
#include "structs/ewram.h"
struct FragmentFrames { u32 frame[3]; };
extern struct FragmentFrames sEventScene30FragmentFrames;
extern struct EwramData_EntityData *EventScene30_CreateEffectSprite(s32);
extern void EventScene30Effect_UpdateBallistic(struct EwramData_EntityData *);
/**
 * @brief Unreferenced: as EventScene30_SpawnDriftFragment with a faster
 * horizontal start velocity.
 */
struct EwramData_EntityData *EventScene30_SpawnFastDriftFragment(u16 x, u16 y, s32 reverse, s32 alternate)
{
    struct EwramData_EntityData *effect = EventScene30_CreateEffectSprite(0);
    struct FragmentFrames frames = sEventScene30FragmentFrames;
    if (effect)
    {
        u32 zero;
        effect->updateFunc = (u32 *)EventScene30Effect_UpdateBallistic;
        { u16 *px = &effect->posX.part16.integer; zero = 0; *px = x; }
        effect->posY.part16.integer = y;
        effect->velX.whole = 0x60000;
        effect->velY.whole = zero;
        effect->accelX.whole = 0x800;
        effect->accelY.whole = -0x100;
        if (reverse)
        {
            effect->velX.whole = -effect->velX.whole;
            effect->accelX.whole = -effect->accelX.whole;
            effect->accelY.whole = -effect->accelY.whole;
        }
        asm("" :: "r"(zero)); /* FAKEMATCH: keeps r3 live, so reload picks r5 */
        effect->timer = 64;
        effect->spriteLayer = 3;
        if (!alternate)
            effect->animFrame = frames.frame[RandomNumberGenerator() % 3];
        else
            effect->animFrame = RandomNumberGenerator() % 7 + 2;
        effect->group = 128;
    }
    return effect;
}
