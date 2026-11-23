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

/*
 * The same four bytes read as an array (cursor drawing, list counting):
 * [0] = count, [1] = cursor, [2] = top, [3] = item.
 */

/*
 * Graphics slot of the menu sprites, loaded into the cursor entity (slot 18)
 * by MenuCursor_LoadGraphics and used for every menu sprite; the item icon
 * entities keep their icon graphics slot in the same byte.
 * Button config page: the action of each of its four rows (index into
 * sButtonConfigCursorFrames).
 */

/*
 * Page-title entity (slot 20; MenuPageTitle_Create / _Update / _Delete):
 * state (1 = place and start the zoom-in, 2 = zooming), its affine slot as
 * allocated / as the u16 slot number read by the update.
 */
#define MENU_TITLE_AFFINE(e) e->unk_510.whole
#define MENU_TITLE_AFFINE_SLOT(e) e->unk_510.part16.unk_510

#endif /* MENU_PAGE_H */
