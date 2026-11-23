/* Worm_FollowGround: keep the entity on the floor (Collision_GetPushUp 10 px below) and
 * test the wall ahead (Collision_GetPushRight left / Collision_GetPushLeft right, by flipX and
 * unk_502/unk_503 or the controller byte 0x1F); flag 4 in controller 0x1D
 * when airborne or blocked. */
#include "code_08001194.h"
#include "structs/ewram.h"

/**
 * @brief Push the entity up out of the floor probed 10 px below its position.
 */
void Worm_FollowGround(struct EwramData_EntityData *entity)
{
    u8 *ctl = (u8 *)entity->ext.worm.root;
    u16 x = GetEntityRoomXPositionInteger(entity);
    u16 y = GetEntityRoomYPositionInteger(entity);
    s16 d = Collision_GetPushUp((s16)x, (s16)y + 10);
    u16 hit;

    if (d != 0)
        entity->posY.part16.integer += d + 2;
    else if (ctl[0x1D] == 0)
        ctl[0x1D] = 4;
    hit = 0;
    if (entity->ext.worm.segmentIndex == 0)
    {
        if (entity->ext.worm.crawlDir == 0 && !entity->flipX)
            hit = Collision_GetPushRight((s16)x - 0x18, (s16)y - 8);
        if (entity->ext.worm.crawlDir != 0 && entity->flipX)
            hit = Collision_GetPushLeft((s16)x + 0x18, (s16)y - 8);
    }
    else if (entity->ext.worm.segmentIndex == 4)
    {
        if (ctl[0x1F] != 0 && !entity->flipX)
            hit = Collision_GetPushLeft((s16)x + 0x18, (s16)y - 8);
        if (ctl[0x1F] == 0 && entity->flipX)
            hit = Collision_GetPushRight((s16)x - 0x18, (s16)y - 8);
    }
    if (hit != 0 && ctl[0x1D] == 0)
        ctl[0x1D] = 4;
}
