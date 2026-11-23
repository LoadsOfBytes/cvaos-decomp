/* Menu helpers 0x08047C1C..0x08048107 (graphics state, page list, cursor
 * and scroll arrows). */
#include "code_08011DD0.h"
#include "menu_page.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern void HudMap_WriteTiles(s32 x, s32 y, u16 tile, s32 count);
void MenuCursor_DrawScrollArrows(struct EwramData_EntityData *entity);
void Cursor_Draw(void);
void Cursor_DrawWithOptionalBob(s32 bob);

/* Cursor frames for the four entries unk_4FC..4FF (ButtonConfigMenu_DrawCursor). */
extern const u8 sButtonConfigCursorFrames[];
extern u8 *sMenuCursorPalette;
/* Language dependent graphics sets (English/other, French, German). */
extern u8 sMenuCursorGfx[];
extern u8 sMenuCursorGfxLanguage2[];
extern u8 sMenuCursorGfxLanguage3[];

/**
 * @brief 47C1C | ObjAffine_SaveState, ObjGfx_SaveState, ItemIconCache_Save, Palette_SaveState.
 */
void Menu_SaveGraphicsState(void)
{
    ObjAffine_SaveState();
    ObjGfx_SaveState();
    ItemIconCache_Save();
    Palette_SaveState();
}

/**
 * @brief 47C34 | Palette_RestoreState, ItemIconCache_Restore, ObjGfx_RestoreState, ObjAffine_RestoreState
 * (counterpart of Menu_SaveGraphicsState).
 */
void Menu_RestoreGraphicsState(void)
{
    Palette_RestoreState();
    ItemIconCache_Restore();
    ObjGfx_RestoreState();
    ObjAffine_RestoreState();
}

/**
 * @brief 47C4C | Draw the seven 6-tile rows of a list at (2, 12..18) from
 * tile 0x340 on, palette 3 for the row pauseMenuPage and 7 for the others, then
 * MenuText_ClearAndPrint(0x362 + pauseMenuPage).
 */
void PauseMenu_DrawPageList(void)
{
    s32 x = 2;
    s32 y = 12;
    s32 sel = gEwramData->pauseMenuPage;
    s32 i;
    s32 tile;
    s32 t;

    for (i = 0, tile = 0x40; i <= 6; i++)
    {
        t = tile + 0x300;
        if (i == sel)
            t |= 0x3000;
        else
            t |= 0x7000;
        HudMap_WriteTiles(x, y + i, t, 6);
        tile += 0x10;
    }
    MenuText_ClearAndPrint(gEwramData->pauseMenuPage + 0x362);
}

/**
 * @brief 47CC8 | Draw the list scroll arrows of entity at x 8 (frame 0x18):
 * the up arrow at y 0x54 when unk_4FA (presumably the first shown row) is
 * not 0, the down arrow (flipped) at y 0x6C when unk_4F8 (row count?)
 * > unk_4FA + 8.
 */
void MenuCursor_DrawScrollArrows(struct EwramData_EntityData *entity)
{
    entity->posX.part16.integer = 8;
    entity->animFrame = 0x18;
    if (entity->ext.menuPageList.bytes[2] != 0)
    {
        entity->posY.part16.integer = 0x54;
        Sprite_Draw(entity);
    }
    if (entity->ext.menuPageList.bytes[0] > entity->ext.menuPageList.bytes[2] + 8)
    {
        entity->posY.part16.integer = 0x6C;
        entity->flipY = 1;
        Sprite_Draw(entity);
        entity->flipY = 0;
    }
}

/**
 * @brief 47D24 | Draw the cursor of entity on the two-column grid at slot
 * unk_4F9 - unk_4FA, bobbing 3 px with frameCounter, then the scroll arrows
 * and at (0x80, 0x38) the frame chosen by bits 2/1/0 of statusEffects.
 */
void ItemMenu_DrawCursor(struct PlayerStatus *menu, struct EwramData_EntityData *entity)
{
    s32 index = entity->ext.menuPageList.bytes[1] - entity->ext.menuPageList.bytes[2];
    u32 t = gEwramData->frameCounter >> 1;
    s32 jitter;
    s32 flags;
    s32 kind;

    if (t & 8)
        jitter = 3 - (t & 7);
    else
        jitter = (t & 7) - 3;

    entity->posX.part16.integer = (index & 1) * 112 + 8 + jitter;
    entity->posY.part16.integer = (index & 6) * 4 + 0x54;
    Sprite_Draw(entity);
    MenuCursor_DrawScrollArrows(entity);

    flags = gEwramData->playerStatus.statusEffects;
    if (flags & 4)
        kind = 0x31;
    else if (flags & 2)
        kind = 2;
    else if (flags & 1)
        kind = 3;
    else
        kind = 1;
    entity->posX.part16.integer = 0x80;
    entity->posY.part16.integer = 0x38;
    entity->animFrame = kind;
    Sprite_Draw(entity);
}


/**
 * @brief 47DD4 | Draw entity as the cursor at (0x18 +- 3, 0x24 + 16 *
 * unk_4F9), then at x 0xAC the four frames sButtonConfigCursorFrames[unk_4FC..4FF]
 * at y 0x24 + 16 * i.
 */
void ButtonConfigMenu_DrawCursor(struct PlayerStatus *menu, struct EwramData_EntityData *entity)
{
    u32 t = gEwramData->frameCounter >> 1;
    s32 jitter;
    s32 i;
    s32 y;

    if (t & 8)
        jitter = 3 - (t & 7);
    else
        jitter = (t & 7) - 3;

    entity->posX.part16.integer = jitter + 0x18;
    entity->posY.part16.integer = entity->ext.menuPageList.bytes[1] * 16 + 0x24;
    Sprite_Draw(entity);
    entity->posX.part16.integer = 0xAC;
    for (i = 3; i >= 0; i--)
    {
        entity->posY.part16.integer = 0x24 + i * 0x10;
        entity->animFrame = sButtonConfigCursorFrames[entity->ext.menu.pageButtons[i]];
        Sprite_Draw(entity);
    }
}


/**
 * @brief 47E6C | Draw entity as the cursor for mode subState: 1 = two
 * columns 128 px apart by bit 0 of unk_4FA, row unk_4F9; 2 = as
 * ItemMenu_DrawCursor with the scroll arrows.
 */
void BestiaryMenu_DrawCursor(struct PlayerStatus *menu, struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 1:
        {
            u32 t = gEwramData->frameCounter >> 1;
            s32 jitter;

            if (t & 8)
                jitter = 3 - (t & 7);
            else
                jitter = (t & 7) - 3;
            entity->posX.part16.integer = (entity->ext.menuPage.top & 1) * 128 + 8 + jitter;
            entity->posY.part16.integer = entity->ext.menuPageList.bytes[1] * 16 + 0x34;
            Sprite_Draw(entity);
            break;
        }
        case 2:
        {
            s32 index = entity->ext.menuPageList.bytes[1] - entity->ext.menuPageList.bytes[2];
            u32 t = gEwramData->frameCounter >> 1;
            s32 jitter;

            if (t & 8)
                jitter = 3 - (t & 7);
            else
                jitter = (t & 7) - 3;
            entity->posX.part16.integer = (index & 1) * 112 + 8 + jitter;
            entity->posY.part16.integer = (index & 6) * 4 + 0x54;
            Sprite_Draw(entity);
            MenuCursor_DrawScrollArrows(entity);
            break;
        }
        case 0:
            break;
    }
}


/**
 * @brief 47F34 | Load the language dependent graphics set into unk_4F5 and
 * set up five ObjPalette_LoadRange entries (1, k) = 10..14 of sMenuCursorPalette.
 */
void MenuCursor_LoadGraphics(struct EwramData_EntityData *entity)
{
    u8 *set;

    switch (gEwramData->gameState.language)
    {
        case 2:
            set = sMenuCursorGfxLanguage2;
            break;
        case 3:
            set = sMenuCursorGfxLanguage3;
            break;
        default:
            set = sMenuCursorGfx;
            break;
    }
    entity->ext.menu.gfxSlot = ObjGfx_LoadResource(set, 0);
    ObjPalette_LoadRange(sMenuCursorPalette, 2, 1, 10);
    ObjPalette_LoadRange(sMenuCursorPalette, 3, 1, 11);
    ObjPalette_LoadRange(sMenuCursorPalette, 4, 1, 12);
    ObjPalette_LoadRange(sMenuCursorPalette, 1, 1, 13);
    ObjPalette_LoadRange(sMenuCursorPalette, 0, 1, 14);
}


/**
 * @brief Place and draw the menu cursor (entity 18) at the selected row, or
 * the bobbing yes/no cursor plus the row marker during confirmation.
 */
void Shop_DrawListCursor(struct EwramData_EntityData *entity)
{
    if (entity->stage == 0)
    {
        entity->unk_518.part16.hp = 12;
        entity->unk_518.part16.unk_51A = entity->ext.menuPageList.bytes[1] * 8 + 0x5C;
        Cursor_Draw();
    }
    else
    {
        entity->unk_518.part16.hp = entity->timer ? 0x88 : 0x3C;
        entity->unk_518.part16.unk_51A = 0x88;
        Cursor_DrawWithOptionalBob(1);
        if (gEwramData->frameCounter & 2)
        {
            entity->unk_518.part16.hp = 0x10;
            entity->unk_518.part16.unk_51A = entity->ext.menuPageList.bytes[1] * 8 + 0x5C;
            Cursor_DrawWithOptionalBob(0);
        }
    }
}


/**
 * @brief 48018 | Draw the cursor entityData[18] (frame 0x1E) at its stored
 * position unk_518/unk_51A, bobbing horizontally.
 */
void Cursor_Draw(void)
{
    struct EwramData_EntityData *cursor = &gEwramData->entityData[18];
    u32 t;
    s32 jitter;

    cursor->animFrame = 0x1E;
    cursor->palette = 13;
    t = gEwramData->frameCounter >> 1;
    if (t & 8)
        jitter = 3 - (t & 7);
    else
        jitter = (t & 7) - 3;
    cursor->posX.part16.integer = cursor->unk_518.part16.hp + jitter;
    cursor->posY.part16.integer = cursor->unk_518.part16.unk_51A;
    Sprite_Draw(cursor);
}


/**
 * @brief 48084 | As Cursor_Draw; the bobbing only when bob != 0.
 */
void Cursor_DrawWithOptionalBob(s32 bob)
{
    struct EwramData_EntityData *cursor = &gEwramData->entityData[18];

    cursor->animFrame = 0x1E;
    cursor->palette = 13;
    cursor->posX.part16.integer = cursor->unk_518.part16.hp;
    cursor->posY.part16.integer = cursor->unk_518.part16.unk_51A;
    if (bob)
    {
        u32 t = gEwramData->frameCounter >> 1;
        s32 jitter;

        if (t & 8)
            jitter = 3 - (t & 7);
        else
            jitter = (t & 7) - 3;
        cursor->posX.part16.integer += jitter;
    }
    Sprite_Draw(cursor);
}
