#include "structs/ewram.h"
/* These callers use full return registers, without 16-bit truncation. */
extern s32 GetEntityRoomXPositionInteger(struct EwramData_EntityData *entity);
extern s32 GetEntityRoomYPositionInteger(struct EwramData_EntityData *entity);
extern s32 Collision_GetPushLeft(s32 x, s32 y);
extern s32 Collision_GetPushRight(s32 x, s32 y);
extern s32 Collision_GetPushUp(s32 x, s32 y);
extern s32 Collision_GetPushDown(s32 x, s32 y);

static inline s32 RoomX(s32 pos) { struct BgLayerState *bg = &gEwramData->bgInfo[1]; return pos + (s16)bg->xPos.part16.integer; }
static inline s32 RoomY(s32 pos) { struct BgLayerState *bg = &gEwramData->bgInfo[1]; return pos + (s16)bg->yPos.part16.integer; }

/**
 * @brief Reverse velocity at room boundaries; apply directional tile corrections and stop the affected velocity component on collision.
 */
void KillerDoll_ResolveMovementBounds(struct EwramData_EntityData *entity)
{
    s32 size;
    s32 tile;
    s32 limit;
    s32 outside;
    s32 outsideY;
    s32 correction;
    struct EwramData **state;

    GetEntityRoomYPositionInteger(entity);
    state = &gEwramData;
    size = ((u8 *)(*state)->bgInfo[1].pBgMetadata)[0];
    tile = GetEntityRoomXPositionInteger(entity) / 8;
    limit = 30;
    if (size != 1)
        limit = size << 5;
    if (tile < 0 || (outside = tile >= limit))
    {
        entity->velX.whole = -entity->velX.whole;
        entity->posX.whole += entity->velX.whole * 2;
        entity->flipX = 1 - entity->flipX;
    }
    else
    {
        if (entity->velX.whole >= 0)
            correction = Collision_GetPushLeft(RoomX((s16)entity->posX.part16.integer) + 8,
                                     RoomY((s16)entity->posY.part16.integer) - 16);
        else
            correction = Collision_GetPushRight(RoomX((s16)entity->posX.part16.integer) - 8,
                                     RoomY((s16)entity->posY.part16.integer) - 16);
        if (correction != 0)
        {
            entity->posX.part16.integer += correction;
            entity->velX.whole = outside;
            entity->flipX = 1 - entity->flipX;
        }
    }

    GetEntityRoomXPositionInteger(entity);
    state = &gEwramData;
    size = ((u8 *)(*state)->bgInfo[1].pBgMetadata)[1];
    tile = GetEntityRoomYPositionInteger(entity) / 8;
    limit = 26;
    if (size != 1)
        limit = size << 5;
    if (tile < 0 || (outsideY = tile >= limit))
    {
        entity->velY.whole = -entity->velY.whole;
        entity->posY.whole += entity->velY.whole * 2;
    }
    else
    {
        if (entity->velY.whole >= 0)
            correction = Collision_GetPushUp(RoomX((s16)entity->posX.part16.integer),
                                     RoomY((s16)entity->posY.part16.integer) + 1);
        else
            correction = Collision_GetPushDown(RoomX((s16)entity->posX.part16.integer),
                                     RoomY((s16)entity->posY.part16.integer) - 32);
        if (correction != 0)
        {
            entity->posY.part16.integer += correction;
            entity->velY.whole = outsideY;
        }
    }
}
