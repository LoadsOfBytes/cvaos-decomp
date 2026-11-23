/* Menu item-icon helpers 0x08048804..0x08048A63. */
#include "code_08009A0.h"
#include "menu_page.h"
#include "code_08011DD0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

void ItemMenu_DrawCursor(struct PlayerStatus *menu, struct EwramData_EntityData *entity);
void ButtonConfigMenu_DrawCursor(struct PlayerStatus *menu, struct EwramData_EntityData *entity);
void BestiaryMenu_DrawCursor(struct PlayerStatus *menu, struct EwramData_EntityData *entity);
extern const void *Item_GetInfo(s32 type, s32 item);
extern const void *Item_GetInfoByGlobalId(s32 item);
extern s32 Item_GetTypeByGlobalId(s32 item);

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
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
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
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
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
