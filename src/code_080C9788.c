/* Lightning Doll: reflect motion at room boundaries and tile collisions. */
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

static inline s32 AtLimit(s32 a,s32 b) { return a >= b; }
static inline s32 RoomLimit(s32 size,s32 base) { return size == 1 ? base : size << 5; }
/**
 * @brief Reverse velocity at room boundaries and directional tile collisions, apply coordinate corrections and update horizontal facing.
 */
void LightningDoll_ResolveMovementBounds(struct EwramData_EntityData *entity)
{
    s32 size;
    s32 tile;
    s32 limit;
    s32 outside;
    s32 outsideY;
    s32 correction;
    s16 *position;
    struct EwramData **state;

    GetEntityRoomYPositionInteger(entity);
    state = &gEwramData;
    size = ((u8 *)(*state)->bgInfo[1].pBgMetadata)[0];
    tile = GetEntityRoomXPositionInteger(entity) / 8;
    limit = RoomLimit(size,30);
    if (tile < 0 || AtLimit(tile,limit))
    {
        entity->velX.whole = -entity->velX.whole;
        entity->posX.whole += entity->velX.whole * 2;
        entity->flipX = 1 - entity->flipX;
    }
    else
    {
        if (entity->velX.whole >= 0)
        {
            position = (s16 *)&entity->posX.part16.integer;
            correction = Collision_GetPushLeft(RoomX(*position) + 8,
                                     RoomY((s16)entity->posY.part16.integer) - 16);
        }
        else
        {
            position = (s16 *)&entity->posX.part16.integer;
            correction = Collision_GetPushRight(RoomX(*position) - 8,
                                     RoomY((s16)entity->posY.part16.integer) - 16);
        }
        if (correction != 0)
        {
            *position += correction;
            entity->velX.whole = -entity->velX.whole;
            entity->flipX = 1 - entity->flipX;
        }
    }

    GetEntityRoomXPositionInteger(entity);
    state = &gEwramData;
    size = ((u8 *)(*state)->bgInfo[1].pBgMetadata)[1];
    tile = GetEntityRoomYPositionInteger(entity) / 8;
    limit = RoomLimit(size,26);
    if (tile < 0 || AtLimit(tile,limit))
    {
        entity->velY.whole = -entity->velY.whole;
        entity->posY.whole += entity->velY.whole * 2;
    }
    else
    {
        if (entity->velY.whole >= 0)
        {
            s32 x = RoomX((s16)entity->posX.part16.integer);
            position = (s16 *)&entity->posY.part16.integer;
            correction = Collision_GetPushUp(x,
                                     RoomY(*position) + 1);
        }
        else
        {
            s32 x = RoomX((s16)entity->posX.part16.integer);
            position = (s16 *)&entity->posY.part16.integer;
            correction = Collision_GetPushDown(x,
                                     RoomY(*position) - 32);
        }
        if (correction != 0)
        {
            *position += correction;
            entity->velY.whole = -entity->velY.whole;
        }
    }
}
