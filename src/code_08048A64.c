/* Helpers around fixed entity slots 18, 19 and 0xD0..0xD2
 * (0x08048A64..0x08048C43). */
#include "code_08009A0.h"
#include "menu_page.h"
#include "code_08011DD0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void Shop_DrawListCursor(struct EwramData_EntityData *entity);
extern void Cursor_Draw(void);
void MenuItemIcon_Draw(void);

/* Animation sets for Sprite_Attach; the second one is used for German. */
extern u8 sMenuSpriteSheet[];
extern u8 sMenuPageTitleSpriteSheet[];

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
            entity->unk_518.part16.hp = 0x30;
            entity->unk_518.part16.unk_51A = entity->subState * 24 + 0x28;
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

    Sprite_Attach(cursor, sMenuSpriteSheet, MENU_GFX_SLOT(cursor), 0xD);
    cursor->animFrame = 0x1E;
}

/**
 * @brief 48AD8 | Set the 0x518/0x51A values of entity slot 18 from tile
 * coordinates (param * 8 plus offsets 6 and 3).
 */
void Cursor_SetTilePosition(s32 column, s32 row)
{
    struct EwramData_EntityData *cursor = &gEwramData->entityData[18];

    cursor->unk_518.part16.hp = column * 8 + 6;
    cursor->unk_518.part16.unk_51A = row * 8 + 3;
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
    s32 tileSlot = MENU_GFX_SLOT(cursor);

    if (gEwramData->gameState.language == 3)
        sheet = sMenuPageTitleSpriteSheet;

    Sprite_Attach(title, sheet, tileSlot, 0xE);
    MENU_TITLE_AFFINE(title) = ObjAffine_Alloc(2);
    title->animFrame = frame;
    MENU_TITLE_STATE(title)++;
}

/**
 * @brief Release the affine slot allocated by MenuPageTitle_Create, request
 * deletion and reset the state.
 */
void MenuPageTitle_Delete(struct EwramData_EntityData *title)
{
    ObjAffine_Free(MENU_TITLE_AFFINE(title));
    title->deleteRequested = 1;
    MENU_TITLE_STATE(title) = 0;
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
