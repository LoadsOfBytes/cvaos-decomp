#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"

struct MenuPage
{
    u8 pad_0[0x14];
    s8 count;
    s8 cursor;
    s8 top;
    u8 item;
    u8 pad_18[0x39 - 0x18];
    s8 unk_39;
};

struct ItemInfo
{
    u16 name;
    u8 oneBasedIconId;
    u8 pad_3[0x10 - 0x3];
};

extern const struct ItemInfo sConsumableInfo[];
extern const u16 sItemDescriptionTextIds[];

extern s32 ItemMenu_UseItem(struct PlayerStatus *stats, struct MenuPage *page);
extern void ItemMenu_DrawList(struct PlayerStatus *stats, struct MenuPage *page);
extern void ItemMenu_CountList(struct PlayerStatus *stats, struct MenuPage *page);
extern s32 MenuGrid_MoveCursor(struct MenuPage *page, s32 repeated);
extern void Menu_DrawHpMpBars(s32 x, s32 y);
extern void HudMap_DrawStatusConditionIcon(s32 x, s32 y);
extern void MenuItemIcon_Show(s32 x, s32 y, s32 type, s32 item);
extern void MenuPageTitle_Delete(struct EwramData_EntityData *ent);
extern void MenuPageTitle_Update(struct EwramData_EntityData *ent);
extern void MenuItemIcon_Delete(void);
extern void Cursor_SetTilePosition(s32 a, s32 b);
extern void Cursor_Draw(void);

static inline void ShowItemInfo(struct MenuPage *page)
{
    Text12_ClearRegion();
    if (page->count != 0)
    {
        const struct ItemInfo *info = &sConsumableInfo[page->item];
        s32 text = sItemDescriptionTextIds[info->name];

        MenuItemIcon_Show(0x1C, 0x94, 2, info->oneBasedIconId - 1);
        MenuItemIcon_SetHidden(0);
        MenuText_ClearAndPrint(text);
    }
    else
    {
        MenuItemIcon_SetHidden(1);
    }
}

/**
 * @brief 4B808 | Item page update: A uses the selected item (one fewer in
 * itemInventory, cursor moves up when the last entry ran out), cursor
 * movement updates the item info, B leaves.
 *
 * @return -1 when the page is left, 0 otherwise
 */
s32 ItemMenu_Update(struct MenuPage *page)
{
    struct EwramData *ew;
    struct PlayerStatus *stats;
    s32 repeated;
    s32 new;
    s32 moved;
    s32 oldItem;

    ew = gEwramData;
    stats = &ew->playerStatus;
    repeated = ew->inputData.repeatedInput;
    new = ew->inputData.newInput;
    if (new & 1)
    {
        moved = ItemMenu_UseItem(stats, page);
        if (moved != 0)
        {
            ew->playerStatus.itemInventory[page->item]--;
            if (ew->playerStatus.itemInventory[page->item] == 0 && page->cursor == page->count - 1)
            {
                if (page->top != 0)
                {
                    if (page->cursor == page->top)
                        page->top -= 8;
                    page->cursor--;
                }
                else if (page->cursor != 0)
                {
                    page->cursor--;
                }
            }
            Menu_DrawHpMpBars(0xA, 4);
            HudMap_DrawStatusConditionIcon(0x14, 7);
            ItemMenu_DrawList(stats, page);
            ShowItemInfo(page);
        }
        else
        {
            PlaySong(0xEF);
        }
    }
    else
    {
        moved = MenuGrid_MoveCursor(page, repeated);
        if (new & 2)
        {
            MenuItemIcon_Delete();
            MenuPageTitle_Delete(&gEwramData->entityData[20]);
            PlaySong(0xF0);
            return -1;
        }
        if (moved)
            PlaySong(0xF1);
    }
    if (moved)
    {
        oldItem = page->item;
        if (page->unk_39 != page->top)
        {
            page->unk_39 = page->top;
            ItemMenu_DrawList(stats, page);
        }
        else
        {
            ItemMenu_CountList(&gEwramData->playerStatus, page);
        }
        if (oldItem != page->item)
            ShowItemInfo(page);
    }
    Cursor_SetTilePosition((page->cursor & 1) ? 0x10 : 1, ((page->cursor - page->top) >> 1) + 0xB);
    Cursor_Draw();
    MenuPageTitle_Update(&gEwramData->entityData[20]);
    return (new & 2) ? -1 : 0;
}
