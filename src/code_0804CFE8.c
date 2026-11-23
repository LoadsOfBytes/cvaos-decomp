#include "code/code_08039340.h"
#include "menu_page.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
#include "structs/entity_template.h"

extern const u16 sItemNameTextIds[];

/* Bit N of a u32 set; the word offset is computed before the set address,
 * as in the original. */
#define TestBit(SET, N) ({ u32 o_ = ((N) >> 5) * 4; u32 *b_ = (SET); *(u32 *)((u8 *)b_ + o_) & (1 << ((N) & 0x1F)); })

/**
 * @brief Print the two drop item names of the selected enemy; a drop not yet
 * obtained (bit sets enemyFlags[4..7] / [8..11]) prints text 10, no drop
 * prints the none text 0x34F.
 */
void BestiaryMenu_PrintDrops(struct EwramData_EntityData *menu)
{
    struct EwramData *ewram = gEwramData;
    struct PlayerStatus *player = &ewram->playerStatus;
    u8 index = MENU_PAGE_CURSOR(menu);
    struct EntityTemplate *template = &sEnemyData[index];
    s32 text;

    Text8_SetRegion(0, 9, 14, 1);
    Text8_ClearRegion();
    if (template->dropItem0 == 0)
        text = 0x34F;
    else if (TestBit(&ewram->playerStatus.enemyFlags[4], index))
        text = sItemNameTextIds[template->dropItem0 - 1];
    else
        text = 10;
    Text8_Print(Text_GetLocalizedEntry(text), 0);
    Text8_SetRegion(0x15, 9, 14, 1);
    Text8_ClearRegion();
    Text8_SetPixelCursor(0x80, 9);
    if (template->dropItem1 == 0)
        text = 0x34F;
    else if (TestBit(&player->enemyFlags[8], index))
        text = sItemNameTextIds[template->dropItem1 - 1];
    else
        text = 10;
    Text8_Print(Text_GetLocalizedEntry(text), 0);
}
