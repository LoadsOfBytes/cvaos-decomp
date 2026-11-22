/* Debris motion, ground impact and particle emission. */
#include "structs/ewram.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code_080D73B8.h"
#include "code_08001194.h"

extern void Sprite_GetPartCenterOffset(struct EwramData_EntityData *, s32, s16 *);
extern s32 RandomSigned(s32);
extern s32 Entity_IsInRect(struct EwramData_EntityData *, s16, s16, u16, u16);

/**
 * @brief Damp X by 79/80, accelerate and move the sprite debris.
 * While state is 0 sample part 0 center plus 4 pixels for ground
 * impact; emit the shared impact particle or remove outside bounds.
 */
void Effect_UpdateSpriteDebris(struct EwramData_EntityData *entity)
{
    s16 offset[2];
    s32 factor = 79;
    s32 speed = entity->velX.whole;
    s32 limit;
    speed *= factor;
    speed /= 80;
    limit = 0x80000;
    entity->velX.whole = speed + entity->accelX.whole;
    if (entity->accelX.whole >= 0)
    {
        if (entity->velX.whole > limit)
            entity->velX.whole = limit;
    }
    else if (entity->velX.whole < -0x80000)
        entity->velX.whole = -0x80000;
    entity->velY.whole += entity->accelY.whole;
    if (entity->accelY.whole >= 0)
    {
        if (entity->velY.whole > limit)
            entity->velY.whole = limit;
    }
    else if (entity->velY.whole < -0x80000)
        entity->velY.whole = -0x80000;
    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
    if (entity->state == 0)
    {
        s16 *point = offset;
        Sprite_GetPartCenterOffset(entity, 0, point);
        entity->unk_4FC.part16.unk_4FC = offset[0];
        entity->unk_4FC.part16.unk_4FE[0] = point[1] + 4;
    }
    {
        s32 x = (s16)entity->posX.part16.integer;
        struct BgLayerState *background = &gEwramData->bgInfo[1];
        if ((s16)Collision_GetGroundOffset(x + (s16)background->xPos.part16.integer + entity->unk_4FC.part16.unk_4FC,
                             entity->posY.part16.integer + (s16)background->yPos.part16.integer + entity->unk_4FC.part16.unk_4FE[0]))
        {
            struct EwramData_EntityData *particle = Effect_SpawnShared(
                entity->posX.whole + (entity->unk_4FC.part16.unk_4FC << 16),
                entity->posY.whole + (entity->unk_4FC.part16.unk_4FE[0] << 16) - 0x40000, 4, 6);
            if (particle != NULL)
            {
                s32 random;
                particle->drawSkip = 1;
                random = RandomSigned(0x1000);
                particle->velY.whole = random - 0x4000;
                particle->accelY.whole = particle->velY.whole / 32;
                particle->velX.whole = RandomSigned(0x2000) - entity->velX.whole;
                particle->accelX.whole = -particle->velX.whole / 64;
            }
            PlaySong(0x162);
            entity->deleteRequested = 1;
        }
        else if (!Entity_IsInRect(entity, -32, -32, 304, 224))
            entity->deleteRequested = 1;
    }
}

/* Sample the old world position, move with damping, then emit the
 * appropriate impact effect for the tile beneath that sampled position. */
/**
 * @brief Sample the old room-space position, then damp X by 39/40
 * and move. Use the sampled ground/tile result to choose an impact
 * effect and request deletion; preserve sampling before movement.
 */
void Effect_UpdateDebrisWithTileImpact(struct EwramData_EntityData *entity)
{
    u16 *selfX = &entity->posX.part16.integer;
    struct BgLayerState *background = &gEwramData->bgInfo[1];
    u16 x = *selfX + (u16)background->xPos.part16.integer;
    u16 *selfY = (u16 *)&entity->posY.part16.integer;
    u16 y = *selfY + (u16)background->yPos.part16.integer;
    s32 factor = 39;
    s32 speed = entity->velX.whole;
    s32 limit;
    speed *= factor;
    speed /= 40;
    limit = 0x80000;
    entity->velX.whole = speed + entity->accelX.whole;
    if (entity->accelX.whole >= 0)
    {
        if (entity->velX.whole > limit)
            entity->velX.whole = limit;
    }
    else if (entity->velX.whole < -0x80000)
        entity->velX.whole = -0x80000;
    entity->velY.whole += entity->accelY.whole;
    if (entity->accelY.whole >= 0)
    {
        if (entity->velY.whole > limit)
            entity->velY.whole = limit;
    }
    else if (entity->velY.whole < -0x80000)
        entity->velY.whole = -0x80000;
    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
    if ((s16)Collision_GetGroundOffset((s16)x, (s16)y))
    {
        if (!Collision_IsPointSolid((s16)x, (s16)y))
        {
            Effect_SpawnShared(entity->posX.whole, entity->posY.whole - 0x40000, 0, 7);
            entity->deleteRequested = 1;
        }
        else
        {
            s32 offset = (s16)Collision_GetPushUpFiltered((s16)x, (s16)y, 2);
            if (offset != 0)
                Effect_SpawnShared(entity->posX.whole, entity->posY.whole + (offset << 16), 11, 6);
            entity->deleteRequested = 1;
        }
    }
}
