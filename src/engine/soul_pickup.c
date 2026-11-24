/*
 * Soul pickups and containers, homing orbs, bouncing pickups and item info lookup.
 */
#include "soul_pickup.h"
#include "core.h"
#include "graphics.h"
#include "soul_inventory.h"
#include "structs/ewram.h"
#include "sound.h"
#include "entity_hit.h"
#include "gameplay.h"
#include "entity_util.h"
#include "gba.h"
#include "player_core.h"
#include "structs/equipment_stats.h"
#include "types.h"
#include "effects.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "hitbox.h"
#include "math_anim.h"
#include "obj_gfx.h"
#include "pickup.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"
#include "math_inline.h"
#include "red_souls_b.h"
#include "bg_and_collision.h"

extern struct WeaponInfo sWeaponInfo[]; // weapons, see player/player_core.c
extern const struct ArmorInfo sArmorInfo[];

void SoulPickup_Update(struct EwramData_EntityData *);
void SoulPickup_UpdateContainerAnimation(struct EwramData_EntityData *);
void Effect_UpdateHomingOrb(struct EwramData_EntityData *);

/* Usable item info (16-byte records). */
struct ItemInfo {
    u8 pad_0[0x10];
};

extern const struct ItemInfo sConsumableInfo[];

/**
 * @brief Create an animated soul pickup with soul kind/id and collection callback; select homing initialization or state 0x63 from the supplied variant.
 */
struct EwramData_EntityData *SoulPickup_Create(s32 x, s32 y, s32 palette, s32 soulId, s32 stationary)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, SoulPickup_Update);
    if (entity != NULL)
    {
        s32 graphics;
        entity->spriteLayer = 3;
        graphics = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, Soul_GetPickupPalette(palette));
        Anim_StartIndexed(entity, 6, 0, 1);
        Anim_Update(entity);
        entity->stage = 1;
        entity->drawFunc = Sprite_DrawEntity;
        Hit_Enable(entity, 0xB, Pickup_Collect, 0);
        entity->hit.flags |= 4;
        HitBox_SetAttackRaw(entity, 0x1010F8F8);
        entity->posX.part16.integer = x;
        entity->posY.part16.integer = y;
        entity->scrollWithBg = 1;
        if (stationary == 0)
        {
            struct EwramData_EntityData *player = gEwramData->playerState.player;
            s32 px = player->posX.whole;
            s32 py = player->posY.whole;
            py -= (RandomNumberGenerator() & 0x7F) << 12;
            py += -0x80000;
            entity->ext.soulPickup.angle = (Entity_AngleToPoint(entity, px, py) + 0x8000) & 0xFFFF;
            entity->ext.soulPickup.turn = stationary;
            entity->ext.soulPickup.speed = 0x30000;
        }
        else
        {
            entity->state = 0x63;
        }
        entity->enemyId = palette + 5;
        entity->spawnParam1 = soulId;
        entity->timer = 200;
    }
    return entity;
}

/**
 * @brief Home toward the point sixteen pixels above the player in states zero/one, check collection, emit a particle trail and expire under the existing timer states.
 */
void SoulPickup_Update(struct EwramData_EntityData *entity)
{
    struct HitResult res;

    if (entity->state <= 1)
    {
        struct EwramData_EntityData *player = gEwramData->playerState.player;
        entity->ext.soulPickup.angle = Entity_StepAngleTowardPoint(entity, player->posX.whole, player->posY.whole - 0x100000,
                                               entity->ext.soulPickup.angle, entity->ext.soulPickup.turn);
        entity->ext.soulPickup.turn += 42;
        if (entity->ext.soulPickup.turn > 0x4000) entity->ext.soulPickup.turn = 0x4000;
        entity->ext.soulPickup.speed += 0x1800;
        if (entity->ext.soulPickup.speed > 0x80000) entity->ext.soulPickup.speed = 0x80000;
        entity->velX.whole = FixedMulFast(Sin(entity->ext.soulPickup.angle + 0x4000), entity->ext.soulPickup.speed);
        entity->velY.whole = FixedMulFast(Sin(entity->ext.soulPickup.angle), entity->ext.soulPickup.speed);
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
    }
    Hit_CheckWithPlayer(&res, entity);
    if ((entity->timer & 1) == 0)
    {
        s32 x = entity->posX.whole + RandomSigned(0x200000);
        s32 y = entity->posY.whole + RandomSigned(0x200000);
        struct EwramData_EntityData *particle = Effect_CreateFromTable(x, y, entity->velX.whole / 64, entity->velY.whole / 64, 8, entity->palette);
        if (particle != NULL)
        {
            particle->velX.whole = -entity->velX.whole / 4;
            particle->velY.whole = -entity->velY.whole / 4;
            particle->spriteLayer = entity->spriteLayer;
        }
    }
    switch (entity->state)
    {
    case 0:
        if (--entity->timer == 0)
        {
            entity->drawSkip = 1;
            entity->timer = 64;
            entity->state++;
        }
        break;
    case 1:
        if (--entity->timer == 0) entity->deleteRequested = 1;
        break;
    default: entity->timer++; break;
    }
    if (entity->stage != 0) Anim_Update(entity);
}

/**
 * @brief Emit particles and position the child animation; on a qualifying hit create the soul pickup, then hide and expire with the existing particle sequence.
 */
void SoulPickup_UpdateContainer(struct EwramData_EntityData *entity)
{
    struct HitResult res;
    u16 hit;
    struct EwramData_EntityData *other;

    hit = Hit_CheckWithPlayer(&res, entity);
    switch (entity->state)
    {
    case 0:
        entity->timer++;
        if ((entity->timer & 7) == 0)
        {
            s32 x = entity->posX.whole + (RandomSigned(8) << 16);
            s32 y = entity->posY.whole - (((RandomNumberGenerator() & 7) + 0x14) << 16);
            other = Effect_CreateFromTable(x, y, RandomSigned(0x1000), 0, 4, entity->ext.soulContainer.palette);
            if (other != NULL)
            {
                other->velY.whole = RandomSigned(0x4000) - 0x10000;
                other->accelY.whole = -other->velY.whole / 32;
            }
        }
        if (entity->ext.soulContainer.soul != NULL)
        {
            other = entity->ext.soulContainer.soul;
            other->posX.whole = entity->posX.whole;
            other->posY.whole = entity->posY.whole;
        }
        if (hit & 0x1E)
        {
            other = SoulPickup_Create(entity->posX.part16.integer, (u16)entity->posY.part16.integer,
                entity->enemyId, entity->spawnParam1, 0);
            if (other != NULL)
            {
                PlaySong(0xA2);
                other->spawnParam0 = entity->spawnParam0;
                entity->state = 1;
                entity->timer = 0x10;
                entity->hidden = 1;
                if (entity->ext.soulContainer.soul != NULL)
                    entity->ext.soulContainer.soul->deleteRequested = 1;
            }
        }
        Anim_Update(entity);
        break;
    case 1:
        if (!(entity->timer & 1))
            Effect_SpawnRandom(entity->posX.whole, entity->posY.whole - 0x180000, 0x80000, 0x80000, entity->spriteLayer - 1);
        if (entity->timer != 0)
            entity->timer--;
        else
            entity->deleteRequested = 1;
        break;
    }
}

/**
 * @brief Create the grounded soul container and optional colored animation child using the supplied soul kind/index; return NULL if the container allocation fails.
 */
struct EwramData_EntityData *SoulPickup_CreateContainer(s32 x, s32 y, s32 variant, s32 index)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, SoulPickup_UpdateContainer);
    if (entity != NULL)
    {
        s32 graphics;
        struct EwramData_EntityData *child;
        entity->scrollWithBg = 1;
        entity->spriteLayer = 0x13;
        entity->enemyId = variant;
        entity->spawnParam1 = index;
        graphics = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, 8);
        Anim_StartIndexed(entity, 0x13, 0, 1);
        Anim_Update(entity);
        entity->drawFunc = Sprite_DrawEntity;
        Hit_Enable(entity, 0, NULL, 0);
        entity->hit.flags |= 2;
        HitBox_SetBodyRaw(entity, 0x2010E0F8);
        entity->posX.part16.integer = x;
        entity->posY.part16.integer = y;
        Entity_SnapToGround(entity);
        entity->ext.soulContainer.palette = Soul_GetPickupPalette(variant);
        child = EntityCreateInRange(0x49, 0xCF, SoulPickup_UpdateContainerAnimation);
        if (child != NULL)
        {
            child->scrollWithBg = 1;
            child->spriteLayer = entity->spriteLayer;
            child->posX.whole = entity->posX.whole;
            child->posY.whole = entity->posY.whole;
            graphics = ObjGfx_Find(sCommonObjectGfx);
            Sprite_Attach(child, sCommonObjectSpriteSheet, graphics, (u8)entity->ext.soulContainer.palette);
            Anim_StartIndexed(child, 0x12, 0, 1);
            Anim_Update(child);
            child->drawFunc = Sprite_DrawEntity;
            entity->ext.soulContainer.soul = child;
        }
    }
    return entity;
}

/**
 * @brief Create a homing visual orb aimed toward the supplied owner and vertical offset with existing random initial angle; preserve the non-NULL owner assumption.
 */
struct EwramData_EntityData *Effect_CreateHomingOrb(struct EwramData_EntityData *owner, s32 x, s32 y, s32 dy, s32 palette)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, Effect_UpdateHomingOrb);
    if (entity != NULL)
    {
        s32 graphics;
        entity->spriteLayer = 3;
        graphics = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, Soul_GetPickupPalette(palette));
        Anim_StartIndexed(entity, 6, 0, 1);
        Anim_Update(entity);
        entity->stage = 1;
        entity->drawFunc = Sprite_DrawEntity;
        entity->posX.part16.integer = x;
        entity->posY.part16.integer = y;
        entity->scrollWithBg = 1;
        entity->ext.homingOrb.angle = Entity_AngleToPoint(entity, owner->posX.whole, owner->posY.whole + dy);
        if (owner == gEwramData->playerState.player)
            entity->ext.homingOrb.angle += 0x6000 + (RandomNumberGenerator() & 0x3FFF);
        else
            entity->ext.homingOrb.angle += -0x1000 + (RandomNumberGenerator() & 0x1FFF);
        entity->ext.homingOrb.angle = (u16)entity->ext.homingOrb.angle;
        entity->ext.homingOrb.turn = 0;
        entity->ext.homingOrb.speed = 0x30000;
        entity->timer = 200;
        entity->ext.homingOrb.target = owner;
        entity->ext.homingOrb.targetDy = dy;
    }
    return entity;
}

/* Homing projectile, including impact and its particle trail. */
/**
 * @brief Home toward the stored target, emit a trail and expire by timer or proximity; on proximity attach an effect and play sound 0xBC.
 */
void Effect_UpdateHomingOrb(struct EwramData_EntityData *entity)
{
    u32 unused[4]; /* The original reserves 16 unused stack bytes. */
    struct EwramData_EntityData *target = entity->ext.homingOrb.target;
    if (entity->state <= 1)
    {
        entity->ext.homingOrb.angle = Entity_StepAngleTowardPoint(entity, target->posX.whole, target->posY.whole + entity->ext.homingOrb.targetDy,
                                               entity->ext.homingOrb.angle, entity->ext.homingOrb.turn);
        entity->ext.homingOrb.turn += 42;
        if (entity->ext.homingOrb.turn > 0x4000) entity->ext.homingOrb.turn = 0x4000;
        entity->ext.homingOrb.speed += 0x1800;
        if (entity->ext.homingOrb.speed > 0x80000) entity->ext.homingOrb.speed = 0x80000;
        entity->velX.whole = FixedMulFast(Sin(entity->ext.homingOrb.angle + 0x4000), entity->ext.homingOrb.speed);
        entity->velY.whole = FixedMulFast(Sin(entity->ext.homingOrb.angle), entity->ext.homingOrb.speed);
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
    }
    if ((entity->timer & 1) == 0)
    {
        s32 x = entity->posX.whole + RandomSigned(0x200000);
        s32 y = entity->posY.whole + RandomSigned(0x200000);
        struct EwramData_EntityData *particle = Effect_CreateFromTable(x, y, entity->velX.whole / 64, entity->velY.whole / 64, 8, entity->palette);
        if (particle != NULL)
        {
            particle->velX.whole = -entity->velX.whole / 4;
            particle->velY.whole = -entity->velY.whole / 4;
        }
    }
    switch (entity->state)
    {
    case 0:
        if (--entity->timer == 0)
        {
            entity->drawSkip = 1;
            entity->timer = 64;
            entity->state++;
        }
        break;
    case 1:
        if (--entity->timer == 0) entity->deleteRequested = 1;
        break;
    default: entity->timer++; break;
    }
    if (entity->stage != 0) Anim_Update(entity);
    if (Entity_IsInRect(entity->ext.homingOrb.target, (s16)(entity->posX.part16.integer - 10), (s16)(entity->posY.part16.integer - 10), 20, 20))
    {
        Effect_SpawnAttachedAnimation(entity->ext.homingOrb.target, 12, 6, 0, entity->ext.homingOrb.targetDy);
        PlaySong(0xBC);
        entity->deleteRequested = 1;
    }
}

/* Bouncing object: collision with the room walls / floor. */

/**
 * @brief Check collection, move with acceleration, damp horizontal speed and bounce against room walls/floor while stopping at the ceiling; retain signed probe conversions.
 */
void Pickup_UpdateBouncing(struct EwramData_EntityData *e)
{
    struct HitResult res;
    u16 *px;
    u16 x;
    s32 r;
    u16 y;
    struct BgLayerState *bg;

    Hit_CheckWithPlayer(&res, e);
    EntityAccelerateAndMove(e, 0x40000);
    px = &e->posX.part16.integer;
    bg = &gEwramData->bgInfo[1];
    x = *px + bg->xPos.part16.integer;
    y = e->posY.part16.integer + bg->yPos.part16.integer;
    EntityDampVelocityX(e, 0x40);
    if (e->velX.whole > 0)
    {
        r = Collision_GetPushLeft((s16)x, (s16)y - 4);
        x = r;
        if ((s16)r)
        {
            *px = ({ s32 t_ = (s16)x; t_; }) + *px;
            e->velX.whole = -e->velX.whole / 2;
        }
    }
    else if (e->velX.whole < 0)
    {
        r = Collision_GetPushRight((s16)x, (s16)y - 4);
        x = r;
        if ((s16)r)
        {
            *px = ({ s32 t_ = (s16)x; t_; }) + *px;
            e->velX.whole = -e->velX.whole / 2;
        }
    }
    {
        u16 *p = &e->posX.part16.integer;
        struct BgLayerState *bg2 = &gEwramData->bgInfo[1];
        x = *p + bg2->xPos.part16.integer;
    }
    if (e->velY.whole > 0)
    {
        r = Collision_GetPushUp((s16)x, (s16)y);
        x = r;
        if ((s16)r)
        {
            PlaySong(0x14F);
            e->posY.part16.integer += ({ s32 t_ = (s16)x; t_; });
            if (e->velY.whole <= 0x7FFF)
            {
                e->velY.whole = 0;
                e->accelY.whole = 0;
            }
            else
            {
                e->velY.whole = -e->velY.whole * 6 / 10;
            }
        }
    }
    else if (e->velY.whole < 0)
    {
        r = Collision_GetPushDown((s16)x, (s16)y - 4);
        x = r;
        if ((s16)r)
        {
            e->posY.part16.integer += ({ s32 t_ = (s16)x; t_; });
            e->velY.whole = 0;
        }
    }
    Anim_Update(e);
}

/* Item tables 0x08044E94..0x08044EDB (Item_GetInfoByGlobalId is in this file, the rest
 * of the item helpers are in this file).
 *
 * Global item ids are 1-based: 1..0x20 usable items (type 2), 0x21..0x5B
 * weapons (type 3), 0x5C..0x88 armor and accessories (type 4). */

#define GOLD_MAX 999999

/**
 * @brief Return the indexed record for item, weapon or armor type 2..4, otherwise NULL; preserve unchecked per-type indices.
 */
const void *Item_GetInfo(s32 category, s32 index)
{
    const void *info = NULL;

    switch (category)
    {
        case 2:
            info = &sConsumableInfo[index];
            break;
        case 3:
            info = &sWeaponInfo[index];
            break;
        case 4:
            info = &sArmorInfo[index];
            break;
    }
    return info;
}

/* Resolve the record family and stride for a one-based item identifier. */
/**
 * @brief Resolve a one-based global item identifier to its record family and stride; return NULL for zero or above the final range, preserving signed comparisons.
 */
void *Item_GetInfoByGlobalId(s32 id)
{
    void *record = NULL;
    if (id == 0) return NULL;
    else
    {
        id--;
        if (id <= 31) record = (u8 *)sConsumableInfo + id * 16;
        else if (id <= 90) record = sUnk_085059BC + id * 28;
        else if (id <= 135) record = sUnk_08505C94 + id * 20;
    }
    return record;
}

/* Item and inventory helpers 0x08044F24..0x08044FFF (see this file).
 *
 * Global item ids are 1-based: 1..0x20 usable items (type 2), 0x21..0x5B
 * weapons (type 3), 0x5C..0x88 armor and accessories (type 4). */

/**
 * @brief Resolve the global item identifier to type 2, 3 or 4; retain zero, out-of-range and signed comparison behavior.
 */
s32 Item_GetTypeByGlobalId(s32 globalId)
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

/**
 * @brief Resolve the global item identifier to its per-type index, retaining zero/default and signed range behavior.
 */
s32 Item_GetIndexByGlobalId(s32 globalId)
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

/**
 * @brief Add the supplied value through the unsigned gold field and cap results above 999999; preserve unsigned arithmetic behavior.
 */
void Player_AddGold(s32 amount)
{
    u32 *gold = &gEwramData->playerStatus.currentGold;

    *gold += amount;
    if (*gold > GOLD_MAX)
        *gold = GOLD_MAX;
}

/**
 * @brief Return the inventory count pointer for type 2..4 and index; unsupported types return the start of itemInventory.
 */
u8 *ItemInventory_GetCountPtr(s32 type, s32 index)
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

/**
 * @brief Advance only the animation of the child installed by SoulPickup_CreateContainer.
 */
void SoulPickup_UpdateContainerAnimation(struct EwramData_EntityData *entity)
{
    Anim_Update(entity);
}
