/*
 * Equipment menu: names, descriptions, icons, stat preview, list, open / update and stat refresh.
 */
#include "gba.h"
#include "structs/ewram.h"
#include "gameplay.h"
#include "player_core.h"
#include "structs/equipment_stats.h"
#include "types.h"
#include "core.h"
#include "syscalls.h"
#include <string.h>
#include "blue_souls.h"
#include "equipment_menu.h"
#include "menu_common.h"
#include "obj_gfx.h"
#include "pause_menu.h"
#include "soul_menu.h"
#include "soul_pickup.h"
#include "text12.h"
#include "sound.h"
#include "data/data_4F1444.h"
#include "menu_page.h"

extern struct WeaponInfo sWeaponInfo[];
extern const struct ArmorInfo sArmorInfo[];

/**
 * @brief Print the equipped weapon, armor and accessory names (or the none text
 * 0x34F) into their one-row Text8 regions of the equipment menu.
 */
void EquipmentMenu_PrintEquippedNames(void)
{
    struct EwramData *ew;
    s32 text;
    s32 i;
    s32 j;
    s32 odd;
    u8 *items;
    u8 item;
    struct PlayerStatus *equip;

    ew = gEwramData;
    equip = &ew->playerStatus;
    text = 0x34F;
    if (equip->equippedWeapon != 0xFF)
    {
        struct WeaponInfo *w = &((struct WeaponInfo *)sWeaponInfo)[equip->equippedWeapon];
        text = sItemNameTextIds[w->name];
    }
    Text8_SetRegion(0x15, 8, 0xE, 1);
    Text8_ClearRegion();
    Text8_SetPixelCursor(0x80, 8);
    Text8_Print(Text_GetLocalizedEntry(text), 0);
    i = 1;
    items = &ew->playerStatus.equippedArmor;
loop:
    {
        text = 0x34F;
        j = i - 1;
        if (items[j] != 0xFF)
        {
            const struct ArmorInfo *a = &((const struct ArmorInfo *)sArmorInfo)[items[j]];
            text = sItemNameTextIds[a->name];
        }
        odd = (i + 1) & 1;
        Text8_SetRegion(odd ? 0x15 : 0, ((i + 1) >> 1) + 8, 0xE, 1);
        Text8_ClearRegion();
        if (odd)
            Text8_SetPixelCursor(0x80, 9);
        Text8_Print(Text_GetLocalizedEntry(text), 0);
    }
    if (++i <= 2)
        goto loop;
}

/* EquipmentMenu_ShowSlotDescription; slot switch as in
 * EquipmentMenu_DrawEquippedIcons. */

extern void MenuItemIcon_SetHidden(u8 value);

/* The six equipped ids of PlayerStatus read as one array (weapon, red, blue
 * and yellow soul, armor, accessory); the page entity's subState is the slot. */
struct EquippedItemIds {
    u8 ids[6];
};

/**
 * @brief Show the description and icon of the equipment slot under the cursor
 * (unk_B): slot 0 shows weapon unk_C[0] (type 3), slots 1-2 the
 * armor/accessory unk_C[slot + 3] (type 4), each named through
 * sItemDescriptionTextIds; an empty slot (0xFF) shows text 0x351 and hides the icon
 * (MenuItemIcon_SetHidden(1)).
 */
void EquipmentMenu_ShowSlotDescription(struct PlayerStatus *stats, struct EwramData_EntityData *page)
{
    s32 text;
    s32 item;
    s32 type;
    s32 slot;

    Text12_ClearRegion();
    text = 0x351;
    item = -1;
    slot = page->subState;
    switch (slot)
    {
    case 0:
        if (stats->equippedWeapon != 0xFF)
        {
            s32 id = stats->equippedWeapon;
            struct WeaponInfo *info = &sWeaponInfo[id];
            text = sItemDescriptionTextIds[info->name];
            type = 3;
            item = id;
        }
        break;
    case 1:
    case 2:
        if (((struct EquippedItemIds *)&stats->equippedWeapon)->ids[slot + 3] != 0xFF)
        {
            s32 id = ((struct EquippedItemIds *)&stats->equippedWeapon)->ids[slot + 3];
            const struct ArmorInfo *info = &sArmorInfo[id];
            text = sItemDescriptionTextIds[info->name];
            type = 4;
            item = id;
        }
        break;
    }
    MenuText_ClearAndPrint(text);
    if (item != -1)
    {
        MenuItemIcon_Show(0x1C, 0x94, type, item);
        MenuItemIcon_SetHidden(0);
    }
    else
        MenuItemIcon_SetHidden(1);
}

/* EquipmentMenu_DrawEquippedIcons. */

/* Caller-owned record (asm caller in this file.s); only the item ids at
 * 0xC (slot 0) and 0x10..0x11 (slots 1-2, 0xFF = empty) are read here. */

/**
 * @brief For the three slots at y = 0x32 + 0x10 * slot draw the equipped item
 * (type 3 for slot 0, type 4 otherwise) via MenuSlotIcon_Show and clear the
 * slot's flag with MenuSlotIcon_SetHidden, or set the flag when the slot is empty.
 */
void EquipmentMenu_DrawEquippedIcons(struct PlayerStatus *stats, void *unusedPage)
{
    s32 i;
    s32 y;
    s32 type;
    s32 item;

    i = 0;
    y = 0x32;
    for (; i <= 2; y += 0x10, i++)
    {
        item = -1;
        switch (i)
        {
        case 0:
            if (stats->equippedWeapon != 0xFF)
            {
                type = 3;
                item = stats->equippedWeapon;
            }
            break;
        case 1:
        case 2:
            if (((struct EquippedItemIds *)&stats->equippedWeapon)->ids[i + 3] != 0xFF)
            {
                type = 4;
                item = ((struct EquippedItemIds *)&stats->equippedWeapon)->ids[i + 3];
            }
            break;
        }
        if (item != -1)
        {
            MenuSlotIcon_Show(0x70, y, type, item, i);
            MenuSlotIcon_SetHidden(0, i);
        }
        else
            MenuSlotIcon_SetHidden(1, i);
    }
}


extern void HudMap_PrintNumber(s32 a, s32 b, s32 c, s32 d, s32 e, s32 f);

/**
 * @brief Temporarily equip the highlighted item, refresh stats/yellow soul and draw stat values and comparison indicators, then restore equipment and recalculate.
 */
void EquipmentMenu_DrawStatPreview(struct EwramData_EntityData *page)
{
    s16 *stats;
    s16 old[6];
    u8 saved[2];
    u8 savedWeapon;
    u8 *weapon;
    u8 *armor;
    u8 *accessory;
    s32 i;
    s16 kind;
    s32 color;
    s32 value;
    s16 diff;
    s32 row;

    stats = &gEwramData->playerState.currentATK;
    for (i = 0; i < 6; i++)
        old[i] = stats[i];
    weapon = &gEwramData->playerStatus.equippedWeapon;
    savedWeapon = *weapon;
    armor = &gEwramData->playerStatus.equippedArmor;
    saved[0] = *armor;
    accessory = &gEwramData->playerStatus.equippedAccessory;
    saved[1] = *accessory;
    switch (page->subState)
    {
    case 0:
        *weapon = page->ext.menuList.item;
        break;
    case 1:
        *armor = page->ext.menuList.item;
        break;
    case 2:
        *accessory = page->ext.menuList.item;
        break;
    }
    Player_RefreshEquipmentStats();
    YellowSoul_Restart(gEwramData->playerStatus.equippedYellowSoul, 1);
    for (i = 0; i < 6; i++)
    {
        diff = stats[i] - old[i];
        if (diff == 0)
        {
            kind = 0;
            color = 3;
        }
        else if (diff > 0)
        {
            kind = 1;
            color = 9;
        }
        else
        {
            kind = 2;
            color = 8;
        }
        value = stats[i];
        if (value > 1000)
            value = 999;
        if (i <= 1)
        {
            row = i + 2;
            HudMap_DrawStatChangeIndicator(0x18, row, kind);
            HudMap_PrintNumber(0x1B, row, value, 3, color, 0);
        }
        else
        {
            row = i + 3;
            HudMap_DrawStatChangeIndicator(0x18, row, kind);
            HudMap_PrintNumber(0x1B, row, value, 3, color, 0);
        }
    }
    gEwramData->playerStatus.equippedWeapon = savedWeapon;
    gEwramData->playerStatus.equippedArmor = saved[0];
    gEwramData->playerStatus.equippedAccessory = saved[1];
    Player_RefreshEquipmentStats();
    YellowSoul_Restart(gEwramData->playerStatus.equippedYellowSoul, 1);
}

/* Equipment item list page. */

/**
 * @brief 4A2A4 | Draw the item list of an equipment slot (0 weapon,
 * 1 armor, 2 accessory): an optional "none" row on the first page, the
 * equipped items, then the owned items with their counts (8 rows), and
 * store the number of entries and the item under the cursor in page.
 */
void EquipmentMenu_DrawList(struct PlayerStatus *stats, struct EwramData_EntityData *page, s32 slot)
{
    const struct ArmorInfo *table;
    s32 count;
    s32 type;
    u8 *owned;
    u8 *equipped;
    s32 row;
    s32 idx;
    s32 k;
    s32 i;
    s32 j;
    s32 found;
    s32 slots;
    u16 yy;
    const u16 *info;

    /* kept in a variable: it gets no register, reload substitutes the
     * constant (cmp #4 / blt and unfolded loop guards as in the original) */
    slots = 4;
    row = 0;
    page->ext.menuList.item = 0xFF;
    switch (slot)
    {
    case 0:
        table = (const struct ArmorInfo *)sWeaponInfo;
        count = 0x3B;
        type = 2;
        owned = (u8 *)stats + 0x58;
        equipped = (u8 *)stats + 0x174;
        break;
    case 1:
        table = sArmorInfo;
        count = 0x2D;
        type = 0;
        owned = (u8 *)stats + 0x93;
        equipped = (u8 *)stats + 0x178;
        break;
    case 2:
        table = sArmorInfo;
        count = 0x2D;
        type = 1;
        owned = (u8 *)stats + 0x93;
        equipped = (u8 *)stats + 0x17C;
        break;
    }
    if (page->ext.menuList.top == 0)
    {
        Text8_SetRegion(0, 10, 13, 1);
        Text8_ClearRegion();
        Text8_Print(Text_GetLocalizedEntry(0x34F), 0);
        HudMap_ClearTilesLeft((row & 1) ? 0x1B : 0xD, (row >> 1) + 0xB, 2);
        row = 1;
    }
    else
        row = 0;
    idx = 1;
    page->ext.menuList.item = 0xFF;
    k = 0;
    for (; idx < page->ext.menuList.top && k < slots; k++)
    {
        if (equipped[k] != 0xFF && owned[equipped[k]] != 0)
            idx++;
    }
    for (i = 0; idx < page->ext.menuList.top && i < count; i++)
    {
        if (owned[i] != 0 && (slot == 0 || table[i].type == type))
        {
            idx++;
            for (j = 0; j < slots; j++)
            {
                if (equipped[j] != 0xFF && i == equipped[j])
                    idx--;
            }
        }
    }
    for (; row <= 7 && k < slots; k++)
    {
        s32 odd;
        s32 x;

        if (equipped[k] != 0xFF && owned[equipped[k]] != 0 && (slot == 0 || table[equipped[k]].type == type))
        {
            if (idx == page->ext.menuList.cursor)
                page->ext.menuList.item = equipped[k];
            if (slot != 0)
                info = &table[equipped[k]].name;
            else
                info = &sWeaponInfo[equipped[k]].name;
            do /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
                odd = row & 1;
            while (0); /* register priority */
            x = odd ? 0x15 : 0;
            yy = ((row + 4) >> 1) + 8;
            Text8_SetRegion(x, yy, 13, 1);
            Text8_ClearRegion();
            if (odd)
                Text8_SetPixelCursor((row & 1) << 7, yy);
            Text8_Print(Text_GetLocalizedEntry(sItemNameTextIds[*info]), 0);
            HudMap_PrintPaddedNumber(odd ? 0x1B : 0xD, (row >> 1) + 0xB, owned[equipped[k]], 1);
            idx++;
            row++;
        }
    }
    for (; row <= 7 && i < count; i++)
    {
        if (owned[i] != 0 && (slot == 0 || table[i].type == type))
        {
            s32 dup;

            dup = 0;
            for (j = 0; j < slots; j++)
            {
                if (equipped[j] != 0xFF && i == equipped[j])
                    dup = 1;
            }
            if (!dup)
            {
                s32 odd;
                s32 x;

                if (idx == page->ext.menuList.cursor)
                    page->ext.menuList.item = i;
                do /* FAKEMATCH: register priority */
                {
                    if (slot != 0)
                        info = &table[i].name;
                    else
                        info = &sWeaponInfo[i].name;
                } while (0);
                odd = row & 1;
                x = odd ? 0x15 : 0;
                yy = ((row + 4) >> 1) + 8;
                Text8_SetRegion(x, yy, 13, 1);
                Text8_ClearRegion();
                if (odd)
                    Text8_SetPixelCursor((row & 1) << 7, yy);
                Text8_Print(Text_GetLocalizedEntry(sItemNameTextIds[*info]), 0);
                HudMap_PrintPaddedNumber(odd ? 0x1B : 0xD, (row >> 1) + 0xB, owned[i], 1);
                idx++;
                row++;
            }
        }
    }
    for (; row <= 7; row++)
    {
        Text8_SetRegion((row & 1) << 4, (row >> 1) + 10, 0x14, 1);
        Text8_ClearRegion();
        HudMap_ClearTilesLeft((row & 1) ? 0x1B : 0xD, (row >> 1) + 0xB, 2);
    }
    for (; i < count; i++)
    {
        if (owned[i] != 0 && (slot == 0 || table[i].type == type))
        {
            found = 0;
            for (j = 0; j < slots; j++)
            {
                if (equipped[j] != 0xFF && i == equipped[j])
                    found = 1;
            }
            if (!found)
                idx++;
        }
    }
    page->ext.menuList.count = idx;
    if (idx == 1)
        page->ext.menuList.cursor = 0;
}

/* Equipment item list recount. */

/**
 * @brief 4A700 | Recount the item list of an equipment slot like
 * EquipmentMenu_DrawList without drawing: number of entries and the item under the
 * cursor.
 */
void EquipmentMenu_CountList(struct PlayerStatus *stats, struct EwramData_EntityData *page, s32 slot)
{
    const struct ArmorInfo *table;
    s32 count;
    s32 type;
    u8 *owned;
    u8 *equipped;
    s32 row;
    s32 idx;
    s32 k;
    s32 i;
    s32 j;
    s32 found;
    s32 slots;

    slots = 4;
    page->ext.menuList.item = 0xFF;
    switch (slot)
    {
    case 0:
        table = (const struct ArmorInfo *)sWeaponInfo;
        count = 0x3B;
        type = 2;
        owned = (u8 *)stats + 0x58;
        equipped = (u8 *)stats + 0x174;
        break;
    case 1:
        table = sArmorInfo;
        count = 0x2D;
        type = 0;
        owned = (u8 *)stats + 0x93;
        equipped = (u8 *)stats + 0x178;
        break;
    case 2:
        table = sArmorInfo;
        count = 0x2D;
        type = 1;
        owned = (u8 *)stats + 0x93;
        equipped = (u8 *)stats + 0x17C;
        break;
    }
    if (page->ext.menuList.top == 0)
        row = 1;
    else
        row = 0;
    idx = 1;
    page->ext.menuList.item = 0xFF;
    k = 0;
    for (; idx < page->ext.menuList.top && k < slots; k++)
    {
        if (equipped[k] != 0xFF && owned[equipped[k]] != 0)
            idx++;
    }
    for (i = 0; idx < page->ext.menuList.top && i < count; i++)
    {
        if (owned[i] != 0 && (slot == 0 || table[i].type == type))
        {
            idx++;
            for (j = 0; j < slots; j++)
            {
                if (equipped[j] != 0xFF && i == equipped[j])
                    idx--;
            }
        }
    }
    for (; row <= 7 && k < slots; k++)
    {
        s32 v;

        if (equipped[k] != 0xFF && owned[v = equipped[k]] != 0 && (slot == 0 || table[v].type == type))
        {
            if (idx == page->ext.menuList.cursor)
                page->ext.menuList.item = v;
            idx++;
            row++;
        }
    }
    for (; row <= 7 && i < count; i++)
    {
        if (owned[i] != 0 && (slot == 0 || table[i].type == type))
        {
            s32 dup;

            dup = 0;
            for (j = 0; j < slots; j++)
            {
                if (equipped[j] != 0xFF && i == equipped[j])
                    dup = 1;
            }
            if (!dup)
            {
                if (idx == page->ext.menuList.cursor)
                    page->ext.menuList.item = i;
                idx++;
                row++;
            }
        }
    }
    for (; i < count; i++)
    {
        if (owned[i] != 0 && (slot == 0 || table[i].type == type))
        {
            found = 0;
            for (j = 0; j < slots; j++)
            {
                if (equipped[j] != 0xFF && i == equipped[j])
                    found = 1;
            }
            if (!found)
                idx++;
        }
    }
    page->ext.menuList.count = idx;
    if (idx == 1)
        page->ext.menuList.cursor = 0;
}


extern void HudMap_WriteTiles(s32 x, s32 y, u16 tile, s32 count);

/**
 * @brief 4A95C | Open the equipment page for slot unk_B (0 weapon,
 * 1 armor, 2 accessory): draw the frame and labels, the item list and the
 * stat preview.
 */
void EquipmentMenu_Open(struct EwramData_EntityData *page)
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


extern void EquipmentMenu_UpdateList(struct EwramData_EntityData *page);

/**
 * @brief 4AAF0 | Equipment page update: A opens the item list of the
 * current slot (EquipmentMenu_UpdateList while unk_A == 0xA), up/down switches the
 * slot (weapon / armor / accessory), B leaves.
 *
 * @return -1 when the page is left, 0 otherwise
 */
s32 EquipmentMenu_Update(struct EwramData_EntityData *page)
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
        if (page->ext.menuList.item != 0xFF)
        {
            if (page->subState == 0)
            {
                info = (const u8 *)&sWeaponInfo[page->ext.menuList.item];
                type = 3;
            }
            else
            {
                info = (const u8 *)&sArmorInfo[page->ext.menuList.item];
                type = 4;
            }
            text = sItemDescriptionTextIds[*(u16 *)info];
            MenuItemIcon_Show(0x1C, 0x94, type, page->ext.menuList.item);
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
        page->ext.menuList.top = ret;
        page->ext.menuList.cursor = ret;
        page->ext.menuList.item = 0xFF;
        MENU_PAGE_DRAWN_TOP(page) = ret;
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

/* Aggregate equipment bonuses, optionally replacing one equipped slot. */
/**
 * @brief Clear the shared preview and aggregate equipped bonuses with an optional category override; exclude weapon flags and preserve unchecked category indexing.
 */
void Equipment_PrepareBonuses(s32 category, s32 item)
{
    struct EwramData *ewram = gEwramData;
    struct EquipmentStats *preview = (struct EquipmentStats *)ewram->sharedScratchBuffer;
    struct PlayerStatus *player = &ewram->playerStatus;
    u8 equipped[3];
    s32 i;
    memset(preview, 0, sizeof(*preview));
    equipped[0] = player->equippedWeapon;
    equipped[1] = player->equippedArmor;
    equipped[2] = player->equippedAccessory;
    if (category >= 0)
        equipped[category] = item;
    for (i = 0; i <= 2; i++)
    {
        if (equipped[i] != 0xFF)
        {
            const struct ArmorInfo *bonus;
            if (i == 0)
                bonus = (const struct ArmorInfo *)&sWeaponInfo[equipped[0]];
            else
                bonus = &sArmorInfo[equipped[i]];
            preview->attack = bonus->attack + preview->attack;
            preview->defense = bonus->defense + preview->defense;
            preview->stats[0] = preview->stats[0] + bonus->stats[0];
            preview->stats[1] = preview->stats[1] + bonus->stats[1];
            preview->stats[2] = preview->stats[2] + bonus->stats[2];
            preview->stats[3] = preview->stats[3] + bonus->stats[3];
            if (i != 0)
                preview->flags |= bonus->flags;
        }
    }
}

static inline void CalculateStats(s32 category, s32 item)
{
    struct EwramData *data = gEwramData;
    struct EquipmentStats *stats = (struct EquipmentStats *)data->sharedScratchBuffer;
    struct PlayerStatus *player = &data->playerStatus;
    Equipment_PrepareBonuses(category, item);
    stats->stats[0] = stats->stats[0] + player->baseStats[0];
    stats->stats[1] = stats->stats[1] + player->baseStats[1];
    stats->stats[2] = stats->stats[2] + player->baseStats[2];
    stats->stats[3] = stats->stats[3] + player->baseStats[3];
}

/**
 * @brief Prepare equipped bonuses plus base stats, copy prepared fields to player state, apply prepared stats and recalculate attack/defense.
 */
void Player_RefreshEquipmentStats(void)
{
    struct EwramData **ewram = &gEwramData;
    struct EwramData *data = *ewram;
    struct EquipmentStats *preview = (struct EquipmentStats *)data->sharedScratchBuffer;
    CalculateStats(-1, 0);
    {
        struct EwramData *current = *ewram;
        current->playerState.preparedStats[0] = preview->stats[0];
        current->playerState.preparedStats[1] = preview->stats[1];
        current->playerState.preparedStats[2] = preview->stats[2];
        current->playerState.preparedStats[3] = preview->stats[3];
        current->playerState.preparedResistances = preview->flags;
        current->playerState.equipmentAttackBonus = preview->attack;
        current->playerState.equipmentDefenseBonus = preview->defense;
    }
    Player_ApplyPreparedStats();
    Player_RecalculateAttackDefense();
}

/* Four-entry equipment lists: insert once into an empty slot, or slot 3. */

/**
 * @brief Select the equipment list from item type/record, avoid duplicates and insert into its first 0xFF slot or replace slot three when full.
 */
void EquipmentList_Insert(s32 type, s32 item)
{
    struct PlayerStatus *player = &gEwramData->playerStatus;
    u8 *list = NULL;
    const u8 *info = Item_GetInfo(type, item);
    s32 category = info[8];
    s32 i;
    if (type == 3)
        category = 0;
    else if (category == 0)
        category = 1;
    else
        category = 2;
    switch (category)
    {
        case 0:
            list = player->weaponList;
            break;
        case 1:
            list = player->armorList;
            break;
        case 2:
            list = player->accessoryList;
            break;
    }
    for (i = 0; i <= 3; i++)
    {
        if (list[i] == item)
            return;
    }
    for (i = 0; i <= 3; i++)
    {
        if (list[i] == 0xFF)
        {
            list[i] = item;
            break;
        }
    }
    if (i == 4)
        list[3] = item;
}
