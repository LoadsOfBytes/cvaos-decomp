/*
 * Shared object palettes (find / load / release) and enemy reward drops.
 */
#include "obj_palette.h"
#include "structs/ewram.h"
#include "core.h"
#include "bg_and_collision.h"
#include "soul_inventory.h"
#include "player_level.h"
#include "syscalls.h"
#include "palette.h"
#include "pickup.h"

/* Find or load a palette in the four shared object slots. */
s8 ObjPalette_FindOrLoad(void *palette)
{
    s8 index;
    for (index = 0; index < 4; index++)
    {
        struct EwramData *ram = gEwramData;
        if ((void *)ram->gameState.sharedObjPaletteSources[index] == palette && ram->gameState.sharedObjPaletteVariants[index] == 0)
            return index + 10;
        if (gEwramData->gameState.sharedObjPaletteSources[index] == 0)
        {
            gEwramData->gameState.sharedObjPaletteSources[index] = (s32)palette;
            gEwramData->gameState.sharedObjPaletteVariants[index] = 0;
            ObjPalette_LoadRange(palette, 0, 1, index + 10);
            return index + 10;
        }
    }
    return -1;
}

/* Find or load a palette variant in the four shared object slots. */
s8 ObjPalette_FindOrLoadVariant(void *palette, u8 variant)
{
    s8 index;
    for (index = 0; index < 4; index++)
    {
        struct EwramData *ram = gEwramData;
        if ((void *)ram->gameState.sharedObjPaletteSources[index] == palette && ram->gameState.sharedObjPaletteVariants[index] == variant)
            return index + 10;
        if (gEwramData->gameState.sharedObjPaletteSources[index] == 0)
        {
            gEwramData->gameState.sharedObjPaletteSources[index] = (s32)palette;
            gEwramData->gameState.sharedObjPaletteVariants[index] = variant;
            ObjPalette_LoadRange(palette, variant, 1, index + 10);
            return index + 10;
        }
    }
    return -1;
}

/* Restore the four cached palette rows reserved for shared objects. */
/**
 * @brief Reload the four shared object palette rows 10..13 from their cached
 * sources (after the pause menu).
 */
void SharedObjPalette_RestoreAll(void)
{
    s8 index;
    for(index=0;index<4;index++)
    {
        struct EwramData *ram=gEwramData;
        void *palette=(void *)ram->gameState.sharedObjPaletteSources[index];
        if(palette)ObjPalette_LoadRange(palette,ram->gameState.sharedObjPaletteVariants[index],1,index+10);
    }
}

/* Release the cached slot for a palette and variant. */
static inline void ReleasePalette(u8 slot)
{
    s8 index = slot - 10;
    if (index >= 0)
    {
        if (index <= 3)
        {
            gEwramData->gameState.sharedObjPaletteSources[index] = 0;
            gEwramData->gameState.sharedObjPaletteVariants[index] = 0;
        }
    }
}
/**
 * @brief Release the shared object palette row that holds (palette, variant).
 */
void SharedObjPalette_Release(void *palette, u8 variant)
{
    s32 index;
    for (index = 0; index < 4; index++)
    {
        if ((void *)gEwramData->gameState.sharedObjPaletteSources[index] == palette && gEwramData->gameState.sharedObjPaletteVariants[index] == variant)
        {
            ReleasePalette(index + 10);
            break;
        }
    }
}

/* Local 0x24-byte template-table view: retain this TU's types and padding. */
struct EnemyTemplateView
{
    u8 pad_0[0x8];
    u16 item0;
    u16 item1;
    u16 hp;
    u16 mp;
    u16 exp;
    u8 soulRate;
    u8 pad_13[0x15 - 0x13];
    u8 rate0;
    u8 rate1;
    u8 soulType;
    u8 soulIndex; /* 1-based, 0 = none */
    u8 pad_19[0x24 - 0x19];
};

extern const struct EnemyTemplateView sEnemyData[];
extern void Effect_CreatePlayerFlash(s32);
extern s32 Player_GetEquippedItem(s32 kind);
extern struct EwramData_EntityData *SoulPickup_Create(s32 x, s32 y, s32 palette, s32 soulId, s32 stationary);
extern struct EwramData_EntityData *Pickup_CreateGold(s32 x, s32 y, s32 index);
extern struct EwramData_EntityData *Pickup_CreateMana(s32 x, s32 y, s32 large, s32 jump);

static inline s32 IsSolid(u8 t)
{
    if (t & 8)
    {
        if (t & 1)
            return 1;
        return 1;
    }
    return 0;
}

void Enemy_DropRewards(struct EwramData_EntityData *entity)
{
    u8 unused[0x10];
    s32 id;
    const struct EnemyTemplateView *info;
    struct EwramData *ew;
    struct PlayerStatus *stats;
    struct PlayerState *player;
    s32 x;
    s32 y;
    s32 which;
    s32 chance;
    s32 range;
    s32 luck;
    s32 rate;
    u8 t;
    u32 item;
    u32 r;
    u32 r2;

    id = entity->enemyId;
    info = &sEnemyData[id];
    ew = gEwramData;
    stats = &ew->playerStatus;
    player = &ew->playerState;
    if (entity->enemyFlags & 2)
        return;
    entity->enemyFlags |= 2;
    if (ew->gameState.bossRushMode)
        return;
    if (stats->currentCharacter)
        return;

    ew->playerStatus.enemyFlags[id >> 5] |= 1 << (id & 0x1F);
    range = info->exp;
    if (ew->playerStatus.soulStateFlags & 0x2000000)
        range = (u32)(range * 0x133) >> 8;
    if (range)
    {
        stats->currentExperience += range;
        if (stats->currentExperience > 99999999)
            stats->currentExperience = 99999999;
        if (Player_ApplyEarnedLevels(stats))
            Effect_CreatePlayerFlash(0);
    }
    x = (s16)entity->posX.part16.integer;
    y = (s16)entity->posY.part16.integer - 8;
    if (info->soulRate && info->soulIndex)
    {
        chance = 3;
        if (!SoulInventory_GetSoulTotal(info->soulType, info->soulIndex - 1))
        {
            chance = 6;
            if (gEwramData->gameState.difficultyMode)
                chance = 7;
        }
        if (Player_GetEquippedItem(1) == 0x28)
            chance += 8;
        rate = info->soulRate;
        rate *= 8;
        luck = (player->currentStats[3] >> 4) - 0x20;
        range = rate - luck;
        if (range <= 0xF)
            range = 0x10;
        range = Mod(RandomNumberGenerator() >> 2, range);
        if ((u32)range < chance)
        {
            SoulPickup_Create(x, y, info->soulType, info->soulIndex - 1, 0);
            return;
        }
    }
    t = Collision_GetTileOnScreen(x, y);
    if (t && !IsSolid(t))
        return;
    which = 0;
    chance = 0;
    if (Player_GetEquippedItem(1) == 0x27)
        chance = 0x20;
    if (((RandomNumberGenerator() >> 1) & 0xFF) < chance + 0x40)
    {
        item = info->item1;
        range = info->rate1;
        which = 1;
    }
    else
    {
        item = info->item0;
        range = info->rate0;
    }
    if (item)
    {
        chance = 4;
        if (Player_GetEquippedItem(1) == 0x27)
            chance = 8;
        rate = range * 4;
        luck = (player->currentStats[3] >> 4) - 0x10;
        range = rate - luck;
        if (range <= 0xF)
            range = 0x10;
        range = Mod(RandomNumberGenerator() >> 3, range);
        if ((u32)range < chance)
        {
            Pickup_CreateItem(x, y, 0xFF, item, 0);
            {
                s32 offset = (id >> 5) * 4 + which * 16; /* bit sets enemyFlags[4..7] and [8..11] */
                u8 *bits = (u8 *)&stats->enemyFlags[4];
                *(u32 *)(bits + offset) |= 1 << (id & 0x1F);
            }
            return;
        }
    }
    if (!(info->item0 | info->item1))
        return;
    range = 0;
    if (Player_GetEquippedItem(1) == 0x2B)
        range = 3;
    r = (RandomNumberGenerator() << 19) >> 22;
    if (range ? r <= 0x3F : r <= 7)
    {
        Pickup_CreateGold(x, y, range + ((RandomNumberGenerator() >> 1) & 3));
        return;
    }
    r2 = (RandomNumberGenerator() << 18) >> 22;
    if (Player_GetEquippedItem(1) == 0x1D ? r2 <= 0x3F : r2 <= 7)
        Pickup_CreateMana(x, y, (RandomNumberGenerator() & 0xC0) == 0, 1);
}
