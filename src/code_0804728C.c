/* Menu text redraw and transient entity-byte reset helpers.
 * for source evidence. */
#include "code/code_08039340.h"
#include "menu_page.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

/**
 * @brief Clear the configured Text12 region, then print the selected localized menu entry.
 */
void MenuText_ClearAndPrint(s32 index)
{
    Text12_ClearRegion();
    MenuText_Print(index);
}

/**
 * @brief Clear the menu page cursor bytes unk_4F7..unk_4FB.
 */
void MenuPage_Reset(struct EwramData_EntityData *entity)
{
    MENU_PAGE_CATEGORY(entity) = 0;
    MENU_PAGE_COUNT(entity) = 0;
    MENU_PAGE_CURSOR(entity) = 0;
    MENU_PAGE_TOP(entity) = 0;
    MENU_PAGE_ITEM(entity) = 0;
}
