#include "code_08009A0.h"
#include "room_object_entity.h"
#include "code_08001194.h"
#include "code_08011DD0.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern void Debris_SpawnBurst(struct EwramData_EntityData *entity, s32 dy);
extern void Object09Debris_Update(struct EwramData_EntityData *entity);
extern void BreakableBlock_ClearCollision(struct EwramData_EntityData *entity);

struct BreakableBlockVariant {
    u8 pad_0[9];
    u8 collisionMode;
    u8 pad_A[2];
};

extern const struct BreakableBlockVariant sBreakableBlockVariants[];

/**
 * @brief 50270 | Breakable wall that crumbles once its flag (unk_516 in
 * gameFlagsTail) is set: dust Effect_SpawnShared every 16th frame while Player_TestSoulStateFlags
 * holds; when the room entry sBreakableBlockVariants[unk_514].collisionMode is set it just
 * opens (Debris_SpawnBurst(0)), otherwise up to 3 x 7 debris pieces
 * Object09Debris_Update scatter over the wall and it opens 32 px up; then song
 * 0xAB, BreakableBlock_ClearCollision, the map update (GameFlag_FindRoomCoordinate / GameFlag_Set) and it
 * finishes.
 */
void Object09Update(struct EwramData_EntityData *entity)
{
    u8 unused[16];
    struct EwramData_EntityData *dust;
    struct EwramData_EntityData *piece;
    s32 i, j, r;
    u16 x, y;
    const struct BreakableBlockVariant *info;

    if (Player_TestSoulStateFlags(0x1000000))
    {
        entity->timer++;
        if ((entity->timer & 0xF) == 0)
        {
            dust = Effect_SpawnShared(entity->posX.whole, entity->posY.whole, 12, 6);
            if (dust != NULL)
                dust->spriteLayer = entity->spriteLayer - 1;
        }
    }
    if (entity->state != 0)
        return;
    if (!(gEwramData->gameState.gameFlagsTail[entity->unk_514.part16.spawnParam1 >> 5] & (1 << (entity->unk_514.part16.spawnParam1 & 0x1F))))
        return;
    info = &sBreakableBlockVariants[entity->unk_514.part16.spawnParam0];
    if (info->collisionMode != 0)
        Debris_SpawnBurst(entity, 0);
    else
    {
        for (i = -1; i <= 1; i++)
        {
            for (j = -3; j <= 3; j++)
            {
                piece = EntityCreateInRange(0x49, 0xCF, (u32 *)Object09Debris_Update);
                if (piece == NULL)
                    break;
                piece->scrollWithBg = 1;
                piece->posX.part16.integer = entity->posX.part16.integer - (i << 3) + RandomSigned(4);
                piece->posY.part16.integer = entity->posY.part16.integer - ((j << 3) + 0x20) + RandomSigned(4);
                piece->unk_514.part16.spawnParam0 = entity->unk_514.part16.spawnParam0;
            }
        }
        Debris_SpawnBurst(entity, -0x20);
    }
    PlaySong(0xAB);
    BreakableBlock_ClearCollision(entity);
    x = GetEntityRoomXPositionInteger(entity);
    y = GetEntityRoomYPositionInteger(entity);
    r = GameFlag_FindRoomCoordinate((s16)x, (s16)y, (u8)entity->ext.object09.collisionMode);
    if (r >= 0)
        GameFlag_Set(r);
    entity->deleteRequested = 1;
}
