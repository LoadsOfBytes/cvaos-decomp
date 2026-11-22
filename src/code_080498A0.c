#include "gba.h"
#include "structs/ewram.h"
#include "code_08032444.h"

struct SoulMenu
{
    u8 pad[0x13];
    u8 type;
    u8 count;
    s8 cursor;
    u8 pad16;
    u8 selected;
};

/**
 * @brief Before equipping, update the four-entry list of the menu's soul type:
 * drop the selected soul when only one is owned and the currently equipped
 * soul, compact the entries and insert the currently equipped soul (at the
 * front when the list is full).
 */
void SoulList_UpdateForEquip(struct SoulMenu *menu)
{
    u8 *equip;
    u8 *hist;
    u8 cur;
    u8 empty;
    s32 type;
    s32 i;

    equip = (u8 *)gEwramData + 0x1325C;
    empty = 0;
    switch (menu->type)
    {
        case 0:
            type = 0;
            hist = (u8 *)gEwramData + 0x133DC;
            cur = equip[0xD];
            break;
        case 1:
            type = 1;
            hist = (u8 *)gEwramData + 0x133E0;
            cur = equip[0xE];
            break;
        case 2:
            type = 2;
            hist = (u8 *)gEwramData + 0x133E4;
            cur = equip[0xF];
            break;
    }
    for (i = 0; i < 4; i++)
    {
        if (hist[i] == menu->selected && SoulInventory_GetSoulTotal(type, hist[i] - 1) == 1)
            hist[i] = 0xFF;
        if (hist[i] == cur)
            hist[i] = 0xFF;
        if (hist[i] == 0xFF)
            empty++;
    }
    for (i = 2; i >= 0; i--)
    {
        while (hist[i + 1] == 0xFF && hist[i] != 0xFF)
        {
            hist[i + 1] = hist[i];
            hist[i] = 0xFF;
            if (i <= 1)
                i++;
        }
    }
    if (empty == 0)
    {
        for (i = 2; i >= 0; i--)
        {
            if (hist[i] != 0xFF)
                hist[i + 1] = hist[i];
        }
        hist[0] = cur;
    }
    else
    {
        hist[empty - 1] = cur;
    }
}
