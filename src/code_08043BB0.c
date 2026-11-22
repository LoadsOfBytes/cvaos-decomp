/* Player_RestoreMPPercent. */
#include "structs/ewram.h"

/**
 * @brief Apply the original signed fixed-point percentage of maximum MP, substitute one for a zero amount and cap only at maximum MP.
 */
void Player_RestoreMPPercent(s32 percent)
{
    s32 v = (gEwramData->playerStatus.maxMP << 16) / 100 * percent;
    s32 amount = (v < 0) ? -(-v >> 16) : v >> 16;

    if (amount == 0)
        amount = 1;
    gEwramData->playerStatus.currentMP += amount;
    if (gEwramData->playerStatus.currentMP > gEwramData->playerStatus.maxMP)
        gEwramData->playerStatus.currentMP = gEwramData->playerStatus.maxMP;
}
