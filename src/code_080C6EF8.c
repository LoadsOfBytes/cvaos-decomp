#include "code_08009A0.h"
#include "code/code_08039340.h"
extern void HitBox_SetAttackRaw(struct EwramData_EntityData *, u32);
extern void HitBox_SetBodyRaw(struct EwramData_EntityData *, u32);

/**
 * @brief Derive signed X and byte Y box offsets from the current angle and install the same packed attack and body box.
 */
void Nemesis_UpdateRotatedBoxes(struct EwramData_EntityData *entity)
{
    s8 x = FixedMultiply(Sin(entity->ext.nemesis.angle + 0x8000), 0x100000) >> 16;
    u8 y = FixedMultiply(Sin(entity->ext.nemesis.angle + 0x4000), 0x80000) >> 16;
    u32 packed;
    if (x > 0) x -= 0x10;
    packed = ((u8)x | (y << 8)) + 0x08100000;
    HitBox_SetAttackRaw(entity, packed);
    HitBox_SetBodyRaw(entity, packed);
}
