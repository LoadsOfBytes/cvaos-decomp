/* Set up a Slime surface turn from the masked angle distance.
 * The explicit register lifetimes reproduce agbcc allocation; a match does
 * not establish the original source spelling. Keep the signed rounding
 * below: it implements division by 16 with truncation toward zero. */
#include "code_08001194.h"
#include "slime_entity.h"
extern const u16 sSlimeSurfaceTurnTargets[];
extern s32 __divsi3(s32, s32);

void Slime_BeginSurfaceTurn(struct EwramData_EntityData *entity, u8 surface, s32 bias, u16 step)
{
    u16 angle = SLIME_ANGLE(entity);
    register s32 mask asm("r3") = 0x7FFF; /* FAKEMATCH: pinned register */
    u16 target = sSlimeSurfaceTurnTargets[surface];
    s32 previous = SLIME_SPEED(entity);
    s32 velocity;
    register s32 current asm("r1"); /* FAKEMATCH: pinned register */

    SLIME_TURN_SURFACE(entity) = surface;
    current = mask;
    current &= angle;
    mask &= target;
    if (current > mask) SLIME_SPEED(entity) = __divsi3(current - mask, 4);
    else SLIME_SPEED(entity) = __divsi3(mask - current, 4);
    if (step == 0)
    {
        register s32 loaded asm("r0") = SLIME_SPEED(entity); /* FAKEMATCH: pinned register */
        velocity = loaded;
        if (velocity < 0) loaded += 15;
        SLIME_TURN_RATE(entity) = loaded >> 4;
    }
    else
    {
        SLIME_TURN_RATE(entity) = step;
        velocity = SLIME_SPEED(entity);
    }
    SLIME_SPEED(entity) = velocity + bias;
    if (previous < 0) SLIME_SPEED(entity) = -SLIME_SPEED(entity);
    entity->subState = 1;
}
