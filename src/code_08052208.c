/* Move an entity, settle it on the ground and turn it toward the slope ahead. */
#include "code_08009A0.h"
#include "room_object_entity.h"
#include "code_08001194.h"
#include "code/code_080211F0.h"
#include "syscalls.h"
#include "structs/ewram.h"

extern u8 Angle_TurnTowards(u16 *, u16, u16);

/**
 * @brief Variant-1 terrain movement: move with gravity, snap onto the floor and
 * follow the slope.
 */
s16 Object14_MoveOnTerrainSolid(struct EwramData_EntityData *entity, s32 layer)
{
    u16 fall = entity->velY.part16.integer;
    u16 x;
    u16 y;
    s16 ground;
    u16 dir;
    u16 off;
    s32 hit;
    s32 i;

    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
    entity->velY.whole += entity->accelY.whole;
    Player_QueueEntity(entity, 7);
    x = GetEntityRoomXPositionInteger(entity);
    y = GetEntityRoomYPositionInteger(entity) + 8;
    if (entity->velY.whole == 0)
        fall = 8;
    ground = Collision_GetPushUpFiltered((s16)x, (s16)y + (s16)fall, layer);
    if ((s16)ground != 0)
    {
        entity->posY.whole += (s16)ground << 16;
        if (entity->velY.whole == 0)
            entity->posY.whole += (s16)fall << 16;
    }
    dir = 8;
    if (entity->velX.whole < 0)
        dir *= -1;
    y = GetEntityRoomYPositionInteger(entity) + 8;
    for (off = 0, i = 0; i <= 1; i++)
    {
        s16 so = (s16)off;
        hit = (u16)Collision_GetPushUpFiltered((s16)x + (s16)dir, (s16)y + so, layer);
        if ((s16)hit != 0)
        {
            hit = ((u32)(((s16)hit + so) << 16)) >> 16;
            break;
        }
        asm("" :: "r"(off)); /* FAKEMATCH: keeps off live across the call (callee-saved reg) */
        off = so + 8;
    }
    if (hit << 16)
    {
        u32 h = (u32)(hit << 16) >> 16;
        asm("" :: "r"(hit)); /* FAKEMATCH: hit overlaps h, so it avoids r1 */
        if (entity->velY.whole == 0)
            h = ((h << 16) + 0xFFF80000) >> 16; 
        {
             s32 a = ArcTan2(dir, h);
            Angle_TurnTowards((u16 *)&entity->ext.object14.angle, 0x400, a);
        }
    }
    return ground;
}
