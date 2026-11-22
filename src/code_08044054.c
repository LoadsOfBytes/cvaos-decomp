#include "code_08009A0.h"
#include "code_08011DD0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

struct ItemRecord { u16 name; u8 icon; s8 palette; };
extern u8 sConsumableInfo[], sWeaponInfo[], sArmorInfo[];
extern u8 sUnk_085059BC[], sUnk_08505C94[];
extern void Pickup_Update(struct EwramData_EntityData *);
extern void Pickup_Collect(struct EwramData_EntityData *);
extern struct EwramData_EntityData *Pickup_CreateGold(s32, s32, s32);

static inline void *LocalInfo(s32 type, s32 id)
{
    void *record = NULL;
    switch(type)
    {
    case 2: record = sConsumableInfo + id * 16; break;
    case 3: record = sWeaponInfo + id * 28; break;
    case 4: record = sArmorInfo + id * 20; break;
    }
    return record;
}

static inline void *GlobalInfo(s32 id)
{
    void *record = NULL;
    if (id == 0) return NULL;
    else
    {
        id--;
        if (id <= 31) record = sConsumableInfo + id * 16;
        else if (id <= 90) record = sUnk_085059BC + id * 28;
        else if (id <= 135) record = sUnk_08505C94 + id * 20;
    }
    return record;
}

static inline s32 GlobalType(s32 id)
{
    s32 result = 0;
    if (id == 0) return 0;
    else
    {
        id--;
        if (id <= 31) result = 2;
        else if (id <= 90) result = 3;
        else if (id <= 135) result = 4;
    }
    return result;
}

static inline s32 LocalId(s32 id)
{
    s32 result = 0;
    if (id == 0) return 0;
    else
    {
        s32 index = id - 1;
        if (index <= 31) result = index;
        else if (index <= 90) result = id - 0x21;
        else if (index <= 135) result = id - 0x5C;
    }
    return result;
}

/**
 * @brief Create gold or an item pickup, resolving type 0xFF through the global item identifier and optionally selecting state 0x63; retain existing index assumptions.
 */
struct EwramData_EntityData *Pickup_CreateItem(s32 x, s32 y, s32 type, s32 id, s32 hidden)
{
    struct EwramData_EntityData *entity;
    struct ItemRecord *info;
    if (type == 1)
    {
        entity = Pickup_CreateGold(x, y, id);
        if (!entity)
            goto done;
    }
    else
    {
        do { entity = EntityCreateInRange(0x49, 0xCF, (u32 *)Pickup_Update); } while (0);
        if (!entity)
            goto done;
        if (type != 0xFF)
            info = LocalInfo(type, id);
        else
        {
            info = GlobalInfo(id);
            type = GlobalType(id);
            id = LocalId(id);
        }
        entity->spriteLayer = 0x13;
        ItemIcon_AttachSprite(entity, ItemIconCache_Load(info->icon), info->palette);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        Hit_Enable(entity, 0xB, Pickup_Collect, 0);
        { struct EwramData_EntityData *e = entity; e->hit.flags |= 4; }
        HitBox_SetAttackRaw(entity, 0x1010F8F8);
        entity->posX.part16.integer = x;
        entity->posY.part16.integer = y;
        entity->scrollWithBg = 1;
        entity->unk_518.part8.enemyId = type;
        entity->unk_514.part16.spawnParam1 = id;
    }
    if (hidden)
        entity->state = 0x63;
done:
    return entity;
}
