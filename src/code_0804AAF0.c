#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "syscalls.h"

struct MenuPage
{
    u8 pad_0[0xA];
    u8 state;
    u8 subState;
    u8 stage;
    u8 pad_D[0x15 - 0xD];
    s8 cursor;
    s8 top;
    u8 item;
    u8 pad_18[0x39 - 0x18];
    s8 unk_39;
};

extern const u8 sWeaponInfo[][28];
extern const u8 sArmorInfo[][20];
extern const u16 sItemDescriptionTextIds[];

extern void EquipmentMenu_UpdateList(struct MenuPage *page);
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

/**
 * @brief 4AAF0 | Equipment page update: A opens the item list of the
 * current slot (EquipmentMenu_UpdateList while unk_A == 0xA), up/down switches the
 * slot (weapon / armor / accessory), B leaves.
 *
 * @return -1 when the page is left, 0 otherwise
 */
s32 EquipmentMenu_Update(struct MenuPage *page)
{
    struct PlayerStatus *stats;
    s32 repeated;
    s32 new;
    s32 ret;
    s32 text;
    s32 type;
    const u8 *info;

    stats = &gEwramData->playerStatus;
    repeated = gEwramData->inputData.repeatedInput;
    new = gEwramData->inputData.newInput;
    ret = 0;
    if (page->state == 0xA)
    {
        EquipmentMenu_UpdateList(page);
        MenuPageTitle_Update(&gEwramData->entityData[20]);
        if (gEwramData->frameCounter & 2)
        {
            struct EwramData *ew = gEwramData;
            struct EwramData_EntityData *ent = &ew->entityData[18];
            ew->entityData[18].posX.part16.integer = 0x16;
            ew->entityData[18].posY.part16.integer = page->subState * 16 + 0x2B;
            Sprite_Draw(ent);
        }
        return 0;
    }
    if (new & 2)
    {
        PlaySong(0xF0);
        MenuPageTitle_Delete(&gEwramData->entityData[20]);
        MenuItemIcon_Delete();
        MenuSlotIcons_Delete();
        ret = -1;
    }
    else if (new & 1)
    {
        PlaySong(0xF3);
        page->state = 0xA;
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
        page->stage = 0;
    }
    else if (repeated & 0xC0)
    {
        PlaySong(0xF1);
        page->top = ret;
        page->cursor = ret;
        page->item = 0xFF;
        page->unk_39 = ret;
        {
            s32 b = page->subState;
            page->subState = Mod((repeated & 0x40) ? b + 2 : b + 1, 3);
        }
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
        EquipmentMenu_DrawList(stats, page, page->subState);
        MenuList_DrawScrollArrows(page, 0);
        EquipmentMenu_ShowSlotDescription(stats, page);
    }
    MenuPageTitle_Update(&gEwramData->entityData[20]);
    Cursor_SetTilePosition(2, page->subState * 2 + 5);
    Cursor_Draw();
    return ret;
}
