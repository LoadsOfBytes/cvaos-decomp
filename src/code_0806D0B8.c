/* Entity/tile helper routines 0x0806D0B8..0x0806D2F7 (no upstream C existed). */
#include <string.h>
#include "code_08001194.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
#include "syscalls.h"

extern s32 Entity_MoveWithCollisionAlt(struct EwramData_EntityData *entity, s32 landingVelocity, s32 ceilingOffset, s32 speedLimit);

/* Rectangle with 32-bit fields (x, y, w, h order is a guess from the flips). */
struct FlipRect32 {
    s32 x;
    s32 y;
    s32 width;
    s32 height;
};

/* Seven (x, y) s16 margin pairs at ROM 0x08118D08, copied by EntityIsOutsideScreen.
 * Declared as bytes: the original calls memcpy instead of an inline copy. */
extern const u8 sOffscreenMargins[];

/**
 * @brief 6D0B8 | Classify the tile at (x, y) via Collision_GetTile.
 *
 * @return 0 or 1 for an empty tile (depending on the tile below), 2 or 3 for
 * a non-empty one (bit 3 clear / set)
 */
s32 Tile_Classify(s16 x, s16 y)
{
    s32 tile;

    tile = Collision_GetTile(x, y);
    if (tile == 0)
    {
        tile = Collision_GetTile(x, y + 1);
        if (tile & 3)
            return 0;
        return 1;
    }

    if (tile & 8)
        return 3;
    return 2;
}

/**
 * @brief Return 1 when the tile collision flags contain bit 6 or 7, otherwise zero.
 */
s32 Collision_IsSlopeTile(s16 x, s16 y)
{
    s32 tile = Collision_GetTile(x, y);
    if (tile & 0xC0)
        return 1;
    return 0;
}

/**
 * @brief 6D128 | Whether the entity is outside the 240x160 screen extended by
 * margin pair margin of sOffscreenMargins.
 *
 * @return 1 when outside, 0 when inside
 */
s32 EntityIsOutsideScreen(struct EwramData_EntityData *entity, s32 margin)
{
    s16 margins[7][2];
    s32 pos;

    memcpy(margins, sOffscreenMargins, sizeof(margins));

    pos = (s16)entity->posX.part16.integer;
    if (pos >= -margins[margin][0] && pos <= margins[margin][0] + 240)
    {
        pos = entity->posY.part16.integer;
        if (pos >= -margins[margin][1] && pos <= margins[margin][1] + 160)
            return 0;
    }
    return 1;
}

/**
 * @brief Compare room X in eight-pixel tiles with the supplied tile margin; return 1 at the left margin, 2 at the right, otherwise zero.
 */
s32 Entity_GetHorizontalRoomEdge(struct EwramData_EntityData *entity, s32 margin)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    struct BgMapMetadata *metadata = bg->pBgMetadata;
    s32 tileX;
    s32 widthTiles;

    tileX = ((s16)entity->posX.part16.integer + (s16)bg->xPos.part16.integer) / 8;

    if (metadata->widthCells != 1)
        widthTiles = metadata->widthCells << 5;
    else
        widthTiles = 0x1E;

    if (tileX < margin)
        return 1;
    if (tileX > widthTiles - margin)
        return 2;
    return 0;
}

/**
 * @brief Apply horizontal and vertical flip adjustments to the four-word rectangle view; retain its existing inferred field interpretation.
 */
void Entity_ApplyFlipToRect32(struct EwramData_EntityData *entity, struct FlipRect32 *rect)
{
    u8 *flags = (u8 *)&entity->accelY + 4;

    if (*flags & 0x40)
        rect->x = -(rect->width + rect->x);
    if (*flags & 0x80)
        rect->y = -(rect->height + rect->y);
}

/**
 * @brief Apply flip adjustments to a packed SpriteBox; preserve the original vertical write into height rather than y.
 */
void Entity_ApplyFlipToSpriteBox(struct EwramData_EntityData *entity, struct SpriteBox *box)
{
    u8 *flags = (u8 *)&entity->accelY + 4;
    s32 extent;

    if (*flags & 0x40)
    {
        extent = box->x + box->width;
        box->x = -extent;
    }
    if (*flags & 0x80)
    {
        extent = box->y + box->height;
        box->height = -extent;
    }
}

/**
 * @brief 6D244 | Activate the entity box slot with type 8, then call
 * Entity_UpdateBoxes.
 */
void Entity_EnableBoxes(struct EwramData_EntityData *entity, void *onHit, s32 onHurt)
{
    Hit_Enable(entity, 8, onHit, onHurt);
    Entity_UpdateBoxes(entity);
}

/**
 * @brief Interpret animation steps as SpriteFrame records and return the largest boxCount over stepCount records.
 */
s32 Anim_GetMaxBoxCount(struct EwramData_EntityData *entity)
{
    struct SpriteFrame *frames;
    s32 maxBoxes;
    s32 i;

    frames = (struct SpriteFrame *)entity->animScript->steps;
    maxBoxes = 0;
    for (i = 0; i < entity->animScript->stepCount; i++)
    {
        if (frames[i].boxCount > maxBoxes)
            maxBoxes = frames[i].boxCount;
    }
    return maxBoxes;
}

/**
 * @brief 6D288 | Entity_MoveWithCollisionAlt with a fixed last argument of 0x80000.
 */
s32 Entity_MoveWithCollisionDefault(struct EwramData_EntityData *entity, s32 landingVelocity, s32 ceilingOffset)
{
    return Entity_MoveWithCollisionAlt(entity, landingVelocity, ceilingOffset, 0x80000);
}

/**
 * @brief Return a 16.16 distance from the supplied point to entity position plus BG1 scroll, using whole-pixel differences before square root.
 */
s32 Entity_GetRoomPointDistance(struct EwramData_EntityData *entity, s32 x, s32 y)
{
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    s32 dxSquared;
    s32 dySquared;

#define DIFF_INT(a, b) (((a) - (b) < 0) ? -(((b) - (a)) >> 16) : (((a) - (b)) >> 16))
    dxSquared = DIFF_INT(x, entity->posX.whole + camera->xPos.whole);
    dxSquared *= dxSquared;
    dySquared = DIFF_INT(y, entity->posY.whole + camera->yPos.whole);
    dySquared *= dySquared;
#undef DIFF_INT

    return Sqrt(dxSquared + dySquared) << 16;
}
