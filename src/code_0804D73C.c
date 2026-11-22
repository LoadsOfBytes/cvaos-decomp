/* Menu helpers 0x0804D73C..0x0804D793. */
#include "structs/ewram.h"

extern void HudMap_WriteTiles(s32 x, s32 y, u16 tile, s32 count);

struct BestiaryAttributeIcon {
    s32 tileColumn; // low four bits of the tile coordinate
    s32 tileRow; // row in the 16-column tile layout
};

extern const struct BestiaryAttributeIcon sBestiaryAttributeIcons[];

/**
 * @brief Return 1 when the defeated flag of enemy id is set (bit set
 * enemyFlags[0..3], set by Enemy_DropRewards), else 0.
 */
s32 Bestiary_IsEnemyDefeated(s32 id)
{
    s32 bit = gEwramData->playerStatus.enemyFlags[id >> 5] & (1 << (id & 0x1F));
    return (u32)(-bit | bit) >> 31; /* bit != 0 */
}

/**
 * @brief Put the single tile of attribute icon index (sBestiaryAttributeIcons) at (x, y);
 * used for the weak/resist rows of the bestiary entry.
 */
void BestiaryMenu_DrawAttributeIcon(s32 x, s32 y, s32 attribute)
{
    const struct BestiaryAttributeIcon *icon = &sBestiaryAttributeIcons[attribute];

    HudMap_WriteTiles(x, y, icon->tileColumn - 0x2D00 + (icon->tileRow << 4), 1);
}
