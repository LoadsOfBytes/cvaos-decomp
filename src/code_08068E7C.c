/* Rotate an entity position around a fixed center using 8.8 products. */
#include "structs/ewram.h"
#include "code_08009A0.h"
#include "syscalls.h"
static inline void RotateAround(struct EwramData_EntityData *entity, u16 angle, s32 centerX, s32 centerY)
{
    s32 cosine = Sin(angle + 0x4000);
    s32 x = ((entity->posX.whole - centerX) >> 8) * (cosine >> 8)
          - ((entity->posY.whole - centerY) >> 8) * (Sin(angle) >> 8) + centerX;
    s32 sine = Sin(angle);
    entity->posY.whole = ((entity->posX.whole - centerX) >> 8) * (sine >> 8)
                      + ((entity->posY.whole - centerY) >> 8) * (Sin(angle + 0x4000) >> 8) + centerY;
    entity->posX.whole = x;
}

/* Place an entity on a sine path, optionally rotating it around its origin. */
/**
 * @brief Set X from travel divided by duration times time and Y from
 * one sine cycle and amplitude; optionally rotate about the origin.
 * Preserve integer division order and the original duration behavior.
 */
void Entity_SetSinePathPosition(struct EwramData_EntityData *entity, s32 originX, s32 originY,
                 u16 rotation, s32 amplitude, s32 travel, u16 duration, u16 time)
{
    u16 phase = (0x10000 / duration) * time;
    entity->posX.whole = originX + Div(travel, duration) * time;
    entity->posY.whole = originY + (amplitude >> 8) * (Sin(phase) >> 8);
    if (rotation)
        RotateAround(entity, rotation, originX, originY);
}

