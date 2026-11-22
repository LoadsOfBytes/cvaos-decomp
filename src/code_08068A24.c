#include "structs/ewram.h"
extern s32 ArcTan2(s32, s32);

/* Aim at a point sixteen pixels above the player's position. */
/**
 * @brief Return the angle to a point 16 pixels above the player.
 * Preserve truncation of each pixel difference to signed 16 bits
 * before ArcTan2; this helper has matching C in its own file.
 */
u16 Entity_AngleToPointAbovePlayer(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *player = gEwramData->playerState.player;
    s32 x = (s16)player->posX.part16.integer - (u16)entity->posX.part16.integer;
    s32 y;
    x = (s32)((u32)x << 16);
    x >>= 16;
    y = player->posY.part16.integer - (u16)entity->posY.part16.integer - 16;
    y = (s32)((u32)y << 16);
    y >>= 16;
    return ArcTan2(x, y);
}
