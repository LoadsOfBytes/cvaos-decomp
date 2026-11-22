#ifndef MENU_PAGE_H
#define MENU_PAGE_H

/*
 * Work slots of a pause-menu page entity, the same bytes that the TU-local
 * `struct MenuPage` views call count/cursor/top/item (offsets 0x14-0x17 from
 * the entity start). MenuPage_Reset clears category and these four bytes;
 * MenuPage_SetMode stores the initial cursor/mode and clears top.
 * - category (unk_4F7): soul menu list category (SoulMenu_CountList/DrawList);
 * - count (unk_4F8): entries on the page; cursor (unk_4F9): selected entry,
 *   wraps at count; top (unk_4FA): first visible entry / scroll; item
 *   (unk_4FB): number of listed items (ItemMenu/SoulMenu_CountList).
 * Expansion is token-identical (no added parentheses); plain identifiers only.
 */
#define MENU_PAGE_CATEGORY(e) e->unk_4F4.part8.unk_4F7
#define MENU_PAGE_COUNT(e) e->unk_4F8.part8.unk_4F8
#define MENU_PAGE_CURSOR(e) e->unk_4F8.part8.unk_4F9
#define MENU_PAGE_TOP(e) e->unk_4F8.part8.unk_4FA
#define MENU_PAGE_ITEM(e) e->unk_4F8.part8.unk_4FB

#endif /* MENU_PAGE_H */
