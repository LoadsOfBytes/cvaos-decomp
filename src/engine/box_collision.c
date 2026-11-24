/* Accelerate/move both axes and resolve collision at four box corners.
 * xOffset/yOffset are interpreted as signed halfwords; width/height are
 * additive unsigned extents. Query-specific flag bits retain numeric names.
 *
 * This file matches with -fno-expensive-optimizations (see Makefile). Keep
 * correction locals scoped to individual queries and the inline sums in
 * their current order: they preserve agbcc allocation and narrowing order.
 * Matching code does not establish the original source or compiler options. */
#include "bg_and_collision.h"
#include "structs/ewram.h"
#include "entity_common.h"
#include "math_inline.h"

static inline s32 SumCornerComponents(u16 entityPart, u16 cameraPart) { return entityPart + cameraPart; }

#define ROOM_X ((s16)camera->xPos.part16.integer + (s16)entity->posX.part16.integer)
#define ROOM_Y ((s16)camera->yPos.part16.integer + (s16)entity->posY.part16.integer)
#define CORNER_X(extra) ((s16)(SignExtendOffset(xOffset) + SumCornerComponents(entity->posX.part16.integer, camera->xPos.part16.integer) + (extra)))
#define CORNER_Y(extra) ((s16)(SignExtendOffset(yOffset) + SumCornerComponents(entity->posY.part16.integer, camera->yPos.part16.integer) + (extra)))

static inline s32 AddCornerOffset(s32 base, s32 offset) { return offset + base; }
#define FINAL_X(extra) ((s16)(AddCornerOffset(SumCornerComponents(entity->posX.part16.integer, camera->xPos.part16.integer), SignExtendOffset(xOffset)) + (extra)))
#define FINAL_Y(extra) ((s16)(AddCornerOffset(SumCornerComponents(entity->posY.part16.integer, camera->yPos.part16.integer), SignExtendOffset(yOffset)) + (extra)))
/**
 * @brief Move with the supplied speed limit and resolve directional
 * collisions at four box corners. Return contact, slope-query and
 * class-3 edge flags; replace landing velocity only when nonzero.
 */
s32 Entity_MoveWithBoxCollision(struct EwramData_EntityData *entity, s32 landingVelocity,
                u16 xOffset, u16 yOffset, u16 width, u16 height, s32 movement)
{
    s32 corners[4];
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    s32 flags = 0;

    EntityAccelerateAndMoveX(entity, movement);
    if (entity->velX.whole <= 0)
    {
        {
            s16 correction = Collision_GetPushRight(ROOM_X + (s16)xOffset, ROOM_Y + (s16)yOffset);
            if (correction != 0)
            {
                flags = -(Collision_IsSlopeTile(CORNER_X(0), CORNER_Y(0)) != 0) & 0x800;
                entity->posX.whole += correction << 16;
                flags |= 2;
            }
        }
        {
            s16 correction = Collision_GetPushRight(ROOM_X + (s16)xOffset, ROOM_Y + (s16)yOffset + height);
            if (correction != 0)
            {
                if (Collision_IsSlopeTile(CORNER_X(0), CORNER_Y(height))) flags |= 0x200;
                entity->posX.whole += correction << 16;
                flags |= 2;
            }
        }
    }
    if (entity->velX.whole >= 0)
    {
        {
            s16 correction = Collision_GetPushLeft(ROOM_X + (s16)xOffset + width, ROOM_Y + (s16)yOffset);
            if (correction != 0)
            {
                if (Collision_IsSlopeTile(CORNER_X(width), CORNER_Y(0))) flags |= 0x400;
                entity->posX.whole += correction << 16;
                flags |= 1;
            }
        }
        {
            s16 correction = Collision_GetPushLeft(ROOM_X + (s16)xOffset + width, ROOM_Y + (s16)yOffset + height);
            if (correction != 0)
            {
                if (Collision_IsSlopeTile(CORNER_X(width), CORNER_Y(height))) flags |= 0x100;
                entity->posX.whole += correction << 16;
                flags |= 1;
            }
        }
    }
    EntityAccelerateAndMoveY(entity, movement);
    if (entity->velY.whole <= 0)
    {
        {
            s16 correction = Collision_GetPushDown(ROOM_X + (s16)xOffset, ROOM_Y + (s16)yOffset);
            if (correction != 0)
            {
                entity->posY.whole += correction << 16;
                flags |= 8;
            }
        }
        {
            s16 correction = Collision_GetPushDown(ROOM_X + (s16)xOffset + width, ROOM_Y + (s16)yOffset);
            if (correction != 0)
            {
                entity->posY.whole += correction << 16;
                flags |= 8;
            }
        }
    }
    if (entity->velY.whole >= 0)
    {
        {
            s16 correction = Collision_GetPushUp(ROOM_X + (s16)xOffset, ROOM_Y + (s16)yOffset + height);
            if (correction != 0)
            {
                entity->posY.whole += correction << 16;
                if (landingVelocity != 0) entity->velY.whole = landingVelocity;
                flags |= 4;
            }
        }
        {
            s16 correction = Collision_GetPushUp(ROOM_X + (s16)xOffset + width, ROOM_Y + (s16)yOffset + height);
            if (correction != 0)
            {
                entity->posY.whole += correction << 16;
                if (landingVelocity != 0) entity->velY.whole = landingVelocity;
                flags |= 4;
            }
        }
    }
    corners[0] = Tile_Classify(FINAL_X(0), FINAL_Y(0));
    corners[1] = Tile_Classify(FINAL_X(width), FINAL_Y(0));
    corners[2] = Tile_Classify(FINAL_X(0), FINAL_Y(height));
    corners[3] = Tile_Classify(FINAL_X(width), FINAL_Y(height));
    if (corners[0] == 3 && corners[1] == 3) flags |= 0x80;
    if (corners[2] == 3 && corners[3] == 3) flags |= 0x40;
    if (corners[0] == 3 && corners[2] == 3) flags |= 0x20;
    if (corners[1] == 3 && corners[3] == 3) flags |= 0x10;
    return flags;
}
