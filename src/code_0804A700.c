/* Equipment item list recount. */
#include "gba.h"
#include "structs/ewram.h"

struct MenuPage
{
    u8 pad_0[0x14];
    s8 count;
    s8 cursor;
    s8 top;
    u8 item;
};

struct Item20
{
    u16 name;
    u8 pad_2[6];
    u8 type;
    u8 pad_9[0x14 - 0x9];
};

extern const u8 sWeaponInfo[];
extern const struct Item20 sArmorInfo[];

/**
 * @brief 4A700 | Recount the item list of an equipment slot like
 * EquipmentMenu_DrawList without drawing: number of entries and the item under the
 * cursor.
 */
void EquipmentMenu_CountList(struct PlayerStatus *stats, struct MenuPage *page, s32 slot)
{
    const struct Item20 *table;
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
    page->item = 0xFF;
    switch (slot)
    {
    case 0:
        table = (const struct Item20 *)sWeaponInfo;
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
    if (page->top == 0)
        row = 1;
    else
        row = 0;
    idx = 1;
    page->item = 0xFF;
    k = 0;
    for (; idx < page->top && k < slots; k++)
    {
        if (equipped[k] != 0xFF && owned[equipped[k]] != 0)
            idx++;
    }
    for (i = 0; idx < page->top && i < count; i++)
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
            if (idx == page->cursor)
                page->item = v;
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
                if (idx == page->cursor)
                    page->item = i;
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
    page->count = idx;
    if (idx == 1)
        page->cursor = 0;
}
