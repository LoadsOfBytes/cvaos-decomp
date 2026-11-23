/* Item / gold / soul pickup handler (Pickup_Collect). Needs -fno-cse-skip-blocks
 * (Makefile): the target reloads the inventory count after the 0x4EF test. */
#include "code_0800CB00.h"
#include "code_080D73B8.h"
#include "code_08032444.h"
#include "structs/ewram.h"

struct GoldDrop { u16 amount; u16 pad; s32 message; u8 rest[4]; };
extern struct GoldDrop sGoldDrops[];
extern u8 sConsumableInfo[], sWeaponInfo[], sArmorInfo[];
extern const u16 sItemNameTextIds[];
extern void Player_RestoreMPPercent(s32 percent);
extern void EquipmentList_Insert(s32 type, s32 item);
extern s32 SoulAcquisition_CreateEffect(s32);
extern void Effect_SpawnAttachedAnimation(struct EwramData_EntityData *, u8, u8, s32, s32);
extern void SoulList_Insert(s32 kind, s32 soul);

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

static inline u8 *Inv(struct PlayerStatus *v, s32 t)
{
    switch (t)
    {
    case 2: return v->itemInventory;
    case 3: return v->weaponInventory;
    case 4: return v->armorInventory;
    }
    return v->itemInventory;
}
static inline u8 *InvSlot(struct PlayerStatus *v, s32 t, s32 i)
{
    switch (t)
    {
    case 2: return &v->itemInventory[i];
    case 3: return &v->weaponInventory[i];
    case 4: return &v->armorInventory[i];
    }
    return v->itemInventory;
}
static inline s32 CountA(struct PlayerStatus *v, s32 t, s32 i) { return Inv(v, t)[i]; }
static inline s32 CountB(struct PlayerStatus *v, s32 t, s32 i) { return *InvSlot(v, t, i); }

/**
 * @brief Apply the pickup type reward, inventory limits, messages, effects and persistence flags; retain the full-inventory path that cancels deletion.
 */
void Pickup_Collect(struct EwramData_EntityData *e)
{
    register struct EwramData **pg asm("r2") = &gEwramData; /* FAKEMATCH: pinned register */
    struct PlayerStatus *inv = &(*pg)->playerStatus;
    s32 isNew = 0;
    u8 *slot;

    e->deleteRequested = 1;
    switch (e->unk_518.part8.enemyId)
    {
    case 0:
        Player_RestoreMPPercent(e->unk_514.part16.spawnParam1);
        PlaySong(0xB4);
        break;
    case 1:
    {
        u32 *gold;
        u16 amount = sGoldDrops[e->unk_514.part16.spawnParam1].amount;
        gold = &(*pg)->playerStatus.currentGold;
        *gold += amount;
        if (*gold > 999999)
            *gold = 999999;
        Hud_RequestPickupMessage(sGoldDrops[e->unk_514.part16.spawnParam1].message);
        PlaySong(0xB6);
        break;
    }
    case 2:
    {
        u16 *info;
        s32 id;
        u8 *items;
        s32 c;

        info = LocalInfo(e->unk_518.part8.enemyId, e->unk_514.part16.spawnParam1);
        id = e->unk_514.part16.spawnParam1;
        c = inv->itemInventory[id];
        items = inv->itemInventory;
        if (c > 8 && !(e->subState & 2))
            goto full;
        if (items[id] < 9)
            items[id] += 1;
        Hud_RequestPickupMessage(sItemNameTextIds[*info]);
        PlaySong(0xB5);
        break;
    }
    case 3:
    {
        u16 *info;
        s32 id;
        u8 *items;
        s32 c;

        info = LocalInfo(e->unk_518.part8.enemyId, e->unk_514.part16.spawnParam1);
        id = e->unk_514.part16.spawnParam1;
        c = inv->weaponInventory[id];
        items = inv->weaponInventory;
        if (c > 8 && !(e->subState & 2))
            goto full;
        if (items[id] < 9)
            items[id] += 1;
        Hud_RequestPickupMessage(sItemNameTextIds[*info]);
        EquipmentList_Insert(3, id);
        PlaySong(0xB7);
        break;
    }
    case 4:
    {
        u16 *info;
        s32 id;
        u8 *items;
        s32 c;

        info = LocalInfo(e->unk_518.part8.enemyId, e->unk_514.part16.spawnParam1);
        id = e->unk_514.part16.spawnParam1;
        c = inv->armorInventory[id];
        items = inv->armorInventory;
        if (c > 8 && !(e->subState & 2))
        {
        full:
            if (e->ext.pickup.delay == 0)
                PlaySong(0x13B);
            e->ext.pickup.delay = 0x20;
            e->deleteRequested = 0;
            return;
        }
        if (items[id] < 9)
            items[id] += 1;
        Hud_RequestPickupMessage(sItemNameTextIds[*info]);
        EquipmentList_Insert(4, id);
        PlaySong(0xB7);
        break;
    }
    case 5:
    case 6:
    case 7:
    case 8:
    {
        s32 soul = e->unk_514.part16.spawnParam1;
        u8 *type = &e->unk_518.part8.enemyId;
        s32 kind = *type - 5;
        if (SoulInventory_GetSoulTotal(kind, soul) == 0)
        {
            SoulAcquisition_CreateEffect(kind);
            isNew = 1;
            if (*type == 8)
                Player_SetAbility(soul, 1);
        }
        else
            Effect_SpawnAttachedAnimation(*(struct EwramData_EntityData **)((u8 *)gEwramData + 0x13110), 12, 6, 0, 0xFFF00000);
        SoulInventory_AddAmountToSoulTotal(kind, soul, 1);
        SoulInventory_AddCollectedTotal(1);
        Hud_ShowSoulAcquiredMessage(soul, kind, isNew);
        SoulList_Insert(kind, soul);
        PlaySong(0xBC);
        break;
    }
    }

    {
        register u16 flag asm("r5") = e->unk_514.part16.spawnParam0; /* FAKEMATCH: pinned register */
        s32 sh = flag << 16;
        if (sh)
        {
            if (!(e->subState & 2))
                gEwramData->gameState.collectedPickupFlags[sh >> 21] |= 1 << (flag & 0x1F);
            else
            {
                gEwramData->gameState.bossEventFlags |= 1 << e->unk_514.part16.spawnParam0;
                gEwramData->gameState.stateFlags &= ~2;
            }
        }
    }
    if (e->subState & 1)
        gEwramData->gameState.stateFlags |= 0x400;
}
