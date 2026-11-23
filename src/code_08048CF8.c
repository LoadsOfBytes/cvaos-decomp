/* Entity helpers 0x08048CF8..0x0804933B. */
#include "code_08009A0.h"
#include "menu_page.h"
#include "code_08032444.h"
#include "gba.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "menu_entity.h"
#include "structs/ewram.h"

/* Language dependent graphics sets (English/other, French, German). */
extern u8 sMenuCursorGfx[];
extern u8 sMenuCursorGfxLanguage2[];
extern u8 sMenuCursorGfxLanguage3[];

extern void SoulTypeSelector_Update(struct EwramData_EntityData *entity);
extern void MenuPageTitle_Create(struct EwramData_EntityData *title, s32 frame);
extern void MenuPage_Reset(struct EwramData_EntityData *entity);
extern void Menu_DrawAttackDefense(s32 x, s32 y);
extern void Menu_DrawCurrentStats(s32 x, s32 y);
extern void HudMap_DrawStatusConditionIcon(s32 x, s32 y);
extern void SoulMenu_ClearStatPreview(void);
extern void Text12_SetupTileRegion(s32 x, s32 y, u16 tile, s32 width, s32 height);
extern void HudMap_WriteTiles(s32 x, s32 y, u16 tile, s32 count);
extern void SoulMenu_DrawEquippedNames(struct EwramData_EntityData *entity);
extern void SoulMenu_DrawList(struct EwramData_EntityData *entity, s32 row);
extern void SoulMenu_PrintEquippedName(struct EwramData_EntityData *entity);
extern void SoulMenu_DrawEquippedSoulValue(struct EwramData_EntityData *entity);
extern void Cursor_SetTilePosition(s32 column, s32 row);
extern void MenuPageTitle_Delete(struct EwramData_EntityData *title);
extern void Cursor_Draw(void);
extern void MenuPageTitle_Update(struct EwramData_EntityData *entity);
extern void SoulTypeSelector_SetDirection(struct EwramData_EntityData *entity, s8 dir);
extern s32 MenuGrid_MoveCursor(struct EwramData_EntityData *entity, u16 input);
extern void SoulMenu_ShowSelection(struct EwramData_EntityData *entity);
void SoulMenu_CountList(struct EwramData_EntityData *entity, s32 kind);
extern void SoulMenu_DrawSelectedSoulValue(struct EwramData_EntityData *entity);
extern void SoulMenu_EquipSelected(struct EwramData_EntityData *entity);
extern void SoulMenu_DrawCategoryIcon(struct EwramData_EntityData *entity);


/**
 * @brief 48CF8 | Delete entity and release the language dependent
 * animation set loaded by CursorSprite_Create.
 */
void CursorSprite_Delete(struct EwramData_EntityData *entity)
{
    u8 *resource;

    EntityDelete(entity);
    switch (gEwramData->gameState.language)
    {
        case 2:
            resource = sMenuCursorGfxLanguage2;
            break;
        case 3:
            resource = sMenuCursorGfxLanguage3;
            break;
        default:
            resource = sMenuCursorGfx;
            break;
    }
    ObjGfx_Unload(ObjGfx_Find(resource));
}

/**
 * @brief 48D3C | Update: draw entity shifted horizontally by a
 * triangle wave of the frame counter (-3..4), then restore posX.
 */
void CursorSprite_Update(struct EwramData_EntityData *entity)
{
    s16 baseX = entity->posX.part16.integer;
    u32 phase = gEwramData->frameCounter >> 1;
    s32 wobble;

    if (phase & 8)
        wobble = 3 - (phase & 7);
    else
        wobble = (phase & 7) - 3;

    entity->posX.part16.integer = (s16)baseX + wobble;
    Sprite_Draw(entity);
    entity->posX.part16.integer = baseX;
}

/**
 * @brief 48D84 | Set up the soul/equipment menu screen for entity: reset
 * entityData[21] and [20], clear 0x2000 bytes at 0x3200 and 0x3300 (as
 * written in the original), reset the cursor state (unk_4F8 = 3, unk_4F9 =
 * unk_4F7), draw the headers, frames and the eight 10-tile rows, then the
 * lists and the equipped souls.
 */
void SoulMenu_Open(struct EwramData_EntityData *entity)
{
    s32 i;
    s32 tile;

    EntityDelete(&gEwramData->entityData[21]);
    SoulTypeSelector_Update(&gEwramData->entityData[21]);
    MenuPageTitle_Create(&gEwramData->entityData[20], 0x1F);
    DMA_FILL_32(3, 0, 0x3200, 0x2000);
    DMA_FILL_32(3, 0, 0x3300, 0x2000);
    MenuPage_Reset(entity);
    entity->ext.menuPage.count = 3;
    entity->ext.menuPage.cursor = entity->ext.menuPage.category;
    Menu_DrawAttackDefense(0x16, 2);
    Menu_DrawCurrentStats(0x16, 4);
    HudMap_DrawStatusConditionIcon(0x15, 8);
    SoulMenu_ClearStatPreview();
    Text12_SetupTileRegion(7, 15, 0x3200, 30, 2);
    HudMap_WriteTiles(6, 3, 0x3350, 10);
    HudMap_WriteTiles(6, 5, 0x3360, 10);
    HudMap_WriteTiles(6, 7, 0x3370, 10);
    for (i = 0, tile = 0x80; i <= 7; i++)
    {
        HudMap_WriteTiles((i & 1) ? 0x11 : 3, ((i + 4) >> 1) + 8, tile + 0x3300, 10);
        tile += 0x10;
    }
    SoulMenu_DrawEquippedNames(entity);
    SoulMenu_DrawList(entity, entity->ext.menuPageList.bytes[1]);
    SoulMenu_PrintEquippedName(entity);
    SoulMenu_DrawEquippedSoulValue(entity);
}

/**
 * @brief Update of the soul-set page entity: state 0 = init, 1 = choose the
 * red/blue/yellow category with up/down (A opens it, B closes the page),
 * 2 = pick a soul in the list (MenuGrid_MoveCursor moves the cursor, A equips
 * via SoulMenu_EquipSelected, A/B go back). The category row and list top are swapped
 * between unk_4F9/unk_4FA and unk_4F7/byte 0x51D on entering/leaving the
 * list. Returns -1 once the page has closed, else 0.
 */
s32 SoulMenu_Update(struct EwramData_EntityData *entity)
{
    s32 result = 0;
    u16 repeated = gEwramData->inputData.repeatedInput;
    u16 pressed = gEwramData->inputData.newInput;
    u8 tmp;

    switch (entity->subState)
    {
        case 0:
            entity->ext.menuPage.category = result;
            MENU_SAVED_TOP(entity) = result;
            entity->ext.menuPage.top = result;
            entity->ext.menuPage.count = 3;
            entity->ext.menuPage.cursor = result;
            entity->ext.menuPage.item = result;
            entity->subState++;
        case 1:
            if (pressed & KEY_A)
            {
                PlaySong(0xF3);
                entity->subState++;
                tmp = entity->ext.menuPage.cursor;
                entity->ext.menuPage.cursor = entity->ext.menuPage.category;
                entity->ext.menuPage.category = tmp;
                {
                    u8 *saved = &MENU_SAVED_TOP(entity);
                    tmp = entity->ext.menuPage.top;
                    entity->ext.menuPage.top = *saved;
                    *saved = tmp;
                }
                SoulMenu_CountList(entity, entity->ext.menuPage.category);
                SoulMenu_ShowSelection(entity);
                SoulMenu_DrawSelectedSoulValue(entity);
                Cursor_SetTilePosition((entity->ext.menuPage.cursor & 1) ? 0x10 : 2,
                             ((entity->ext.menuPageList.bytes[1] - entity->ext.menuPageList.bytes[2]) >> 1) + 10);
            }
            else if (pressed & KEY_B)
            {
                PlaySong(0xF0);
                entity->subState = 0xFF;
            }
            else if (repeated & (KEY_UP | KEY_DOWN))
            {
                s32 dir = 0;

                entity->ext.menuPage.count = 3;
                MENU_SAVED_TOP(entity) = dir;
                entity->ext.menuPage.category = dir;
                if (repeated & KEY_UP)
                {
                    if ((s8)--entity->ext.menuPage.cursor < 0)
                        entity->ext.menuPage.cursor = entity->ext.menuPage.count - 1;
                    dir = -1;
                }
                else if (repeated & KEY_DOWN)
                {
                    entity->ext.menuPage.cursor++;
                    if (entity->ext.menuPageList.bytes[1] > entity->ext.menuPageList.bytes[0] - 1)
                        entity->ext.menuPage.cursor = dir;
                    dir = 1;
                }
                SoulTypeSelector_SetDirection(&gEwramData->entityData[21], dir);
                SoulMenu_DrawEquippedNames(entity);
                SoulMenu_DrawList(entity, entity->ext.menuPageList.bytes[1]);
                SoulMenu_PrintEquippedName(entity);
                SoulMenu_DrawEquippedSoulValue(entity);
                PlaySong(0xF1);
            }
            break;
        case 2:
            if (pressed & (KEY_A | KEY_B))
            {
                if (pressed & KEY_A)
                {
                    PlaySong(0xF4);
                    SoulMenu_EquipSelected(entity);
                    SoulMenu_DrawList(entity, entity->ext.menuPage.category);
                }
                else
                {
                    PlaySong(0xF0);
                }
                entity->subState--;
                entity->ext.menuPage.count = 3;
                tmp = entity->ext.menuPage.cursor;
                entity->ext.menuPage.cursor = entity->ext.menuPage.category;
                entity->ext.menuPage.category = tmp;
                {
                    u8 *saved = &MENU_SAVED_TOP(entity);
                    tmp = entity->ext.menuPage.top;
                    entity->ext.menuPage.top = *saved;
                    *saved = tmp;
                }
                SoulMenu_DrawEquippedNames(entity);
                SoulMenu_DrawEquippedSoulValue(entity);
                SoulMenu_PrintEquippedName(entity);
                SoulMenu_ClearStatPreview();
            }
            else
            {
                s32 row = entity->ext.menuPageList.bytes[1];
                u8 last = entity->ext.menuPage.item;
                s32 top = entity->ext.menuPageList.bytes[2];

                if (MenuGrid_MoveCursor(entity, repeated) != 0 && row != entity->ext.menuPageList.bytes[1])
                {
                    PlaySong(0xF1);
                    if (top != entity->ext.menuPageList.bytes[2])
                        SoulMenu_DrawList(entity, entity->ext.menuPage.category);
                    else
                        SoulMenu_CountList(entity, entity->ext.menuPage.category);
                    if (last != entity->ext.menuPage.item)
                    {
                        SoulMenu_ShowSelection(entity);
                        SoulMenu_DrawSelectedSoulValue(entity);
                        Cursor_SetTilePosition((entity->ext.menuPage.cursor & 1) ? 0x10 : 2,
                                     ((entity->ext.menuPageList.bytes[1] - entity->ext.menuPageList.bytes[2]) >> 1) + 10);
                    }
                }
                Cursor_Draw();
            }
            break;
        case 0xFF:
        {
            struct EwramData_EntityData *cursor;

            entity->subState = 0;
            cursor = &gEwramData->entityData[21];
            cursor->state = 0xFF;
            SoulTypeSelector_Update(&gEwramData->entityData[21]);
            EntityDelete(&gEwramData->entityData[21]);
            MenuPageTitle_Delete(&gEwramData->entityData[20]);
            result = -1;
            return result;
        }
    }

    if (entity->subState != 0xFF)
    {
        MenuPageTitle_Update(&gEwramData->entityData[20]);
        SoulTypeSelector_Update(&gEwramData->entityData[21]);
        SoulMenu_DrawCategoryIcon(entity);
    }
    return result;
}

/**
 * @brief 49148 | Rebuild the soul list of category kind (0 red, 1 blue,
 * 2 yellow) for the menu entity: the equipped souls first, then every
 * other owned soul; counts the rows into unk_4F8 and stores the soul under
 * the cursor row unk_4F9 in unk_4FB, at most eight rows from the list top
 * unk_4FA. The bound n = 4 must be a variable (constant only after
 * reload, as in the original: entry tests `cmp rX, #4` stay unfolded).
 */
void SoulMenu_CountList(struct EwramData_EntityData *entity, s32 kind)
{
    struct EwramData *ewram = gEwramData;
    s32 type;
    s32 max;
    u8 *equipped;
    s32 shown;
    s32 pos;
    s32 i;
    s32 soul;
    s32 j;
    s32 found;
    s32 n = 4;

    entity->ext.menuPage.item = 0xFF;
    switch (kind)
    {
        case 0:
            type = 0;
            max = 0x38;
            equipped = ewram->playerStatus.redSoulList;
            break;
        case 1:
            type = 1;
            max = 0x19;
            equipped = ewram->playerStatus.blueSoulList;
            break;
        case 2:
            type = 2;
            max = 0x24;
            equipped = ewram->playerStatus.yellowSoulList;
            break;
    }

    if (entity->ext.menuPageList.bytes[2] == 0)
        shown = 1;
    else
        shown = 0;
    pos = 1;
    entity->ext.menuPage.item = 0;

    for (i = 0; pos < entity->ext.menuPageList.bytes[2] && i < n; i++)
    {
        if (equipped[i] != 0xFF && SoulInventory_GetSoulTotal(type, equipped[i] - 1))
            pos++;
    }

    for (soul = 1; pos < entity->ext.menuPageList.bytes[2] && soul < max; soul++)
    {
        if (SoulInventory_GetSoulTotal(type, soul - 1))
        {
            pos++;
            j = 0;
            while (j < n)
            {
                if (equipped[j] != 0xFF && soul == equipped[j])
                    pos--;
                j++;
            }
        }
    }

    while (shown <= 7 && i < n)
    {
        if (equipped[i] != 0xFF && SoulInventory_GetSoulTotal(type, equipped[i] - 1))
        {
            if (pos == entity->ext.menuPageList.bytes[1])
                entity->ext.menuPage.item = equipped[i];
            pos++;
            shown++;
        }
        i++;
    }

    while (shown <= 7 && soul < max)
    {
        if (SoulInventory_GetSoulTotal(type, soul - 1))
        {
            s32 dup = 0;
            j = 0;
            while (j < n)
            {
                if (equipped[j] != 0xFF && soul == equipped[j])
                    dup = 1;
                j++;
            }
            if (!dup)
            {
                if (pos == entity->ext.menuPageList.bytes[1])
                    entity->ext.menuPage.item = soul;
                pos++;
                shown++;
            }
        }
        soul++;
    }

    while (soul < max)
    {
        if (SoulInventory_GetSoulTotal(type, soul - 1))
        {
            found = 0;
            j = 0;
            while (j < n)
            {
                if (equipped[j] != 0xFF && soul == equipped[j])
                    found = 1;
                j++;
            }
            if (!found)
                pos++;
        }
        soul++;
    }
    entity->ext.menuPage.count = pos;
}
