/*
 * Ability, save and button config menus and the bestiary menu entry.
 */
#include "system_menus.h"
#include "soul_inventory.h"
#include "gameplay.h"
#include "structs/ewram.h"
#include "core.h"
#include "save_data.h"
#include "menu_entity.h"
#include "in_game.h"
#include "sound.h"
#include "bestiary.h"
#include "entity_common.h"
#include "math_anim.h"
#include "menu_common.h"
#include "palette.h"
#include "pause_menu.h"
#include "soul_menu.h"
#include "text12.h"
#include "bestiary_cursor.h"
#include "data/data_4F1444.h"
#include "data/data_0E5364.h"
#include "data/data_118B48.h"
#include "gba/memory.h"

extern s32 MenuGrid_MoveCursor(struct EwramData_EntityData *page, s32 repeated);
extern void *memcpy(void *dst, const void *src, unsigned long size);

void AbilityMenu_PrintSelectedName(struct EwramData_EntityData *entity);
void AbilityMenu_DrawMarkers(struct EwramData_EntityData *entity);
void ButtonConfigMenu_Redraw(struct EwramData_EntityData *entity);
void ButtonConfigMenu_DrawSprites(void *unusedMenu);
void ButtonConfigMenu_DrawSprites(void *unusedMenu);
void BestiaryMenu_Setup(struct EwramData_EntityData *page);
void BestiaryMenu_Setup(struct EwramData_EntityData *page);
void BestiaryMenu_UpdateList(struct EwramData_EntityData *page);

/* Ability soul page. */

/**
 * @brief 4BA18 | Open the ability soul page: frame, six rows of names
 * (owned ability souls by name, others as "-----") and the cursor.
 */
void AbilityMenu_Open(struct EwramData_EntityData *page)
{
    s32 i;

    MenuPage_Reset(page);
    do /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
        page->ext.menuList.count = 6;
    while (0);
    Text12_SetupTileRegion(4, 0xD, 0x3200, 0x1F, 4);
    AbilityMenu_PrintSelectedName(page);
    for (i = 0; i < page->ext.menuList.count >> 1; i++)
    {
        HudMap_WriteTilesWide(3, i * 2 + 4, i * 0x20 + 0x3380, 10);
        HudMap_WriteTilesWide(0x11, i * 2 + 4, i * 0x20 + 0x3390, 10);
    }
    for (i = 0; i < page->ext.menuList.count; i++)
    {
        s32 odd;
        s32 x;
        u16 yy;

        odd = i & 1;
        x = odd ? 0x15 : 0;
        yy = (i >> 1) + 0xC;
        Text8_SetRegion(x, yy, 0xE, 1);
        Text8_ClearRegion();
        if (odd)
            Text8_SetPixelCursor(0x80, yy);
        if (SoulInventory_GetSoulTotal(3, i))
            Text8_Print(Text_GetLocalizedEntry(i + 0x156), 0);
        else
            Text8_Print(Text_GetLocalizedEntry(0x34F), 0);
    }
    Cursor_SetTilePosition((page->ext.menuList.cursor & 1) ? 0x10 : 2, (page->ext.menuList.cursor >> 1) * 2 + 4);
    Cursor_Draw();
    MenuPageTitle_Create((struct EwramData_EntityData *)((u8 *)gEwramData + 0xF34), 0x22);
    AbilityMenu_DrawMarkers(page);
}

/**
 * @brief 4BB40 | Ability soul page update: A toggles the selected ability
 * (when owned), cursor movement shows its name, B leaves; redraws the
 * on/off markers of all abilities.
 *
 * @return -1 when the page is left, 0 otherwise
 */
s32 AbilityMenu_Update(struct EwramData_EntityData *page)
{
    s32 repeated;
    s32 new;
    s32 moved;
    s32 text;
    s32 i;
    s32 x;
    s32 y;

    repeated = gEwramData->inputData.repeatedInput;
    new = gEwramData->inputData.newInput;
    if (new & 1)
    {
        if (SoulInventory_GetSoulTotal(3, page->ext.menuList.cursor))
        {
            PlaySong(0xF4);
            if (Player_HasAbility(page->ext.menuList.cursor))
                Player_SetAbility(page->ext.menuList.cursor, 0);
            else
                Player_SetAbility(page->ext.menuList.cursor, 1);
        }
        else
        {
            PlaySong(0xEF);
        }
    }
    else
    {
        moved = MenuGrid_MoveCursor(page, repeated);
        if (new & 2)
        {
            PlaySong(0xF0);
            MenuPageTitle_Delete(&gEwramData->entityData[20]);
        }
        if (moved)
        {
            PlaySong(0xF1);
            if (SoulInventory_GetSoulTotal(3, page->ext.menuList.cursor))
                text = page->ext.menuList.cursor + 0x257;
            else
                text = 0x34F;
            Text12_ClearRegion();
            MenuText_Print(text);
        }
    }
    Cursor_SetTilePosition((page->ext.menuList.cursor & 1) ? 0x10 : 2, (page->ext.menuList.cursor >> 1) * 2 + 4);
    Cursor_Draw();
    MenuPageTitle_Update(&gEwramData->entityData[20]);
    for (i = 0; i < page->ext.menuList.count; i++)
    {
        if (SoulInventory_GetSoulTotal(3, i))
        {
            x = (i & 1) ? 0x1B : 0xD;
            y = (i >> 1) * 2 + 4;
            HudMap_WriteTilesWide(x, y, ((Player_HasAbility(i) >> i) << 4) + 0xB30E, 2);
        }
    }
    return (new & 2) ? -1 : 0;
}

/* Ability soul menu helpers 0x0804BCA0..0x0804BD37. */
/* Declared without the u16 tile parameter here: the original passes the
 * tile untruncated (implicit declaration). */

/**
 * @brief 4BCA0 | Show the name of ability soul unk_4F9 when it is owned,
 * otherwise text 0x34F.
 */
void AbilityMenu_PrintSelectedName(struct EwramData_EntityData *entity)
{
    s32 text;

    if (SoulInventory_GetSoulTotal(3, (s8)entity->ext.menuPage.cursor))
        text = (s8)entity->ext.menuPage.cursor + 0x257;
    else
        text = 0x34F;

    Text12_ClearRegion();
    MenuText_Print(text);
}

/**
 * @brief 4BCDC | Draw the on/off marker of every owned ability soul
 * (two columns, unk_4F8 entries).
 */
void AbilityMenu_DrawMarkers(struct EwramData_EntityData *entity)
{
    s32 i;

    for (i = 0; i < entity->ext.menuPage.count; i++)
    {
        if (SoulInventory_GetSoulTotal(3, i))
        {
            s32 x;
            s32 y;

            x = (i & 1) ? 0x1B : 0xD;
            y = (i >> 1) * 2 + 4;
            HudMap_WriteTilesWide(x, y, ((Player_HasAbility(i) >> i) << 4) + 0xB30E, 2);
        }
    }
}

/**
 * @brief 4BD38 | Menu save page: when saving is not possible show message
 * 0x360 / 0x361 and play the error sound, otherwise show the save prompt
 * (texts 0x34B / 0x34D) and place the cursor.
 */
void SaveMenu_Open(struct EwramData_EntityData *page)
{
    struct SaveHeader *slot;
    struct EwramData *ew;
    struct EwramData_EntityData *ent;

    ew = gEwramData;
    slot = &ew->saveHeaders[0];
    SaveData_ReadSlotHeader(ew->gameState.currentSaveSlot, 0);
    if (ew->gameState.bossRushMode)
    {
        MenuText_ClearAndPrint(0x360);
        page->state = 0xFF;
        PlaySong(0xEF);
    }
    else if (gEwramData->gameState.stateFlags & 2)
    {
        MenuText_ClearAndPrint(0x360);
        page->state = 0xFF;
        PlaySong(0xEF);
    }
    else if (slot->playTime == 0)
    {
        MenuText_ClearAndPrint(0x361);
        page->state = 0xFF;
        PlaySong(0xEF);
    }
    else
    {
        MenuText_ClearAndPrint(0x36A);
        Text12_SetCursor(4, 1);
        Text12_Print(Text_GetLocalizedEntry(0x34B), 0);
        Text12_SetCursor(0xC, 1);
        Text12_Print(Text_GetLocalizedEntry(0x34D), 0);
        Text12_SetCursor(0, 0);
        page->ext.menuListU.cursor = 1;
        page->state = 0;
        page->subState = 0;
        ent = &gEwramData->entityData[18];
        MENU_CURSOR_X(ent) = 0x95;
        MENU_CURSOR_Y(ent) = 0x89;
    }
}

/**
 * @brief 4BE0C | Menu save page update: A saves to the current slot
 * (messages 0x366 / 0x36B / 0x386), left/right toggles the yes/no cursor,
 * after a successful save fades out and returns to the title.
 *
 * @return -1 when the page is left, 0 otherwise
 */
s32 SaveMenu_Update(struct EwramData_EntityData *page)
{
    s32 ret;
    struct EwramData *ew;
    u16 repeated;
    u16 new;
    struct GameState *g;
    struct EwramData_EntityData *ent;
    s32 msg;

    ret = 0;
    ew = gEwramData;
    repeated = ew->inputData.repeatedInput;
    new = ew->inputData.newInput;
    g = &ew->gameState;
    switch (page->state)
    {
    case 0:
        if (new & 3)
        {
            msg = 0x366;
            if ((new & 1) && !(page->ext.menuListU.cursor & 1))
            {
                SaveData_UpdateSlotSummary(g);
                if (SaveData_SaveSlotToSram(gEwramData->gameState.currentSaveSlot + 1) == 0)
                {
                    msg += 5;
                    PlaySong(0xF4);
                    page->state++;
                }
                else
                {
                    PlaySong(0xEF);
                    msg = 0x386;
                    page->state = 0xFF;
                }
            }
            else
            {
                PlaySong(0xF0);
                page->state = 0xFF;
            }
            MenuText_ClearAndPrint(msg);
        }
        else if (repeated & 0x30)
        {
            ent = &ew->entityData[18];
            page->ext.menuListU.cursor++;
            MENU_CURSOR_X(ent) = ((page->ext.menuListU.cursor & 1) * 4 + 8) * 12 + 5;
            MENU_CURSOR_Y(ent) = 0x89;
            PlaySong(0xF1);
        }
        break;
    case 1:
        if (page->subState == 0)
        {
            PlaySong(0x1010);
            gDisplayRegisters.bldY = ret;
            page->timer = 0;
            page->subState++;
        }
        else if (page->subState == 1)
        {
            page->timer++;
            gDisplayRegisters.bldY = page->timer >> 2;
            if ((page->timer >> 2) == 0x10)
            {
                page->timer = 0x20;
                page->subState++;
            }
        }
        else
        {
            page->timer--;
            if (page->timer == 0xFF)
            {
                Game_ResetSystems(g);
                gDisplayRegisters.bldY = 0x10;
                gDisplayRegisters.bldCnt = 0xFF;
                gEwramData->gameMode = 0;
            }
        }
        break;
    case 0xFF:
        Cursor_SetTilePosition(1, 0x10);
        page->state = ret;
        ret = -1;
        break;
    }
    if (page->state != 1)
        Cursor_Draw();
    return ret;
}

/* Open the button configuration page. */

/**
 * @brief 4BF8C | Open the button configuration menu: find the choice index
 * of each of the four bindings, draw the frame and help text.
 * Matching notes: do-while loops with `next = i + 1` computed early; the
 * loop-invariant bases (bindings, choices) are computed in the outer body
 * so loop.c hoists them after `i = 0`; the second loop reuses `j`.
 */
void ButtonConfigMenu_Open(struct EwramData_EntityData *menu)
{
    struct EwramData *ew;
    u16 *bindings;
    s32 i;
    s32 j;

    ew = gEwramData;
    i = 0;
    bindings = (u16 *)((u8 *)ew + 0x13398);
    do
    {
        s32 next;
        const u16 *t;
        u16 *b;
        s32 o;
        u8 *c;

        u8 *choices = menu->ext.buttonConfigPage.choices;
        j = 0;
        o = i * 2;
        next = i + 1;
        b = (u16 *)((u8 *)bindings + o);
        c = &choices[i];
        t = sButtonConfigButtons;
        do
        {
            if (*b == *t)
            {
                *c = j;
                break;
            }
            t++;
            j++;
        } while (j <= 3);
        i = next;
    } while (i <= 3);
    menu->state = 0;
    menu->subState = 0;
    menu->stage = 0;
    MenuPage_Reset(menu);
    menu->ext.buttonConfigPage.error = 3;
    for (j = 0; j <= 2; j++)
        HudMap_WriteTilesWide(5, j + 0xF, j * 0x20 + 0x3200, 0x18);
    Text12_SetRegion(0, 0, 0x20, 2);
    MenuText_ClearAndPrint(0x352);
    menu->ext.buttonConfigPage.error = 0;
    menu->ext.buttonConfigPage.active = 1;
    ButtonConfigMenu_Redraw(menu);
    ButtonConfigMenu_DrawSprites(menu);
    MenuPageTitle_Create((struct EwramData_EntityData *)((u8 *)gEwramData + 0xF34), 0x24);
}

/* Button configuration page update. */
struct Settings
{
    u8 pad_0[0x13C];
    u16 buttons[4];
};

/**
 * @brief 4C05C | Button configuration page update: B or start (0xA) confirm the
 * four assignments (sButtonConfigButtons[choice]) and reject duplicates with an
 * error message (start keeps the page open), up/down moves the cursor,
 * right/A (0x11) and left cycle the button of the selected action, select (4) restores the
 * defaults. Redraws the page and closes it after a valid confirm.
 * Matching notes: do-while confirm loop (loop-weighted refs make the
 * "used" mask a caller-saved pseudo), the duplicate-error block placed in
 * the final else after `if (0)`, defaults copied as `*(u8 *)(i + t)`.
 *
 * @return -1 when the page is left, 0 otherwise
 */

/**
 * @brief Button-configuration page update: up/down selects a row, A/right and left
 * change its choice, Select restores the default choices. B or Start
 * stores the four bindings; a duplicate binding plays the error sound, shows
 * text 0x353 and keeps the page open. Start applies without leaving.
 *
 * @return -1 when the page is left, 0 otherwise
 */
s32 ButtonConfigMenu_Update(struct EwramData_EntityData *page)
{
    struct Settings *settings;
    u16 repeated;
    u16 new;
    s32 ret;
    s32 used;
    s32 i;
    u8 *choices;

    settings = (struct Settings *)&gEwramData->playerStatus;
    repeated = gEwramData->inputData.repeatedInput;
    new = gEwramData->inputData.newInput;
    ret = (new & 0xA) ? -1 : 0;
    if (page->ext.buttonConfigPage.error && (new & ~0xA))
    {
        MenuText_ClearAndPrint(0x352);
        page->ext.buttonConfigPage.error = 0;
    }
    if (ret)
    {
        used = 0;
        i = 0;
        choices = page->ext.buttonConfigPage.choices;
        do
        {
            settings->buttons[i] = sButtonConfigButtons[choices[i]];
            if (settings->buttons[i] & used)
                goto error;
            PlaySong(0xF0);
            used |= settings->buttons[i];
        } while (++i <= 3);
        if (ret)
        {
            page->ext.buttonConfigPage.active = 0;
            if (new & 8)
                ret = 0;
        }
    }
    else if (repeated & 0x40)
    {
        PlaySong(0xF1);
        page->ext.buttonConfigPage.selection--;
        choices = page->ext.buttonConfigPage.choices;
    }
    else if (repeated & 0x80)
    {
        PlaySong(0xF1);
        page->ext.buttonConfigPage.selection++;
        choices = page->ext.buttonConfigPage.choices;
    }
    else if (repeated & 0x11)
    {
        PlaySong(0xF4);
        page->ext.buttonConfigPage.choices[sButtonConfigChoiceSlots[page->ext.buttonConfigPage.selection]]++;
        choices = page->ext.buttonConfigPage.choices;
    }
    else if (repeated & 0x20)
    {
        PlaySong(0xF4);
        page->ext.buttonConfigPage.choices[sButtonConfigChoiceSlots[page->ext.buttonConfigPage.selection]]--;
        choices = page->ext.buttonConfigPage.choices;
    }
    else
    {
        if (0)
        {
        error:
            PlaySong(0xEF);
            MenuText_ClearAndPrint(0x353);
            page->ext.buttonConfigPage.error = 1;
            ret = 0;
        }
        else
        {
            s32 f = new & 4;
            choices = page->ext.buttonConfigPage.choices;
            if (f)
            {
                PlaySong(0xF4);
                i = 0;
                {
                    u8 *c = choices;
                    const u8 *t = sButtonConfigDefaultChoices;
                    for (; i <= 3; i++)
                        c[i] = *(u8 *)((u32)i + (u32)t);
                }
            }
        }
    }
    page->ext.buttonConfigPage.selection &= 3;
    choices[sButtonConfigChoiceSlots[page->ext.buttonConfigPage.selection]] &= 3;
    if (repeated || new)
        ButtonConfigMenu_Redraw(page);
    ButtonConfigMenu_DrawSprites(page);
    if (ret)
        MenuPageTitle_Delete((struct EwramData_EntityData *)((u8 *)gEwramData + 0xF34));
    return ret;
}

/**
 * @brief 4C264 | Draw the button configuration page: the four action
 * labels (texts 0x80..0x87) and the button currently assigned to each.
 */
void ButtonConfigMenu_Draw(struct EwramData_EntityData *page)
{
    s32 i;
    s32 count;

    Text8_SetRegion(0, 5, 0x1E, 1);
    Text8_ClearRegion();
    gEwramData->text8ColumnPixelOffset += 7;
    gEwramData->text8PixelX += 7;
    Text8_PutChar(0x80);
    Text8_PutChar(0x81);
    gEwramData->text8ColumnPixelOffset += 8;
    gEwramData->text8PixelX += 8;
    gEwramData->text8ColumnPixelOffset += 7;
    gEwramData->text8PixelX += 7;
    Text8_PutChar(0x82);
    Text8_PutChar(0x83);
    gEwramData->text8ColumnPixelOffset += 8;
    gEwramData->text8PixelX += 8;
    gEwramData->text8ColumnPixelOffset += 6;
    gEwramData->text8PixelX += 6;
    Text8_PutChar(0x86);
    Text8_PutChar(0x87);
    gEwramData->text8ColumnPixelOffset += 7;
    gEwramData->text8PixelX += 7;
    gEwramData->text8ColumnPixelOffset += 6;
    gEwramData->text8PixelX += 6;
    Text8_PutChar(0x84);
    Text8_PutChar(0x85);
    gEwramData->text8ColumnPixelOffset += 7;
    gEwramData->text8PixelX += 7;
    for (i = 0; i <= 3; i++)
    {
        count = 7;
        if (i == 1)
            count = 6;
        HudMap_WriteTilesWide(6, i * 2 + 4, i * 0x10 + 0x3359, count);
        HudMap_WriteTilesWide(0x12, i * 2 + 4, page->ext.buttonConfigPage.choices[sButtonConfigChoiceSlots[i]] * 3 + 0x32A0, 3);
    }
}

/* Menu helpers 0x0804C38C..0x0804C403. */

/* Default button assignment: indices into sButtonConfigButtons. */

/**
 * @brief 4C38C | Cursor_Draw, then MenuPageTitle_Update on entity slot 20.
 */
void ButtonConfigMenu_DrawSprites(void *unusedMenu)
{
    Cursor_Draw();
    MenuPageTitle_Update(&gEwramData->entityData[20]);
}

/**
 * @brief 4C3AC | ButtonConfigMenu_Draw and move the cursor (slot 18) to row ext.menuPage.cursor.
 */
void ButtonConfigMenu_Redraw(struct EwramData_EntityData *entity)
{
    ButtonConfigMenu_Draw(entity);
    Cursor_SetTilePosition(5, (s8)entity->ext.menuPage.cursor * 2 + 4);
}

/**
 * @brief 4C3C8 | Reset the attack / jump / ability / guardian buttons.
 */
void ButtonConfig_ResetDefaults(void)
{
    u16 *buttons = &gEwramData->playerStatus.attackButton;
    s32 i;

    for (i = 0; i <= 3; i++)
        buttons[i] = sButtonConfigButtons[sButtonConfigDefaultChoices[i]];
}

/* Backup of the four palette slots inside the save buffer (0x1340C). */
#define BACKUP_SLOT(i) (*(s32 *)((u8 *)gEwramData + 0x1340C + (i) * 4))
#define BACKUP_VARIANT(i) ((u8 *)gEwramData + 0x13420 + (i) * 2)

/**
 * @brief Open the bestiary page: back up the palette slots, load the page
 * graphics (per language) and reset the page state.
 */
void BestiaryMenu_Open(struct EwramData_EntityData *page)
{
    u8 *gfx;

    BACKUP_SLOT(0) = gEwramData->gameState.sharedObjPaletteSources[0];
    BACKUP_SLOT(1) = gEwramData->gameState.sharedObjPaletteSources[1];
    BACKUP_SLOT(2) = gEwramData->gameState.sharedObjPaletteSources[2];
    BACKUP_SLOT(3) = gEwramData->gameState.sharedObjPaletteSources[3];
    *(u16 *)BACKUP_VARIANT(0) = *(u16 *)&gEwramData->gameState.sharedObjPaletteVariants[0];
    memcpy(BACKUP_VARIANT(1), &gEwramData->gameState.sharedObjPaletteVariants[1], 2);
    *(u16 *)BACKUP_VARIANT(2) = *(u16 *)&gEwramData->gameState.sharedObjPaletteVariants[2];
    memcpy(BACKUP_VARIANT(3), &gEwramData->gameState.sharedObjPaletteVariants[3], 2);
    BgPalette_Load((u8 *)sBestiaryMenuBgPalette);
    BgPalette_LoadRange((u8 *)sSaveSelectBgPalette, 0xD, 1, 0xD);
    BgPalette_LoadRange((u8 *)sTextBgPalette, 1, 1, 0xF);
    switch (gEwramData->gameState.language)
    {
    case 2:
        gfx = (u8 *)sBestiaryMenuGfx3;
        break;
    case 3:
        gfx = (u8 *)sBestiaryMenuGfx2;
        break;
    default:
        gfx = (u8 *)sBestiaryMenuGfx;
        break;
    }
    Gfx_CopyResource(gfx, (void *)VRAM_BASE, 0);
    page->ext.menuListU.cursor = 0;
    Heap_SaveAndReset();
    gEwramData->gameState.bestiaryPreview = 1;
    page->subState = 0;
    page->stage = 0;
    MenuPage_Reset(page);
    page->ext.menuListU.descriptionMode = 0;
    page->ext.menuListU.count = 0x70;
    BestiaryMenu_Setup(page);
}

/**
 * @brief Bestiary page state machine (fades between the list,
 * BestiaryMenu_UpdateDetailScreen and BestiaryMenu_UpdateDetail).
 *
 * @return -1 when the page is left, 0 otherwise
 */
s32 BestiaryMenu_Update(struct EwramData_EntityData *page)
{
    s32 ret = 0;

    switch (page->subState)
    {
    case 0:
        BestiaryMenu_Setup(page);
        page->subState = 4;
        page->stage = ret;
        break;
    case 4:
        if ((s16)(gDisplayRegisters.bldY -= 4) != 0)
            break;
        gDisplayRegisters.bldY = ret;
        page->subState = 1;
        page->stage = 0;
        break;
    case 1:
        BestiaryMenu_UpdateList(page);
        break;
    case 5:
        if ((u16)(gDisplayRegisters.bldY += 4) <= 0xF)
            break;
        gDisplayRegisters.bldY = 0x10;
        page->subState = 2;
        page->stage = 0;
        break;
    case 2:
        BestiaryMenu_UpdateDetailScreen(page);
        break;
    case 3:
        if ((s16)(gDisplayRegisters.bldY -= 4) != 0)
            break;
        gDisplayRegisters.bldY = ret;
        page->subState = 7;
        break;
    case 7:
        BestiaryMenu_UpdateDetail(page);
        break;
    case 6:
        if ((u16)(gDisplayRegisters.bldY += 4) <= 0xF)
            break;
        gDisplayRegisters.bldY = 0x10;
        page->subState = 0;
        page->stage = 0;
        break;
    case 0xFF:
        BestiaryMenu_Close(page);
        ret = -1;
        break;
    }
    switch (page->subState)
    {
    case 1:
    case 4:
    case 5:
        Cursor_Draw();
        break;
    case 7:
        Entity_UpdateReserved();
        break;
    }
    return ret;
}

/**
 * @brief Load the language specific frame graphics, draw the enemy list and the
 * header tiles and place the cursor.
 */
void BestiaryMenu_Setup(struct EwramData_EntityData *page)
{
    void *gfx;

    switch (gEwramData->gameState.language)
    {
    case 2:
        gfx = (void *)sLocalizedWindowGfxLanguage2;
        break;
    case 3:
        gfx = (void *)sLocalizedWindowGfxLanguage3;
        break;
    default:
        gfx = (void *)sLocalizedWindowGfx;
        break;
    }
    Gfx_CopyResource(gfx, (void *)(VRAM_BASE + 0xA000), 0);
    BgMap_Setup(1, (struct BgMapMetadata *)sBestiaryMenuBgMap, 0, 0);
    HudMap_ClearAll();
    Text8_SetRegion(0, 0xF, 0x29, 1);
    Text8_ClearRegion();
    BestiaryMenu_DrawList(page);
    BestiaryMenu_ReleaseResources();
    MenuCursor_LoadGraphics(&gEwramData->entityData[18]);
    Cursor_StartAnimation();
    HudMap_WriteTilesWide(0xB, 3, 0xF3D0, 0xE);
    HudMap_WriteTilesWide(0x11, 0xF, 0xF3E0, 4);
    HudMap_WriteTilesWide(0x15, 0xF, 0x13E4, 1);
    HudMap_WriteTilesWide(0x19, 0xF, 0x13E8, 1);
    HudMap_WriteTilesWide(0x1B, 0xF, 0xF3EA, 1);
    Cursor_SetTilePosition((page->ext.menuList.cursor & 1) ? 0xF : 1, ((page->ext.menuList.cursor - page->ext.menuList.top) >> 1) * 2 + 5);
    {
        // writing through a pointer keeps the store order of the original
        u8 *p = &page->subState;
        p[0] = 1;
        p[1] = 0;
    }
}

/**
 * @brief Enemy list update: up/down via BestiaryMenu_MoveCursor, R / L page by 10
 * entries (wrapping), B leaves, A opens the detail view when
 * Bestiary_IsEnemyDefeated allows it.
 */
void BestiaryMenu_UpdateList(struct EwramData_EntityData *page)
{
    s32 repeated;
    s32 new;
    s32 top;
    s32 count;
    s32 last;
    s32 rest;

    repeated = gEwramData->inputData.repeatedInput;
    new = gEwramData->inputData.newInput;
    if (BestiaryMenu_MoveCursor(page, repeated) != 0)
    {
        PlaySong(0xF1);
    }
    else if (repeated & 0x100)
    {
        top = page->ext.menuList.top;
        count = page->ext.menuList.count;
        last = count - 1;
        rest = last % 10 + 1;
        if (top != count - rest)
        {
            if (top + 10 > last)
                page->ext.menuList.top = page->ext.menuList.count - rest;
            else
                page->ext.menuList.top = page->ext.menuList.top + 10;
            page->ext.menuList.cursor = page->ext.menuList.top;
            BestiaryMenu_DrawList(page);
        }
        else
        {
            page->ext.menuList.top = 0;
            page->ext.menuList.cursor = 0;
            BestiaryMenu_DrawList(page);
        }
        PlaySong(0xF1);
    }
    else if (repeated & 0x200)
    {
        if (page->ext.menuList.top != 0)
        {
            if (page->ext.menuList.top - 10 < 0)
                page->ext.menuList.top = 0;
            else
                page->ext.menuList.top = page->ext.menuList.top - 10;
            page->ext.menuList.cursor = page->ext.menuList.top;
            BestiaryMenu_DrawList(page);
        }
        else
        {
            page->ext.menuList.cursor = page->ext.menuList.top = page->ext.menuList.count - 1 - (page->ext.menuList.count - 1) % 10;
            BestiaryMenu_DrawList(page);
        }
        PlaySong(0xF1);
    }
    else if (new & 2)
    {
        PlaySong(0xF0);
        page->subState = 0xFF;
        page->stage = 0;
    }
    else if (new & 1)
    {
        if (Bestiary_IsEnemyDefeated(page->ext.menuList.cursor))
        {
            PlaySong(0xF4);
            gDisplayRegisters.bldY = 0;
            page->subState = 5;
            page->stage = 0;
        }
        else
        {
            PlaySong(0xEF);
        }
    }
    Cursor_SetTilePosition((page->ext.menuList.cursor & 1) ? 0xF : 1, ((page->ext.menuList.cursor - page->ext.menuList.top) >> 1) * 2 + 5);
    Cursor_Draw();
}
