/* Accelerate/move both axes and resolve collision at four box corners, with
 * a tile-type exclusion: a corner whose tile flags satisfy (flags & 3) == 1
 * is skipped. Otherwise the directional correction and the slope query use
 * the same cached room coordinates. xOffset/yOffset are interpreted as signed
 * halfwords; width/height are additive unsigned extents. A nonzero
 * landingVelocity replaces the Y velocity after a downward correction.
 * Query-specific flag bits retain numeric names.
 *
 * Compiler-sensitive constructs (default project flags; do not copy the
 * -fno-expensive-optimizations exception of Entity_MoveWithBoxCollision):
 * - The r1 tile locals and the ReadTileFlags helper reproduce the original
 *   zero extension, mask and compare registers.
 * - Final corners: the entity+camera sum is truncated to s16 directly, so
 *   agbcc adds the two halfword memory operands without extending them (the
 *   spilled camera pointer is then reloaded for every component).
 * - FINAL_X0/FINAL_Y0: the statement expression delays the offset's sign
 *   extension until after the sum, and its halfword result makes agbcc put
 *   the offset first in the addition.
 * Matching code does not establish the original source spelling. */
#include "code_08001194.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
extern s32 Collision_IsSlopeTile(s16 x, s16 y);
extern s32 Tile_Classify(s16 x, s16 y);

static inline s32 ReadTileFlags(s32 x, s32 y) { return Collision_GetTile(x, y); }
static inline s32 SignExtendOffset(u16 value) { return (s16)value; }

static inline s16 TruncatedSignedOffset(u16 v) { s32 x = (s16)v; return x; }
#define ROOM_X ((s16)camera->xPos.part16.integer + (s16)entity->posX.part16.integer)
#define ROOM_Y ((s16)camera->yPos.part16.integer + (s16)entity->posY.part16.integer)

#define FINAL_X0 ((s16)((s16)(camera->xPos.part16.integer + entity->posX.part16.integer) + ({ TruncatedSignedOffset(xOffset); })))
#define FINAL_Y0 ((s16)((s16)(camera->yPos.part16.integer + entity->posY.part16.integer) + ({ TruncatedSignedOffset(yOffset); })))
#define FINAL_X(extra) ((s16)((extra) + (SignExtendOffset(xOffset) + (s16)(camera->xPos.part16.integer + entity->posX.part16.integer))))
#define FINAL_Y(extra) ((s16)((extra) + (SignExtendOffset(yOffset) + (s16)(camera->yPos.part16.integer + entity->posY.part16.integer))))
/**
 * @brief Resolve four-corner box movement while excluding corners
 * whose tile flags satisfy (flags & 3) == 1. Preserve cached room
 * coordinates, query-specific slope bits and narrowing order.
 */
s32 Entity_MoveWithFilteredBoxCollision(struct EwramData_EntityData *entity, s32 landingVelocity,
                u16 xOffset, u16 yOffset, u16 width, u16 height, s32 movement)
{
    s32 corners[4];
    s32 x, y;
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    s32 flags = 0;

    EntityAccelerateAndMoveX(entity, movement);
    if (entity->velX.whole <= 0)
    {
        x = ROOM_X + (s16)xOffset;
        y = ROOM_Y + (s16)yOffset;
        {
            register s32 tile asm("r1") = ReadTileFlags(x, y); /* FAKEMATCH: pinned register */
            tile &= 3;
            if (tile != 1)
            {
            s16 correction = Collision_GetPushRight(x, y);
            if (correction != 0)
            {
                flags = -(Collision_IsSlopeTile((s16)x, (s16)y) != 0) & 0x800;
                entity->posX.whole += correction << 16;
                flags |= 2;
            }
        }
        }
        x = ROOM_X + (s16)xOffset;
        y = ROOM_Y + (s16)yOffset + height;
        {
            register s32 tile asm("r1") = ReadTileFlags(x, y); /* FAKEMATCH: pinned register */
            tile &= 3;
            if (tile != 1)
            {
            s16 correction = Collision_GetPushRight(x, y);
            if (correction != 0)
            {
                if (Collision_IsSlopeTile((s16)x, (s16)y)) flags |= 0x200;
                entity->posX.whole += correction << 16;
                flags |= 2;
            }
        }
        }
    }
    if (entity->velX.whole >= 0)
    {
        x = ROOM_X + (s16)xOffset + width;
        y = ROOM_Y + (s16)yOffset;
        {
            register s32 tile asm("r1") = ReadTileFlags(x, y); /* FAKEMATCH: pinned register */
            tile &= 3;
            if (tile != 1)
            {
            s16 correction = Collision_GetPushLeft(x, y);
            if (correction != 0)
            {
                if (Collision_IsSlopeTile((s16)x, (s16)y)) flags |= 0x400;
                entity->posX.whole += correction << 16;
                flags |= 1;
            }
        }
        }
        x = ROOM_X + (s16)xOffset + width;
        y = ROOM_Y + (s16)yOffset + height;
        {
            register s32 tile asm("r1") = ReadTileFlags(x, y); /* FAKEMATCH: pinned register */
            tile &= 3;
            if (tile != 1)
            {
            s16 correction = Collision_GetPushLeft(x, y);
            if (correction != 0)
            {
                if (Collision_IsSlopeTile((s16)x, (s16)y)) flags |= 0x100;
                entity->posX.whole += correction << 16;
                flags |= 1;
            }
        }
        }
    }
    EntityAccelerateAndMoveY(entity, movement);
    if (entity->velY.whole <= 0)
    {
        x = ROOM_X + (s16)xOffset;
        y = ROOM_Y + (s16)yOffset;
        {
            register s32 tile asm("r1") = ReadTileFlags(x, y); /* FAKEMATCH: pinned register */
            tile &= 3;
            if (tile != 1)
            {
            s16 correction = Collision_GetPushDown(x, y);
            if (correction != 0)
            {
                if (Collision_IsSlopeTile((s16)x, (s16)y)) flags |= 0x800;
                entity->posY.whole += correction << 16;
                flags |= 8;
            }
        }
        }
        x = ROOM_X + (s16)xOffset + width;
        y = ROOM_Y + (s16)yOffset;
        {
            register s32 tile asm("r1") = ReadTileFlags(x, y); /* FAKEMATCH: pinned register */
            tile &= 3;
            if (tile != 1)
            {
            s16 correction = Collision_GetPushDown(x, y);
            if (correction != 0)
            {
                if (Collision_IsSlopeTile((s16)x, (s16)y)) flags |= 0x400;
                entity->posY.whole += correction << 16;
                flags |= 8;
            }
        }
        }
    }
    if (entity->velY.whole >= 0)
    {
        x = ROOM_X + (s16)xOffset;
        y = ROOM_Y + (s16)yOffset + height;
        {
            register s32 tile asm("r1") = ReadTileFlags(x, y); /* FAKEMATCH: pinned register */
            tile &= 3;
            if (tile != 1)
            {
            s16 correction = Collision_GetPushUp(x, y);
            if (correction != 0)
            {
                if (Collision_IsSlopeTile((s16)x, (s16)y)) flags |= 0x200;
                entity->posY.whole += correction << 16;
                if (landingVelocity != 0) entity->velY.whole = landingVelocity;
                flags |= 4;
            }
        }
        }
        x = ROOM_X + (s16)xOffset + width;
        y = ROOM_Y + (s16)yOffset + height;
        {
            register s32 tile asm("r1") = ReadTileFlags(x, y); /* FAKEMATCH: pinned register */
            tile &= 3;
            if (tile != 1)
            {
            s16 correction = Collision_GetPushUp(x, y);
            if (correction != 0)
            {
                if (Collision_IsSlopeTile((s16)x, (s16)y)) flags |= 0x100;
                entity->posY.whole += correction << 16;
                if (landingVelocity != 0) entity->velY.whole = landingVelocity;
                flags |= 4;
            }
        }
        }
    }
    corners[0] = Tile_Classify(FINAL_X0, FINAL_Y0);
    corners[1] = Tile_Classify(FINAL_X(width), FINAL_Y0);
    corners[2] = Tile_Classify(FINAL_X0, FINAL_Y(height));
    corners[3] = Tile_Classify(FINAL_X(width), FINAL_Y(height));
    if (corners[0] == 3 && corners[1] == 3) flags |= 0x80;
    if (corners[2] == 3 && corners[3] == 3) flags |= 0x40;
    if (corners[0] == 3 && corners[2] == 3) flags |= 0x20;
    if (corners[1] == 3 && corners[3] == 3) flags |= 0x10;
    return flags;
}
