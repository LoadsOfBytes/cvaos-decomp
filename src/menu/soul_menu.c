/*
 * Menu page titles, item / slot icons, cursor sprites and the soul menu.
 */
#include "menu_page.h"
#include "structs/ewram.h"
#include "structs/entity_render_flags.h"
#include "core.h"
#include "map_flags.h"
#include "menu_entity.h"
#include "soul_inventory.h"
#include "gba.h"
#include "sound.h"
#include "bg_map.h"
#include "gameplay.h"
#include "blue_souls.h"
#include "equipment_menu.h"
#include "menu_common.h"
#include "oam.h"
#include "obj_gfx.h"
#include "palette.h"
#include "pause_menu.h"
#include "soul_menu.h"
#include "soul_pickup.h"
#include "text12.h"
#include "data/data_118B48.h"
#include "red_souls_b.h"

extern s32 MenuGrid_MoveCursor(struct EwramData_EntityData *entity, u16 input);

void CursorSprite_Update(struct EwramData_EntityData *);
void SoulMenu_ClearStatPreview(void);
void SoulMenu_DrawList(struct EwramData_EntityData *page, s32 slot);
void SoulMenu_PrintEquippedName(struct EwramData_EntityData *entity);
void SoulMenu_ShowSelection(struct EwramData_EntityData *entity);
void SoulMenu_CountList(struct EwramData_EntityData *entity, s32 kind);
void SoulMenu_DrawCategoryIcon(struct EwramData_EntityData *entity);

struct Tbl12 { u8 pad[4]; u8 value; s8 row; u8 pad2[6]; };
extern const struct Tbl12 sBlueSoulInfo[];

/* Page-title entity (slot 20) state machine (state at unk_508): state 1
 * places the entity at (0x35, 9) and moves to state 2; state 2 enables affine
 * slot ENTITY_HALF_510 on the first frame and grows the scale by 0x1000 per frame for
 * 16 frames, then disables the affine flag; both draw via Sprite_Draw. */

/* The canonical u16 affine-slot field preserves the original value/address
 * ordering without a separate local bitfield view. */

void MenuPageTitle_Update(struct EwramData_EntityData *entity)
{
    s32 t;

    switch (entity->ext.menuTitle.state)
    {
        case 0:
            break;
        case 1:
            entity->posX.part16.integer = 0x35;
            entity->posY.part16.integer = 9;
            entity->timer = 0;
            entity->ext.menuTitle.state = 2;
        case 2:
            if (entity->timer == 0)
            {
                entity->palette = 14;
                {
                    entity->affineIndex = MENU_TITLE_AFFINE_SLOT(entity);
                }
                entity->affine = 1;
            }
            if (entity->timer < 16)
            {
                t = entity->timer++;
                ObjAffine_Set(0, t << 12, t << 12, 0, 0, MENU_TITLE_AFFINE_SLOT(entity));
            }
            else
            {
                entity->affine = 0;
            }
            Sprite_Draw(entity);
            break;
    }
}

/* Menu item-icon helpers 0x08048804..0x08048A63. */

/**
 * @brief 48804 | Show the icon of item (type, item) as entityData[19] at
 * (x, y); type 0xFF looks the item up by number alone. An icon already
 * shown there is released first, with ItemIcon_Free(.., 0) when one of the
 * three slot icons entityData[208..210] shows the same item, else (.., 1).
 */
void MenuItemIcon_Show(s32 x, s32 y, s32 type, s32 item)
{
    struct EwramData_EntityData *entity = &gEwramData->entityData[19];
    const u8 *info;

    if (entity->enemyId != 0)
    {
        struct EwramData_EntityData *other = &gEwramData->entityData[208];
        s32 i;

        for (i = 0; i < 3; i++, other++)
        {
            if (other->enemyId == entity->enemyId
                && other->spawnParam1 == entity->spawnParam1)
                break;
        }
        if (i == 3)
            ItemIcon_Free(entity, 1);
        else
            ItemIcon_Free(entity, 0);
        EntityDelete(entity);
    }

    if (entity != NULL)
    {
        if (type != 0xFF)
        {
            info = Item_GetInfo(type, item);
        }
        else
        {
            info = Item_GetInfoByGlobalId(item);
            type = Item_GetTypeByGlobalId(item);
        }
        entity->ext.menu.gfxSlot = ItemIconCache_Load(info[2]);
        ItemIcon_AttachSprite(entity, entity->ext.menu.gfxSlot, (s8)info[3]);
        entity->drawFunc = Sprite_DrawEntity;
        entity->posX.part16.integer = x;
        entity->posY.part16.integer = y;
        entity->scrollWithBg = 1;
        entity->enemyId = type;
        entity->spawnParam1 = item;
        entity->spriteLayer = 0;
    }
}

/**
 * @brief 488FC | As MenuItemIcon_Show for the slot icon entityData[208 + slot],
 * compared against entityData[19]; also stores the first u16 of the item
 * info in spawnParam0.
 */
void MenuSlotIcon_Show(s32 x, s32 y, s32 type, s32 item, s32 slot)
{
    struct EwramData_EntityData *entity = &gEwramData->entityData[208 + slot];
    const u8 *info;

    if (entity->enemyId != 0)
    {
        struct EwramData_EntityData *other = &gEwramData->entityData[19];

        if (other->enemyId == entity->enemyId
            && other->spawnParam1 == entity->spawnParam1)
            ItemIcon_Free(entity, 0);
        else
            ItemIcon_Free(entity, 1);
        EntityDelete(entity);
    }

    if (entity != NULL)
    {
        if (type != 0xFF)
        {
            info = Item_GetInfo(type, item);
        }
        else
        {
            info = Item_GetInfoByGlobalId(item);
            type = Item_GetTypeByGlobalId(item);
        }
        entity->ext.menu.gfxSlot = ItemIconCache_Load(info[2]);
        ItemIcon_AttachSprite(entity, entity->ext.menu.gfxSlot, (s8)info[3]);
        entity->drawFunc = Sprite_DrawEntity;
        entity->posX.part16.integer = x;
        entity->posY.part16.integer = y;
        entity->scrollWithBg = 1;
        entity->enemyId = type;
        entity->spawnParam1 = item;
        entity->spawnParam0 = *(const u16 *)Item_GetInfo(type, item);
        entity->spriteLayer = 0;
    }
}

/**
 * @brief Draw the cursor entity for its page state: 3 item page, 6 button
 * configuration, 7 bestiary; the other states draw nothing here.
 */
void MenuCursor_DrawForPage(struct EwramData_EntityData *entity)
{
    struct PlayerStatus *menu = &gEwramData->playerStatus;

    entity->animFrame = 0;
    switch (entity->state)
    {
        case 1:
        case 2:
        case 4:
        case 5:
        case 8:
        case 9:
        case 10:
            break;
        case 3:
            ItemMenu_DrawCursor(menu, entity);
            break;
        case 6:
            ButtonConfigMenu_DrawCursor(menu, entity);
            break;
        case 7:
            BestiaryMenu_DrawCursor(menu, entity);
            break;
    }
}

/* Helpers around fixed entity slots 18, 19 and 0xD0..0xD2
 * (0x08048A64..0x08048C43). */

/**
 * @brief Draw function of the shop menu entity: the category cursor, or the
 * list cursor and the item icon.
 */
void Shop_DrawCursors(struct EwramData_EntityData *entity)
{
    entity->animFrame = 0;
    switch (entity->state)
    {
        case 0:
            MENU_CURSOR_X(entity) = 0x30;
            MENU_CURSOR_Y(entity) = entity->subState * 24 + 0x28;
            Cursor_Draw();
            break;
        case 1:
        case 2:
            Shop_DrawListCursor(entity);
            MenuItemIcon_Draw();
            break;
    }
}

/**
 * @brief Start the cursor animation set sMenuSpriteSheet on entity slot 18
 * (palette 0xD, frame 0x1E).
 */
void Cursor_StartAnimation(void)
{
    struct EwramData_EntityData *cursor = &gEwramData->entityData[18];

    Sprite_Attach(cursor, sMenuSpriteSheet, cursor->ext.menu.gfxSlot, 0xD);
    cursor->animFrame = 0x1E;
}

/**
 * @brief 48AD8 | Set the 0x518/0x51A values of entity slot 18 from tile
 * coordinates (param * 8 plus offsets 6 and 3).
 */
void Cursor_SetTilePosition(s32 column, s32 row)
{
    struct EwramData_EntityData *cursor = &gEwramData->entityData[18];

    MENU_CURSOR_X(cursor) = column * 8 + 6;
    MENU_CURSOR_Y(cursor) = row * 8 + 3;
}

/**
 * @brief Return the cursor entity (slot 18).
 */
struct EwramData_EntityData *Cursor_GetEntity(void)
{
    return &gEwramData->entityData[18];
}

/**
 * @brief Start the (language dependent) animation set on the page-title entity
 * with the frame base of slot 18, allocate an affine slot, set the page's
 * title frame (0x1F + page index as passed by the page open callbacks) and
 * advance its state.
 */
void MenuPageTitle_Create(struct EwramData_EntityData *title, s32 frame)
{
    u8 *sheet = sMenuSpriteSheet;
    struct EwramData_EntityData *cursor = &gEwramData->entityData[18];
    s32 tileSlot = cursor->ext.menu.gfxSlot;

    if (gEwramData->gameState.language == 3)
        sheet = sMenuPageTitleSpriteSheet;

    Sprite_Attach(title, sheet, tileSlot, 0xE);
    MENU_TITLE_AFFINE(title) = ObjAffine_Alloc(2);
    title->animFrame = frame;
    title->ext.menuTitle.state++;
}

/**
 * @brief Release the affine slot allocated by MenuPageTitle_Create, request
 * deletion and reset the state.
 */
void MenuPageTitle_Delete(struct EwramData_EntityData *title)
{
    ObjAffine_Free(MENU_TITLE_AFFINE(title));
    title->deleteRequested = 1;
    title->ext.menuTitle.state = 0;
}

/**
 * @brief Draw the menu item-icon entity (slot 19).
 */
void MenuItemIcon_Draw(void)
{
    Sprite_DrawEntity(&gEwramData->entityData[19]);
}

/**
 * @brief Free the menu item icon (releasing its cache slot) and delete entity
 * slot 19.
 */
void MenuItemIcon_Delete(void)
{
    struct EwramData_EntityData *entity = &gEwramData->entityData[19];

    ItemIcon_Free(entity, 1);
    EntityDelete(entity);
}

/**
 * @brief Set the hidden flag of the menu item-icon entity (slot 19) to bit 0
 * of value.
 */
void MenuItemIcon_SetHidden(u8 value)
{
    s32 bit = value;
    gEwramData->entityData[19].hidden = bit;
}

/**
 * @brief Draw the three equipment-slot icon entities 0xD0..0xD2.
 */
void MenuSlotIcons_Draw(void)
{
    struct EwramData_EntityData *entity = &gEwramData->entityData[0xD0];
    s32 i;

    for (i = 2; i >= 0; i--, entity++)
        Sprite_DrawEntity(entity);
}

/**
 * @brief Free the three slot icons (releasing their cache slots) and delete
 * entities 0xD0..0xD2.
 */
void MenuSlotIcons_Delete(void)
{
    struct EwramData_EntityData *entity = &gEwramData->entityData[0xD0];
    s32 i;

    for (i = 2; i >= 0; i--, entity++)
    {
        ItemIcon_Free(entity, 1);
        EntityDelete(entity);
    }
}

/* MenuSlotIcon_SetHidden (matches by widening the value first). */
/**
 * @brief Set the hidden flag of slot icon entity 0xD0 + index to bit 0 of value.
 */
void MenuSlotIcon_SetHidden(u8 value, s32 index)
{
    struct EwramData_EntityData *e;
    s32 v = value;
    e = &gEwramData->entityData[index];
    e[0xD0].hidden = v;
}

/**
 * @brief Delete the entity, load the language dependent cursor graphics and
 * palette, attach the cursor animation set and install CursorSprite_Update.
 *
 * @return 1 on success, 0 when the graphics could not be loaded
 */
s32 CursorSprite_Create(struct EwramData_EntityData *entity, u16 palette)
{
    s32 graphics, result;
    EntityDelete(entity);
    switch (gEwramData->gameState.language)
    {
    case 2: graphics = ObjGfx_LoadResource(sMenuCursorGfxLanguage2, 0); break;
    case 3: graphics = ObjGfx_LoadResource(sMenuCursorGfxLanguage3, 0); break;
    default: graphics = ObjGfx_LoadResource(sMenuCursorGfx, 0); break;
    }
    if (graphics >= 0)
    {
        ObjPalette_LoadRange(sCursorSpritePalette, 1, 1, palette);
        Sprite_Attach(entity, sMenuSpriteSheet, graphics, palette);
        entity->animFrame = 0x1E;
        entity->updateFunc = CursorSprite_Update;
        result = 1;
    }
    else result = 0;
    return result;
}

/* Entity helpers 0x08048CF8..0x0804933B. */
/* Language dependent graphics sets (English/other, French, German). */

/* Draw the three equipped soul names in the soul menu and highlight the cursor row. */
struct SoulMenu
{
    u8 pad[0x13];
    u8 type;
    u8 count;
    s8 cursor;
    u8 pad16;
    u8 selected;
};

void SoulMenu_DrawEquippedNames(struct SoulMenu *menu);
/* Soul menu: list of souls of one type (red/blue/yellow) - none row, equipped, owned, empty fill. */

void SoulMenu_DrawEquippedSoulValue(struct SoulMenu *menu);
void SoulMenu_DrawSelectedSoulValue(struct SoulMenu *menu);
void SoulMenu_EquipSelected(struct SoulMenu *menu);

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
    SoulTypeSelector_Update((struct SoulTypeIcons *)&gEwramData->entityData[21]);
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
    SoulMenu_DrawEquippedNames((struct SoulMenu *)entity);
    SoulMenu_DrawList(entity, entity->ext.menuPageList.bytes[1]);
    SoulMenu_PrintEquippedName(entity);
    SoulMenu_DrawEquippedSoulValue((struct SoulMenu *)entity);
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
                SoulMenu_DrawSelectedSoulValue((struct SoulMenu *)entity);
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
                SoulMenu_DrawEquippedNames((struct SoulMenu *)entity);
                SoulMenu_DrawList(entity, entity->ext.menuPageList.bytes[1]);
                SoulMenu_PrintEquippedName(entity);
                SoulMenu_DrawEquippedSoulValue((struct SoulMenu *)entity);
                PlaySong(0xF1);
            }
            break;
        case 2:
            if (pressed & (KEY_A | KEY_B))
            {
                if (pressed & KEY_A)
                {
                    PlaySong(0xF4);
                    SoulMenu_EquipSelected((struct SoulMenu *)entity);
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
                SoulMenu_DrawEquippedNames((struct SoulMenu *)entity);
                SoulMenu_DrawEquippedSoulValue((struct SoulMenu *)entity);
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
                        SoulMenu_DrawSelectedSoulValue((struct SoulMenu *)entity);
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
            SoulTypeSelector_Update((struct SoulTypeIcons *)&gEwramData->entityData[21]);
            EntityDelete(&gEwramData->entityData[21]);
            MenuPageTitle_Delete(&gEwramData->entityData[20]);
            result = -1;
            return result;
        }
    }

    if (entity->subState != 0xFF)
    {
        MenuPageTitle_Update(&gEwramData->entityData[20]);
        SoulTypeSelector_Update((struct SoulTypeIcons *)&gEwramData->entityData[21]);
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

/* SoulMenu_DrawCategoryIcon */
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
        kind = entity->ext.menuPage.category;
    else
        kind = entity->ext.menuPageList.bytes[1];
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

/**
 * @brief Print the equipped red, blue and yellow soul names (or the none text
 * 0x34F) and recolor the three category rows, highlighting the cursor row.
 */
void SoulMenu_DrawEquippedNames(struct SoulMenu *menu)
{
    struct PlayerStatus *equip;
    s32 i;
    s32 text;
    u16 pal;
    s32 odd;
    s32 x;
    u16 y;
    u16 *dst;
    s32 j;
    s32 col;

    equip = &gEwramData->playerStatus;
    col = 6;
    for (i = 0; i < 3; i++)
    {
        text = 0x34F;
        switch (i)
        {
            case 0:
                if (equip->equippedRedSoul != 0)
                    text = equip->equippedRedSoul + 0xE2;
                pal = 0x8000;
                break;
            case 1:
                if (equip->equippedBlueSoul != 0)
                    text = 0x11A + equip->equippedBlueSoul;
                pal = 0x9000;
                break;
            case 2:
                if (equip->equippedYellowSoul != 0)
                    text = 0x132 + equip->equippedYellowSoul;
                pal = 0x6000;
                break;
        }
        odd = (i + 1) & 1;
        x = 0;
        if (odd)
            x = 0x15;
        y = ((i + 1) >> 1) + 10;
        Text8_SetRegion(x, y, 0xE, 1);
        Text8_ClearRegion();
        if (odd)
            Text8_SetPixelCursor(0x80, y);
        Text8_Print(Text_GetLocalizedEntry(text), 0);
        { s32 x_ = col << 1; s32 y_ = (i << 1) + 3; asm("" : "+r"(y_)); y_ <<= 6; y_ += BG_MAP_VRAM; dst = (u16 *)(x_ + y_); } /* FAKEMATCH: opaque row keeps (row*2+3)<<6 unfolded */
        if (menu->cursor != i)
            pal = 0x3000;
        { s32 m_ = 0xFFF; asm volatile("" : "+r"(m_)); for (j = 9; j >= 0; j--) /* FAKEMATCH: mask loaded per row (not hoisted) */
        {
            *dst = (*dst & m_) | pal; asm("" :: "r"(text)); /* FAKEMATCH: text outlives, lowers its priority (r9) */
            dst++;
        } }
    }
}

/**
 * @brief 494F8 | Draw the soul list of soul type slot (red, blue, yellow):
 * an optional "none" row on the first page, the equipped souls, then the
 * owned souls with their counts (8 rows); store the number of entries and
 * the soul under the cursor in page.
 */
void SoulMenu_DrawList(struct EwramData_EntityData *page, s32 slot)
{
    u8 total;
    struct EwramData *ew;
    s32 type;
    s32 count;
    s32 textBase;
    u8 *equipped;
    s32 row;
    s32 idx;
    s32 k;
    s32 i;
    s32 j;
    s32 found;
    s32 slots;

    ew = gEwramData;
    slots = 4;
    row = 0;
    idx = 0;
    page->ext.menuList.item = idx;
    switch (slot)
    {
    case 0:
        type = 0;
        count = 0x38;
        textBase = 0xE3;
        equipped = (u8 *)ew + 0x133DC;
        break;
    case 1:
        type = 1;
        count = 0x19;
        textBase = 0x11B;
        equipped = (u8 *)ew + 0x133E0;
        break;
    case 2:
        type = 2;
        count = 0x24;
        textBase = 0x133;
        equipped = (u8 *)ew + 0x133E4;
        break;
    }
    if (page->ext.menuList.top == 0)
    {
        s32 x;
        s32 odd;
        s32 half;

        odd = row & 1;
        x = odd ? 0x15 : 0;
        half = row >> 1;
        Text8_SetRegion(x, half + 0xC, 0xE, 1);
        Text8_ClearRegion();
        Text8_Print(Text_GetLocalizedEntry(0x34F), 0);
        HudMap_ClearTilesLeft(odd ? 0x1B : 0xD, half + 0xA, 1);
        row = 1;
    }
    else
        row = 0;
    idx = 1;
    page->ext.menuList.item = 0;
    k = 0;
    for (; idx < page->ext.menuList.top && k < slots; k++)
    {
        if (equipped[k] != 0xFF && SoulInventory_GetSoulTotal(type, equipped[k] - 1) != 0)
            idx++;
    }
    for (i = 1; idx < page->ext.menuList.top && i < count; i++)
    {
        if (SoulInventory_GetSoulTotal(type, i - 1) != 0)
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
        if (equipped[k] != 0xFF && (total = SoulInventory_GetSoulTotal(type, equipped[k] - 1)) != 0)
        {
            s32 x;
            u16 yy;
            s32 odd;
            s32 half;

            if (idx == page->ext.menuList.cursor)
                page->ext.menuList.item = equipped[k];
            odd = row & 1;
            x = odd ? 0x15 : 0;
            half = row >> 1;
            yy = half + 0xC;
            Text8_SetRegion(x, yy, 0xE, 1);
            Text8_ClearRegion();
            if (odd)
                Text8_SetPixelCursor((row & 1) << 7, yy);
            Text8_Print(Text_GetLocalizedEntry(textBase + equipped[k] - 1), 0);
            HudMap_PrintPaddedNumber(odd ? 0x1B : 0xD, half + 0xA, total, 1);
            idx++;
            row++;
        }
    }
    for (; row <= 7 && i < count; i++)
    {
        if ((total = SoulInventory_GetSoulTotal(type, i - 1)) != 0)
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
                s32 x;
                s32 odd;
                s32 half;

                if (idx == page->ext.menuList.cursor)
                    page->ext.menuList.item = i;
                odd = row & 1;
                x = odd ? 0x15 : 0;
                Text8_SetRegion(x, ((row + 8) >> 1) + 8, 0xE, 1);
                Text8_ClearRegion();
                half = row >> 1;
                if (odd)
                    Text8_SetPixelCursor((row & 1) << 7, half + 0xC);
                Text8_Print(Text_GetLocalizedEntry(textBase + i - 1), 0);
                HudMap_PrintPaddedNumber(odd ? 0x1B : 0xD, half + 0xA, total, 1);
                idx++;
                row++;
            }
        }
    }
    if (row <= 7)
    {
    fill:
        {
            s32 x;
            s32 odd;
            s32 half;

            odd = row & 1;
            x = odd ? 0x15 : 0;
            half = row >> 1;
            Text8_SetRegion(x, half + 0xC, 0xE, 1);
            Text8_ClearRegion();
            HudMap_ClearTilesLeft(odd ? 0x1B : 0xD, half + 0xA, 1);
            do { row++; } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
        }
        if (row <= 7)
            goto fill;
    }
    while (i < count)
    {
        if (SoulInventory_GetSoulTotal(type, i - 1) != 0)
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
        i++;
    }
    page->ext.menuList.count = idx;
    MenuList_DrawScrollArrows(page, 1);
}

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

/**
 * @brief Equip the selected soul (0 unequips) in the menu's red/blue/yellow slot:
 * refresh stats, stop the yellow soul when a soul was equipped, update the
 * soul list, restart the yellow soul, adjust the list count/cursor when the
 * last copy was equipped and redraw the stats.
 */
void SoulMenu_EquipSelected(struct SoulMenu *menu)
{
    u8 *equip;
    u8 *slot;
    s32 type;
    u8 sel;
    u8 count;

    equip = (u8 *)gEwramData + 0x1325C;
    switch (menu->type)
    {
        case 0:
            slot = (u8 *)gEwramData + 0x13269;
            type = 0;
            break;
        case 1:
            slot = (u8 *)gEwramData + 0x1326A;
            type = 1;
            break;
        case 2:
            slot = (u8 *)gEwramData + 0x1326B;
            type = 2;
            break;
    }
    Player_RefreshEquipmentStats();
    if (*slot != 0)
    {
        if (SoulInventory_GetSoulTotal(type, *slot - 1) == 1)
            menu->count++;
        YellowSoul_Stop();
    }
    SoulList_UpdateForEquip(menu);
    if (menu->selected != 0)
    {
        *slot = menu->selected;
        if (type == 2)
            YellowSoul_Restart(menu->selected, 1);
        else
            YellowSoul_Restart(equip[0xF], 1);
        if (SoulInventory_GetSoulTotal(type, menu->selected - 1) == 0)
        {
            count = menu->count;
            menu->count = count - 1;
            if (menu->cursor > (s8)menu->count - 1)
                menu->cursor = count - 2;
        }
    }
    else
    {
        *slot = 0;
        if (type != 2)
            YellowSoul_Restart(equip[0xF], 1);
    }
    Menu_DrawAttackDefense(0x16, 2);
    Menu_DrawCurrentStats(0x16, 4);
    HudMap_DrawStatusConditionIcon(0x15, 8);
    SoulMenu_ClearStatPreview();
}

static inline void FillRow(u16 *dst, s32 tile)
{
    s32 i;
    for (i = 3; i >= 0; i--)
    {
        *dst = tile;
        dst++;
        tile++;
    }
}

/**
 * @brief For the red or blue category row under the cursor, draw the label tiles
 * and the table value of the equipped soul (blue souls also draw their
 * four-tile row label); yellow or an empty slot draws no number.
 */
void SoulMenu_DrawEquippedSoulValue(struct SoulMenu *menu)
{
    struct PlayerStatus *equip;
    s32 value;
    s32 skip;
    s32 row;
    s32 cur;

    equip = &gEwramData->playerStatus;
    value = 0;
    skip = 0;
    HudMap_ClearTiles(1, 0x10, 4);
    HudMap_ClearTiles(1, 0x11, 4);
    HudMap_ClearTiles(1, 0x12, 4);
    cur = menu->cursor;
    if (cur == 2)
        return;
    if (menu->cursor == 0)
        goto case0;
    if (menu->cursor == 1)
        goto case1;
    goto tail;
case0:
    if (equip->equippedRedSoul == 0)
        skip = 1;
    else
    {
    value = sSomaRedSoulInfo[equip->equippedRedSoul].mpCost;
    FillRow((u16 *)(VRAM_BASE + 0xE442), 0x33AB);
    HudMap_ClearTiles(1, 0x12, 4);
    }
    goto tail;
case1:
    if (equip->equippedBlueSoul == 0)
        skip = 1;
    else
    {
    value = sBlueSoulInfo[equip->equippedBlueSoul].value;
    row = sBlueSoulInfo[equip->equippedBlueSoul].row >> 1;
    FillRow((u16 *)(VRAM_BASE + 0xE442), 0x33BB);
    {
        u16 *dst = (u16 *)(VRAM_BASE + 0xE482);
        s32 tile = row * 16 + 0x33CB;
        s32 i;
        for (i = 3; i >= 0; i--)
        {
            *dst = tile;
            dst++;
            tile++;
        }
    }
    }
tail:
    if (skip == 0)
        HudMap_PrintNumber(4, 0x10, value, 3, 3, skip);
}

/**
 * @brief As SoulMenu_DrawEquippedSoulValue for the soul selected in the open
 * red/blue list; nothing when the cursor is on the first row or for yellow.
 */
void SoulMenu_DrawSelectedSoulValue(struct SoulMenu *menu)
{
    s32 value;
    s32 i;
    s32 row;
    u16 *dst;
    s32 tile;

    value = 0;
    HudMap_ClearTiles(1, 0x10, 4);
    HudMap_ClearTiles(1, 0x11, 4);
    HudMap_ClearTiles(1, 0x12, 4);
    if (menu->cursor == 0)
        return;
    if (menu->type == 2)
        return;
    switch (menu->type)
    {
    case 0:
        value = sSomaRedSoulInfo[menu->selected].mpCost;
        FillRow((u16 *)(VRAM_BASE + 0xE442), 0x33AB);
        HudMap_ClearTiles(1, 0x12, 4);
        break;
    case 1:
        value = sBlueSoulInfo[menu->selected].value;
        row = sBlueSoulInfo[menu->selected].row >> 1;
        FillRow((u16 *)(VRAM_BASE + 0xE442), 0x33BB);
        {
            u16 *dst = (u16 *)(VRAM_BASE + 0xE482);
            s32 tile = row * 16 + 0x33CB;
            s32 i;
            for (i = 3; i >= 0; i--)
            {
                *dst = tile;
                dst++;
                tile++;
            }
        }
        break;
    }
    HudMap_PrintNumber(4, 0x10, value, 3, 3, 0);
}

/**
 * @brief Temporarily clear the selected equipped-soul slot, recalculate with the selected yellow soul when appropriate, draw comparison values and restore the saved stats and slot.
 */
void SoulMenu_DrawStatPreview(struct SoulMenu *menu)
{
    s16 *stats;
    u8 *slot;
    u8 saved;
    s16 old[6];
    s32 i;
    s16 kind;
    s32 color;
    s32 value;
    s16 diff;

    stats = (s16 *)((u8 *)gEwramData + 0x131EE);
    slot = (u8 *)gEwramData + 0x13269;
    slot += menu->type;
    saved = *slot;
    *slot = 0;
    for (i = 0; i < 6; i++)
        old[i] = stats[i];
    Player_RefreshEquipmentStats();
    if (menu->type == 2)
        YellowSoul_Restart(menu->selected, 1);
    else
        YellowSoul_Restart(*((u8 *)gEwramData + 0x1326B), 1);
    for (i = 0; i < 6; i++)
    {
        diff = stats[i] - old[i];
        if (diff == 0)
        {
            kind = 0; color = 3;
        }
        else if (diff > 0)
        {
            kind = 1; color = 9;
        }
        else
        {
            kind = 2; color = 8;
        }
        value = stats[i];
        if (value > 1000)
            value = 999;
        HudMap_DrawStatChangeIndicator(0x18, i + 2, kind);
        HudMap_PrintNumber(0x1B, i + 2, value, 3, color, 0);
    }
    for (i = 0; i < 6; i++)
        stats[i] = old[i];
    *slot = saved;
}

/* Soul menu helpers 0x08049D98..0x08049E63. */

/**
 * @brief Clear four background-map columns at 0x18 across rows two through seven.
 */
void SoulMenu_ClearStatPreview(void)
{
    s32 i;

    for (i = 0; i <= 5; i++)
        HudMap_ClearTiles(0x18, i + 2, 4);
}

/**
 * @brief Print the selected red/blue/yellow equipped-soul name or its none text in the configured two-row Text12 region.
 */
void SoulMenu_PrintEquippedName(struct EwramData_EntityData *entity)
{
    struct PlayerStatus *player = &gEwramData->playerStatus;
    s32 text = 0x34F;

    switch ((s8)entity->ext.menuPage.cursor)
    {
        case 0:
            if (player->equippedRedSoul != 0)
                text = player->equippedRedSoul + 0x1E3;
            else
                text = 0x37D;
            break;
        case 1:
            if (player->equippedBlueSoul != 0)
                text = player->equippedBlueSoul + 0x21B;
            else
                text = 0x37E;
            break;
        case 2:
            if (player->equippedYellowSoul != 0)
                text = player->equippedYellowSoul + 0x233;
            else
                text = 0x37F;
            break;
    }

    Text12_SetRegion(0, 0, 0x1D, 2);
    Text12_ClearRegion();
    MenuText_Print(text);
}

/**
 * @brief 49E4C | Menu_PrintSoulName(unk_4F7, unk_4FB), then SoulMenu_DrawStatPreview.
 */
void SoulMenu_ShowSelection(struct EwramData_EntityData *entity)
{
    Menu_PrintSoulName(entity->ext.menuPage.category, entity->ext.menuPage.item);
    SoulMenu_DrawStatPreview((struct SoulMenu *)entity);
}

/**
 * @brief Insert soulIndex+1 once into the kind-selected four-entry list at its first empty slot or slot three; preserve the incomplete kind guard and dispatch.
 */
void SoulList_Insert(s32 kind, s32 soul)
{
    struct PlayerStatus *player = &gEwramData->playerStatus;
    u8 *list;
    s32 i;

    if (kind > 7)
        return;

    soul++;
    switch (kind)
    {
        case 0:
            list = player->redSoulList;
            break;
        case 1:
            list = player->blueSoulList;
            break;
        case 2:
            list = player->yellowSoulList;
            break;
    }
    for (i = 0; i <= 3; i++)
    {
        if (list[i] == soul)
            return;
    }
    for (i = 0; i <= 3; i++)
    {
        if (list[i] == 0xFF)
        {
            list[i] = soul;
            break;
        }
    }
    if (i == 4)
        list[3] = soul;
}

/**
 * @brief Write the value to slot zero, returning if its low byte is nonzero; otherwise write all four entries and retain the final slot-three assignment.
 */
void SoulList_WriteValue(s32 kind, s32 value)
{
    struct PlayerStatus *player = &gEwramData->playerStatus;
    u8 *list;
    s32 i;

    switch (kind)
    {
        case 0:
            list = player->redSoulList;
            break;
        case 1:
            list = player->blueSoulList;
            break;
        case 2:
            list = player->yellowSoulList;
            break;
    }
    for (i = 0; i <= 3; i++)
    {
        list[i] = value;
        if ((u8)value != 0)
            return;
    }
    list[3] = value;
}
