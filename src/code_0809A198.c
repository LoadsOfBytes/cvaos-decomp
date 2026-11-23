#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void Effect_SpawnTimedFrameParticle(s32 x, s32 y, s32 vx, s32 vy, s32 layer, s32 damping, s32 flags, s32 palette, s32 duration);

/* x and y are function-level variables (globally allocated, as in the original);
 * the statement expression keeps -0x100000 next to the random part. */
#define EMIT()                                                                                    \
    {                                                                                             \
        x = entity->posX.whole + (RandomNumberGenerator() & 0x7FFFF) - 0x40000;                   \
        y = entity->posY.whole + ({ s32 d = (RandomNumberGenerator() & 0x7FFFF) - 0x100000; d; }); \
        Effect_SpawnTimedFrameParticle(x, y, 0, -0x2000, entity->spriteLayer - 1, 0, 0, 7, 0x18);                       \
    }

/**
 * @brief 9A198 | Randomly flip, emit a particle on animation frames 1 and
 * 0x101, then delete when the animation ends or advance it.
 */
void BiphronFlame_Update(struct EwramData_EntityData *entity)
{
    u8 unused[0x10];
    s32 one;
    u16 *frame;
    u32 mask;
    s32 x, y;

    {
        u32 r = RandomNumberGenerator();
        one = 1;
        r &= one;
        entity->flipX = r;
    }
    frame = (u16 *)&entity->animStep;
    mask = 0xFFFF;
    if (*frame == 1)
        EMIT();
    mask &= *frame;
    if (mask == 0x101)
        EMIT();
    /* animDone; cse tests it with the shared constant 1 (r9) */
    if (entity->animDone)
        EntityDelete(entity);
    else
        Anim_Update(entity);
}
