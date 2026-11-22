/* Witch homing motion and orbiting particle. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "macros.h"
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern u8 sWitchGfx[], sWitchPalette[], sWitchSpriteSheet[];
#define ABS_MOTION(v) ((v) < 0 ? -(v) : (v))

/**
 * @brief Steer toward stored room coordinates with damping and bounded
 * speed. Return nonzero when both pre-move deltas are within 0x7FFFF.
 */
s32 WitchSpell_MoveTowardStoredPoint(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = (struct BgLayerState *)((u8 *)gEwramData + OFFSET_OF(struct EwramData, bgInfo[1]));
    s32 dx = entity->unk_500.whole - (entity->posX.whole + bg->xPos.whole);
    s32 dy = entity->unk_504.whole - (entity->posY.whole + bg->yPos.whole);
    if (dx > 0x40000) entity->accelX.whole = 0x1000;
    else if (dx < -0x40000) entity->accelX.whole = -0x1000;
    else entity->accelX.whole = 0;
    if (dy > 0x40000) entity->accelY.whole = 0x1000;
    else if (dy < -0x40000) entity->accelY.whole = -0x1000;
    else entity->accelY.whole = 0;
    EntityDampVelocityX(entity, 0x20);
    EntityDampVelocityY(entity, 0x20);
    EntityAccelerateAndMove(entity, 0x28000);
    if (ABS_MOTION(dx) <= 0x7FFFF && ABS_MOTION(dy) <= 0x7FFFF) return 1;
    return 0;
}

/**
 * @brief Animate a particle around a scrolling center with shrinking radius
 * and changing phase; remove after its countdown. Preserve signed truncation.
 */
void WitchOrbitParticle_Update(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg;
    s16 radius;
    s32 radiusValue;
    s32 value;
    s32 product;
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sWitchGfx, sWitchPalette, 0, sWitchSpriteSheet))
        {
            EntityDelete(entity);
            return;
        }
        Anim_StartIndexed(entity, 0xD, 0, 0);
        entity->spriteLayer = 3;
        entity->timer = 0x20;
        entity->unk_4F4.whole = 0x500000;
        entity->state++;
        /* fall through */
    case 1:
        bg = (struct BgLayerState *)((u8 *)gEwramData + OFFSET_OF(struct EwramData, bgInfo[1]));
        entity->accelX.whole -= bg->xVel.whole;
        entity->accelY.whole -= bg->yVel.whole;
        radiusValue = entity->unk_4F4.whole;
        if (radiusValue < 0) radiusValue = -((-radiusValue) >> 16) << 16;
        radius = (u32)radiusValue >> 16;
        value = Sin((s16)entity->unk_4F8.part16.unk_4F8 + 0x4000);
        radius = (s16)radius;
        product = radius * value;
        value = product;
        entity->posX.whole = entity->accelX.whole + value;
        value = Sin((s16)entity->unk_4F8.part16.unk_4F8);
        product = radius * value;
        value = product;
        entity->posY.whole = entity->accelY.whole + value;
        entity->velX.whole -= 0x2800;
        entity->unk_4F4.whole += entity->velX.whole;
        entity->unk_4F8.part16.unk_4F8 += entity->unk_4F8.part16.unk_4FA;
        Anim_Update(entity);
        if (entity->timer != 0)
        {
            if (entity->timer == 0x10) entity->drawSkip = 1;
            entity->timer--;
        }
        else EntityDelete(entity);
        break;
    }
}
