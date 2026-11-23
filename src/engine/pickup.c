/*
 * Item lookup by global id, MP restore, the mana / gold / item pickups and collecting them.
 * Built with -fno-cse-skip-blocks (Pickup_Collect needs it; the rest compiles the same either way).
 */
#include "structs/ewram.h"
#include "core.h"
#include "graphics.h"
#include "gameplay.h"
#include "bg_and_collision.h"
#include "entity_hit.h"
#include "map_flags.h"
#include "bg_effects.h"
#include "sound.h"
#include "soul_inventory.h"
#include "effects.h"
#include "equipment_menu.h"
#include "math_anim.h"
#include "obj_gfx.h"
#include "pickup.h"
#include "soul_menu.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"

static inline s32 Item_GetTypeByGlobalId(s32 globalId)
{
    s32 type = 0;

    if (globalId == 0)
        return 0;
    globalId--;
    if (globalId <= 0x1F)
        type = 2;
    else if (globalId <= 0x5A)
        type = 3;
    else if (globalId <= 0x87)
        type = 4;
    return type;
}

static inline s32 Item_GetIndexByGlobalId(s32 globalId)
{
    s32 index = 0;

    if (globalId == 0)
        return 0;
    globalId--;
    if (globalId <= 0x1F)
        index = globalId;
    else if (globalId <= 0x5A)
        index = globalId - 0x20;
    else if (globalId <= 0x87)
        index = globalId - 0x5B;
    return index;
}

static inline u8 *ItemInventory_GetCountPtr2(s32 type, s32 index)
{
    struct PlayerStatus *playerStatus = &gEwramData->playerStatus;

    switch (type)
    {
        case 2:
            return &playerStatus->itemInventory[index];
        case 3:
            return &playerStatus->weaponInventory[index];
        case 4:
            return &playerStatus->armorInventory[index];
    }
    return playerStatus->itemInventory;
}

/* Add one item to the inventory, capped at nine copies. */
/**
 * @brief Resolve the one-based item identifier and increment its inventory count below nine; preserve the invalid-type fallback.
 */
void ItemInventory_AddOneByGlobalId(s32 item)
{
    s32 type = Item_GetTypeByGlobalId(item);
    s32 index = Item_GetIndexByGlobalId(item);
    u8 *count = ItemInventory_GetCountPtr2(type, index);
    if (*count < 9) (*count)++;
}

/* Player_RestoreMPPercent. */
/**
 * @brief Apply the original signed fixed-point percentage of maximum MP, substitute one for a zero amount and cap only at maximum MP.
 */
void Player_RestoreMPPercent(s32 percent)
{
    s32 v = (gEwramData->playerStatus.maxMP << 16) / 100 * percent;
    s32 amount = (v < 0) ? -(-v >> 16) : v >> 16;

    if (amount == 0)
        amount = 1;
    gEwramData->playerStatus.currentMP += amount;
    if (gEwramData->playerStatus.currentMP > gEwramData->playerStatus.maxMP)
        gEwramData->playerStatus.currentMP = gEwramData->playerStatus.maxMP;
}

/* Spawn a pickup-like object at (x, y): graphics 081C15F4 / 0820ED60
 * (palette 7), touch callback Pickup_Collect, box 0x0808FCFC passed by value
 * (x -4, y -4, w 8, h 8); large (frame 0x27, spawnParam1 15, update
 * Pickup_Update) or small (frame 0x28, spawnParam1 3); optionally thrown up. */
extern void Pickup_UpdateDriftingMana(struct EwramData_EntityData *);
extern void Pickup_Update(struct EwramData_EntityData *);

/**
 * @brief Create a small or large MP pickup with the existing hit callback, frame, percentage and optional upward velocity; return NULL on allocation failure.
 */
struct EwramData_EntityData *Pickup_CreateMana(s32 x, s32 y, s32 large, s32 jump)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)Pickup_UpdateDriftingMana);
    s32 graphics;

    if (entity != NULL)
    {
        entity->spriteLayer = 0x13;
        graphics = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, 7);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        Hit_Enable(entity, 0xB, Pickup_Collect, 0);
        entity->hit.flags |= 4;
        HitBox_SetAttackRaw(entity, 0x0808FCFC);
        entity->posX.part16.integer = x;
        entity->posY.part16.integer = y;
        entity->scrollWithBg = 1;
        entity->enemyId = 0;
        if (large)
        {
            entity->animFrame = 0x27;
            entity->spawnParam1 = 0xF;
            entity->updateFunc = (u32 *)Pickup_Update;
        }
        else
        {
            entity->animFrame = 0x28;
            entity->spawnParam1 = 3;
        }
        if (jump)
            entity->velY.whole = -0x20000;
    }
    return entity;
}

/* Falling/settling pickup update (set by Pickup_CreateMana): rise until velY >= 0,
 * then sway down (sine of timer) until the ground (Collision_GetPushUp) is hit,
 * wait 200 frames, blink 64 frames and disappear. */

static inline s32 RoomX(s32 pos) { struct BgLayerState *bg = &gEwramData->bgInfo[1]; return pos + (s16)bg->xPos.part16.integer; }
static inline s32 RoomY(s32 pos) { struct BgLayerState *bg = &gEwramData->bgInfo[1]; return pos + (s16)bg->yPos.part16.integer; }
/**
 * @brief Check player contact, rise then drift down to the ground, wait 200 frames and request deletion after the final 64-frame draw-skip phase.
 */
void Pickup_UpdateDriftingMana(struct EwramData_EntityData *entity)
{
    struct HitResult res;
    s16 dy;

    Hit_CheckWithPlayer(&res, (u8 *)entity);
    switch (entity->state)
    {
    case 0:
        if (entity->velY.whole >= 0)
            entity->state = 1;
        else
        {
            entity->velY.whole += 0x2000;
            entity->posY.whole += entity->velY.whole;
        }
        break;
    case 1:
    {
        entity->velY.whole = 0x4000;
        entity->velX.whole = Sin((entity->timer << 9) + 0x4000) >> 1;
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        entity->timer++;
        dy = Collision_GetPushUp(RoomX((s16)entity->posX.part16.integer), RoomY((s16)entity->posY.part16.integer) + 4);
        if (dy != 0)
        {
            entity->posY.whole -= dy;
            entity->timer = 200;
            entity->state++;
        }
        break;
    }
    case 2:
        if (--entity->timer == 0)
        {
            entity->timer = 64;
            entity->state++;
        }
        break;
    case 3:
        entity->timer--;
        entity->drawSkip = 1;
        if (entity->timer == 0)
            entity->deleteRequested = 1;
        break;
    }
}

/* Update of item/pickup objects (Pickup_CreateMana large, Pickup_CreateGold): fall with
 * gravity until the ground (Collision_GetPushUp, probe +8 px for enemyId <= 1),
 * or bob on a sine for enemyId > 4; then wait 300 frames, blink 64 and
 * disappear; states 0x63-0x65 flash the palette. */

/**
 * @brief Check player contact, apply type-dependent falling or bobbing motion, countdown expiration or palette-flash states, and optional animation.
 */
void Pickup_Update(struct EwramData_EntityData *entity)
{
    struct HitResult res;
    s32 dy;

    Hit_CheckWithPlayer(&res, (u8 *)entity);
    if (entity->enemyId <= 4)
    {
        s32 x, y, h;
        entity->posY.whole += entity->velY.whole;
        x = RoomX((s16)entity->posX.part16.integer);
        y = RoomY((s16)entity->posY.part16.integer);
        h = entity->accelY.part16.integer;
        dy = (s16)Collision_GetPushUp(x, entity->enemyId <= 1 ? y + 8 + h : y + h);
        entity->velY.whole += 0x2000;
        if ((u32)entity->velY.whole > 0x40000)
            entity->velY.whole = 0x40000;
        if (entity->ext.pickup.delay != 0)
            entity->ext.pickup.delay--;
        if (dy != 0)
        {
            entity->posY.part16.integer += dy;
            entity->velY.whole = 0;
            if (entity->state == 0)
            {
                entity->ext.pickup.lifetime = 300;
                entity->state++;
            }
        }
    }
    else
    {
        s32 v = Sin((u8)entity->ext.pickup.bob.low << 9) / 4;
        entity->velY.whole = v;
        entity->ext.pickup.bob.frame++;
        entity->posY.whole += v;
    }
    switch (entity->state)
    {
    case 1:
        if (--entity->ext.pickup.lifetime == 0)
        {
            entity->timer = 64;
            entity->state++;
        }
        break;
    case 2:
        entity->timer--;
        entity->drawSkip = 1;
        if (entity->timer == 0)
            entity->deleteRequested = 1;
        break;
    case 0x63:
        entity->basePalette = entity->palette;
        entity->state = 0x64;
        entity->timer = 0x80;
        break;
    case 0x64:
        if (entity->timer != 0)
            entity->timer--;
        else
        {
            entity->palette = 4;
            entity->state = 0x65;
            entity->timer = 4;
        }
        break;
    case 0x65:
        if (entity->timer != 0)
            entity->timer--;
        else
        {
            entity->palette = entity->basePalette;
            entity->state = 0x64;
            entity->timer = 0x80;
        }
        break;
    }
    if (entity->stage != 0)
        Anim_Update(entity);
}

/* Spawn an item object at (x, y) from table 085250E4[index] (palette,
 * frame, animated flag): graphics 081C15F4 / 0820ED60, touch callback
 * Pickup_Collect, box 0x1010F8F8 by value (x -8, y -8, w 16, h 16);
 * updated by Pickup_Update. */
/**
 * @brief Create a table-selected gold pickup with its frame or animation, palette and collection callback; return NULL on entity allocation failure.
 */
struct EwramData_EntityData *Pickup_CreateGold(s32 x, s32 y, s32 index)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)Pickup_Update);
    if (entity != NULL)
    {
        struct GoldDrop *info = &sGoldDrops[index];
        s32 graphics;
        entity->spriteLayer = 9;
        graphics = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, info->palette);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        if (info->animated)
        {
            entity->accelY.part16.integer = -8;
            entity->stage = 1;
            Anim_StartIndexed(entity, info->frame, 0, 1);
            Anim_Update(entity);
        }
        else
            entity->animFrame = info->frame;
        Hit_Enable(entity, 0xB, Pickup_Collect, 0);
        entity->hit.flags |= 4;
        HitBox_SetAttackRaw(entity, 0x1010F8F8);
        entity->posX.part16.integer = x;
        entity->posY.part16.integer = y;
        entity->scrollWithBg = 1;
        entity->enemyId = 1;
        entity->spawnParam1 = index;
    }
    return entity;
}

struct ItemRecord { u16 name; u8 icon; s8 palette; };
extern u8 sConsumableInfo[], sWeaponInfo[], sArmorInfo[];

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
        do { entity = EntityCreateInRange(0x49, 0xCF, (u32 *)Pickup_Update); } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
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
        entity->enemyId = type;
        entity->spawnParam1 = id;
    }
    if (hidden)
        entity->state = 0x63;
done:
    return entity;
}

/* Item / gold / soul pickup handler (Pickup_Collect). Needs -fno-cse-skip-blocks
 * (Makefile): the target reloads the inventory count after the 0x4EF test. */

extern void Effect_SpawnAttachedAnimation(struct EwramData_EntityData *, u8, u8, s32, s32);

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
    switch (e->enemyId)
    {
    case 0:
        Player_RestoreMPPercent(e->spawnParam1);
        PlaySong(0xB4);
        break;
    case 1:
    {
        u32 *gold;
        u16 amount = sGoldDrops[e->spawnParam1].amount;
        gold = &(*pg)->playerStatus.currentGold;
        *gold += amount;
        if (*gold > 999999)
            *gold = 999999;
        Hud_RequestPickupMessage(sGoldDrops[e->spawnParam1].message);
        PlaySong(0xB6);
        break;
    }
    case 2:
    {
        u16 *info;
        s32 id;
        u8 *items;
        s32 c;

        info = LocalInfo(e->enemyId, e->spawnParam1);
        id = e->spawnParam1;
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

        info = LocalInfo(e->enemyId, e->spawnParam1);
        id = e->spawnParam1;
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

        info = LocalInfo(e->enemyId, e->spawnParam1);
        id = e->spawnParam1;
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
        s32 soul = e->spawnParam1;
        u8 *type = &e->enemyId;
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
        register u16 flag asm("r5") = e->spawnParam0; /* FAKEMATCH: pinned register */
        s32 sh = flag << 16;
        if (sh)
        {
            if (!(e->subState & 2))
                gEwramData->gameState.collectedPickupFlags[sh >> 21] |= 1 << (flag & 0x1F);
            else
            {
                gEwramData->gameState.bossEventFlags |= 1 << e->spawnParam0;
                gEwramData->gameState.stateFlags &= ~2;
            }
        }
    }
    if (e->subState & 1)
        gEwramData->gameState.stateFlags |= 0x400;
}
