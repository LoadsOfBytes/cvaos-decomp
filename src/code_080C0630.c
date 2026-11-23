/* Set up a Slime surface turn from the masked angle distance.
 * The explicit register lifetimes reproduce agbcc allocation; a match does
 * not establish the original source spelling. Keep the signed rounding
 * below: it implements division by 16 with truncation toward zero. */
#include "code_08001194.h"
extern const u16 sSlimeSurfaceTurnTargets[];
extern s32 __divsi3(s32, s32);

void Slime_BeginSurfaceTurn(struct EwramData_EntityData *entity, u8 surface, s32 bias, u16 step)
{
    u16 angle = entity->ext.slime.angle;
    register s32 mask asm("r3") = 0x7FFF; /* FAKEMATCH: pinned register */
    u16 target = sSlimeSurfaceTurnTargets[surface];
    s32 previous = entity->ext.slime.speed;
    s32 velocity;
    register s32 current asm("r1"); /* FAKEMATCH: pinned register */

    entity->ext.slime.turnSurface = surface;
    current = mask;
    current &= angle;
    mask &= target;
    if (current > mask) entity->ext.slime.speed = __divsi3(current - mask, 4);
    else entity->ext.slime.speed = __divsi3(mask - current, 4);
    if (step == 0)
    {
        register s32 loaded asm("r0") = entity->ext.slime.speed; /* FAKEMATCH: pinned register */
        velocity = loaded;
        if (velocity < 0) loaded += 15;
        entity->ext.slime.turnRate = loaded >> 4;
    }
    else
    {
        entity->ext.slime.turnRate = step;
        velocity = entity->ext.slime.speed;
    }
    entity->ext.slime.speed = velocity + bias;
    if (previous < 0) entity->ext.slime.speed = -entity->ext.slime.speed;
    entity->subState = 1;
}
