/* Ukoback flight: room-edge and tile collision, facing and timer updates. */
#include "code_08001194.h"
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 Entity_GetTargetDistanceX(struct EwramData_EntityData *entity);
extern s32 Entity_GetTargetDistanceY(struct EwramData_EntityData *entity);
extern s32 Ukoback_SpawnFireball(struct EwramData_EntityData *entity);

/* 1 when the tile coordinate lies outside a room of the given size in
 * screens (one screen: single tiles, otherwise 32 tiles per screen). */
static inline s32 IsTileOutsideRoom(s32 tile, u8 screens, s32 single)
{
    s32 limit = single;

    if (screens != 1)
        limit = screens << 5;
    if (tile < 0)
        return 1;
    return tile >= limit;
}

/**
 * @brief Fly diagonally, bounce at room boundaries and collision probes, and periodically spawn a fireball while fewer than eight are counted.
 */
void Ukoback_UpdateFlight(struct EwramData_EntityData *entity)
{
    u16 probe;
    u8 cells;
    /* Keep collision displacement in r2; the local sums below use r0. */
    register s32 push asm("r2"); /* FAKEMATCH: pinned register */
    s16 roomY;

    if (entity->unk_4F8.part16.unk_4FA != 0)
        entity->unk_4F8.part16.unk_4FA--;
    else
        entity->unk_4F8.part16.unk_4FA = RandomNumberGenerator() % 360 + 80;
    switch (entity->subState)
    {
        case 0:
            entity->velX.whole = entity->velY.whole = 0xC000;
            if ((s16)Entity_GetTargetDistanceX(entity) > 0)
                entity->velX.whole = -entity->velX.whole;
            if ((s16)Entity_GetTargetDistanceY(entity) < 0)
                entity->velY.whole = -entity->velY.whole;
            entity->subState++;
        case 1:
            if (entity->velX.whole < 0)
                entity->flipX = 0;
            else
                entity->flipX = 1;

            probe = GetEntityRoomYPositionInteger(entity) + entity->velY.part16.integer;
            cells = gEwramData->bgInfo[1].pBgMetadata->widthCells;
            if (IsTileOutsideRoom(GetEntityRoomXPositionInteger(entity) / 8, cells, 30))
            {
                entity->velX.whole = -entity->velX.whole;
                entity->posX.whole += entity->velX.whole * 2;
            }
            else
            {
                push = Collision_GetPushRightFiltered(GetEntityRoomXPositionInteger(entity) - 8, (s16)probe, 1);
                if (push == 0)
                    push = Collision_GetPushLeftFiltered(GetEntityRoomXPositionInteger(entity) + 8, (s16)probe, 1);
                if (push != 0)
                {
                    { register s32 position asm("r0") = push + (u16)entity->posX.part16.integer; entity->posX.part16.integer = position; } /* FAKEMATCH: pinned register */
                    entity->velX.whole = -entity->velX.whole;
                }
            }

            probe = GetEntityRoomXPositionInteger(entity) + entity->velX.part16.integer;
            cells = gEwramData->bgInfo[1].pBgMetadata->heightCells;
            if (IsTileOutsideRoom(GetEntityRoomYPositionInteger(entity) / 8, cells, 26))
            {
                entity->velY.whole = -entity->velY.whole;
                entity->posY.whole += entity->velY.whole * 2;
            }
            else
            {
                roomY = GetEntityRoomYPositionInteger(entity);
                push = Collision_GetPushDownFiltered((s16)probe, roomY - 5, 1);
                if (push == 0)
                {
                    roomY = GetEntityRoomYPositionInteger(entity);
                    push = Collision_GetPushUpFiltered((s16)probe, roomY + 1, 1);
                }
                if (push != 0)
                {
                    { register s32 position asm("r0") = push + (u16)entity->posY.part16.integer; entity->posY.part16.integer = position; } /* FAKEMATCH: pinned register */
                    entity->velY.whole = -entity->velY.whole;
                }
            }
            break;
    }
    if (entity->unk_4F8.part16.unk_4FA == 0 && entity->unk_504.part8.unk_506 <= 7
        && Ukoback_SpawnFireball(entity))
        entity->unk_504.part8.unk_506++;
    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
    entity->velX.whole += entity->accelX.whole;
    entity->velY.whole += entity->accelY.whole;
}
