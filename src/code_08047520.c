#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code_08011DD0.h"
#include "code_08033CAC.h"
#include "syscalls.h"

extern void HudMap_ClearAll(void);
extern void YellowSoul_Restart(u8 soul, s32 force);
extern void Menu_DrawHpMpBars(s32 x, s32 y);
extern void Menu_DrawCurrentStats(s32 x, s32 y);
extern void Menu_DrawAttackDefense(s32 x, s32 y);
extern void Menu_DrawPlayTime(s32 x, s32 y, s32 a, u32 value);
extern void HudMap_PrintNumber(s32 x, s32 y, s32 value, s32 digits, s32 color, s32 flag);
extern void HudMap_PrintLargeNumber(s32 x, s32 y, s32 value, s32 a, s32 b);
extern void HudMap_DrawStatusConditionIcon(s32 x, s32 y);
extern void HudMap_WriteTiles(s32 x, s32 y, u16 tile, s32 count);
extern void PauseMenu_DrawPageList(void);
extern void MenuItemIcon_SetHidden(s32 value);

/**
 * @brief 47520 | Draw the status page of the menu: play time, level,
 * gold, experience and next level, plus the static frame tiles.
 */
void PauseMenu_DrawStatusPage(struct GameState *gameState)
{
    struct PlayerStatus *stats;
    s32 value;
    s32 i;

    stats = &gEwramData->playerStatus;
    BgMap_Setup(1, (struct BgMapMetadata *)0x0815E940, 0, 0);
    HudMap_ClearAll();
    Player_RefreshEquipmentStats();
    YellowSoul_Restart(gEwramData->playerStatus.equippedYellowSoul, 1);
    Menu_DrawHpMpBars(0xE, 2);
    Menu_DrawCurrentStats(0xE, 5);
    Menu_DrawAttackDefense(0x18, 5);
    Menu_DrawPlayTime(0x1C, 8, 3, gEwramData->gameState.inGameTimer);
    value = Map_GetCompletionTenths(gameState, gameState->unk_8D_6);
    HudMap_PrintNumber(0x17, 9, Div(value, 10), 3, 3, 0);
    HudMap_PrintNumber(0x19, 9, Mod(value, 10), 1, 3, 1);
    HudMap_PrintNumber(0x1A, 0xA, stats->currentGold, 6, 6, 0);
    HudMap_PrintNumber(0x19, 0xC, stats->currentExperience, 8, 3, 0);
    HudMap_PrintNumber(0x19, 0xD, Player_GetExperienceToNextLevel(stats), 8, 3, 0);
    HudMap_PrintLargeNumber(5, 9, stats->currentLevel, 2, 3);
    HudMap_DrawStatusConditionIcon(0xC, 9);
    Text_Initialize();
    Text_SetTileBuffer((void *)0x06008000);
    Text_SetForegroundColor(1);
    Text_SetBackgroundColor(0);
    for (i = 0; i < 3; i++)
        HudMap_WriteTiles(10, i + 0xF, i * 0x20 + 0x3200, 0x13);
    Text12_SetRegion(0, 0, 0x1A, 3);
    Text12_ClearRegion();
    PauseMenu_DrawPageList();
    MenuItemIcon_SetHidden(1);
}
