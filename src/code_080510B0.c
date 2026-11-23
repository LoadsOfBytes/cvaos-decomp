#include "code_08009A0.h"
#include "room_object_entity.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, u8, void *);
extern s32 Item_GetTypeByGlobalId(s32 item);
extern s32 Item_GetIndexByGlobalId(s32 item);
extern void BossRushResult_Draw(struct EwramData_EntityData *);

struct Object10Reward
{
    u32 maxValue;
    s32 item;
};

extern u8 sObject10Gfx[], sObject10Palette[], sObject10SpriteSheet[];
extern u8 sObject10GfxLanguage2[], sObject10SpriteSheetLanguage2[], sObject10GfxLanguage3[], sObject10SpriteSheetLanguage3[];
extern struct Object10Reward sObject10Rewards[];

void Object10Create(struct EwramData_EntityData *entity)
{
    u32 value;
    struct Object10Reward *reward;
    s32 i;
    s32 item;
    struct EwramData_EntityData *child;

    switch (gEwramData->gameState.language)
    {
    case 2:
        if (!Entity_LoadGraphics(entity, sObject10GfxLanguage2, sObject10Palette, 0, sObject10SpriteSheetLanguage2))
            goto fail;
    case 3:
        if (!Entity_LoadGraphics(entity, sObject10GfxLanguage3, sObject10Palette, 0, sObject10SpriteSheetLanguage3))
            goto fail;
        break;
    default:
        if (!Entity_LoadGraphics(entity, sObject10Gfx, sObject10Palette, 0, sObject10SpriteSheet))
        {
        fail:
            EntityDelete(entity);
            return;
        }
        break;
    }
    child = &gEwramData->entityData[26];
    value = child->ext.bossRushTimer.time;
    for (reward = sObject10Rewards, i = 0; i <= 2; i++, reward++)
    {
        if (value <= reward->maxValue && *ItemInventory_GetCountPtr(Item_GetTypeByGlobalId(reward->item), Item_GetIndexByGlobalId(reward->item)) == 0)
        {
            item = reward->item;
            goto found;
        }
    }
    item = 1;
found:
    entity->ext.object10.rewardItem = item;
    child = Pickup_CreateItem(entity->posX.part16.integer, (u16)entity->posY.part16.integer, 0xFF, item, 1);
    if (child != NULL)
        child->subState = 1;
    entity->scrollWithBg = 0;
    entity->hidden = 1;
    entity->posX.part16.integer = 0x80;
    entity->posY.part16.integer = 0x50;
    entity->drawFunc = (u32 *)BossRushResult_Draw;
}
