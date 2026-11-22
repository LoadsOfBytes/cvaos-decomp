/* Player-relative entity geometry and indexed slot clearing.
 * Positions use 16.16 fixed point; distance results are integer pixels
 * shifted back to fixed point. */
#include "structs/ewram.h"
#include "syscalls.h"

#define PLAYER_ENTITY (gEwramData->playerState.player)

/**
 * @brief 68938 | Clear slot (palette - 10) of sharedObjPaletteSources/sharedObjPaletteVariants for slots 0..3.
 */
void SharedObjPalette_ClearSlot(u8 palette)
{
    s8 slot = palette - 10;

    if (slot >= 0)
    {
        if (slot <= 3)
        {
            gEwramData->gameState.sharedObjPaletteSources[slot] = 0;
            gEwramData->gameState.sharedObjPaletteVariants[slot] = 0;
        }
    }
}

/**
 * @brief Return integer-pixel Euclidean distance to the actual player
 * as a fixed-point value. Preserve signed arithmetic shifts before
 * squaring; this does not select an alternate enemy target.
 */
s32 Entity_DistanceToPlayer(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *player = PLAYER_ENTITY;
    s32 playerX = player->posX.whole;
    s32 playerY = player->posY.whole;
    s32 dx2;
    s32 dy2;

    dx2 = (entity->posX.whole - playerX) >> 16;
    dx2 *= dx2;
    dy2 = (entity->posY.whole - playerY) >> 16;
    dy2 *= dy2;
    return Sqrt(dx2 + dy2) << 16;
}

/**
 * @brief Return absolute fixed-point horizontal distance to the player.
 */
s32 Entity_GetPlayerDistanceX(struct EwramData_EntityData *entity)
{
    s32 playerX = PLAYER_ENTITY->posX.whole;
    s32 dist = entity->posX.whole - playerX;

    if (dist < 0)
        dist = playerX - entity->posX.whole;
    return dist;
}

/**
 * @brief Return absolute fixed-point vertical distance to the player.
 */
s32 Entity_GetPlayerDistanceY(struct EwramData_EntityData *entity)
{
    s32 playerY = PLAYER_ENTITY->posY.whole;
    s32 dist = entity->posY.whole - playerY;

    if (dist < 0)
        dist = playerY - entity->posY.whole;
    return dist;
}

/**
 * @brief 689F8 | Distance between two entities (integer part, as 16.16).
 */
s32 EntityDistance(struct EwramData_EntityData *entity, struct EwramData_EntityData *target)
{
    s32 dx2;
    s32 dy;

    dx2 = (entity->posX.whole - target->posX.whole) >> 16;
    dx2 *= dx2;
    dy = (entity->posY.whole - target->posY.whole) >> 16;
    return Sqrt(dx2 + dy * dy) << 16;
}
