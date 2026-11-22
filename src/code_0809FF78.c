/* Kyoma Demon helpers 0x0809FF78..0x0809FFEB (Legion_SteerX / Legion_SteerY
 * before them read an s32 at offset 0x4F0 of the parent and stay asm). */
#include "code_08009A0.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern void EntityMove(struct EwramData_EntityData *entity);
extern void Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);

/**
 * @brief 9FF78 | Which of the lines y = 0x190, y = 0x70, x = 0x70,
 * x = 0x190 (16.16) lies within pos +- size: 0, 2, 1, 3, otherwise -1.
 */
s32 Legion_GetPathEdge(s32 x, s32 y, s32 size)
{
    if (y + size >= 0x1900000 && y - size <= 0x1900000)
        return 0;
    if (y + size >= 0x700000 && y - size <= 0x700000)
        return 2;
    if (x + size >= 0x700000 && x - size <= 0x700000)
        return 1;
    if (x + size >= 0x1900000 && x - size <= 0x1900000)
        return 3;
    return -1;
}

/**
 * @brief 9FFEC | Move with a hitbox; delete when off screen or after
 * 0xF0 frames.
 */
void LegionShot_Update(struct EwramData_EntityData *entity)
{
    u8 collision[0x10];

    EntityMove(entity);
    Hit_CheckWithPlayer(collision, entity);
    if (EntityIsOutsideScreen(entity, 1))
        EntityDelete(entity);
    else if (entity->timer++ >= 0xF0)
        EntityDelete(entity);
}
