#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"

struct MenuPage
{
    u8 pad_0[0xA];
    u8 state;
    u8 subState;
};

extern void MenuPage_Reset(struct MenuPage *page);
extern void HudMap_WriteTiles(s32 x, s32 y, u16 tile, s32 count);
extern void EquipmentMenu_DrawList(struct PlayerStatus *stats, struct MenuPage *page, s32 slot);
extern void MenuList_DrawScrollArrows(struct MenuPage *page, s32 param_1);
extern void YellowSoul_Restart(u8 soul, s32 force);
extern void Menu_DrawAttackDefense(s32 x, s32 y);
extern void Menu_DrawCurrentStats(s32 x, s32 y);
extern void MenuPageTitle_Create(struct EwramData_EntityData *entity, s32 type);
extern void Cursor_SetTilePosition(s32 a, s32 b);
extern void Cursor_Draw(void);
extern void EquipmentMenu_DrawEquippedIcons(struct PlayerStatus *stats, struct MenuPage *page);
extern void EquipmentMenu_PrintEquippedNames(void);
extern void EquipmentMenu_ShowSlotDescription(struct PlayerStatus *stats, struct MenuPage *page);

/**
 * @brief 4A95C | Open the equipment page for slot unk_B (0 weapon,
 * 1 armor, 2 accessory): draw the frame and labels, the item list and the
 * stat preview.
 */
void EquipmentMenu_Open(struct MenuPage *page)
{
    struct PlayerStatus *stats;
    s32 i;
    s32 t;

    stats = &gEwramData->playerStatus;
    MenuPage_Reset(page);
    for (i = 0; i <= 2; i++)
        HudMap_WriteTiles(5, i + 0x10, i * 0x20 + 0x3200, 0x18);
    Text12_SetRegion(0, 0, 0x20, 2);
    switch (page->subState)
    {
    case 0:
        HudMap_WriteTiles(2, 3, 0x33D0, 0xB);
        break;
    case 1:
        HudMap_WriteTiles(2, 3, 0x33E0, 0xB);
        break;
    case 2:
        HudMap_WriteTiles(2, 3, 0x33F0, 0xB);
        break;
    }
    HudMap_WriteTiles(3, 5, 0x3310, 9);
    HudMap_WriteTiles(3, 7, 0x3320, 9);
    HudMap_WriteTiles(3, 9, 0x3330, 9);
    for (i = 0, t = 0x40; i <= 7; t += 0x10, i++)
        HudMap_WriteTiles((i & 1) ? 0x11 : 3, (i >> 1) + 0xB, t + 0x3300, 9);
    EquipmentMenu_DrawList(stats, page, page->subState);
    MenuList_DrawScrollArrows(page, 0);
    page->state = 2;
    Player_RefreshEquipmentStats();
    YellowSoul_Restart(gEwramData->playerStatus.equippedYellowSoul, 1);
    Menu_DrawAttackDefense(0x16, 2);
    Menu_DrawCurrentStats(0x16, 5);
    MenuPageTitle_Create(&gEwramData->entityData[20], 0x20);
    Cursor_SetTilePosition(2, 5);
    Cursor_Draw();
    for (i = 0; i < 3; i++)
        EntityDelete(&gEwramData->entityData[208 + i]);
    EntityDelete(&gEwramData->entityData[19]);
    EquipmentMenu_DrawEquippedIcons(stats, page);
    EquipmentMenu_PrintEquippedNames();
    EquipmentMenu_ShowSlotDescription(stats, page);
}
