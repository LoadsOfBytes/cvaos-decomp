/* SoulMenu_DrawCategoryIcon */
#include "code/code_08039340.h"
#include "menu_page.h"
#include "structs/ewram.h"

/**
 * @brief 4933C | Draw the category icon entityData[21] at (0x15, 0x20):
 * frame 0x1B/0x1C/0x1D with palette 10/11/12 for the red/blue/yellow soul
 * category (unk_4F7 while a list is open, else the cursor row unk_4F9).
 */
void SoulMenu_DrawCategoryIcon(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *cursor = &gEwramData->entityData[21];
    s32 kind;

    cursor->affine = 0;
    cursor->posX.whole = 0x150000;
    cursor->posY.whole = 0x200000;
    if (entity->subState == 2)
        kind = MENU_PAGE_CATEGORY(entity);
    else
        kind = entity->unk_4F8.bytes[1];
    switch (kind)
    {
        case 0:
            cursor->palette = 10;
            cursor->animFrame = 0x1B;
            break;
        case 1:
            cursor->palette = 11;
            cursor->animFrame = 0x1C;
            break;
        case 2:
            cursor->palette = 12;
            cursor->animFrame = 0x1D;
            break;
    }
    Sprite_Draw(cursor);
}
