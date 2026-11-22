/* Dryad projectile creation: random launch spread and target-distance correction. */
#include "code_08009A0.h"
#include "structs/ewram.h"

extern s32 Target_GetX(struct EwramData_EntityData *);
extern s32 Target_GetY(struct EwramData_EntityData *);
extern void DryadSeed_Update(struct EwramData_EntityData *);

/* Keep the signed offsets separate from the wrapping 16-bit positions. */
static inline s16 NegateDryadOffset(s16 value)
{
    return -value;
}

static inline s32 AddDryadOffset(u16 position, s16 offset)
{
    return offset + position;
}

static inline s32 AdjustDryadY(s16 offset)
{
    return offset - 16;
}

static inline s32 AddDryadY(u16 position, u16 offset)
{
    return position + AdjustDryadY(offset);
}

/**
 * @brief Throw a seed from parent toward the target with a random spread and
 * three quarters of its attack (DryadSeed_Update).
 */
void Dryad_ThrowSeed(struct EwramData_EntityData *parent)
{
    struct EwramData_EntityData *shot = EntityCreateInRange(0x1B, 0x48, (u32 *)DryadSeed_Update);
    if (shot != NULL)
    {
        u16 x, y;
        s32 vx, velocity;
        shot->unk_4F8.unk_4F8_entity = parent;
        shot->flipX = parent->flipX;
        /* Signed spread -10..5, stored modulo 65536 before mirroring. */
        x = (RandomNumberGenerator() & 15) + 0xFFF6;
        y = (RandomNumberGenerator() & 15) + 0xFFF6;
        if (shot->flipX)
            x = NegateDryadOffset(x);
        shot->posX.part16.integer = AddDryadOffset(parent->posX.part16.integer, x);
        shot->posY.part16.integer = AddDryadY(parent->posY.part16.integer, y);
        vx = -0x800 - ((RandomNumberGenerator() % 16) << 10);
        if (Target_GetX(parent) - shot->posX.whole < 0)
            velocity = vx - ((shot->posX.whole - Target_GetX(parent)) >> 6);
        else
            velocity = vx - ((Target_GetX(parent) - shot->posX.whole) >> 6);
        shot->velX.whole = velocity;
        if (shot->flipX)
            shot->velX.whole = -velocity;
        shot->velY.whole = ((RandomNumberGenerator() % 16) << 8) - 0x20000;
        shot->velY.whole += (Target_GetY(parent) - parent->posY.whole) / 64;
        /* Parent contact damage minus its truncated quarter. */
        shot->attack = parent->attack - (parent->attack >> 2);
    }
}
