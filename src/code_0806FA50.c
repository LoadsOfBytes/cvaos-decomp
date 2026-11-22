#include "code_08009A0.h"
#include "killer_fish_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 Entity_MoveAndCollide(struct EwramData_EntityData *, s32, s32, s32, s32);
extern s32 Collision_FindWaterSurfaceOffset(s32, s32);
extern void KillerFish_SpawnOscillatingEffect(struct EwramData_EntityData *parent, s32 x, s32 y, s32 frame, s32 vx, s32 vy);
extern s32 Target_GetPixelX(struct EwramData_EntityData *);
extern s32 Target_GetPixelY(struct EwramData_EntityData *);
extern s32 Tile_Classify(s16 x, s16 y);
extern s32 Target_IsWithinDistance(struct EwramData_EntityData *, s32);

static inline s16 BgX(s32 v, struct BgLayerState *bg)
{
    v += bg->xPos.part16.integer;
    return v;
}

static inline s16 BgY(s32 v, struct BgLayerState *bg)
{
    /* the original loads bgY into the (dying) bg register */
    { register u32 y asm("r6") = bg->yPos.part16.integer; v += y; } /* FAKEMATCH: pinned register */
    return v;
}

static inline s32 TileAt(struct EwramData_EntityData *entity, struct BgLayerState *bg)
{
    s16 x = BgX(Target_GetPixelX(entity), bg);
    return Tile_Classify(x, BgY(Target_GetPixelY(entity), bg));
}

/**
 * @brief Swim with vertical oscillation and surface correction; turn after stalled movement and select attack when the target checks succeed.
 */
void KillerFish_UpdateSwimming(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    s32 res;

    switch (entity->subState)
    {
    case 0:
        if (entity->flipX)
            entity->velX.whole = 0x4000;
        else
            entity->velX.whole = -0x4000;
        entity->accelX.whole = 0;
        entity->velY.whole = 0;
        entity->accelY.whole = 0x400;
        KILLER_FISH_STALL_TIMER(entity) = 0;
        KILLER_FISH_LAST_X(entity) = 0;
        KILLER_FISH_EFFECT_TIMER(entity) = 0;
        entity->subState++;
        break;
    case 1:
        res = Entity_MoveAndCollide(entity, 0x80000, -0x70000, 1, 0x40000);
        if (entity->velY.whole > 0x4000)
            entity->accelY.whole = -0x400;
        else if (entity->velY.whole < -0x4000)
            entity->accelY.whole = 0x400;
        if (res & 4)
            entity->accelY.whole = -0x400;
        if (res & 8)
        {
            entity->velY.whole = 0;
            entity->accelY.whole = 0x400;
        }
        if (entity->velX.whole > 0)
        {
            if (KILLER_FISH_LAST_X(entity) == (s16)bg->xPos.part16.integer + (s16)entity->posX.part16.integer)
            {
                if (KILLER_FISH_STALL_TIMER(entity)++ > 0x3B)
                {
                    entity->flipX ^= 1;
                    if (entity->flipX)
                        entity->velX.whole = 0x4000;
                    else
                        entity->velX.whole = -0x4000;
                    KILLER_FISH_STALL_TIMER(entity) = 0;
                    KILLER_FISH_LAST_X(entity) = 0;
                    entity->velY.whole = 0;
                    entity->accelY.whole = 0x400;
                }
            }
            else
            {
                KILLER_FISH_LAST_X(entity) = bg->xPos.part16.integer + entity->posX.part16.integer;
                KILLER_FISH_STALL_TIMER(entity) = 0;
            }
        }
        else
        {
            if (KILLER_FISH_LAST_X(entity) == (s16)bg->xPos.part16.integer + (s16)entity->posX.part16.integer)
            {
                if (KILLER_FISH_STALL_TIMER(entity)++ > 0x3B)
                {
                    entity->flipX ^= 1;
                    if (entity->flipX)
                        entity->velX.whole = 0x4000;
                    else
                        entity->velX.whole = -0x4000;
                    KILLER_FISH_STALL_TIMER(entity) = 0;
                    KILLER_FISH_LAST_X(entity) = 0;
                    entity->velY.whole = 0;
                    entity->accelY.whole = 0x400;
                }
            }
            else
            {
                KILLER_FISH_LAST_X(entity) = bg->xPos.part16.integer + entity->posX.part16.integer;
                KILLER_FISH_STALL_TIMER(entity) = 0;
            }
        }
        if (!(res & 0x80))
        {
            entity->posY.part16.integer += Collision_FindWaterSurfaceOffset((s16)entity->posX.part16.integer, (s16)entity->posY.part16.integer - 7);
            entity->accelY.whole = 0x400;
        }
        if ((s16)--KILLER_FISH_EFFECT_TIMER(entity) <= 0)
        {
            KillerFish_SpawnOscillatingEffect(entity, entity->flipX ? entity->posX.whole + 0x100000 : entity->posX.whole - 0x100000, entity->posY.whole, 0x10, 0, -0x4000);
            KILLER_FISH_EFFECT_TIMER(entity) = RandomNumberGenerator() % 0x78 + 0x78;
        }
        if (gEwramData->playerState.enemyTarget)
        {
            if (TileAt(entity, bg) == 3 && Target_IsWithinDistance(entity, 0x500000))
            {
                entity->state = 1;
                entity->subState = 0;
            }
        }
        break;
    }
}
