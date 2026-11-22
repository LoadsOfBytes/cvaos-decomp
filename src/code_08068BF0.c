/* Rotate an entity position around a fixed center using 8.8 products. */
#include "structs/ewram.h"
#include "code_08009A0.h"
/**
 * @brief Rotate the current position around the supplied center using
 * the original products of values shifted by 8. Preserve Y-before-X
 * assignment so both calculations use the original X coordinate.
 */
void Entity_RotateAroundPoint(struct EwramData_EntityData *entity, u16 angle, s32 centerX, s32 centerY)
{
    s32 cosine = Sin(angle + 0x4000);
    s32 x = ((entity->posX.whole - centerX) >> 8) * (cosine >> 8)
          - ((entity->posY.whole - centerY) >> 8) * (Sin(angle) >> 8) + centerX;
    s32 sine = Sin(angle);
    entity->posY.whole = ((entity->posX.whole - centerX) >> 8) * (sine >> 8)
                      + ((entity->posY.whole - centerY) >> 8) * (Sin(angle + 0x4000) >> 8) + centerY;
    entity->posX.whole = x;
}
