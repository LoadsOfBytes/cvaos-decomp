/* Equipment menu input handler: A equips the highlighted item (weapon or
 * armor/accessory slot unk_B - 1), A/B closes the list and redraws the page,
 * cursor moves refresh the item description. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "syscalls.h"
#include "code_080D73B8.h"

extern void MenuItemIcon_SetHidden(u8 value);

struct MenuPage
{
    u8 pad_0[0xA];
    u8 state;
    u8 subState;
    u8 stage;
    u8 pad_D[0x14 - 0xD];
    s8 count;
    s8 cursor;
    s8 top;
    u8 item;
    u8 pad_18[0x39 - 0x18];
    s8 unk_39;
};

struct ItemInfo { u16 name; };

extern const u8 sWeaponInfo[][28];
extern const u8 sArmorInfo[][20];
extern const u16 sItemDescriptionTextIds[];

extern void MenuPageTitle_Update(struct EwramData_EntityData *ent);
extern void MenuPageTitle_Delete(struct EwramData_EntityData *ent);
extern void MenuItemIcon_Delete(void);
extern void MenuSlotIcons_Delete(void);
extern void MenuItemIcon_Show(s32 x, s32 y, s32 type, s32 item);
extern void EquipmentMenu_DrawStatPreview(struct MenuPage *page);
extern void HudMap_WriteTiles(s32 x, s32 y, u16 tile, s32 count);
extern void EquipmentMenu_DrawList(struct PlayerStatus *stats, struct MenuPage *page, s32 slot);
extern void MenuList_DrawScrollArrows(struct MenuPage *page, s32 param_1);
extern void EquipmentMenu_ShowSlotDescription(struct PlayerStatus *stats, struct MenuPage *page);
extern void Cursor_SetTilePosition(s32 a, s32 b);
extern void Cursor_Draw(void);

extern void EquipmentList_UpdateForEquip(struct MenuPage *page);
extern void EquipmentMenu_CountList(struct PlayerStatus *stats, struct MenuPage *page, s32 slot);
extern s32 MenuGrid_MoveCursor(struct MenuPage *page, s32 repeated);
extern void YellowSoul_Restart(u8 soul, s32 force);
extern void HudMap_ClearTiles(s32 x, s32 y, s32 count);
extern void EquipmentMenu_PrintEquippedNames(void);
extern void EquipmentMenu_DrawEquippedIcons(struct PlayerStatus *stats, struct MenuPage *page);
extern void Menu_DrawAttackDefense(s32 x, s32 y);
extern void Menu_DrawCurrentStats(s32 x, s32 y);

s32 EquipmentMenu_UpdateList(struct MenuPage *page)
{
    struct PlayerStatus *stats;
    s32 repeated;
    s32 new;
    s32 moved;
    s32 i;
    s32 old;
    s32 text;
    s32 type;
    const u8 *info;
    u8 *slots;

    stats = &gEwramData->playerStatus;
    repeated = gEwramData->inputData.repeatedInput;
    new = gEwramData->inputData.newInput;
    if (page->stage == 0)
    {
        Cursor_SetTilePosition((page->cursor & 1) ? 0x10 : 2, ((page->cursor - page->top) >> 1) + 0xB);
        page->stage++;
    }
    if (new & 1)
    {
        PlaySong(0xF4);
        if (page->subState == 0)
        {
            if (stats->equippedWeapon != 0xFF)
                EquipmentList_UpdateForEquip(page);
            stats->equippedWeapon = page->item;
        }
        else
        {
            s32 slot = page->subState - 1;
            slots = &stats->equippedArmor;
            if (slots[slot] != 0xFF)
                EquipmentList_UpdateForEquip(page);
            {
                s32 slot2 = page->subState - 1;
                slots[slot2] = page->item;
            }
        }
        Player_RefreshEquipmentStats();
        YellowSoul_Restart(gEwramData->playerStatus.equippedYellowSoul, 1);
    }
    moved = MenuGrid_MoveCursor(page, repeated);
    if (new & 3)
    {
        for (i = 0; i <= 6; i++)
            HudMap_ClearTiles(0x18, i + 2, 4);
        page->state = 2;
        for (i = 3; i >= 0; i--)
            HudMap_ClearTiles(0x19, i + 5, 3);
        EquipmentMenu_PrintEquippedNames();
        EquipmentMenu_ShowSlotDescription(stats, page);
        if (new & 1)
        {
            EquipmentMenu_DrawEquippedIcons(stats, page);
            PlaySong(0xF3);
        }
        else
        {
            PlaySong(0xF0);
        }
        Menu_DrawAttackDefense(0x16, 2);
        Menu_DrawCurrentStats(0x16, 5);
        if (page->cursor >= page->count)
        {
            page->cursor = page->count - 1;
            if (page->top > page->cursor)
                page->top = page->top - 2;
        }
        EquipmentMenu_DrawList(stats, page, page->subState);
        MenuList_DrawScrollArrows(page, 0);
    }
    else if (moved)
    {
        old = page->item;
        if (page->unk_39 != page->top)
        {
            page->unk_39 = page->top;
            EquipmentMenu_DrawList(stats, page, page->subState);
            MenuList_DrawScrollArrows(page, 0);
        }
        else
        {
            EquipmentMenu_CountList(&gEwramData->playerStatus, page, page->subState);
        }
        if (old != page->item)
        {
            Text12_ClearRegion();
            if (page->item != 0xFF)
            {
                if (page->subState == 0)
                {
                    info = sWeaponInfo[page->item];
                    type = 3;
                }
                else
                {
                    info = sArmorInfo[page->item];
                    type = 4;
                }
                text = sItemDescriptionTextIds[*(u16 *)info];
                MenuItemIcon_Show(0x1C, 0x94, type, page->item);
                MenuItemIcon_SetHidden(0);
            }
            else
            {
                text = 0x350;
                MenuItemIcon_SetHidden(1);
            }
            MenuText_ClearAndPrint(text);
            EquipmentMenu_DrawStatPreview(page);
        }
        Cursor_SetTilePosition((page->cursor & 1) ? 0x10 : 2, ((page->cursor - page->top) >> 1) + 0xB);
        PlaySong(0xF1);
    }
    Cursor_Draw();
    return 0;
}
