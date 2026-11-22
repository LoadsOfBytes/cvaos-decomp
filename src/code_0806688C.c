#include "types.h"

void PlaySong(u32 song);
u32 Inventory_GetOwnedCount(s32 category, s32 index);

struct ShopStockEntry {
    u8 type;
    u8 id;
};
extern const struct ShopStockEntry sShopStock[];

struct Owner {
    u8 pad_0[0x3D1];
    u8 rowQuantities[1];
};

struct ShopMenu {
    u8 pad_0[0xA];
    u8 state;
    u8 pad_B[0x14 - 0xB];
    s8 count;
    s8 visible;
    s8 top;
    u8 cursor;
};

/**
 * @brief Move the list cursor or change the quantity; return 0 nothing, 1
 * quantity changed, 2 cursor moved, 3 list scrolled.
 */
s32 Shop_HandleListInput(struct Owner *owner, struct ShopMenu *menu, void *unused, u8 *list, s32 input)
{
    const struct ShopStockEntry *entry;
    s32 owned;
    s32 limit;
    u8 *quantity;
    s32 top;
    s32 ret;

    list++;
    entry = &sShopStock[list[menu->cursor]];
    owned = Inventory_GetOwnedCount(entry->type, entry->id);
    limit = owned;
    quantity = &owner->rowQuantities[menu->cursor];
    top = menu->top;
    ret = 0;

    if ((input & 0x10) && *quantity != 0)
    {
        if (menu->state == 2)
        {
            if (*quantity < owned)
            {
                (*quantity)++;
                ret = 1;
            }
        }
        else if (*quantity + limit <= 8)
        {
            (*quantity)++;
            ret = 1;
        }
    }
    else if ((input & 0x20) && *quantity > 1)
    {
        (*quantity)--;
        ret = 1;
    }
    else if (input & 0x40)
    {
        if (menu->cursor != 0)
        {
            menu->cursor--;
            if (menu->top > menu->cursor)
                menu->top = menu->cursor;
            ret = 2;
        }
    }
    else if (input & 0x80)
    {
        if (menu->cursor < menu->count - 1)
        {
            menu->cursor++;
            if (menu->top < menu->cursor - 3)
                menu->top = menu->cursor - 3;
            ret = 2;
        }
    }
    else if (input & 0x100)
    {
        if (menu->cursor < menu->count - 1)
        {
            menu->top += 4;
            menu->cursor += 4;
            if (menu->top > menu->count - 4)
            {
                menu->top = menu->count - 4;
                menu->cursor = menu->count - 1;
            }
            ret = 2;
        }
    }
    else if (input & 0x200)
    {
        if (menu->cursor != 0)
        {
            if (menu->top <= 3)
            {
                menu->top = 0;
                menu->cursor = 0;
            }
            else
            {
                menu->top -= 4;
                menu->cursor -= 4;
            }
            ret = 2;
        }
    }
    menu->visible = menu->cursor - menu->top;
    if (menu->visible >= menu->count && menu->count != 0)
    {
        menu->cursor = menu->visible = menu->count - 1;
        ret = 3;
    }
    if (menu->top != top)
        ret = 3;
    if (ret != 0)
        PlaySong(0xF1);
    return ret;
}
