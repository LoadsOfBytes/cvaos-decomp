/* Merman movement and transitions between swimming, jumping and landing.
 * This reconstruction requires the source-specific -fno-gcse build rule.
 * A byte match establishes generated code, not the original compiler options. */
#include "code_08009A0.h"
#include "merman_entity.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
extern void EntityMoveY(struct EwramData_EntityData *);
extern void EntityMove(struct EwramData_EntityData *);
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Target_GetPixelX(struct EwramData_EntityData *);
extern s32 Entity_MoveWithOptionalCeilingCollision(struct EwramData_EntityData *, s32, s32, s32);
extern s32 Entity_MoveAndProbePointCollision(struct EwramData_EntityData *, s32, s32, s32);
/* Preserve the original caller's full-register argument arithmetic. The callee
 * consumes signed 16-bit coordinates; its implementation performs narrowing. */
extern s32 Collision_FindWaterSurfaceOffset(s32, s32);
extern void Effect_SpawnStationaryAnimation(s32, s32, s32);
#define LEAP_SPLASHED(e) (MERMAN_LEAP_SPLASHED((e)))
#define SWIM_MODE(e) (*(s8 *)&MERMAN_SWIM_MODE((e)))
#define LAND_SPLASHED(e) (*(s8 *)&MERMAN_LAND_SPLASHED((e)))
#define WALK_COUNT(e) (MERMAN_WALK_COUNT((e)))
#define HOME_Y(e) ((s16)MERMAN_HOME_Y((e)))
#define ACCEL_X(e) ((e)->accelX.whole)
#define ACCEL_Y(e) ((e)->accelY.whole)
#define ROOM_Y(e,c) ((e)->posY.part16.integer + (s16)(c)->yPos.part16.integer)

/**
 * @brief Movement state machine (state 0): swim, jump out of the water, land and
 * walk; starts the shot attack (state 1).
 */
void Merman_UpdateMove(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    s32 flags;
restart:
    switch (entity->subState)
    {
    case 0:
        if (entity->timer++ > 59)
        {
            entity->timer = 0;
            entity->subState++;
            entity->velX.whole = 0;
            entity->velY.whole = -0x8000;
            ACCEL_X(entity) = 0;
            ACCEL_Y(entity) = 0;
        }
        break;
    case 1:
        EntityMoveY(entity);
        if (ROOM_Y(entity, camera) < HOME_Y(entity)) EntityDampVelocityY(entity, 16);
        EntityAccelerateAndMove(entity, 0x80000);
        if (entity->velY.whole > -0x4000) entity->subState++;
        if (RandomNumberGenerator() & 1)
        {
            SWIM_MODE(entity) = 0;
            Anim_StartIndexed(entity, 4, 0, 1);
        }
        else
        {
            SWIM_MODE(entity) = 1;
            Anim_StartIndexed(entity, 5, 0, 1);
            entity->flipX = Entity_IsTargetRight(entity);
        }
        break;
    case 2:
    {
        s32 random = RandomNumberGenerator() & 3;
        if (random)
        {
            if (SWIM_MODE(entity) == 0)
            {
                ACCEL_Y(entity) = 0;
                if (ROOM_Y(entity, camera) < HOME_Y(entity) - 24)
                    entity->velY.whole = 0x8000;
                else if (ROOM_Y(entity, camera) <= HOME_Y(entity) + 8 && (RandomNumberGenerator() & 1))
                    entity->velY.whole = 0x8000;
                else entity->velY.whole = -0x8000;
                ACCEL_X(entity) = 0;
                if (RandomNumberGenerator() & 1) entity->velX.whole = 0x8000;
                else entity->velX.whole = -0x8000;
            }
            else
            {
                entity->flipX = Entity_IsTargetRight(entity);
                entity->velX.whole = 0;
                ACCEL_X(entity) = 0;
                ACCEL_Y(entity) = 0;
                if (ROOM_Y(entity, camera) < HOME_Y(entity) - 24)
                    entity->velY.whole = 0x8000;
                else if (ROOM_Y(entity, camera) <= HOME_Y(entity) + 8 && (RandomNumberGenerator() & 1))
                    entity->velY.whole = 0x8000;
                else entity->velY.whole = -0x8000;
            }
            entity->subState++;
            entity->timer = 0;
            goto restart;
        }
        else
        {
            s32 distance = (s16)Target_GetPixelX(entity) - (s16)entity->posX.part16.integer;
            if (distance < 0) distance = -distance;
            if (distance <= 32)
            {
                entity->velY.whole = 0x8000;
                if (RandomNumberGenerator() & 1) entity->velX.whole = 0x8000;
                else entity->velX.whole = -0x8000;
                ACCEL_X(entity) = 0;
                ACCEL_Y(entity) = 0;
                entity->timer = 0;
                entity->subState = 4;
            }
            else
            {
                entity->velX.whole = random;
                ACCEL_X(entity) = random;
                entity->velY.whole = -0x54000;
                ACCEL_Y(entity) = 0x2800;
                entity->subState = 5;
                Anim_StartIndexed(entity, 0, 0, 1);
                LEAP_SPLASHED(entity) = random;
                SWIM_MODE(entity) = random;
                LAND_SPLASHED(entity) = random;
            }
        }
        break;
    }
    case 3:
        if (entity->timer++ > 31) entity->subState = 2;
        /* fall through */
    case 4:
        EntityMove(entity);
        if ((s16)entity->unk_514.part16.spawnParam0 == 1)
        {
            s32 roomX = (s16)(entity->posX.part16.integer + camera->xPos.part16.integer);
            /* Narrow the partial sum before subtraction to retain agbcc ordering. */
            if (roomX < 136) entity->posX.part16.integer = (u16)(entity->posX.part16.integer + 136) - roomX;
            else if (roomX > 408) entity->posX.part16.integer = (u16)(entity->posX.part16.integer + 408) - roomX;
        }
        break;
    case 5:
    {
        flags = Entity_MoveWithOptionalCeilingCollision(entity, 0x20000, -0x200000, 1);
        if (entity->velY.whole < 0 && LEAP_SPLASHED(entity) == 0 && ROOM_Y(entity, camera) < HOME_Y(entity) && !(flags & 0x80))
        {
            s32 offset;
            LEAP_SPLASHED(entity) = 1;
            offset = Collision_FindWaterSurfaceOffset((s16)entity->posX.part16.integer, entity->posY.part16.integer - 32);
            Effect_SpawnStationaryAnimation(entity->posX.whole, entity->posY.whole + ({
                /* Keep the offset calculation grouped after the X load. */
                s32 adjusted = (offset << 16) - 0x200000;
                adjusted;
            }), entity->spriteLayer - 1);
        }
        if (entity->velY.whole > 0 && LAND_SPLASHED(entity) == 0 && (flags & 0x40))
        {
            s32 offset;
            LAND_SPLASHED(entity) = 1;
            offset = Collision_FindWaterSurfaceOffset((s16)entity->posX.part16.integer, entity->posY.part16.integer - 7);
            Effect_SpawnStationaryAnimation(entity->posX.whole, entity->posY.whole + ({
                /* Keep the offset calculation grouped after the X load. */
                s32 adjusted = (offset << 16) - 0x70000;
                adjusted;
            }), entity->spriteLayer - 1);
            entity->subState = 9;
        }
        if (flags & 4)
        {
            entity->subState = 6;
            ACCEL_X(entity) = 0;
            entity->velY.whole = 0x20000;
            ACCEL_Y(entity) = 0x1800;
            LEAP_SPLASHED(entity) = 0;
            SWIM_MODE(entity) = 0;
            LAND_SPLASHED(entity) = 0;
            WALK_COUNT(entity) = 0;
            Anim_StartIndexed(entity, 2, 0, 1);
            entity->flipX = Entity_IsTargetRight(entity);
            if (entity->flipX) entity->velX.whole = 0x8000;
            else entity->velX.whole = -0x8000;
            entity->timer = 0;
        }
        break;
    }
    case 6:
    {
        flags = Entity_MoveWithOptionalCeilingCollision(entity, 0x20000, -0x200000, 0);
        if ((flags & 0x44) == 0x40 && LAND_SPLASHED(entity) == 0)
        {
            s32 offset;
            LAND_SPLASHED(entity) = 1;
            offset = Collision_FindWaterSurfaceOffset((s16)entity->posX.part16.integer, entity->posY.part16.integer - 7);
            Effect_SpawnStationaryAnimation(entity->posX.whole, entity->posY.whole + ({
                /* Keep the offset calculation grouped after the X load. */
                s32 adjusted = (offset << 16) - 0x70000;
                adjusted;
            }), entity->spriteLayer - 1);
            entity->subState = 9;
        }
        else if (!(flags & 4)) entity->subState = 9;
        else if (entity->timer++ > 59)
        {
            entity->timer = 0;
            if ((s8)++WALK_COUNT(entity) > 8)
            {
                entity->velX.whole = 0;
                ACCEL_X(entity) = 0;
                entity->velY.whole = -0x30000;
                ACCEL_Y(entity) = 0x1800;
                entity->subState = 9;
                Anim_StartIndexed(entity, 0, 0, 1);
                LEAP_SPLASHED(entity) = 0;
                LAND_SPLASHED(entity) = 0;
            }
            else if (RandomNumberGenerator() & 3)
            {
                entity->flipX = Entity_IsTargetRight(entity);
                if (entity->flipX) entity->velX.whole = 0x8000;
                else entity->velX.whole = -0x8000;
            }
            else
            {
                entity->state = 1;
                entity->subState = 0;
            }
        }
        break;
    }
    case 9:
    {
        flags = Entity_MoveAndProbePointCollision(entity, 0x2800, -0x200000, 0);
        if (entity->velY.whole > 0 && LAND_SPLASHED(entity) == 0 && (flags & 0x40))
        {
            s32 offset;
            LAND_SPLASHED(entity) = 1;
            offset = Collision_FindWaterSurfaceOffset((s16)entity->posX.part16.integer, entity->posY.part16.integer - 7);
            Effect_SpawnStationaryAnimation(entity->posX.whole, entity->posY.whole + ({
                /* Keep the offset calculation grouped after the X load. */
                s32 adjusted = (offset << 16) - 0x70000;
                adjusted;
            }), entity->spriteLayer - 1);
        }
        break;
    }
    case 10:
    {
        flags = Entity_MoveWithOptionalCeilingCollision(entity, 0x20000, -0x200000, 0);
        if ((flags & 0x44) == 0x40 && LAND_SPLASHED(entity) == 0)
        {
            s32 offset;
            LAND_SPLASHED(entity) = 1;
            offset = Collision_FindWaterSurfaceOffset((s16)entity->posX.part16.integer, entity->posY.part16.integer - 7);
            Effect_SpawnStationaryAnimation(entity->posX.whole, entity->posY.whole + ({
                /* Keep the offset calculation grouped after the X load. */
                s32 adjusted = (offset << 16) - 0x70000;
                adjusted;
            }), entity->spriteLayer - 1);
        }
        if (entity->timer++ > 59)
        {
            entity->timer = 0;
            if (!(flags & 4)) break;
            if (RandomNumberGenerator() & 3)
            {
                entity->flipX = Entity_IsTargetRight(entity);
                if (entity->flipX) entity->velX.whole = 0x8000;
                else entity->velX.whole = -0x8000;
            }
            else
            {
                entity->state = 1;
                entity->subState = 0;
            }
        }
        if (flags & 4) LAND_SPLASHED(entity) = 0;
        break;
    }
    }
}
