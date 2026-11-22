/* Test the tile at a point and its nearby vertical neighbors. */
#include "code_08001194.h"
s32 sub_0806B1A0(s16 x, s16 y)
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

