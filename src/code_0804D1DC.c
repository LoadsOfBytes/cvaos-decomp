#include "code_08032444.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"

/* Local 0x24-byte template-table view: retain this TU's types and padding. */
struct EnemyTemplateView
{
    u8 pad_0[0x17];
    u8 soulType;
    u8 soulIndex; /* 1-based, 0 = none */
    u8 pad_19[0x24 - 0x19];
};

extern const struct EnemyTemplateView sEnemyData[];
extern const u16 sEnemyNameTextIds[];

extern void HudMap_WriteTiles(s32 x, s32 y, u16 tile, s32 count);
extern void HudMap_ClearTiles(s32 x, s32 y, s32 count);
extern void Menu_PrintSoulName(s32 soulType, s32 soulId);

/**
 * @brief Draw the description box of enemy id; mode 1 (only when its soul is
 * owned) shows the soul name instead of the enemy description text.
 *
 * @return the mode actually used
 */
s32 BestiaryMenu_DrawDescription(s32 id, s32 mode)
{
    const struct EnemyTemplateView *entry;
    u8 total;

    entry = &sEnemyData[id];
    if (entry->soulIndex != 0)
        total = SoulInventory_GetSoulTotal(entry->soulType, entry->soulIndex - 1);
    else
        total = 0;
    HudMap_WriteTiles(3, 0xE, 0x1200, 0x19);
    HudMap_WriteTiles(3, 0xF, 0x1220, 0x19);
    HudMap_WriteTiles(3, 0x10, 0x1240, 0x19);
    Text12_SetRegion(0, 0, 0x22, 2);
    if (total == 0 && mode == 1)
        mode = 0;
    if (mode == 0)
    {
        MenuText_ClearAndPrint(sEnemyNameTextIds[id] + 0x71);
    }
    else
    {
        Text12_ClearRegion();
        Menu_PrintSoulName(entry->soulType, entry->soulIndex);
    }
    if (total != 0)
    {
        if (mode == 0)
        {
            HudMap_WriteTiles(0x1C, 0x10, 0x134E, 1);
            HudMap_ClearTiles(0x1C, 0xE, 1);
        }
        else
        {
            HudMap_WriteTiles(0x1C, 0xE, 0x134D, 1);
            HudMap_ClearTiles(0x1C, 0x10, 1);
        }
    }
    else
    {
        HudMap_ClearTiles(0x1C, 0xE, 1);
        HudMap_ClearTiles(0x1C, 0x10, 1);
    }
    return mode;
}
