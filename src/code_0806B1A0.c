/*
 * Walk check for a point at the feet one pixel ahead: 0 when the walker can
 * continue (empty point with floor 8 px below and headroom 7 px above, or a
 * solid point with an empty / slope (0xC0) tile 7 px above, i.e. a step), 1 for
 * a wall or a ledge. Used by Curly_UpdateWalk to turn around.
 */
#include "code_08001194.h"
s32 Collision_IsWalkBlocked(s16 x, s16 y)
{
    u8 tile;
    if (Collision_GetTile(x, y) == 0)
    {
        if (Collision_GetTile(x, y + 8) != 0)
        {
            if (Collision_GetTile(x, y - 7) == 0)
                goto empty;
        }
    }
    else
    {
        tile = Collision_GetTile(x, y - 7);
        if (tile == 0 || (tile & 0xC0))
            goto empty;
    }
    return 1;
empty:
    return 0;
}

