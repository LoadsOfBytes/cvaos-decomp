/*
 * Player (Soma): new game / entity init, collision corrections and floor probes, ground / air / ladder / swim movement, ceiling impact, dive attack, jumps, red soul cast and weapon input, knockback, status reactions, bat flight, death and scripted animations.
 */
#include "player_soma.h"
#include "player_entity.h"
#include "core.h"
#include "bg_and_collision.h"
#include "bg_effects.h"
#include "player_core.h"
#include "soul_inventory.h"
#include "player_effects.h"
#include "player_level.h"
#include "graphics.h"
#include "gameplay.h"
#include "sound.h"
#include "gba.h"
#include "macros.h"
#include "constants/main.h"
#include "data/data_0E0334.h"
#include "data/data_0E3464.h"
#include "structs/agb_sram.h"
#include "structs/ewram.h"
#include "structs/main.h"
#include "map_flags.h"
#include "blue_souls.h"
#include "damage_numbers.h"
#include "effects.h"
#include "enemy_common.h"
#include "hitbox.h"
#include "item_menu.h"
#include "math_anim.h"
#include "menu_common.h"
#include "obj_gfx.h"
#include "palette.h"
#include "red_souls_b.h"
#include "system_menus.h"
#include "data/data_0E0B68.h"
#include "data/data_118B48.h"
#include "data/data_4F0B18.h"

/**
 * @brief When currentLevel is zero, initialize equipment, inventory,
 * base stats, button settings, HP/MP and the starting area/room
 * coordinates. Leave an already initialized level unchanged.
 */
void Player_InitNewGameIfNeeded(void)
{
    if (gEwramData->playerStatus.currentLevel == 0)
    {
        Player_ClearEquipmentAndSoulLists();

        gEwramData->playerStatus.equippedRedSoul = 0;
        gEwramData->playerStatus.equippedBlueSoul = 0;
        gEwramData->playerStatus.equippedYellowSoul = 0;

        Player_EquipItem(2, 0);
        Player_EquipItem(0, 0);
        Player_EquipItem(1, 0xFF);
        gEwramData->playerStatus.weaponInventory[0] = 1;
        gEwramData->playerStatus.armorInventory[0] = 1;
        Player_InitBaseLevelStats();
        ButtonConfig_ResetDefaults();

        gEwramData->playerStatus.currentHP = gEwramData->playerStatus.maxHP;
        gEwramData->playerStatus.currentMP = gEwramData->playerStatus.maxMP;

        gEwramData->gameState.currentArea = 0;
        gEwramData->gameState.currentRoom = 0;

        gEwramData->gameState.savedCameraX = 0x110;
        gEwramData->gameState.savedCameraY = 0x230;
        gEwramData->gameState.savedPlayerX = 0xD0;
        gEwramData->gameState.savedPlayerY = 0x6F;
    }
}

/**
 * @brief Initialize the character-0 player entity graphics, palette,
 * animation, body box, draw callback and contact setup. Entry 0
 * of sPlayerInitFuncs; preserve the existing animation-mode sequence.
 */
void Soma_InitEntity(struct EwramData_EntityData *entity)
{
    s32 gfx;
    s32 palette;
    struct PlayerBodyBox unk_0;

    gfx = ObjGfx_LoadResource(sSomaGfx, 0x10000);
    ObjPalette_LoadRange(sSomaPalette, 0, 1, 0);
    Sprite_Attach(entity, sSomaSpriteSheet, gfx, 0);
    entity->animScriptIndex = 0xFF;
    unk_0 = *sSomaStandingBodyBox;
    if (!(gEwramData->playerState.actionLockFlags & 0x800) && (entity->animScriptIndex != 0))
    {
        Anim_StartIndexed(entity, 0, 3, 1);
        HitBox_SetBody(entity, &unk_0);
        entity->ext.player.requestedAnim = 0xFF;
        entity->ext.player.actionFlags &= ~0x200000;
        palette = sSomaAnimPalettes[0];
        if (entity->ext.player.animPalette != palette)
        {
            entity->ext.player.animPalette = palette;
            ObjPalette_LoadRange(sSomaPalette, palette, 1, 0);
        }
    }
    Anim_Update(entity);
    entity->drawFunc = (u32 *) Player_DrawWithOffset;
    Hit_Enable(entity, 1, Player_MarkAttackContact, 0);
    HitBox_SetBody(entity, &sSomaStandingBodyBox);
    entity->hit.flags |= 2;
    gEwramData->playerStatus.soulStateFlags = 0;
}

/**
 * @brief Initialize the character-1 player entity, level/equipment,
 * graphics, palettes and linked weapon setup. Entry 1 of
 * sPlayerInitFuncs; preserve resource order and fixed entity slot use.
 */
void Julius_InitEntity(struct EwramData_EntityData *entity)
{
    s32 gfx;
    s32 i;

    ButtonConfig_ResetDefaults();
    if (gEwramData->playerStatus.currentLevel > 1)
    {
        Player_SetLevel(1);
    }

    gfx = ObjGfx_LoadResource(sUnk_080E12C8, 0x10000);
    ObjPalette_LoadRange(sJuliusEntityPalette, 0, 1, 0);
    Sprite_Attach(entity, sJuliusSpriteSheet, gfx, 0);

    entity->animScriptIndex = 0xFF;
    Player_TryStartAnimation(entity, 0, 1);
    Anim_Update(entity);

    entity->drawFunc = (u32 *) Player_DrawMirrored;
    Hit_Enable(entity, 1, Player_MarkAttackContact, 0);
    Entity_UpdateBoxes(entity);

    gEwramData->playerStatus.soulStateFlags = 0x8000;
    Player_SetAbility(5, 1);

    for (i = 0; i < 8; i++)
    {
        gEwramData->playerState.activeBlueSoulEntities[i] = 0;
    }

    Hit_SetTimerOverride(0, 0x1E);
    Player_EquipItem(2, 0x32);
    Player_EquipItem(0, 0xFF);
    Player_EquipItem(1, 0xFF);
    Player_EquipSoulsAndRestore(0, 0, 0);

    ObjGfx_LoadResource(sJuliusGfx, 0x10000);
    ObjPalette_LoadRange(sJuliusPalette, 0, 1, 2);
    ObjPalette_LoadRange(sJuliusPalette, 1, 1, 3);
    EntityDelete(&gEwramData->entityData[0x11]);

    EntityCreateInRange(0x11, 0x11, (u32 *) Julius_UpdateSubweaponIcon);
    PLAYER_JULIUS_OVERLAY(entity) = Julius_CreateOverlaySprite();
    gEwramData->playerState.whipChain = Whip_CreateChain(0x01300000, 0, sJuliusGfx, PLAYER_JULIUS_OVERLAY(entity));
    Whip_DeactivateChain(gEwramData->playerState.whipChain);
}

// Soma_InitEntity, Julius_InitEntity
extern void (*sPlayerInitFuncs[])(struct EwramData_EntityData *);
// Soma_Update, Julius_Update
extern void (*sPlayerUpdateFuncs[])(struct EwramData_EntityData *);

/**
 * @brief 1487C | Create the player entity for the current character
 *
 * Resets the player's soul/weapon state, creates entity slot 0 with the
 * character's update function (sPlayerUpdateFuncs) and runs its init
 * function (sPlayerInitFuncs); gEwramData->playerState.player points to it.
 *
 * @param unused Unused
 */
void Player_Create(s32 unused)
{
    struct EwramData_EntityData *player;

    gEwramData->playerState.nbrActiveRedSoulEntities = 0;
    gEwramData->playerState.weaponGfx = -1;
    gEwramData->playerState.redSoulGfx = -1;
    gEwramData->playerState.blueSoulGfx = -1;
    gEwramData->playerState.weaponEntities[0] = 0;
    gEwramData->playerState.whipChain = 0;
    gEwramData->playerState.cachedWeaponGraphicsIndices[0] = -1;
    gEwramData->playerState.cachedWeaponGraphicsIndex1 = -1;

    RedSoul_CancelAll();
    BlueSoul_StopAll();

    player = EntityCreateInRange(0, 0, (u32*) sPlayerUpdateFuncs[gEwramData->playerStatus.currentCharacter]);
    sPlayerInitFuncs[gEwramData->playerStatus.currentCharacter](player);
    gEwramData->playerState.player = player;
    gEwramData->playerState.enemyTarget = player;

    Player_ClearStatusEffects(7);
    player->scrollWithBg = 1;
    player->spriteLayer = 0xE;
    player->ext.player.actionFlags = 0x300000;
    /* address of the default wall probe offset list (s8 offsets) */
    player->ext.player.wallProbes = (u32)sSomaWallProbesStand;

    Player_CopyBaseStats();
    gEwramData->playerState.cameraAnchorX = 0x78;
    gEwramData->playerState.cameraAnchorY = 0x6F;
    gEwramData->playerState.collisionQueueCount = 0;
    player->ext.player.requestedAnim = 0xFF;
    player->ext.player.drawOffsetX = 0;
    player->ext.player.drawOffsetY = 0;
    player->ext.player.floorKind = 0xFF;
    player->ext.player.weaponSlot = 0;
    gEwramData->playerState.wallProbeCount = 4;
    gEwramData->playerState.wallProbeY0 = 0xF8;
    gEwramData->playerState.wallProbeY1 = 0xF4;
    gEwramData->playerState.wallProbeY2 = 0xEC;
    gEwramData->playerState.wallProbeY3 = 0xE4;
    gEwramData->playerState.actionLockFlags = 0;
    HudHPNumber_Create();
    gEwramData->gameState.timeStopTimer = 0;
}

static inline s32 Entity_TestFlag100000(struct EwramData_EntityData *entity)
{
    return entity->ext.player.actionFlags & 0x100000;
}

static inline s32 Entity_TestFlag1000(struct EwramData_EntityData *entity)
{
    return entity->ext.player.actionFlags & 0x1000;
}

static inline s32 CollisionProbe_TestBit1()
{
    return gUnk_03002CB0.collisionProbeFlags & 0x2;
}

static inline s32 CollisionProbe_TestSlopeBits()
{
    return gUnk_03002CB0.collisionProbeFlags & 0xC0;
}

/* Player collision with the tile map, the carrying object and moving platforms. */

/* Function table of the object in solidObject (pointer at its unk_4F4). */
struct SolidObjectCallbacks {
    void *flags;
    s16 (*ceiling)(struct EwramData_EntityData *object, s32 x, s32 y);
    s16 (*floor)(struct EwramData_EntityData *object, s32 x, s32 y);
    s16 (*wallRight)(struct EwramData_EntityData *object, s32 x, s32 y);
    s16 (*wallLeft)(struct EwramData_EntityData *object, s32 x, s32 y);
    s32 (*push)(struct EwramData_EntityData *object, s16 x, s16 y);
};

#define FUNCS(object) ((struct SolidObjectCallbacks *)object->ext.solidObject.callbacks.words)
#define FLAGS(e) (e->ext.player.actionFlags)
#define TILE_FLAGS (gUnk_03002CB0.collisionProbeFlags)
#define TIMER(e) (*(u16 *)&e->ext.player.dropThroughTimer)
#define X ((s16)x)
#define Y ((s16)y)

static inline void SetPlayerAnimBox(struct EwramData_EntityData *entity, u8 anim, struct PlayerBodyBox *table, s32 flag)
{
    struct PlayerBodyBox box = table[0];
    s32 pal;

    if (!(gEwramData->playerState.actionLockFlags & 0x800) && entity->animScriptIndex != anim)
    {
        Anim_StartIndexed(entity, anim, 3, flag);
        HitBox_SetBody(entity, &box);
        entity->ext.player.requestedAnim = 0xFF;
        entity->ext.player.actionFlags &= ~0x200000;
        pal = sSomaAnimPalettes[anim];
        if (entity->ext.player.animPalette != pal)
        {
            entity->ext.player.animPalette = pal;
            ObjPalette_LoadRange(sSomaPalette, pal, 1, 0);
        }
    }
}

static inline void SetPlayerBox(struct EwramData_EntityData *entity, s32 value, struct PlayerBodyBox *table)
{
    struct PlayerBodyBox box = table[0];

    entity->ext.player.requestedAnim = value;
    HitBox_SetBody(entity, &box);
}

static inline s16 CeilingCorrection(s16 x, s16 y)
{
    u16 c = Collision_GetPushDown(x, y);
    struct EwramData_EntityData *object = gEwramData->playerState.solidObject;

    if (object != NULL)
    {
        s16 d = FUNCS(object)->ceiling(object, x, y);
        return ((s16)c < d) ? d : c;
    }
    return c;
}

static inline s16 WallLeftCorrection(s16 x, s16 y)
{
    u16 c = Collision_GetPushRight(x, y);
    struct EwramData_EntityData *object = gEwramData->playerState.solidObject;

    if (object != NULL)
    {
        s16 d = FUNCS(object)->wallLeft(object, x, y);
        return ((s16)c < d) ? d : c;
    }
    return c;
}

static inline s16 WallRightCorrection(s16 x, s16 y)
{
    u16 c = Collision_GetPushLeft(x, y);
    struct EwramData_EntityData *object = gEwramData->playerState.solidObject;

    if (object != NULL)
    {
        s16 d = FUNCS(object)->wallRight(object, x, y);
        return ((s16)c > d) ? d : c;
    }
    return c;
}

static inline s32 SolidPair(s32 x, s32 y)
{
    s32 result;

    if (Collision_IsPointSolid(x - 5, y) != 0)
        result = 1;
    else
    {
        s32 t = Collision_IsPointSolid(x + 5, y);

        result = (u32)(-t | t) >> 31;
    }
    return result;
}

/* 0: flat, 1 / 2: slope kinds. */
static inline s32 SlopeKind(s32 d)
{
    if (d & 8)
    {
        if (d & 1)
            return 2;
        return 1;
    }
    return 0;
}

static inline s32 StairKind(u8 d)
{
    if (d & 0xC0)
    {
        if (d & 2)
            return 2;
        return 1;
    }
    return 0;
}

static inline s32 TileBit1(s32 d)
{
    return d & 2;
}

static inline s32 TileBits67(s32 d)
{
    return d & 0xC0;
}

static inline s32 IsSoftFloor(void)
{
    s32 d = TILE_FLAGS;
    s32 result = 0;

    if (TileBit1(d) || TileBits67(d))
    {
        if (SlopeKind(d) != 1)
            result = 1;
    }
    return result;
}

static inline void UpdateFloorKind(s16 c)
{
    if (c != 0 && gEwramData->playerState.player->ext.player.floorKind == 0xFF)
    {
        s32 kind;
        u8 d = TILE_FLAGS;

        kind = SlopeKind(d);
        if (kind == 0 && !(d & 0xC0) && (d & 4))
            kind = (d & 0x30) >> 4;
        else
            kind = 0xFF;
        gEwramData->playerState.player->ext.player.floorKind = kind;
    }
}

static inline void UpdateFloorFlags(struct EwramData_EntityData *entity)
{
    if (IsSoftFloor())
        FLAGS(entity) &= ~0x1000;
    if (TILE_FLAGS & 0xC0)
    {
        if (TILE_FLAGS & 4)
            FLAGS(entity) |= 0x4000;
        else
            FLAGS(entity) |= 0x2000;
    }
    if ((u8)entity->ext.player.slopeLevel < (TILE_FLAGS >> 6))
        entity->ext.player.slopeLevel = TILE_FLAGS >> 6;
}

static inline s16 PlatformFloor(s16 x, s16 y, u16 c)
{
    s32 i;

    for (i = 0; i < gEwramData->playerState.collisionQueueCount; i++)
    {
        struct SpriteBox a;
        struct SpriteBox b;
        struct EwramData_EntityData *platform = gEwramData->playerState.collisionQueueEntities[i];
        s16 px = GetEntityRoomXPositionInteger(platform);
        s16 py = GetEntityRoomYPositionInteger(platform);
        s16 right;
        s16 extra;
        s16 up;
        u8 *state;
        struct EwramData *ewram;

        if (HitBox_GetBoxes(&a, &b, platform) == 0)
            continue;
        if (platform->flipX)
        {
            right = px - b.x;
            px = right - b.width;
        }
        else
        {
            px = px + b.x;
            right = b.width + px;
        }
        {
            s32 top = py - 1;

            py = b.y + top;
        }
        extra = 0;
        up = 0;
        ewram = gEwramData;
        {
            u8 *states = ewram->playerState.collisionQueueFlags;

            state = &states[i];
        }
        if (*state & 2)
            up = platform->velY.part16.integer + 1;
        if (*state & 4)
            extra = 8;
        if (x >= px && x <= right && y >= py - up && y <= py + ({ s32 height = extra + 8; height; }))
        {
            c = py - y;
            ewram->playerState.standingPlatform = platform;
            if (ewram->playerState.collisionQueueFlags[i] & 1)
                ewram->playerState.collisionQueueFlags[i] |= 8;
            gUnk_03002CB0.objectFloorContact = 1;
            return c;
        }
    }
    return c;
}

static inline u16 FloorProbe(s16 x, s16 y, s32 flag)
{
    u16 c;

    {
        s32 tx = x;
        s32 ty = y;
        s16 r = Collision_GetPushUpFiltered(tx, ty, flag);
        struct EwramData_EntityData *object;

        c = r;
        object = gEwramData->playerState.solidObject;
        if (object != NULL)
        {
            s16 d = FUNCS(object)->floor(object, tx, ty);
            if (d < r)
            {
                c = d;
                gUnk_03002CB0.collisionProbeFlags |= 2;
                gUnk_03002CB0.objectFloorContact = 1;
            }
        }
    }

    if (gEwramData->playerState.collisionQueueCount != 0)
    {
        struct EwramData_EntityData *player = gEwramData->playerState.player;

        if (player->velY.whole >= 0 && (!(player->ext.player.actionFlags & 2) || player->ext.player.airTime == 0x10))
            c = PlatformFloor(x, y, c);
    }
    UpdateFloorKind(c);
    return c;
}

static inline s16 TileFloorProbe(s16 x, s16 y, s32 flag)
{
    s16 c = Collision_GetPushUpFiltered(x, y, flag);

    UpdateFloorKind(c);
    return c;
}

static inline s32 PlatformVelocityX(void)
{
    s32 i;

    for (i = 0; i < gEwramData->playerState.collisionQueueCount; i++)
    {
        if (gEwramData->playerState.collisionQueueFlags[i] & 8)
            return gEwramData->playerState.collisionQueueEntities[i]->velX.whole;
    }
    return 0;
}

static inline s32 IsWithinStep(struct EwramData_EntityData *entity, s32 c)
{
    s32 result;

    if (-c <= entity->velY.part16.integer + 2)
        result = 1;
    else
        result = 0;
    return result;
}

static inline s32 FlagBit20(struct EwramData_EntityData *entity)
{
    return FLAGS(entity) & 0x100000;
}

static inline s32 FlagBit12(struct EwramData_EntityData *entity)
{
    return FLAGS(entity) & 0x1000;
}

static inline s32 StepOk(struct EwramData_EntityData *entity, s32 c)
{
    if (entity->velY.part16.integer < 0)
        return 0;
    return IsWithinStep(entity, c);
}

#define CanSnap(e, c) (FlagBit20(e) || !FlagBit12(e) || StepOk(e, c))

/* Same test as CanSnap, but as a function: the s16 parameter copy matters. */
static inline s32 CanLand(struct EwramData_EntityData *entity, s16 c)
{
    if (FlagBit20(entity) || !FlagBit12(entity) || StepOk(entity, c))
        return 1;
    return 0;
}

static inline s32 IsLanding(struct EwramData_EntityData *entity, s32 c)
{
    s32 result;

    if (entity->velY.whole > 0)
    {
        result = 1;
        if (c == 0 && (FLAGS(entity) & 0x6000))
            result = 0;
    }
    else
        result = 0;
    return result;
}

static inline void PlayerCollision(struct EwramData_EntityData *entity)
{
    s32 flag = 0;
    u16 left = 0;
    u16 right = 0;
    s32 dir = 0;
    s32 useObjects = 1;
    s32 velX = entity->velX.whole + PLAYER_CONVEYOR_VELOCITY(entity);
    s16 x, y;
    u16 below;
    s16 c;
    /* One variable serves as ceiling flag, stair probe offset and step height;
       sharing it is required for the original register allocation. */
    s16 probe;
    s8 *offsets;
    s32 count;
    s32 i;
    u32 flagsA, flagsB, flagsC, flagsD, flagsE;

    gUnk_03002CB0.objectFloorContact = 0;
    x = GetEntityRoomXPositionInteger(entity);
    y = GetEntityRoomYPositionInteger(entity);
    FLAGS(entity) &= 0xD7FFF7FF;
    if (CeilingCorrection(X, Y - 0x21) != 0)
        FLAGS(entity) |= 0x20000000;
    if (gEwramData->playerStatus.soulStateFlags & 0x4000)
    {
        if (Collision_IsPointSolid(X - 5, Y - 7) == 0 && Collision_IsPointSolid(X + 5, Y - 7) == 0)
            flag = 1;
    }
    below = FloorProbe(X, Y + 1, 0);
    c = below;
    {
        struct EwramData_EntityData *object = gEwramData->playerState.solidObject;

        if (object != NULL || gEwramData->playerState.collisionQueueCount != 0)
        {
            s32 dx = 0;

            if (object != NULL)
            {
                s32 (*push)(struct EwramData_EntityData *, s16, s16) = FUNCS(object)->push;

                if (Player_GetSolidObjectFlags() & 1)
                    useObjects = 0;
                dx = push(gEwramData->playerState.solidObject, x, Y + 1);
                if (dx != 0)
                {
                    entity->accelX.whole = 0;
                    entity->posX.whole += dx;
                    velX += dx;
                }
            }
            if (gEwramData->playerState.collisionQueueCount != 0 && dx == 0)
            {
                if (gEwramData->playerState.standingPlatform != NULL)
                    dx = PlatformVelocityX();
                if (dx != 0)
                {
                    entity->posX.whole += dx;
                    velX += dx;
                }
            }
        }
    }
    offsets = (s8 *)entity->ext.player.wallProbes;
    count = offsets[0];
    if (velX < 0)
    {
        for (i = 0; i < count; i++)
        {
            s16 d = WallLeftCorrection(X - 8, Y + offsets[i + 1]);

            if (d != 0)
            {
                entity->posX.whole += d << 16;
                flagsA = FLAGS(entity);
                if (flagsA & 0x80)
                {
                    entity->accelX.whole = 0;
                    entity->velX.whole = 0;
                    FLAGS(entity) = flagsA | 0x40000;
                    break;
                }
                entity->velX.whole = -entity->velX.whole;
                entity->velX.whole /= 4;
                entity->accelX.whole = -entity->accelX.whole;
                FLAGS(entity) = flagsA | 0x40000;
                break;
            }
        }
        dir = -1;
    }
    else if (velX > 0)
    {
        for (i = 0; i < count; i++)
        {
            s16 d = WallRightCorrection(X + 8, Y + offsets[i + 1]);

            if (d != 0)
            {
                entity->posX.whole += d << 16;
                flagsB = FLAGS(entity);
                if (flagsB & 0x80)
                {
                    entity->accelX.whole = 0;
                    entity->velX.whole = 0;
                    FLAGS(entity) = flagsB | 0x40000;
                    break;
                }
                entity->velX.whole = -entity->velX.whole;
                entity->velX.whole /= 4;
                entity->accelX.whole = -entity->accelX.whole;
                FLAGS(entity) = flagsB | 0x40000;
                break;
            }
        }
        dir = 1;
    }
    x = GetEntityRoomXPositionInteger(entity);
    if (CeilingCorrection(X - 5, Y - 0x14) != 0 || CeilingCorrection(X + 5, Y - 0x14) != 0)
        FLAGS(entity) |= 0x20008000;
    else
        FLAGS(entity) &= 0xFFFF7FFF;
    if (TIMER(entity) != 0)
        TIMER(entity)--;
    probe = SolidPair(X, Y - 0x19);
    if (SolidPair(X, Y - 8))
    {
        if (!(FLAGS(entity) & 0x1000000))
        {
            if (entity->velY.whole > 0x18000)
                Player_SpawnSurfaceEffect(0);
            if (entity->velY.whole > 0 && (gEwramData->playerStatus.soulStateFlags & 0x8000))
            {
                entity->velY.whole >>= 2;
                entity->accelY.whole = 0;
            }
        }
        FLAGS(entity) |= 0x1000000;
    }
    else
    {
        if (probe == 0 && (FLAGS(entity) & 0x1000000) && entity->velY.whole < -0x18000)
            Player_SpawnSurfaceEffect(2);
        FLAGS(entity) &= ~0x1000000;
    }
    if (probe != 0)
    {
        FLAGS(entity) = (FLAGS(entity) & 0xFF7FFFFB) | 0xC00000;
        if (Collision_IsPointSolid(X, Y - 0x1A) == 0)
            FLAGS(entity) |= 0x800;
    }
    else
    {
        flagsC = FLAGS(entity);
        if (flagsC & 0x800000)
        if (!(flagsC & 0x10))
        {
            if (!(flagsC & 0x2000000))
                entity->velY.whole = 0;
            FLAGS(entity) &= ~0x2000000;
        }
        FLAGS(entity) &= ~0xC00000;
    }
    flagsD = FLAGS(entity);
    if (flagsD & 0x800000)
    {
        if ((gEwramData->playerStatus.soulStateFlags & 0x8000) || (flagsD & 0x4000000))
            FLAGS(entity) = flagsD & ~0x800000;
        else
            FLAGS(entity) = flagsD & ~0x400;
    }
    entity->ext.player.onStairs = 0;
    if (dir != 0)
    {
        /* The stair search reuses the wall loop counter. */
        probe = 0;
        for (i = 4; i != 0; i--)
        {
            if (Collision_GetPushUpFiltered(X + probe, Y + 1, flag) != 0)
            {
                if (StairKind(TILE_FLAGS) == 1)
                {
                    entity->ext.player.onStairs = 1;
                    break;
                }
            }
            probe += dir;
        }
    }
    if (!(FLAGS(entity) & 0x100000))
    {
        if (entity->velY.whole <= 0)
        {
            probe = FloorProbe(X, Y, flag);

            if (IsSoftFloor() && probe != 0 && gUnk_03002CB0.objectFloorContact == 0)
                entity->posY.whole += probe << 16;
        }
    }
    if (entity->velY.whole < 0)
        return;
    if (TIMER(entity) != 0)
        return;
    dir = 0;
    FLAGS(entity) = (FLAGS(entity) & 0xFFFF9FFF) | 0x1000;
    entity->ext.player.slopeLevel = 0;
    c = FloorProbe(X, Y + 1, flag);
    UpdateFloorFlags(entity);
    if (c == 0)
        c = below;
    if (!useObjects)
    {
        left = TileFloorProbe(X - 5, Y + 1, flag);
        UpdateFloorFlags(entity);
        right = TileFloorProbe(X + 5, Y + 1, flag);
        UpdateFloorFlags(entity);
    }
    else
    {
        left = FloorProbe(X - 5, Y + 1, flag);
        UpdateFloorFlags(entity);
        right = FloorProbe(X + 5, Y + 1, flag);
        UpdateFloorFlags(entity);
    }
    flagsE = FLAGS(entity);
    if (flagsE & 0x800000)
    {
        FLAGS(entity) = flagsE | 2;
        if (Collision_IsPointSolid(X, Y) != 0)
        {
            if (FLAGS(entity) & 0x800)
                return;
            if (Collision_IsPointSolid(X, Y - 0x1E) != 0)
                return;
        }
    }
    {
        s32 sc = c;

        if (sc != 0 && CanSnap(entity, sc))
        {
            s32 dy = (c << 16) + 0x10000;

            entity->posY.whole += dy;
            dir = 1;
            FLAGS(entity) |= 0x8000000;
        }
    }
    if (!(FLAGS(entity) & 2))
    {
        c = FloorProbe(X, Y + 7, flag);
        if (dir == 0)
        {
            if (c != 0)
            {
                entity->posY.whole += (c + 7) << 16;
                dir = 1;
                FLAGS(entity) |= 0x8000000;
            }
        }
    }
    if (dir == 0)
    {
        s32 sl = (s16)left;

        if (sl != 0 && CanSnap(entity, sl))
        {
            if (!(FLAGS(entity) & 0x6000))
            {
                s32 dy = (left << 16) + 0x10000;
                entity->posY.whole += dy;
                dir = 1;
            }
        }
        if (dir == 0)
        {
            s32 sr = (s16)right;

            if (sr != 0 && CanSnap(entity, sr))
            {
                if (!(FLAGS(entity) & 0x6000))
                {
                    s32 dy = (right << 16) + 0x10000;
                    entity->posY.whole += dy;
                }
            }
        }
    }
    /* The goto layout reproduces the original block order: the
       "no floor" exit sits between the two arms of the landing flag update. */
    if (c == 0 && left == 0 && right == 0)
        goto noFloor;
    {
        s32 sc = c;

        if (!CanLand(entity, sc))
            return;
        if (!IsLanding(entity, sc))
            return;
        FLAGS(entity) |= 0x100000;
        entity->ext.player.airTime = 0;
        if (Weapon_KeepsAttackOnLanding() == 0)
            goto plainLanding;
        FLAGS(entity) &= 0xFFDFFCE1;
        goto landed;
    noFloor:
        FLAGS(entity) &= ~0x100000;
        return;
    plainLanding:
        FLAGS(entity) &= 0xFFDFFE81;
    landed:
        entity->timer = 0;
        PlaySong(0xBB);
        entity->posY.part16.fraction = 0;
        if ((gEwramData->playerState.actionLockFlags & 8) || (gEwramData->playerStatus.statusEffects & 4))
        {
            entity->velY.whole = 0;
            entity->accelY.whole = 0;
            return;
        }
        if (entity->velY.whole > 0x64000 || (FLAGS(entity) & 0x80))
        {
            FLAGS(entity) |= 0x10000;
            if (Weapon_KeepsAttackOnLanding() != 0)
                FLAGS(entity) &= 0xFFFFFE9F;
            PlaySong(0xBB);
            SetPlayerAnimBox(entity, 0x13, sSomaCrouchBodyBoxes, 0);
            entity->state = 4;
        }
        else if (!(FLAGS(entity) & 0x20) || Weapon_KeepsAttackOnLanding() == 0)
        {
            if (!(gEwramData->inputData.playerHeldInput & 0xF0)
             && !((gEwramData->playerStatus.jumpButton | gEwramData->playerStatus.attackButton | gEwramData->playerStatus.abilityButton | gEwramData->playerStatus.guardianButton) & gEwramData->inputData.playerNewInput))
                SetPlayerBox(entity, 0xD, sSomaAirborneBodyBox);
            if (gEwramData->inputData.playerHeldInput & 0x30)
                SetPlayerAnimBox(entity, 1, sSomaStandingBodyBox, 1);
            entity->state = 0;
        }
    }
    entity->velY.whole = 0;
    entity->accelY.whole = 0;
}

/**
 * @brief Resolve Soma collision against the tile map, carrying-object
 * callbacks and moving platforms through the existing inline
 * reconstruction. Preserve all narrow probes and macro boundaries.
 */
void Soma_ResolveWorldCollision(struct EwramData_EntityData *entity)
{
    PlayerCollision(entity);
}

/* Soma bat-form exit collision (floor probe at y + 7, two ceiling probes,
 * walls from the offset table at sSomaOffsets with bounce). Helpers copied
 * from this file. */

/* Function table of the object in solidObject (pointer at its unk_4F4). */

/* 0: flat, 1 / 2: slope kinds. */

/**
 * @brief Push the player out of terrain when leaving bat state 8:
 * floor probe at +7 px, ceiling probes at -15/-31 px and walls
 * at +-8 px from the sSomaOffsets offsets. A wall bounce reverses and
 * quarters velX unless flag 0x80 is set; sets wall-contact bit 0x40000.
 */
void Soma_ResolveBatExitCollision(struct EwramData_EntityData *entity)
{
    u16 x = GetEntityRoomXPositionInteger(entity);
    u16 y = GetEntityRoomYPositionInteger(entity);
    s16 c;
    s32 i;
    s8 *offsets;
    s32 count;
    u16 probe;
    s32 r;
    u32 flags;

    s16 d;

    c = FloorProbe(X, Y + 7, 0);
    if ((s16)c != 0)
    {
        r = c + 7;
        r <<= 16;
        goto adjust;
    found:
        r = d << 16;
    adjust:
        entity->posY.whole += r;
        entity->velY.whole = 0;
        y = GetEntityRoomYPositionInteger(entity);
        goto walls;
    zero_left:
        entity->accelX.whole = 0;
        entity->velX.whole = 0;
        goto hit_left;
    zero_right:
        entity->accelX.whole = 0;
        entity->velX.whole = 0;
        goto hit_right;
    }
    for (i = 15; i <= 31; i += 16)
    {
        d = CeilingCorrection(X, ({ s32 t_ = Y; t_; }) - i);

        if (d != 0)
            goto found;
    }

walls:
    probe = 0;
    offsets = (s8 *)sSomaOffsets;
    count = offsets[0];
    for (i = 0; i < count; i++)
    {
        s16 d = WallLeftCorrection(({ s32 t_ = X; t_; }) - 8, ({ s32 o_ = offsets[i + 1]; (Y) + o_; }));

        probe = d;
        if (d != 0)
        {
            entity->posX.whole += d << 16;
            flags = FLAGS(entity);
            if (flags & 0x80)
                goto zero_left;
            entity->velX.whole = -entity->velX.whole;
            entity->velX.whole /= 4;
            entity->accelX.whole = -entity->accelX.whole;
        hit_left:
            FLAGS(entity) = flags | 0x40000;
            break;
        }
    }
    if (probe == 0)
    {
        for (i = 0; i < count; i++)
        {
            s16 d = WallRightCorrection(({ s32 t_ = X; t_; }) + 8, ({ s32 o_ = offsets[i + 1]; (Y) + o_; }));

            if (d != 0)
            {
                entity->posX.whole += d << 16;
                flags = FLAGS(entity);
                if (flags & 0x80)
                    goto zero_right;
                entity->velX.whole = -entity->velX.whole;
                entity->velX.whole /= 4;
                entity->accelX.whole = -entity->accelX.whole;
            hit_right:
                FLAGS(entity) = flags | 0x40000;
                break;
            }
        }
    }
}

/* Soma bat-form flight collision with the tile map and the carrying object. */
/* Function table of the object in solidObject (pointer at its unk_4F4). */

/* 0: flat, 1 / 2: slope kinds. */

/* Always 0: the slope kind is at most 2, so combine folds "kind & 4" only after
   the (then dead) kind computation has been emitted. */
static inline s32 HardFloor(void)
{
    s32 d = TILE_FLAGS;
    s32 kind;

    if (TileBit1(d) || (kind = TileBits67(d)))
        kind = SlopeKind(d);
    return kind & 4;
}

#define WALL_HIT(d)                                  \
    {                                                \
        entity->posX.whole += (d) << 16;             \
        entity->accelX.whole = 0;              \
        entity->velX.whole = 0;                      \
        FLAGS(entity) |= 0x40000;                    \
    }

#define CEIL_HIT(c)                                  \
    {                                                \
        entity->posY.whole += (c) << 16;             \
        entity->velY.whole = 0;                      \
        hits++;                                      \
    }

#define SOFT_HIT(c)                                 \
    if ((s16)(c) != 0)                               \
    {                                                \
        if (IsSoftFloor())                           \
            CEIL_HIT((s16)(c))                       \
    }

#define SOFT_FLOOR_HIT(c)                                \
    if ((s16)(c) != 0)                               \
    {                                                \
        if (IsSoftFloor() && !HardFloor())           \
            CEIL_HIT((s16)(c))                       \
    }

/**
 * @brief Collision callback for active bat flight: wall probes at +-6 px
 * from the current offset table, ceiling probes at -15/-13 px and
 * floor probes at three columns, through the carrying-object hooks.
 * Keeps the airborne flag set and preserves unset-probe behavior.
 */
void Soma_ResolveBatFormCollision(struct EwramData_EntityData *entity)
{
    s32 hits = 0;
    s16 x;
    s32 velX = entity->velX.whole + PLAYER_CONVEYOR_VELOCITY(entity);
    s32 side = 0;
    s32 off;
    s16 y;
    s8 *offsets;
    s32 count;
    s32 i;
    s32 probe; /* last probe result (u16); may be unset if count <= 0 */

    gUnk_03002CB0.objectFloorContact = side;
    FLAGS(entity) = (FLAGS(entity) & 0xFEEFFBFF) | 2;
    entity->ext.player.onStairs = side;
    x = GetEntityRoomXPositionInteger(entity);
    y = GetEntityRoomYPositionInteger(entity);
    if (Collision_IsPointSolid(X, Y - 8) != 0)
        FLAGS(entity) |= 0x1000000;
    offsets = (s8 *)entity->ext.player.wallProbes;
    count = offsets[0];
    if (velX < 0)
    {
        for (i = 0; i < count; i++)
        {
            s16 d = WallLeftCorrection(x - 6, Y + offsets[i + 1]);

            probe = (u16)d;
            if (d != 0)
            {
                WALL_HIT(d)
                break;
            }
        }
    }
    else if (velX > 0)
    {
        for (i = 0; i < count; i++)
        {
            s16 d = WallRightCorrection(x + 6, Y + offsets[i + 1]);

            probe = (u16)d;
            if (d != 0)
            {
                WALL_HIT(d)
                break;
            }
        }
    }
    else
    {
        for (i = 0; i < count; i++)
        {
            s16 d = WallLeftCorrection(x - 6, Y + offsets[i + 1]);

            probe = (u16)d;
            if (d != 0)
            {
                WALL_HIT(d)
                break;
            }
        }
        if ((s16)probe == 0)
        {
            for (i = 0; i < count; i++)
            {
                s16 d = WallRightCorrection(x + 6, Y + offsets[i + 1]);

                probe = (u16)d;
                if (d != 0)
                {
                    WALL_HIT(d)
                    break;
                }
            }
        }
    }
    if ((s16)probe != 0)
        entity->velX.whole = 0;

    x = GetEntityRoomXPositionInteger(entity);
    {
        probe = (u16)CeilingCorrection(X, Y - 15);
        if ((s16)probe != 0)
        {
            CEIL_HIT((s16)probe)
            if (TILE_FLAGS & 0xC0)
            {
                if (TILE_FLAGS & 4)
                    side |= 0x4000;
                else
                    side |= 0x2000;
            }
        }
    }
    if ((s16)probe == 0)
    {
        probe = (u16)CeilingCorrection(X + 4, Y - 13);
        SOFT_HIT(probe)
        if ((s16)probe == 0)
        {
            probe = (u16)CeilingCorrection(X - 4, Y - 13);
            SOFT_HIT(probe)
        }
    }

    off = ((-hits | hits) >> 31) & 2;
    probe = (u16)FloorProbe(X, Y + off, 0);
    SOFT_HIT(probe)
    if ((s16)probe == 0)
    {
        probe = (u16)FloorProbe(X - 4, Y + off, 0);
        SOFT_FLOOR_HIT(probe)
        if ((s16)probe == 0)
        {
            probe = (u16)FloorProbe(X + 4, Y + off, 0);
            SOFT_FLOOR_HIT(probe)
        }
    }

    if (hits > 1)
    {
        if (side & 0x4000)
        {
            if (velX < 0)
                entity->velX.whole = 0;
        }
        else if (side & 0x2000)
        {
            if (velX > 0)
                entity->velX.whole = 0;
        }
    }
}

/* Soma_TryCeilingImpact; animation
 * block as in Soma_InitEntity (this file). */

/**
 * @brief Called after a ceiling correction while rising. When velY is at
 * least 0x4F000 upward with flag 0x10 but not 0x80, play sound 0xB8,
 * animation 0x25, shake the screen, stop motion, enter state 6 and
 * return 1; otherwise return 0. It is not a hard-landing handler.
 */
s32 Soma_TryCeilingImpact(struct EwramData_EntityData *entity)
{
    struct PlayerBodyBox box;
    s32 pal;

    if (entity->velY.whole <= -0x4F000 && (entity->ext.player.actionFlags & 0x90) == 0x10)
    {
        PlaySong(0xB8);
        Effect_SpawnDebrisBurst(entity);
        box = sSomaStandingBodyBox[0];
        if (!(gEwramData->playerState.actionLockFlags & 0x800) && entity->animScriptIndex != 0x25)
        {
            Anim_StartIndexed(entity, 0x25, 3, 0);
            HitBox_SetBody(entity, &box);
            entity->ext.player.requestedAnim = 0xFF;
            entity->ext.player.actionFlags &= ~0x200000;
            pal = sSomaAnimPalettes[0x25];
            if (entity->ext.player.animPalette != pal)
            {
                entity->ext.player.animPalette = pal;
                ObjPalette_LoadRange(sSomaPalette, pal, 1, 0);
            }
        }
        entity->state = 6;
        ScreenShake_Request(1, 0x10, 0, 7);
        entity->velX.whole = 0;
        entity->velY.whole = 0;
        entity->accelY.whole = 0;
            return 1;
    }
    return 0;
}

/* Soma_TryStartDiveAttack. */

/* Animation switch as in Soma_UpdateStatusReaction, with the sSomaAirborneBodyBox hitbox and flag 1. */
static inline void SetSlideAnim(struct EwramData_EntityData *entity, s32 anim)
{
    struct PlayerBodyBox box = sSomaAirborneBodyBox[0];
    s32 pal;

    if (!(gEwramData->playerState.actionLockFlags & 0x800) && entity->animScriptIndex != anim)
    {
        Anim_StartIndexed(entity, anim, 3, 1);
        HitBox_SetBody(entity, &box);
        entity->ext.player.requestedAnim = 0xFF;
        entity->ext.player.actionFlags &= ~0x200000;
        pal = sSomaAnimPalettes[anim];
        if (entity->ext.player.animPalette != pal)
        {
            entity->ext.player.animPalette = pal;
            ObjPalette_LoadRange(sSomaPalette, pal, 1, 0);
        }
    }
}

/**
 * @brief When ability check 3 passes, the entity is airborne (flag 2)
 * with flag 4 but not 0x800000, and down + new jump is pressed,
 * enter state 7 with downward velY 0x4E000, optional +-0x40000 X,
 * animation 0x26/0x27 and attack box by speed, and sound 0xA9.
 * The old slide description was incorrect.
 */
void Soma_TryStartDiveAttack(struct EwramData_EntityData *entity)
{
    struct EwramData *ew;
    u16 key;
    s32 speed;

    if (!Player_HasAbility(3))
        return;
    if (!(entity->ext.player.actionFlags & 2))
        return;
    if ((entity->ext.player.actionFlags & 0x800004) != 4)
        return;
    ew = gEwramData;
    if (!(ew->inputData.playerHeldInput & 0x80))
        return;
    if (!(ew->inputData.playerNewInput & ew->playerStatus.jumpButton))
        return;
    entity->hit.type = 2;
    entity->state = 7;
    entity->velY.whole = 0x4E000;
    entity->accelY.whole = 0x2000;
    if ((key = ew->inputData.playerHeldInput & 0x20))
    {
        entity->flipX = 1;
        entity->velX.whole = -0x40000;
        entity->accelX.whole = 0;
    }
    else if (ew->inputData.playerHeldInput & 0x10)
    {
        entity->flipX = 0;
        entity->velX.whole = 0x40000;
        entity->accelX.whole = key;
    }
    else
        entity->accelX.whole = (entity->velX.whole < 0) ? 0x4000 : -0x4000;
    speed = entity->velX.whole;
    if (speed < 0)
        speed = -speed;
    if (speed > 0x10000)
    {
        SetSlideAnim(entity, 0x26);
        HitBox_SetAttack(entity, sSomaDiveAttackBoxFast);
    }
    else
    {
        SetSlideAnim(entity, 0x27);
        HitBox_SetAttack(entity, sSomaDiveAttackBoxSlow);
    }
    gEwramData->playerState.actionLockFlags |= 4;
    PlaySong(0xA9);
}

static inline void SetBox(struct EwramData_EntityData *entity, s16 n)
{
    struct PlayerBodyBox box = sSomaAirborneBodyBox[0];
    entity->ext.player.requestedAnim = n;
    HitBox_SetBody(entity, &box);
}

/* Handle the observed flag-dependent state, box, sound and motion changes.
 * Keep the zero assignment after the flag update and its r4 lifetime across
 * the calls: moving or folding it changes agbcc instruction ordering.
 * Matching output does not establish original source spelling. */
/**
 * @brief Update state 7: end on flag 0x400000; on contact flag 0x80000
 * rebound upward (velY -0x38000), set flag 8, clear flag 4, play
 * sound 0xB9 and return to state 0; otherwise set actionLockFlags |= 4.
 */
void Soma_UpdateDiveAttack(struct EwramData_EntityData *entity)
{
    if (entity->ext.player.actionFlags & 0x400000)
    {
        entity->state = 0;
        return;
    }
    entity->hit.flags &= ~2;
    if (entity->ext.player.actionFlags & 0x80000)
    {
        register s32 zero asm("r4"); /* FAKEMATCH: pinned register */
        entity->ext.player.actionFlags = (entity->ext.player.actionFlags | 8) & ~4;
        zero = 0;
        SetBox(entity, 0x14);
        PlaySong(0xB9);
        entity->velY.whole = -0x38000;
        entity->accelY.whole = -0x2000;
        entity->state = zero;
    }
    else
        gEwramData->playerState.actionLockFlags |= 4;
}

/* Soma_UpdateVerticalMovement, decompiled from the original assembly (Soma variant of the
 * vertical physics; Julius uses Julius_UpdateVerticalMovement). 0x08018B98 is only the target
 * of a far jump inside this function. */

/* Function table of the object in solidObject (pointer at its unk_4F4). */

/* 0: flat, 1 / 2: slope kinds. */

#define BOX(i) ((i) ? sSomaAirborneBodyBox[0] : sSomaStandingBodyBox[0])

/* Animation switch with hitbox and palette, as in Soma_TryCeilingImpact. */
static inline void SetAnim(struct EwramData_EntityData *entity, u8 anim, s32 mode, s32 boxIndex)
{
    struct PlayerBodyBox box;

    box = BOX(boxIndex);
    if (!(gEwramData->playerState.actionLockFlags & 0x800) && entity->animScriptIndex != anim)
    {
        s32 pal;
        Anim_StartIndexed(entity, anim, 3, mode);
        HitBox_SetBody(entity, &box);
        entity->ext.player.requestedAnim = 0xFF;
        FLAGS(entity) &= ~0x200000;
        pal = sSomaAnimPalettes[anim];
        if (entity->ext.player.animPalette != pal)
        {
            entity->ext.player.animPalette = pal;
            ObjPalette_LoadRange(sSomaPalette, pal, 1, 0);
        }
    }
}

#define FC(dx, dy) CeilingCorrection(((x << 16) + (dx)) >> 16, ((y << 16) + (dy)) >> 16)
#define FP(dx) FloorProbe(((x << 16) + (dx)) >> 16, ((y << 16) + 0x10000) >> 16, 0)

/**
 * @brief Update Soma grounded/airborne vertical motion, gravity,
 * ceiling/floor and moving-platform probes, landing behavior,
 * animation, body box and palette under the existing flag guards.
 */
void Soma_UpdateVerticalMovement(struct EwramData_EntityData *entity)
{
    u16 corr = 0;
    s16 cs;
    u16 y = GetEntityRoomYPositionInteger(entity);
    u16 x = GetEntityRoomXPositionInteger(entity);
    u32 flags;
    u32 f2;
    u16 c1;
    s32 landed;
    u16 c;

    if (!(FLAGS(entity) & 0x100002))
    {
        FLAGS(entity) |= 2;
        entity->velY.whole = corr;
        entity->accelY.whole = -0x1000;
    }
    flags = FLAGS(entity);
    if ((flags & 0x800010) == 0x800000)
    {
        FLAGS(entity) = flags & 0xEFEFFFFF;
        if (FLAGS(entity) & 0x2000000)
        {
            if (entity->velY.whole < -0x10000)
            {
                entity->velY.whole += 0x4000;
                if (entity->velY.whole > -0x10000)
                {
                    entity->velY.whole = -0x10000;
                    FLAGS(entity) &= ~0x2000000;
                }
            }
            else if (entity->velY.whole > -0x10000)
            {
                entity->velY.whole -= 0x8000;
                if (entity->velY.whole < -0x10000)
                {
                    entity->velY.whole = -0x10000;
                    FLAGS(entity) &= ~0x2000000;
                }
            }
        }
        {
        u32 t = FLAGS(entity);
        u32 m = t & 0x08000800;
        f2 = t;
        if (!m)
        {
            if (!(f2 & 0x2000000))
            {
                entity->accelY.whole = 0;
                entity->velY.whole -= 0x8000;
                if (entity->velY.whole < -0x10000)
                    entity->velY.whole = -0x10000;
            }
            cs = FC(-0x50000, -0x200000);
            corr = cs;
            if (cs != 0)
            {
                entity->posY.whole += corr << 16;
                entity->velY.whole = -0x10000;
                f2 = FLAGS(entity);
            }
            else
            {
                cs = FC(0x50000, -0x200000);
                corr = cs;
                f2 = FLAGS(entity);
                if (cs != 0)
                {
                    entity->posY.whole += corr << 16;
                    entity->velY.whole = -0x10000;
                }
            }
        }
        else if (entity->velY.whole < 0 && !(f2 & 0x2000000))
            entity->velY.whole = 0;
        }
        if (f2 & 0x800)
        {
            cs = FC(-0x50000, -0x200000);
            corr = cs;
            if (cs == 0)
                cs = FC(0x50000, -0x200000);
            corr = cs;
            if (cs != 0)
                entity->posY.whole += corr << 16;
        }
        landed = 0;
        c = FP(0);
        {
            s32 d = (s16)c;
            if (d != 0)
            {
                entity->posY.whole += (d + 1) << 16;
                landed = 1;
            }
        }
        if (entity->velY.whole < 0)
        {
            c1 = FP(-0x50000);
            c = FP(0x50000);
            if (!landed)
            {
                {
                    s32 d = (s16)c1;
                    if (d != 0)
                    {
                        entity->posY.whole += (d + 1) << 16;
                        landed = 1;
                    }
                }
                if (!landed)
                {
                    s32 d = (s16)c;
                    if (d != 0)
                        entity->posY.whole += (d + 1) << 16;
                }
            }
        }
        if (FLAGS(entity) & 0x1E0)
            return;
        if ((u16)entity->ext.player.requestedAnim == 0x29)
            return;
        SetAnim(entity, 0x28, 1, 0);
    }
    else
    {
        if (!(flags & 0x1E))
            return;
        FLAGS(entity) = flags & 0xEFEFFBFF;
        if (FLAGS(entity) & 0x4000000)
        {
            entity->velY.whole += 0x6000;
            entity->accelY.whole = corr;
        }
        if (entity->ext.player.airTime <= 0xF)
            entity->ext.player.airTime++;
        if (entity->velY.whole <= 0 || entity->velX.whole != 0)
        {
            cs = FC(-0x50000, -0x200000);
            corr = cs;
            if (cs != 0)
            {
                entity->posY.part16.integer += ({ s32 t = (s16)corr; t; });
                if (entity->velY.whole < 0)
                {
                    if (Soma_TryCeilingImpact(entity))
                        return;
                    entity->velY.whole = 0x1000;
                    entity->accelY.whole = -0x2000;
                }
            }
            else
            {
                cs = FC(0x50000, -0x200000);
                corr = cs;
                if (cs != 0)
                {
                    entity->posY.part16.integer += ({ s32 t = (s16)corr; t; });
                    if (entity->velY.whole < 0)
                    {
                        if (Soma_TryCeilingImpact(entity))
                            return;
                        entity->velY.whole = 0x1000;
                        entity->accelY.whole = -0x2000;
                    }
                }
            }
        }
        if (entity->velY.whole <= 0x1FFF)
            entity->velY.whole += 0x2000;
        entity->velY.whole += 0x1A00;
        if (entity->velY.whole > 0)
        {
            entity->velY.whole += entity->accelY.whole;
            entity->accelY.whole += 0x400;
            if (entity->accelY.whole > 0x1000)
                entity->accelY.whole = 0x1000;
        }
        if (FLAGS(entity) & 0x10)
        {
            cs = FC(-0x50000, -0x180000);
            corr = cs;
            if (cs != 0)
            {
                entity->posY.part16.integer += ({ s32 t = (s16)corr; t; });
                if (entity->velY.whole < 0)
                {
                    if (Soma_TryCeilingImpact(entity))
                        return;
                    entity->velY.whole = 0x1000;
                    entity->accelY.whole = -0x2000;
                }
            }
            else
            {
                cs = FC(0x50000, -0x180000);
                corr = cs;
                if (cs != 0)
                {
                    entity->posY.part16.integer += ({ s32 t = (s16)corr; t; });
                    if (entity->velY.whole < 0)
                    {
                        if (Soma_TryCeilingImpact(entity))
                            return;
                        entity->velY.whole = 0x1000;
                        entity->accelY.whole = -0x2000;
                    }
                }
            }
            flags = FLAGS(entity);
            if (!(flags & 0x400000))
            {
                if (entity->velY.whole > 0x12000)
                    FLAGS(entity) = flags & ~0x10;
            }
            else if (entity->velY.whole > 0x8000)
                FLAGS(entity) = flags & ~0x10;
        }
        {
            u32 t = FLAGS(entity);
            u32 m = t & 0x1E0;
            f2 = t;
            if (m)
                return;
        }
        if (gEwramData->playerState.actionLockFlags & 4)
            return;
        if ((gEwramData->playerStatus.soulStateFlags & 0x400) && entity->velX.whole != 0)
        {
            SetAnim(entity, 3, 1, 0);
        }
        else if (entity->velY.whole <= 0x3FFF)
        {
            if ((f2 & 4) && (u16)entity->ext.player.requestedAnim != 0xFF)
                return;
            if (!(f2 & 0x10))
            {
                if (entity->velX.whole != 0)
                    SetAnim(entity, 0xB, 0, 1);
                else
                    SetAnim(entity, 0x32, 1, 1);
            }
            else
                SetAnim(entity, 0x24, 1, 1);
        }
        else
            SetAnim(entity, 0xC, 0, 1);
    }
}

/* Soma_TryAirJump. */
/**
 * @brief Used by Soma_HandleJumpInput: when the jump button is pressed in the
 * air state (flag 0x800000 or not 0x04000004) start the extra jump with
 * its hit box, sound 0xB9 and upward velocity. Return 1 when started.
 */
s32 Soma_TryAirJump(struct EwramData_EntityData *entity)
{
    struct PlayerBodyBox box;
    u16 zero;

    if ((entity->ext.player.actionFlags & 0x800000) || !(entity->ext.player.actionFlags & 0x04000004))
    {
        if ((gEwramData->inputData.playerNewInput & gEwramData->playerStatus.jumpButton)
            && (zero = entity->ext.player.dropThroughTimer) == 0)
        {
            u16 time = 0x14;
            box = sSomaAirborneBodyBox[0];
            entity->ext.player.requestedAnim = time;
            HitBox_SetBody(entity, &box);
            PlaySong(0xB9);
            entity->ext.player.actionFlags = (entity->ext.player.actionFlags | 4) & ~0x10;
            if (!(entity->ext.player.actionFlags & 0x400000))
            {
                entity->velY.whole = -0x42000;
                entity->accelY.whole = 0x1400;
            }
            else if (entity->velY.whole >= 0)
            {
                entity->velY.whole = -0x48000;
                entity->accelY.whole = zero;
            }
            return 1;
        }
    }
    return 0;
}

/* Soma_HandleJumpInput. */
extern void Afterimage_Create(struct EwramData_EntityData *, s32, s32, s32);
s32 Soma_TryAirJump(struct EwramData_EntityData *entity);
void Soma_TryStartDiveAttack(struct EwramData_EntityData *entity);

/**
 * @brief Process drop-through, ordinary jump, ability-gated high jump
 * and additional airborne jump input under the existing flags.
 * Retain the helper fallback, timing windows and movement variants.
 */
void Soma_HandleJumpInput(struct EwramData_EntityData *entity)
{
    u32 flags = entity->ext.player.actionFlags;
    u32 zero;

    if (flags & 0x20000000)
        return;
    if (flags & 0x160)
        return;
    if (!(zero = flags & 2) && (flags & 0x1000)
        && (gEwramData->inputData.playerHeldInput & 0x80)
        && (gEwramData->inputData.playerNewInput & gEwramData->playerStatus.jumpButton))
    {
        entity->ext.player.dropThroughTimer = 0x10;
        if (flags & 0x1000000)
            entity->ext.player.dropThroughTimer = 0x20;
        entity->ext.player.actionFlags = (entity->ext.player.actionFlags | 2) & ~0x11000;
        entity->velX.whole = zero;
        entity->velY.whole = zero;
        entity->accelY.whole = -0x1000;
    }
    if (Player_HasAbility(4) && (gEwramData->inputData.playerNewInput & gEwramData->playerStatus.abilityButton))
    {
        flags = entity->ext.player.actionFlags;
        if (((flags & 2) || (gEwramData->inputData.playerHeldInput & 0x40)) && !(zero = flags & 0x10))
        {
            entity->ext.player.requestedAnim = 0xFF;
            entity->state = 5;
            PlaySong(0xBA);
            entity->ext.player.actionFlags = (entity->ext.player.actionFlags | 0x12) & ~0x100404;
            entity->accelY.whole = zero;
            if (!(entity->ext.player.actionFlags & 0x4000000))
                entity->velY.whole = -0xA0000;
            else
                entity->velY.whole = -0x28000;
            Afterimage_Create(entity, 0, 8, 0x1D);
            Afterimage_Create(entity, 0, 6, 0x1C);
            return;
        }
    }
    if ((u16)entity->ext.player.dropThroughTimer != 0)
        return;
    flags = entity->ext.player.actionFlags;
    if (!(zero = flags & 0x800000))
    {
        if (!(flags & 2) || entity->ext.player.airTime <= 3)
        {
            if (gEwramData->inputData.playerNewInput & gEwramData->playerStatus.jumpButton)
            {
                PlaySong(0xB9);
                entity->ext.player.actionFlags = (entity->ext.player.actionFlags | 2) & ~0x100400;
                entity->ext.player.airTime = 0x10;
                entity->accelY.whole = zero;
                if (!(entity->ext.player.actionFlags & 0x4000000))
                    entity->velY.whole = -0x4F400;
                else
                    entity->velY.whole = -0x13C00;
            }
        }
        else if (Player_HasAbility(2) && Soma_TryAirJump(entity))
            return;
    }
    else if (!(zero = flags & 0x2000000))
    {
        if (gEwramData->inputData.playerNewInput & gEwramData->playerStatus.jumpButton)
        {
            entity->accelY.whole = zero;
            entity->ext.player.actionFlags = (flags | 0x02000002) & ~0x100408;
            if (entity->velY.whole > 0x10000)
                entity->velY.whole -= 0x4E000;
            else
                entity->velY.whole = -0x4E000;
            return;
        }
    }
    Soma_TryStartDiveAttack(entity);
}

/* Soma_UpdateJumpControl; unk_4F4 is
 * read from the entity each time (as in Julius_UpdateJumpControl). */

void Soma_UpdateVerticalMovement(struct EwramData_EntityData *entity);

/**
 * @brief Apply held/released-jump vertical control and existing ability
 * flag adjustments, then call Soma vertical movement. Return early
 * under player flag 0x10; preserve the asymmetric speed thresholds.
 */
void Soma_UpdateJumpControl(struct EwramData_EntityData *entity)
{
    struct EwramData *ew = gEwramData;

    if (ew->playerState.actionLockFlags & 0x10)
        return;
    if (entity->ext.player.actionFlags & 0x1E)
    {
        if (!(entity->ext.player.actionFlags & 0x18))
        {
            if (entity->velY.whole < -0x4000 && !(ew->inputData.playerHeldInput & ew->playerStatus.jumpButton))
            {
                entity->velY.whole = -0x4000;
                entity->accelY.whole = -0x2000;
            }
            if (entity->velY.whole >= -0x1BFFF && entity->velY.whole <= 0x1FFF
                && (gEwramData->inputData.playerHeldInput & gEwramData->playerStatus.jumpButton))
            {
                entity->accelY.whole -= 0x800;
                if (entity->accelY.whole < -0x2000)
                    entity->accelY.whole = -0x2000;
            }
        }
        if (((gEwramData->playerStatus.soulStateFlags & 0x100) || (entity->ext.player.actionFlags & 0x400000)) && entity->velY.whole > 0x2800)
            entity->velY.whole -= 0x2800;
        if (gEwramData->playerStatus.soulStateFlags & 0x800)
        {
            entity->velY.whole = 0;
            entity->accelY.whole = 0;
        }
    }
    Soma_UpdateVerticalMovement(entity);
}

/* Soma_CastEquippedRedSoul; soul spawn
 * as in Soma_TryCastRedSoulUnused (src/player/player_julius.c). */

/* Animation switch of the player states (as SetPlayerAnim in
 * this file) with the hitbox copied from table[0]. */
static inline void SetPlayerAnimBox2(struct EwramData_EntityData *entity, u8 anim, struct PlayerBodyBox *table)
{
    struct PlayerBodyBox box = table[0];
    s32 pal;

    if (!(gEwramData->playerState.actionLockFlags & 0x800) && entity->animScriptIndex != anim)
    {
        Anim_StartIndexed(entity, anim, 3, 0);
        HitBox_SetBody(entity, &box);
        entity->ext.player.requestedAnim = 0xFF;
        entity->ext.player.actionFlags &= ~0x200000;
        pal = sSomaAnimPalettes[anim];
        if (entity->ext.player.animPalette != pal)
        {
            entity->ext.player.animPalette = pal;
            ObjPalette_LoadRange(sSomaPalette, pal, 1, 0);
        }
    }
}

/**
 * @brief Validate equipped red soul, MP, active limit and entity slot,
 * create and register its entity and deduct MP. Unless noAnim,
 * apply cast animation/motion and enter casting state 2.
 */
void Soma_CastEquippedRedSoul(struct EwramData_EntityData *entity, s32 noAnim)
{
    const struct RedSoulInfo *info = RedSoul_GetInfo(gEwramData->playerStatus.equippedRedSoul);
    struct EwramData_EntityData *soul;
    s8 slot;
    u8 anim;

    if (gEwramData->playerState.actionLockFlags & 0x100)
        return;
    if (gEwramData->gameState.gameFlagsTail[0] & 0x200000)
        return;
    if (gEwramData->playerState.nbrActiveRedSoulEntities == 0)
    {
        if (gEwramData->playerState.lastUsedRedSoul != gEwramData->playerStatus.equippedRedSoul)
        {
            gEwramData->playerState.lastUsedRedSoul = gEwramData->playerStatus.equippedRedSoul;
            if (gEwramData->playerState.redSoulGfx != -1)
            {
                ObjGfx_Unload(gEwramData->playerState.redSoulGfx);
                gEwramData->playerState.redSoulGfx = -1;
            }
        }
    }
    if (gEwramData->playerState.lastUsedRedSoul != gEwramData->playerStatus.equippedRedSoul)
        return;
    if (info->updateFunc == 0)
        return;
    if (info->mpCost > gEwramData->playerStatus.currentMP)
        return;
    if (RedSoul_GetActiveLimit() <= gEwramData->playerState.nbrActiveRedSoulEntities)
        return;
    slot = RedSoul_FindFreeEntitySlot();
    if (slot < 0)
        return;
    soul = EntityCreateInRange(2, 0x10, info->updateFunc);
    if (soul == NULL)
        return;
    gEwramData->playerStatus.currentMP -= info->mpCost;
    gEwramData->playerState.activeRedSoulEntities[slot] = soul;
    soul->ext.redSoul.owner = entity;
    soul->ext.redSoul.slot = slot;
    soul->ext.redSoul.id = gEwramData->playerStatus.equippedRedSoul;
    soul->ext.redSoul.param = info->param;
    soul->roomActive = 1;
    if (noAnim)
        return;
    entity->subState = 0;
    if (!(entity->ext.player.actionFlags & 2))
    {
        anim = RedSoul_GetCastAnim(0);
        SetPlayerAnimBox2(entity, anim, sSomaStandingBodyBox);
        switch (info->castAnimKind)
        {
        case 1:
        {
            u8 v = 0x10;
            gEwramData->playerState.player->ext.player.invulnerableTimer = v;
            entity->ext.player.actionFlags = (entity->ext.player.actionFlags | 0xA) & ~0x100400;
            entity->velY.whole = -0x38000;
            entity->accelY.whole = -0x1000;
            break;
        }
        case 2:
        {
            u8 v = 0x10;
            gEwramData->playerState.player->ext.player.invulnerableTimer = v;
            if (!entity->flipX)
                entity->velX.whole = 0x30000;
            else
                entity->velX.whole = -0x30000;
            break;
        }
        case 3:
        {
            u8 v = 0x32;
            gEwramData->playerState.player->ext.player.invulnerableTimer = v;
            entity->subState = 1;
            break;
        }
        }
    }
    else
    {
        anim = RedSoul_GetCastAnim(1);
        SetPlayerAnimBox2(entity, anim, sSomaAirborneBodyBox);
        if (info->castAnimKind == 3)
        {
            u8 v = 0x14;
            gEwramData->playerState.player->ext.player.invulnerableTimer = v;
        }
    }
    entity->ext.player.actionFlags = (entity->ext.player.actionFlags & ~0x200060) | 0x100;
    entity->state = 2;
}

/* Soma_HandleWeaponAttackInput. */
/* Animation switch as in Soma_UpdateStatusReaction with the hitbox table as parameter; the
 * u8 anim parameter is needed for the original register allocation. */

/**
 * @brief On a permitted attack request with no active attack entity,
 * start the weapon helper and choose standing/crouching/air animation
 * and box, then enter weapon attack state 1 under existing guards.
 */
void Soma_HandleWeaponAttackInput(struct EwramData_EntityData *entity)
{

    if (gEwramData->playerState.actionLockFlags & 0x80)
        return;
    if (!(gEwramData->inputData.playerNewInput & gEwramData->playerStatus.attackButton) && !(gEwramData->playerStatus.inputOverrides & 2))
        return;
    if (entity->ext.player.actionFlags & 0x160)
        return;
    if (gEwramData->playerState.weaponEntities[0] != NULL)
        return;
    if (Weapon_CreateAttackEntity(entity) == NULL)
        return;
    if (Weapon_GetEquippedInfo()->attackAnimKind == 5)
        return;
    if (entity->ext.player.actionFlags & 2)
    {
        entity->ext.player.actionFlags |= 0x40;
        SetPlayerAnimBox2(entity, Weapon_GetPlayerAttackAnim(2), sSomaAirborneBodyBox);
    }
    else if (!(entity->ext.player.actionFlags & 0x400))
    {
        SetPlayerAnimBox2(entity, Weapon_GetPlayerAttackAnim(0), sSomaStandingBodyBox);
    }
    else
    {
        SetPlayerAnimBox2(entity, Weapon_GetPlayerAttackAnim(1), sSomaCrouchBodyBoxes);
    }
    entity->ext.player.actionFlags |= 0x20;
    entity->state = 1;
}

/* Soma_UpdateKnockback (Soma_UpdateKnockbackFromImpulse
 * without the animation switch). */

void Soma_UpdateVerticalMovement(struct EwramData_EntityData *entity);

/**
 * @brief Update damage-reaction state 13: damp horizontal motion without
 * crossing zero, recover on landing or tile/animation completion and
 * allow the existing ability-gated jump cancel. Update vertical motion.
 */
void Soma_UpdateKnockback(struct EwramData_EntityData *entity)
{
    s32 old, new;
    s32 hit;
    u16 x;
    s16 y;

    entity->ext.player.actionFlags &= 0xEFFFFE9F;
    x = GetEntityRoomXPositionInteger(entity);
    y = GetEntityRoomYPositionInteger(entity);
    hit = 0;
    if (Collision_IsPointSolid((s16)x, y))
    {
        hit = 1;
        entity->accelX.whole = (entity->velX.whole < 0) ? 0x1000 : -0x1000;
    }
    old = entity->velX.whole;
    new = old + entity->accelX.whole;
    entity->velX.whole = new;
    if (old < 0)
        goto check_pos;
    if (new < 0)
        goto clear;
    if (old > 0)
        goto done;
check_pos:
    if (new <= 0)
        goto done;
clear:
    entity->velX.whole = 0;
    entity->accelX.whole = 0;
done:
    if (!(entity->ext.player.actionFlags & 2) || (hit && (entity->ext.player.actionFlags & 0x200000)))
    {
        entity->ext.player.actionFlags = entity->ext.player.actionFlags & 0xFFFEFF7F;
        entity->state = 0;
    }
    else if ((gEwramData->playerStatus.soulStateFlags & 0x400000) && (gEwramData->inputData.playerNewInput & gEwramData->playerStatus.jumpButton))
    {
        entity->accelY.whole = 0;
        entity->ext.player.actionFlags = entity->ext.player.actionFlags & 0xFFEFFB7F;
        PlaySong(0xB9);
        entity->ext.player.actionFlags |= 6;
        entity->velY.whole = 0;
        entity->state = 0;
    }
    Soma_UpdateVerticalMovement(entity);
    Soma_HandleBlueSoulInput(entity);
}

/* Soma_UpdateKnockbackFromImpulse; animation
 * block as in Soma_TryCeilingImpact, velocity stop as in Julius_UpdateKnockback. */

void Soma_UpdateVerticalMovement(struct EwramData_EntityData *entity);

/**
 * @brief Update damage-reaction state 14, entered with stored external
 * impulse velocities. Select animation 0x11, damp horizontal motion,
 * recover on landing/completion and update vertical motion.
 */
void Soma_UpdateKnockbackFromImpulse(struct EwramData_EntityData *entity)
{
    struct PlayerBodyBox box;
    s32 pal;
    s32 old, new;
    s32 hit;
    u16 x;
    s16 y;

    entity->ext.player.actionFlags &= 0xEFFFFE9F;
    box = sSomaStandingBodyBox[0];
    if (!(gEwramData->playerState.actionLockFlags & 0x800) && entity->animScriptIndex != 0x11)
    {
        Anim_StartIndexed(entity, 0x11, 3, 0);
        HitBox_SetBody(entity, &box);
        entity->ext.player.requestedAnim = 0xFF;
        entity->ext.player.actionFlags &= ~0x200000;
        pal = sSomaAnimPalettes[0x11];
        if (entity->ext.player.animPalette != pal)
        {
            entity->ext.player.animPalette = pal;
            ObjPalette_LoadRange(sSomaPalette, pal, 1, 0);
        }
    }
    x = GetEntityRoomXPositionInteger(entity);
    y = GetEntityRoomYPositionInteger(entity);
    hit = 0;
    if (Collision_IsPointSolid((s16)x, y))
    {
        hit = 1;
        entity->accelX.whole = (entity->velX.whole < 0) ? 0x1000 : -0x1000;
    }
    old = entity->velX.whole;
    new = old + entity->accelX.whole;
    entity->velX.whole = new;
    if (old < 0)
        goto check_pos;
    if (new < 0)
        goto clear;
    if (old > 0)
        goto done;
check_pos:
    if (new <= 0)
        goto done;
clear:
    entity->velX.whole = 0;
    entity->accelX.whole = 0;
done:
    if (!(entity->ext.player.actionFlags & 2) || (hit && (entity->ext.player.actionFlags & 0x200000)))
    {
        entity->ext.player.actionFlags &= 0xFFFEFF7F;
        entity->state = 0;
    }
    Soma_UpdateVerticalMovement(entity);
    Soma_HandleBlueSoulInput(entity);
}

/* Soma_UpdateStatusReaction. */
void Soma_UpdateVerticalMovement(struct EwramData_EntityData *entity);

/* Switch the player animation (with hitbox and palette from the per-animation
 * tables) unless actionLockFlags bit 11 or it is already active. */
static inline void SetPlayerAnim(struct EwramData_EntityData *entity, s32 anim)
{
    struct PlayerBodyBox box = sSomaStandingBodyBox[0];
    s32 pal;

    if (!(gEwramData->playerState.actionLockFlags & 0x800) && entity->animScriptIndex != anim)
    {
        Anim_StartIndexed(entity, anim, 3, 0);
        HitBox_SetBody(entity, &box);
        entity->ext.player.requestedAnim = 0xFF;
        entity->ext.player.actionFlags &= ~0x200000;
        pal = sSomaAnimPalettes[anim];
        if (entity->ext.player.animPalette != pal)
        {
            entity->ext.player.animPalette = pal;
            ObjPalette_LoadRange(sSomaPalette, pal, 1, 0);
        }
    }
}

/**
 * @brief Update state 15 while status bit statusEffects & 4 remains set.
 * Select animation 0x49/0x4A from the existing status flag, stop
 * grounded horizontal motion and update vertical movement.
 */
void Soma_UpdateStatusReaction(struct EwramData_EntityData *entity)
{
    u16 x;
    s16 y;
    u16 flag = gEwramData->playerStatus.statusEffects & 4;

    if (!flag)
    {
        entity->ext.player.actionFlags &= 0xFFFEFF7F;
        entity->ext.player.actionFlags &= 0xEFFFFE9F;
        entity->state = flag;
        return;
    }
    x = GetEntityRoomXPositionInteger(entity);
    y = GetEntityRoomYPositionInteger(entity);
    Collision_IsPointSolid((s16)x, y);
    if (gEwramData->playerStatus.petrifyCounter & 2)
        SetPlayerAnim(entity, 0x49);
    else
        SetPlayerAnim(entity, 0x4A);
    if (!(entity->ext.player.actionFlags & 2))
    {
        entity->velX.whole = 0;
        entity->accelX.whole = 0;
    }
    Soma_UpdateVerticalMovement(entity);
}

/* Soma_HandleBatFlightInput. */

/* Animation switch as in Soma_UpdateStatusReaction, with the sSomaBatFormBodyBoxes hitbox, flag 1 and
 * the saved animStep restored afterwards. */
static inline void SetFlyAnim(struct EwramData_EntityData *entity, s32 anim, u8 saved)
{
    struct PlayerBodyBox box = sSomaBatFormBodyBoxes[0];
    s32 pal;

    if (!(gEwramData->playerState.actionLockFlags & 0x800) && entity->animScriptIndex != anim)
    {
        Anim_StartIndexed(entity, anim, 3, 1);
        HitBox_SetBody(entity, &box);
        entity->ext.player.requestedAnim = 0xFF;
        entity->ext.player.actionFlags &= ~0x200000;
        pal = sSomaAnimPalettes[anim];
        if (entity->ext.player.animPalette != pal)
        {
            entity->ext.player.animPalette = pal;
            ObjPalette_LoadRange(sSomaPalette, pal, 1, 0);
        }
        entity->animStep = saved;
    }
}

/**
 * @brief Bat-form flight input: each held d-pad axis sets +-0x20000
 * velocity, otherwise decelerate by 0x4000 without crossing zero.
 * Select animation 0x1B idle or 0x1C moving, restoring animStep.
 */
void Soma_HandleBatFlightInput(struct EwramData_EntityData *entity)
{
    u8 saved = entity->animStep;
    u16 key;
    u16 key2;
    s32 old, new;

    entity->ext.player.wallProbes = (u32)sSomaWallProbesBat;
    if ((key = gEwramData->inputData.playerHeldInput & 0x20))
    {
        entity->flipX = 1;
        entity->velX.whole = -0x20000;
        entity->accelX.whole = 0;
    }
    else if (gEwramData->inputData.playerHeldInput & 0x10)
    {
        entity->flipX = 0;
        entity->velX.whole = 0x20000;
        entity->accelX.whole = key;
    }
    else
        entity->accelX.whole = (entity->velX.whole < 0) ? 0x4000 : -0x4000;
    old = entity->velX.whole;
    new = old + entity->accelX.whole;
    entity->velX.whole = new;
    if (old < 0)
        goto check_x;
    if (new < 0)
        goto clear_x;
    if (old > 0)
        goto done_x;
check_x:
    if (new <= 0)
        goto done_x;
clear_x:
    entity->velX.whole = 0;
    entity->accelX.whole = 0;
done_x:
    if ((key2 = gEwramData->inputData.playerHeldInput & 0x40))
    {
        entity->velY.whole = -0x20000;
        entity->accelY.whole = 0;
    }
    else if (gEwramData->inputData.playerHeldInput & 0x80)
    {
        entity->velY.whole = 0x20000;
        entity->accelY.whole = key2;
    }
    else
        entity->accelY.whole = (entity->velY.whole < 0) ? 0x4000 : -0x4000;
    old = entity->velY.whole;
    new = old + entity->accelY.whole;
    entity->velY.whole = new;
    if (old < 0)
        goto check_y;
    if (new < 0)
        goto clear_y;
    if (old > 0)
        goto done_y;
check_y:
    if (new <= 0)
        goto done_y;
clear_y:
    entity->velY.whole = 0;
    entity->accelY.whole = 0;
done_y:
    if ((u16)entity->ext.player.requestedAnim == 0xFF)
    {
        if (!(gEwramData->inputData.playerHeldInput & 0xF0))
            SetFlyAnim(entity, 0x1B, saved);
        else
            SetFlyAnim(entity, 0x1C, saved);
    }
}

/* Soma bat-form state 8 and its exit path (three floor probes at x, x - 8,
 * x + 8). Helpers copied from this file. */

/* Function table of the object in solidObject (pointer at its unk_4F4). */

/* 0: flat, 1 / 2: slope kinds. */

static inline void StartAnim(struct EwramData_EntityData *entity, u16 anim)
{
    struct PlayerBodyBox box2 = sSomaBatFormBodyBoxes[0];

    entity->ext.player.requestedAnim = anim;
    HitBox_SetBody(entity, &box2);
}

/**
 * @brief Player state 8 entered by SkillGiantBatUse (soulStateFlags bit 0x200).
 * While the bat flag remains, process flight input, optional red-soul
 * cast and bat-form collision; once cleared, enter substate 2, resolve
 * exit collision with horizontal-only control and return to state 0.
 */
void Soma_UpdateBatForm(struct EwramData_EntityData *entity)
{
    if (entity->subState != 2 && !(gEwramData->playerStatus.soulStateFlags & 0x200))
    {
        struct PlayerBodyBox box;

        entity->velX.whole = 0;
        entity->accelX.whole = 0;
        entity->velY.whole = 0;
        entity->accelY.whole = 0;
        FLAGS(entity) &= ~0x200000;
        FLAGS(entity) &= 0xEFFFFE9F;
        entity->ext.player.wallProbes = (u32)sSomaWallProbesBat;
        Soma_ResolveBatExitCollision(entity);
        if (FLAGS(entity) & 0x01000080)
            entity->state = 0;
        entity->subState = 2;
        box = sSomaBatFormBodyBoxes[0];
        if (!(gEwramData->playerState.actionLockFlags & 0x800) && entity->animScriptIndex != 0x1E)
        {
            s32 pal;

            Anim_StartIndexed(entity, 0x1E, 3, 0);
            HitBox_SetBody(entity, &box);
            entity->ext.player.requestedAnim = 0xFF;
            entity->ext.player.actionFlags &= ~0x200000;
            pal = sSomaAnimPalettes[0x1E];
            if (entity->ext.player.animPalette != pal)
            {
                entity->ext.player.animPalette = pal;
                ObjPalette_LoadRange(sSomaPalette, pal, 1, 0);
            }
        }
    }

    switch (entity->subState)
    {
    case 0:
        StartAnim(entity, 0x1D);
        entity->subState = 1;
    case 1:
        if (gEwramData->inputData.playerNewInput & gEwramData->playerStatus.attackButton)
        {
            if (gEwramData->playerStatus.equippedRedSoul == 2
                || (*(u32 *)&gEwramData->playerStatus.equippedWeapon & 0xFF00FF00) == 0x1E000800
                || (*(u32 *)&gEwramData->playerStatus.equippedWeapon & 0xFF00FF00) == 0x07002C00)
                Soma_CastEquippedRedSoul(entity, 1);
        }
        Soma_HandleBatFlightInput(entity);
        Soma_ResolveBatFormCollision(entity);
        break;
    case 2:
    {
        u16 key;
        s32 old, new;
        u16 x, y;

        gEwramData->playerState.actionLockFlags |= 3;
        if ((key = gEwramData->inputData.playerHeldInput & 0x20))
        {
            entity->flipX = 1;
            entity->velX.whole = -0x20000;
            entity->accelX.whole = 0;
        }
        else if (gEwramData->inputData.playerHeldInput & 0x10)
        {
            entity->flipX = 0;
            entity->velX.whole = 0x20000;
            entity->accelX.whole = key;
        }
        else
            entity->accelX.whole = (entity->velX.whole < 0) ? 0x4000 : -0x4000;
        old = entity->velX.whole;
        new = old + entity->accelX.whole;
        entity->velX.whole = new;
        if (old < 0)
            goto check_x;
        if (new < 0)
            goto clear_x;
        if (old > 0)
            goto done_x;
    check_x:
        if (new <= 0)
            goto done_x;
    clear_x:
        entity->velX.whole = 0;
        entity->accelX.whole = 0;
    done_x:
        entity->velY.whole = 0;
        entity->accelY.whole = 0;
        entity->ext.player.wallProbes = (u32)sSomaWallProbesBat;
        Soma_ResolveBatExitCollision(entity);
        if (!(FLAGS(entity) & 0x200000))
            return;
        x = GetEntityRoomXPositionInteger(entity);
        y = GetEntityRoomYPositionInteger(entity);
        if ((s16)FloorProbe(X, Y + 1, 0) != 0
            || (s16)FloorProbe(({ s32 t_ = X; t_; }) - 8, ({ s32 t_ = Y; t_; }) + 1, 0) != 0
            || (s16)FloorProbe(({ s32 t_ = X; t_; }) + 8, ({ s32 t_ = Y; t_; }) + 1, 0) != 0)
            FLAGS(entity) = (FLAGS(entity) & ~0x1E) | 0x100000;
        entity->state = 0;
        break;
    }
    }
}

/* Soma transformation-dash state 9 (Devil/Manticore/Curly souls: charge,
 * sparks, dash with ceiling probes, stop). Helpers copied from
 * this file. */

/* Function table of the object in solidObject (pointer at its unk_4F4). */

/* 0: flat, 1 / 2: slope kinds. */

static inline s16 PlatformFloor2(s16 x, s16 y, u16 c)
{
    s32 i;

    for (i = 0; i < gEwramData->playerState.collisionQueueCount; i++)
    {
        struct SpriteBox a;
        struct SpriteBox b;
        struct EwramData_EntityData *platform = gEwramData->playerState.collisionQueueEntities[i];
        s16 px = GetEntityRoomXPositionInteger(platform);
        s16 py = GetEntityRoomYPositionInteger(platform);
        s16 right;
        s16 extra;
        s16 up;
        u8 *state;
        struct EwramData *ewram;

        if (HitBox_GetBoxes(&a, &b, platform) == 0)
            continue;
        if (platform->flipX)
        {
            right = px - b.x;
            px = right - b.width;
        }
        else
        {
            px = px + b.x;
            right = b.width + px;
        }
        {
            s32 top = py - 1;

            py = b.y + top;
        }
        extra = 0;
        up = 0;
        ewram = gEwramData;
        {
            u8 *states = ewram->playerState.collisionQueueFlags;

            state = &states[i];
        }
        if (*state & 2)
            up = platform->velY.part16.integer + 1;
        if (*state & 4)
            extra = 8;
        if (x >= px && x <= right && y >= py - up && y <= py + (extra + 8))
        {
            c = py - y;
            ewram->playerState.standingPlatform = platform;
            if (ewram->playerState.collisionQueueFlags[i] & 1)
                ewram->playerState.collisionQueueFlags[i] |= 8;
            gUnk_03002CB0.objectFloorContact = 1;
            return c;
        }
    }
    return c;
}

static inline u16 FloorProbe2(s16 x, s16 y, s32 flag)
{
    u16 c;

    {
        s32 tx = x;
        s32 ty = y;
        s16 r = Collision_GetPushUpFiltered(tx, ty, flag);
        struct EwramData_EntityData *object;

        c = r;
        object = gEwramData->playerState.solidObject;
        if (object != NULL)
        {
            s16 d = FUNCS(object)->floor(object, tx, ty);
            if (d < r)
            {
                c = d;
                gUnk_03002CB0.collisionProbeFlags |= 2;
                gUnk_03002CB0.objectFloorContact = 1;
            }
        }
    }

    if (gEwramData->playerState.collisionQueueCount != 0)
    {
        struct EwramData_EntityData *player = gEwramData->playerState.player;

        if (player->velY.whole >= 0 && (!(player->ext.player.actionFlags & 2) || player->ext.player.airTime == 0x10))
            c = PlatformFloor2(x, y, c);
    }
    UpdateFloorKind(c);
    return c;
}

#define B53E(e) (*((u8 *)(e) + 0x5A))

static inline void SetAnim2(struct EwramData_EntityData *entity, u8 anim, s32 flag)
{
    struct PlayerBodyBox box = sSomaStandingBodyBox[0];
    s32 pal;

    if (!(gEwramData->playerState.actionLockFlags & 0x800) && entity->animScriptIndex != anim)
    {
        Anim_StartIndexed(entity, anim, 3, flag);
        HitBox_SetBody(entity, &box);
        entity->ext.player.requestedAnim = 0xFF;
        entity->ext.player.actionFlags &= ~0x200000;
        pal = sSomaAnimPalettes[anim];
        if (entity->ext.player.animPalette != pal)
        {
            entity->ext.player.animPalette = pal;
            ObjPalette_LoadRange(sSomaPalette, pal, 1, 0);
        }
    }
}

/**
 * @brief Player state 9 entered by Skill_Devil_Manticore_Curly_Use
 * (soulStateFlags bit 0x1000): charge animation with sparks and flashing,
 * per-form animations from sUnk_084F10A0, accelerating dash to
 * +-0x60000, ceiling checks that stop blue souls, then deceleration.
 */
void Soma_UpdateTransformationDash(struct EwramData_EntityData *entity)
{
    u8 *anims;

    {
        u8 **tbl = sUnk_084F10A0;
        anims = tbl[entity->ext.player.transformForm];
    }

    if (entity->subState <= 3 && !(gEwramData->playerStatus.soulStateFlags & 0x1000))
        entity->subState = 4;

    switch (entity->subState)
    {
    case 0:
        entity->stage = 0;
        entity->velX.whole = 0;
        entity->ext.player.requestedAnim = 0xFF;
        SetAnim2(entity, 0x20, 1);
        entity->timer = 0x2D;
        entity->subState++;
        FLAGS(entity) &= ~0x400;
        break;
    case 1:
    {
        u8 c = entity->timer;

        if (entity->timer != 0)
        {
            s32 t;

            if (entity->timer <= 0x1D && !(t = c & 3))
            {
                s32 px = entity->posX.whole + (RandomSigned(8) << 16);
                s32 py = entity->posY.whole - (((RandomNumberGenerator() & 7) + 8) << 16);

                Effect_CreateFromTable(px, py, 0, ((RandomNumberGenerator() & 0x1F) << 4) - 0x3000, t, 6);
            }
            if (entity->timer <= 0xF)
            {
                if (entity->timer & ({ s32 k = 2; k; }))
                    entity->palette = 4;
                else
                    entity->palette = 0;
            }
            if (entity->timer == 0x23)
                PlaySong(0x1AB);
            entity->timer--;
            break;
        }
        if ((u16)entity->ext.player.requestedAnim == 0xFF)
        {
            SetAnim2(entity, anims[0], 0);
            entity->stage = 1;
            entity->subState++;
        }
    }
    case 2:
        if (FLAGS(entity) & 0x200000)
        {
            entity->subState = 3;
            SetAnim2(entity, anims[1], 1);
        }
    case 3:
    {
        u16 x, y;

        {
            u8 two = 2;
            gEwramData->playerState.player->ext.player.invulnerableTimer = two;
        }
        entity->ext.player.drawOffsetY = 0xF0;
        if (entity->subState == 3)
        {
            s32 speed;

            entity->timer++;
            if (entity->flipX)
            {
                entity->velX.whole += -0x1000;
                if (entity->velX.whole < -0x60000)
                    entity->velX.whole = -0x60000;
            }
            else
            {
                entity->velX.whole += 0x1000;
                if (entity->velX.whole > 0x60000)
                    entity->velX.whole = 0x60000;
            }
            speed = entity->velX.whole;
            if (speed < 0)
                speed = -speed;
            if ((FLAGS(entity) & 0x1400000) == 0x1000000 && speed > 0xC000 && !(entity->timer & 3))
                Player_SpawnSurfaceEffect(1);
            entity->ext.player.wallProbes = (u32)sSomaWallProbesDash;
            Soma_UpdateVerticalMovement(entity);
        }
        x = GetEntityRoomXPositionInteger(entity);
        y = GetEntityRoomYPositionInteger(entity);
        if ((s16)CeilingCorrection(X, Y - 0x37) != 0
            || (s16)CeilingCorrection(({ s32 t_ = X; t_; }) + 8, ({ s32 t_ = Y; t_; }) - 0x32) != 0
            || (s16)CeilingCorrection(({ s32 t_ = X; t_; }) - 8, ({ s32 t_ = Y; t_; }) - 0x32) != 0)
            BlueSoul_StopAll();
        break;
    }
    case 4:
        FLAGS(entity) &= ~0x200000;
        entity->timer = 0;
        if (entity->stage)
            SetAnim2(entity, anims[2], 0);
        else
            FLAGS(entity) |= 0x200000;
        entity->subState++;
    case 5:
    {
        s32 old, new;

        entity->accelX.whole = (entity->velX.whole < 0) ? 0x6000 : -0x6000;
        old = entity->velX.whole;
        new = old + entity->accelX.whole;
        entity->velX.whole = new;
        if (old < 0)
            goto check_x;
        if (new < 0)
            goto clear_x;
        if (old > 0)
            goto done_x;
    check_x:
        if (new <= 0)
            goto done_x;
    clear_x:
        entity->velX.whole = 0;
        entity->accelX.whole = 0;
    done_x:
        if (!(FLAGS(entity) & 0x200000))
        {
            {
                u8 two = 2;
                gEwramData->playerState.player->ext.player.invulnerableTimer = two;
            }
            gEwramData->playerState.actionLockFlags |= 6;
            entity->timer++;
            entity->palette = 4;
            entity->ext.player.drawOffsetY = 0xF0;
            break;
        }
        if (entity->timer)
            entity->ext.player.drawOffsetY = 0xF0;
        entity->velX.whole = 0;
        entity->velY.whole = 0;
        entity->accelX.whole = 0;
        entity->accelY.whole = 0;
        FLAGS(entity) &= 0xEFFFFE9F;
        entity->palette = 0;
        entity->state = 0;
        break;
    }
    }
}

/* Soma_StartDeath (same as
 * Julius_StartDeath with animation 0x33). */

/**
 * @brief Enter death state 16 from the main update when HP is nonpositive:
 * set the scene flags, animation 0x33, stop blue souls and apply
 * the existing recoil velocity, hit flags and sound.
 */
void Soma_StartDeath(struct EwramData_EntityData *entity)
{
    gEwramData->gameState.subMode = 2;
    gEwramData->gameState.subModeStep = 0;
    SetPlayerAnim(entity, 0x33);
    entity->hit.flags |= 4;
    BlueSoul_StopAll();
    entity->ext.player.actionFlags &= 0xEFFFFE9F;
    gEwramData->playerState.actionLockFlags |= 0xE;
    gEwramData->playerState.pendingHitDamage = 0;
    if (!entity->flipX)
        entity->velX.whole = -0x10000;
    else
        entity->velX.whole = 0x10000;
    entity->velY.whole = -0x18000;
    entity->accelY.whole = 0x2000;
    entity->state = 0x10;
    {
        u8 two = 2;
        gEwramData->playerState.player->ext.player.invulnerableTimer = two;
    }
    PlaySong(0x197);
}

/* Player_UpdateDeath; sign-change
 * stop as in Julius_UpdateAbilityDash. */

/**
 * @brief Shared Soma/Julius death state 16: decelerate X/Y without
 * crossing zero and, while visible, emit randomized shared particles
 * every fourth update. Preserve visibility-dependent timer advance.
 */
void Player_UpdateDeath(struct EwramData_EntityData *entity)
{
    s32 old, new, x, accel;

    old = entity->velX.whole;
    accel = (old < 0) ? 0x200 : -0x200;
    entity->accelX.whole = accel;
    new = old + accel;
    entity->velX.whole = new;
    if (old < 0)
        goto check_pos_x;
    if (new < 0)
        goto clear_x;
    if (old > 0)
        goto done_x;
check_pos_x:
    if (new <= 0)
        goto done_x;
clear_x:
    entity->velX.whole = 0;
    entity->accelX.whole = 0;
done_x:
    old = entity->velY.whole;
    new = old + entity->accelY.whole;
    entity->velY.whole = new;
    if (old < 0)
        goto check_pos_y;
    if (new < 0)
        goto clear_y;
    if (old > 0)
        goto done_y;
check_pos_y:
    if (new <= 0)
        goto done_y;
clear_y:
    entity->velY.whole = 0;
    entity->accelY.whole = 0;
done_y:
    if (!entity->hidden)
    {
        s32 timer = entity->timer + 1;
        entity->timer = timer;
        if (timer & 3)
            return;
        x = entity->posX.whole + RandomSigned(0x80000);
        Effect_SpawnShared(x, entity->posY.whole + RandomSigned(0x80000) - 0x100000, 0xD, 8);
    }
}

/* Player (Soma) main update: damage reactions, collision, state machine. */
extern struct EwramData_EntityData *Effect_CreatePlayerFlash(s32 kind);

#define HELD (gEwramData->inputData.playerHeldInput)
#define NEW (gEwramData->inputData.playerNewInput)
#define REQ(e) (*(u16 *)&e->ext.player.requestedAnim)

static inline void SetPlayerAnimBoxFlags(struct EwramData_EntityData *entity, u8 anim, struct PlayerBodyBox *table, s32 flag, u32 *playerFlags)
{
    struct PlayerBodyBox box = table[0];
    s32 pal;

    if (!(*playerFlags & 0x800) && entity->animScriptIndex != anim)
    {
        Anim_StartIndexed(entity, anim, 3, flag);
        HitBox_SetBody(entity, &box);
        entity->ext.player.requestedAnim = 0xFF;
        entity->ext.player.actionFlags &= ~0x200000;
        pal = sSomaAnimPalettes[anim];
        if (entity->ext.player.animPalette != pal)
        {
            entity->ext.player.animPalette = pal;
            ObjPalette_LoadRange(sSomaPalette, pal, 1, 0);
        }
    }
}

static inline u8 GetHitTimer(struct EwramData_EntityData *entity, u8 id)
{
    struct EntityHit *s = &entity->hit;
    s32 i;

    for (i = 0; i < 3; i++)
    {
        if (s->hitBy[i] == id)
            return s->hitTimer[i];
    }
    return 0;
}

static inline void SetHitTimer(struct EwramData_EntityData *entity, u8 id, u8 timer)
{
    struct EntityHit *s = &entity->hit;
    s32 i;

    for (i = 0; i < 3; i++)
    {
        if (s->hitBy[i] == id)
        {
            s->hitTimer[i] = timer;
            return;
        }
    }
    for (i = 0; i < 3; i++)
    {
        if (s->hitBy[i] == 0)
        {
            s->hitBy[i] = id;
            s->hitTimer[i] = timer;
            return;
        }
    }
}

static inline void SpikeDamage(struct EwramData_EntityData *entity, u16 kind)
{
    struct PlayerContactDamage hit;

    if (entity->ext.player.invulnerableTimer == 0 && GetHitTimer(entity, 8) == 0)
    {
        SetHitTimer(entity, 8, 60);
        {
            s32 dmg = (gEwramData->playerState.equipmentDefenseBonus >> 1) + (gEwramData->playerState.equipmentDefenseBonus >> 2)
                + (gEwramData->playerStatus.maxHP >> 4);

            hit.attack = dmg;
        }
        hit.attributes = 1;
        hit.unused = 0;
        hit.reaction = kind != 0;
        hit.sourceX = entity->posX.whole;
        hit.sourceY = entity->posY.whole;
        Player_ApplyContactDamage(&hit);
    }
}

static inline void ApplyAccelX(struct EwramData_EntityData *entity)
{
    s32 old = entity->velX.whole;
    s32 new = old + entity->accelX.whole;

    entity->velX.whole = new;
    if ((old >= 0 && new < 0) || (old <= 0 && new > 0))
    {
        entity->velX.whole = 0;
        entity->accelX.whole = 0;
    }
}

static inline void SlopeSlowdown(struct EwramData_EntityData *entity, s32 d1, s32 d2, s32 d3)
{
    switch ((u8)entity->ext.player.slopeLevel)
    {
        case 1:
            entity->velX.whole = entity->velX.whole / d1 * 16;
            break;
        case 2:
            entity->velX.whole = entity->velX.whole / d2 * 16;
            break;
        case 3:
            entity->velX.whole = entity->velX.whole / d3 * 16;
            break;
    }
}

static inline void SetPlayerBoxIfFree(struct EwramData_EntityData *entity, u8 value, struct PlayerBodyBox *table)
{
    struct PlayerBodyBox box = table[0];

    if (REQ(entity) == 0xFF)
    {
        entity->ext.player.requestedAnim = value;
        HitBox_SetBody(entity, &box);
    }
}

static inline void Decel(struct EwramData_EntityData *entity, s32 k)
{
    s32 old = entity->velX.whole;
    s32 accel = -k;
    s32 new;

    if (old < 0)
        accel = k;
    entity->accelX.whole = accel;
    new = old + accel;
    entity->velX.whole = new;
    if ((old >= 0 && new < 0) || (old <= 0 && new > 0))
    {
        entity->velX.whole = 0;
        entity->accelX.whole = 0;
    }
}

static inline void AirControl(struct EwramData_EntityData *entity, u32 flags, s32 max)
{
    if (flags & 2)
    {
        u32 left = HELD & 0x20;

        if (left)
        {
            entity->velX.whole = -max;
            entity->accelX.whole = 0;
            return;
        }
        if (HELD & 0x10)
        {
            entity->velX.whole = max;
            entity->accelX.whole = left;
            return;
        }
    }
    entity->accelX.whole = (entity->velX.whole < 0) ? 0x4000 : -0x4000;
}

static inline void GroundControl(struct EwramData_EntityData *entity, s32 max)
{
    u32 left = HELD & 0x20;

    if (left)
    {
        entity->flipX = 1;
        entity->velX.whole = -max;
        entity->accelX.whole = 0;
        return;
    }
    if (HELD & 0x10)
    {
        entity->flipX = 0;
        entity->velX.whole = max;
        entity->accelX.whole = left;
        return;
    }
    entity->accelX.whole = (entity->velX.whole < 0) ? 0x4000 : -0x4000;
}

static inline s32 TryBackdash(struct EwramData_EntityData *entity)
{
    if ((NEW & gEwramData->playerStatus.abilityButton) && !(HELD & 0x40) && Player_HasAbility(0)
        && !(FLAGS(entity) & 0x10008402))
    {
        /* Keep the zero initialization before the box setup in the ROM order. */
        register u8 zero asm("r4") = 0; /* FAKEMATCH: pinned register */
        SetPlayerBox(entity, 0x15, sSomaStandingBodyBox);
        FLAGS(entity) = (FLAGS(entity) & ~0x200000) | 0x10000000;
        entity->timer = zero;
        if (!entity->flipX)
            entity->velX.whole = -0x3C000;
        else
            entity->velX.whole = 0x3C000;
        Afterimage_Create(entity, 0, 8, 0x1D);
        Afterimage_Create(entity, 0, 6, 0x1C);
        PlaySong(0xA9);
        return 1;
    }
    return 0;
}

static inline void TrySlide(struct EwramData_EntityData *entity)
{
    if (Player_HasAbility(1) && !(FLAGS(entity) & 0x1122) && (HELD & 0x80)
        && (NEW & gEwramData->playerStatus.jumpButton))
    {
        entity->state = 3;
        entity->subState = 0;
        entity->ext.player.requestedAnim = 0xFF;
        SetPlayerAnimBox(entity, 0x1F, &sSomaCrouchBodyBoxes[1], 0);
        if (!entity->flipX)
            entity->velX.whole = 0x32000;
        else
            entity->velX.whole = -0x32000;
        entity->accelX.whole = (entity->velX.whole < 0) ? 0x1800 : -0x1800;
        entity->timer = 0;
        FLAGS(entity) = (FLAGS(entity) | 0x20000420) & ~0x10000000;
        HitBox_SetAttack(entity, &sSomaCrouchBodyBoxes[3]);
        entity->hit.type = 4;
        PlaySong(0xBD);
    }
}

#define DUST_SIDE (!gEwramData->playerState.player->flipX)

/* These inline boundaries keep address temporaries separate during allocation.
 * A matching reconstruction does not establish the original helper layout. */
static inline u16 GetDamageType(struct EwramData *data)
{
    return data->playerState.hitReaction;
}

/* Preserve both frame addresses across animation/palette calls. The register
 * constraints and lock-mask destination reproduce agbcc allocation. */
static inline void RefreshPlayerAnim(struct EwramData_EntityData *entity)
{
    register u8 *framePtr asm("r5") = &entity->animStep; /* FAKEMATCH: pinned register */
    u8 frame = *framePtr;
    register u8 *timerPtr asm("r4") = &entity->animTimer; /* FAKEMATCH: pinned register */
    u8 timer = *timerPtr;
    u8 anim = Weapon_GetPlayerAttackAnim(0);

    struct PlayerBodyBox box = sSomaStandingBodyBox[0];
    s32 pal;
    register u32 lock asm("r1") = gEwramData->playerState.actionLockFlags; /* FAKEMATCH: pinned register */
    lock &= 0x800;

    if (!lock && entity->animScriptIndex != anim)
    {
        Anim_StartIndexed(entity, anim, 3, 0);
        HitBox_SetBody(entity, &box);
        entity->ext.player.requestedAnim = 0xFF;
        entity->ext.player.actionFlags &= ~0x200000;
        pal = sSomaAnimPalettes[anim];
        if (entity->ext.player.animPalette != pal)
        {
            entity->ext.player.animPalette = pal;
            ObjPalette_LoadRange(sSomaPalette, pal, 1, 0);
        }
        *framePtr = frame;
        *timerPtr = timer;
    }

}
static inline s32 GetHitX(struct EwramData *data)
{
    return data->playerState.hitSourceX;
}

static inline struct PlayerBodyBox *GetSlideBox(void)
{
    return &sSomaCrouchBodyBoxes[1];
}

/**
 * @brief Character-0 main entity update: process damage reactions,
 * world collision, input/state dispatch, animation and tracked hit
 * state. Entry 0 of sPlayerUpdateFuncs; retain all compiler constraints.
 */
void Soma_Update(struct EwramData_EntityData *entity)
{
    register struct EwramData *hurtData;
    u32 *hurtFlags;
    s32 maxAir;
    s32 maxSpeed;
    s32 speed;
    s32 count;
    u16 x, y;
    /* HI mode retains the temporary -1 and copy shared with the Julius twin. */
    register s16 none asm("r5"); /* FAKEMATCH: pinned register */
    u8 kind;
    u16 left;

    maxAir = 0x18000;
    if (gEwramData->playerState.standingPlatform != NULL)
        gEwramData->playerState.standingPlatform = NULL;
    FLAGS(entity) &= ~0x240000;
    if (entity->animFlags & 4)
    {
        FLAGS(entity) |= 0x200000;
        FLAGS(entity) &= ~0x10000000;
    }
    entity->hit.flags |= 2;
    Player_PruneTrackedEntities();
    entity->ext.player.drawOffsetY = 0;
    entity->ext.player.drawOffsetX = 0;
    entity->posX.whole += entity->velX.whole + PLAYER_CONVEYOR_VELOCITY(entity);
    if (entity->velY.whole > 0x80000)
        entity->velY.whole = 0x80000;
    entity->posY.whole += entity->velY.whole;
    count = 2;
    PLAYER_CONVEYOR_VELOCITY(entity) = 0;
    if (entity->ext.player.floorKind == 0xFF)
    {
        x = GetEntityRoomXPositionInteger(entity);
        y = GetEntityRoomYPositionInteger(entity) - 4;
        if ((FLAGS(entity) & 0x400) || (gEwramData->playerStatus.soulStateFlags & 0x200))
            count = 1;
        if (count != 0)
        {
        probe:
            {
                s32 tx = x << 16;
                s32 ty = (s16)y;
                u8 d = Collision_GetTile(tx >> 16, ty);

                if (SlopeKind(d) == 0 && !(d & 0xC0) && (d & 4))
                    kind = (d & 0x30) >> 4;
                else
                    kind = 0xFF;
                entity->ext.player.floorKind = kind;
                if (kind == 0xFF)
                {
                    y = (u32)((y << 16) - 0x180000) >> 16;
                    count--;
                    if (count != 0)
                        goto probe;
                }
            }
        }
    }
    switch (entity->ext.player.floorKind)
    {
        case 0:
            SpikeDamage(entity, 1);
            break;
        case 1:
            SpikeDamage(entity, 0);
            break;
        case 2:
            PLAYER_CONVEYOR_VELOCITY(entity) = gEwramData->gameState.conveyorSpeed;
            break;
        case 3:
            PLAYER_CONVEYOR_VELOCITY(entity) = -gEwramData->gameState.conveyorSpeed;
            break;
    }
    none = -1;
    entity->ext.player.floorKind = 0xFF;
    if (gEwramData->playerState.pendingHitDamage == 0)
        goto collide;
    if (gEwramData->playerStatus.currentHP <= 0)
    {
        gEwramData->playerState.actionLockFlags &= ~0x820;
        Soma_StartDeath(entity);
        goto collide;
    }
    if (gEwramData->playerStatus.statusEffects & 4)
    {
        if (entity->state == 8)
            Soma_ResolveBatExitCollision(entity);
        entity->palette = 0;
        entity->state = 0xF;
        FLAGS(entity) |= 0x80;
        if (FLAGS(entity) & 2)
        {
            s32 accel;

            if ((s32)gEwramData->playerState.hitSourceX > entity->posX.whole)
            {
                entity->velX.whole = -0x18000;
                accel = 0x200;
            }
            else
            {
                entity->velX.whole = 0x18000;
                accel = -0x200;
            }
            entity->accelX.whole = accel;
            entity->velY.whole = -0x20000;
            entity->accelY.whole = -0x1000;
            FLAGS(entity) |= 2;
        }
        goto knocked;
    }
    if (gEwramData->playerState.hitReaction == 3)
    {
        struct EwramData *ewram;
        struct EwramData_EntityData *player;

        if (entity->state == 8)
            Soma_ResolveBatExitCollision(entity);
        ewram = gEwramData;
        ewram->playerState.actionLockFlags &= ~0x820;
        ewram->playerState.player->velX.whole = ewram->playerState.knockbackVelocityX;
        ewram->playerState.player->velY.whole = ewram->playerState.knockbackVelocityY;
        player = ewram->playerState.player;
        player->accelX.whole = (player->velX.whole < 0) ? 0x1000 : -0x1000;
        {
            u8 state = 0xE;
            FLAGS(entity) |= 0x82;
            entity->state = state;
        }
        FLAGS(entity) &= ~0x200000;
        PlaySong(0x196);
        entity->palette = 0;
        goto collide;
    }
    if (gEwramData->playerState.hitReaction == 2)
        goto collide;
    if ((gEwramData->playerStatus.soulStateFlags & 0x20200) == 0x20000)
    {
        SetHitTimer(entity, 8, 30);
        goto collide;
    }
    if (entity->state == 8)
        Soma_ResolveBatExitCollision(entity);
    entity->palette = 0;
    hurtData = gEwramData;
    hurtFlags = &hurtData->playerState.actionLockFlags;
    hurtData->playerState.actionLockFlags &= ~0x820;
    if (GetDamageType(hurtData) == 0 && !(FLAGS(entity) & 2))
    {
        none |= entity->animScriptIndex;
        entity->animScriptIndex = none;
        if (!(FLAGS(entity) & 0x400))
        {
            if ((!entity->flipX && GetHitX(hurtData) > entity->posX.whole)
                || (entity->flipX && GetHitX(hurtData) < entity->posX.whole))
                SetPlayerAnimBox(entity, 0xE, sSomaStandingBodyBox, 0);
            else
                SetPlayerAnimBox(entity, 0x10, sSomaStandingBodyBox, 0);
        }
        else
        {
            SetPlayerAnimBoxFlags(entity, 0xF, sSomaCrouchBodyBoxes, 0, hurtFlags);
        }
        SetHitTimer(entity, 8, 50);
        FLAGS(entity) |= 0x80;
        entity->state = 0xC;
        PlaySong(0x19B);
    }
    else
    {
        s32 accel;

        if ((s32)gEwramData->playerState.hitSourceX > entity->posX.whole)
        {
            if (!entity->flipX)
                SetPlayerAnimBox(entity, 0x11, sSomaStandingBodyBox, 0);
            else
                SetPlayerAnimBox(entity, 0x10, sSomaStandingBodyBox, 0);
            entity->velX.whole = -0x18000;
            accel = 0x200;
        }
        else
        {
            if (entity->flipX)
                SetPlayerAnimBox(entity, 0x11, sSomaStandingBodyBox, 0);
            else
                SetPlayerAnimBox(entity, 0x10, sSomaStandingBodyBox, 0);
            entity->velX.whole = 0x18000;
            accel = -0x200;
        }
        entity->accelX.whole = accel;
        entity->velY.whole = -0x20000;
        entity->accelY.whole = -0x1000;
        FLAGS(entity) |= 0x82;
        entity->state = 0xD;
        PlaySong(0x195);
    }
knocked:
    FLAGS(entity) &= ~0x200000;
collide:
    if (!(gEwramData->playerState.actionLockFlags & 1))
        Soma_ResolveWorldCollision(entity);
    gEwramData->playerState.actionLockFlags &= ~1;
    gEwramData->playerState.cachedPlayerX = entity->posX.whole;
    gEwramData->playerState.cachedPlayerY = entity->posY.whole;

    switch (entity->state)
    {
        case 0:
            maxSpeed = 0x18000;
            speed = entity->velX.whole;
            if (speed < 0)
                speed = -speed;
            if (!(FLAGS(entity) & 0x10000002))
            {
                if (!(FLAGS(entity) & 0x1000000))
                {
                    if (speed > maxSpeed)
                    {
                        if (!(HELD & 0x30) || (HELD & 0x80))
                        {
                            if (!(entity->timer & 3))
                                Player_SpawnEffectSet(2, !gEwramData->playerState.player->flipX);
                        }
                        else if ((HELD & 0x20) && entity->velX.whole > maxSpeed)
                            Player_SpawnEffectSet(2, !gEwramData->playerState.player->flipX);
                        else if ((HELD & 0x10) && entity->velX.whole < -0x18000)
                            Player_SpawnEffectSet(2, !gEwramData->playerState.player->flipX);
                    }
                }
                else if (!(FLAGS(entity) & 0x400000))
                {
                    if (speed > maxSpeed)
                    {
                        if (!(entity->timer & 3))
                            Player_SpawnSurfaceEffect(1);
                    }
                    else if (speed > 0xC000)
                    {
                        if (!(entity->timer & 7))
                            Player_SpawnSurfaceEffect(1);
                    }
                }
            }
            if (gEwramData->playerStatus.soulStateFlags & 0x400)
                maxSpeed = 0x40000;
            TryBackdash(entity);
            if (!(FLAGS(entity) & 0x10008400))
            {
                if (((HELD & 0x10) && entity->flipX) || ((HELD & 0x20) && !entity->flipX))
                {
                    if (!(FLAGS(entity) & 2))
                        SetPlayerBox(entity, 0x18, sSomaStandingBodyBox);
                    else if ((FLAGS(entity) & 0x800010) == 0x800000)
                        SetPlayerBox(entity, 0x29, sSomaStandingBodyBox);
                }
                GroundControl(entity, maxSpeed);
                if (!(FLAGS(entity) & 2))
                {
                    if ((entity->velX.whole > 0 && (FLAGS(entity) & 0x4000))
                        || (entity->velX.whole < 0 && (FLAGS(entity) & 0x2000)))
                        SlopeSlowdown(entity, 0x18, 0x14, 0x12);
                }
            }
            else if (!(FLAGS(entity) & 0x10000000))
            {
                if (HELD & 0x20)
                    entity->flipX = 1;
                else if (HELD & 0x10)
                    entity->flipX = 0;
                entity->accelX.whole = (entity->velX.whole < 0) ? 0x4000 : -0x4000;
            }
            else
            {
                if (!(FLAGS(entity) & 2) && speed > 0x18000)
                {
                    if (!(entity->timer & 3))
                        Player_SpawnEffectSet(2, !gEwramData->playerState.player->flipX);
                    if (!(FLAGS(entity) & 2))
                    {
                        if ((entity->velX.whole > 0 && (FLAGS(entity) & 0x4000))
                            || (entity->velX.whole < 0 && (FLAGS(entity) & 0x2000)))
                            SlopeSlowdown(entity, 0x17, 0x13, 0x11);
                    }
                }
                entity->accelX.whole = (entity->velX.whole < 0) ? 0x2800 : -0x2800;
            }
            ApplyAccelX(entity);
            if (FLAGS(entity) & 0x100000)
            {
                entity->timer++;
                if (!(FLAGS(entity) & 0x1000001E))
                {
                    if ((HELD & 0x80) || (FLAGS(entity) & 0x8000) || (gEwramData->playerStatus.inputOverrides & 1))
                    {
                        if (!(FLAGS(entity) & 0x400))
                        {
                            FLAGS(entity) = (FLAGS(entity) | 0x400) & ~0x10000000;
                            SetPlayerBox(entity, 9, sSomaCrouchBodyBoxes);
                        }
                        if (REQ(entity) == 0xFF)
                            SetPlayerAnimBox(entity, 2, sSomaCrouchBodyBoxes, 1);
                    }
                    else if (HELD & 0x30)
                    {
                        if (!(FLAGS(entity) & 0x10000000) && REQ(entity) != 0x18)
                        {
                            if (!(gEwramData->playerStatus.soulStateFlags & 0x400))
                            {

                                if (entity->animScriptIndex != 1 && entity->animScriptIndex != 0x1A && entity->animScriptIndex != 0x18)
                                    SetPlayerAnimBox(entity, 0x1A, sSomaStandingBodyBox, 0);
                                if (FLAGS(entity) & 0x200000)
                                    SetPlayerAnimBox(entity, 1, sSomaStandingBodyBox, 1);
                            }
                            else
                                SetPlayerAnimBox(entity, 3, sSomaStandingBodyBox, 1);
                        }
                    }
                    else
                    {
                        if (!(FLAGS(entity) & 0x400) && (HELD & 0x40))
                        {
                            if (entity->ext.player.upHoldTimer < 4)
                                entity->ext.player.upHoldTimer++;
                            else
                            {
                                SetPlayerAnimBox(entity, 0x17, sSomaStandingBodyBox, 0);
                                goto ground_actions;
                            }
                        }
                        else
                            entity->ext.player.upHoldTimer = 0;
                        if (!(FLAGS(entity) & 0x10000000))
                        {
                            u8 anim = entity->animScriptIndex;

                            if (anim != 0x1A && anim != 0 && anim != 0x18 && speed > 0xFFFF)
                                SetPlayerBox(entity, 0x19, sSomaStandingBodyBox);
                        }
                        if (REQ(entity) == 0xFF || entity->animScriptIndex == 0x1A)
                        {
                            FLAGS(entity) &= ~0x10000000;
                            SetPlayerAnimBox(entity, 0, sSomaStandingBodyBox, 1);
                        }
                    }
                    if ((FLAGS(entity) & 0x8400) == 0x400 && !(gEwramData->playerStatus.inputOverrides & 1)
                        && !(HELD & 0x80))
                    {
                        FLAGS(entity) &= ~0x400;
                        SetPlayerBox(entity, 0xA, sSomaStandingBodyBox);
                    }
                }
            }
        ground_actions:
            if ((HELD & 0x40) && (NEW & gEwramData->playerStatus.attackButton) && !(FLAGS(entity) & 0x540))
                Soma_CastEquippedRedSoul(entity, 0);
            Soma_HandleWeaponAttackInput(entity);
            TrySlide(entity);
            Soma_HandleJumpInput(entity);
            Soma_UpdateJumpControl(entity);
            Soma_HandleBlueSoulInput(entity);
            break;
        case 1:
            if (gEwramData->playerStatus.soulStateFlags & 0x400)
                maxAir = 0x40000;
            AirControl(entity, FLAGS(entity), maxAir);
            ApplyAccelX(entity);
            if ((gEwramData->playerStatus.soulStateFlags & 0x800000) && (HELD & 0x40)
                && (NEW & gEwramData->playerStatus.attackButton) && !(FLAGS(entity) & 0x540))
                Soma_CastEquippedRedSoul(entity, 0);
            if (Weapon_KeepsAttackOnLanding() && (FLAGS(entity) & 0x40))
            if (FLAGS(entity) & 0x100000)
            {
                RefreshPlayerAnim(entity);
                FLAGS(entity) &= ~0x40;
            }
            if (FLAGS(entity) & 0x200000)
            {
                u32 flags = FLAGS(entity) & ~0x200060;

                FLAGS(entity) = flags;
                if (!(flags & 2))
                {
                    if (!(flags & 0x400))
                        SetPlayerBoxIfFree(entity, Weapon_GetPlayerAttackAnim(3), sSomaStandingBodyBox);
                    else
                        SetPlayerBoxIfFree(entity, Weapon_GetPlayerAttackAnim(4), sSomaCrouchBodyBoxes);
                }
                entity->state = 0;
            }
            else if (TryBackdash(entity))
            {
                FLAGS(entity) &= ~0x200060;
                entity->state = 0;
            }
            Soma_UpdateJumpControl(entity);
            Soma_HandleBlueSoulInput(entity);
            break;
        case 2:
        {
            u32 flags;

            if (gEwramData->playerStatus.soulStateFlags & 0x400)
                maxAir = 0x40000;
            if (entity->subState && entity->animStep == 3)
            {
                PlaySong(0xEB);
                entity->subState = 0;
                if (!entity->flipX)
                    entity->velX.whole = 0x40000;
                else
                    entity->velX.whole = -0x40000;
            }
            flags = FLAGS(entity);
            AirControl(entity, FLAGS(entity), maxAir);
            ApplyAccelX(entity);
            if (FLAGS(entity) & 0x200000)
            {
                FLAGS(entity) &= ~0x200100;
                entity->state = 0;
            }
            Soma_UpdateJumpControl(entity);
            Soma_HandleBlueSoulInput(entity);
            break;
        }
        case 3:
        {
            /* Keep the short-lived source addresses separate from the saved
             * pointers. The timer uses r8; leave frame unconstrained. Binding
             * frame to r7 also changes allocation in earlier states. Both
             * expanded animation branches retain their own stack box. */
            register u8 *sourceFrame asm("r3") = &entity->animStep; /* FAKEMATCH: pinned register */
            u32 frame = *sourceFrame;
            u8 *sourceTimer = &entity->animTimer;
            u32 timer = *sourceTimer;
            u32 slope = FLAGS(entity) & 0x2000;
            register u8 *framePtr asm("r5") = sourceFrame; /* FAKEMATCH: pinned register */
            u8 *timerPtr = sourceTimer;
            s32 abs;
            u32 flags;
            if ((slope && (u8)entity->ext.player.slopeLevel == 1 && !entity->flipX) || ((FLAGS(entity) & 0x4000) && (u8)entity->ext.player.slopeLevel == 1 && entity->flipX))
            {
                struct PlayerBodyBox box = *GetSlideBox();
                s32 pal;

                if (!(gEwramData->playerState.actionLockFlags & 0x800) && entity->animScriptIndex != 0x2E)
                {
                    Anim_StartIndexed(entity, 0x2E, 3, 0);
                    HitBox_SetBody(entity, &box);
                    entity->ext.player.requestedAnim = 0xFF;
                    entity->ext.player.actionFlags &= ~0x200000;
                    pal = sSomaAnimPalettes[0x2E];
                    if (entity->ext.player.animPalette != pal)
                    {
                        entity->ext.player.animPalette = pal;
                        ObjPalette_LoadRange(sSomaPalette, pal, 1, 0);
                    }
                    *framePtr = frame;
                    *timerPtr = timer;
                }
            }
            else
            {
                struct PlayerBodyBox box = *GetSlideBox();
                s32 pal;

                if (!(gEwramData->playerState.actionLockFlags & 0x800) && entity->animScriptIndex != 0x1F)
                {
                    Anim_StartIndexed(entity, 0x1F, 3, 0);
                    HitBox_SetBody(entity, &box);
                    entity->ext.player.requestedAnim = 0xFF;
                    entity->ext.player.actionFlags &= ~0x200000;
                    pal = sSomaAnimPalettes[0x1F];
                    if (entity->ext.player.animPalette != pal)
                    {
                        entity->ext.player.animPalette = pal;
                        ObjPalette_LoadRange(sSomaPalette, pal, 1, 0);
                    }
                    *framePtr = frame;
                    *timerPtr = timer;
                }
            }
            entity->hit.flags &= ~2;
            abs = entity->velX.whole;
            if (abs < 0)
                abs = -abs;
            if (abs > 0x18000 && !(entity->timer & 3))
                Player_SpawnEffectSet(2, DUST_SIDE);
            ApplyAccelX(entity);
            flags = FLAGS(entity);
            if (flags & 2)
                entity->timer = 0x20;
            if (++entity->timer > 0x1F)
            {
                FLAGS(entity) &= ~0x20;
                entity->state = 0;
            }
            Soma_UpdateVerticalMovement(entity);
            Soma_HandleBlueSoulInput(entity);
            break;
        }
        case 4:
        {
            u32 flags;
            u8 t;

            Decel(entity, 0x3000);
            t = entity->timer;
            if (t <= 7)
            {
                if ((FLAGS(entity) & 0x80) && t == 6)
                {
                    s32 abs = entity->velX.whole;

                    if (abs < 0)
                        abs = -abs;
                    if (abs > 0x2000 && (RandomNumberGenerator() & 3))
                        PlaySong(0x199);
                }
                if (!(entity->timer & 3))
                    Player_SpawnEffectSet(0, DUST_SIDE);
                entity->timer++;
            }
            flags = FLAGS(entity);
            if (flags & 0x10000)
            {
                u32 f2 = flags | 0x400;

                FLAGS(entity) = f2;
                if ((f2 & 0x300000) != 0x100000)
                {
                    FLAGS(entity) = f2 & ~0x10080;
                    entity->state = 0;
                }
                else if (f2 & 0x80)
                {
                    Soma_HandleWeaponAttackInput(entity);
                    TrySlide(entity);
                    Soma_HandleJumpInput(entity);
                    Soma_UpdateJumpControl(entity);
                    if (FLAGS(entity) & 2)
                        entity->state = 0;
                    if (entity->state != 4)
                        FLAGS(entity) &= ~0x10080;
                }
            }
            Soma_HandleBlueSoulInput(entity);
            break;
        }
        case 5:
            if (entity->velY.whole > 0)
                entity->state = 0;
            Soma_UpdateJumpControl(entity);
            Soma_HandleBlueSoulInput(entity);
            break;
        case 6:
            if (FLAGS(entity) & 0x200000)
                entity->state = 0;
            break;
        case 7:
            Soma_UpdateDiveAttack(entity);
            Soma_UpdateVerticalMovement(entity);
            break;
        case 12:
        {
            u32 flags;

            FLAGS(entity) &= 0xEFFFFE9F;
            Decel(entity, 0x8000);
            flags = FLAGS(entity);
            if (flags & 0x200000)
            {
                FLAGS(entity) = flags & ~0x80;
                entity->state = 0;
            }
            Soma_UpdateVerticalMovement(entity);
            Soma_HandleBlueSoulInput(entity);
            break;
        }
        case 13:
            Soma_UpdateKnockback(entity);
            break;
        case 14:
            Soma_UpdateKnockbackFromImpulse(entity);
            break;
        case 15:
            Soma_UpdateStatusReaction(entity);
            break;
        case 8:
            Soma_UpdateBatForm(entity);
            break;
        case 9:
            Soma_UpdateTransformationDash(entity);
            break;
        case 10:
            if (gEwramData->playerState.activeBlueSoulEntities[0] == NULL)
            {
                entity->ext.player.requestedAnim = 0xFF;
                entity->state = 0;
            }
            break;
        case 16:
        {
            u8 four = 4;
            gEwramData->playerState.player->ext.player.invulnerableTimer = four;
        }
            Player_UpdateDeath(entity);
            break;
        case 17:
            FLAGS(gEwramData->playerState.player) &= 0x40000000;
            entity->state = 0;
            break;
    }

    if (entity->ext.player.invulnerableTimer != 0 && --entity->ext.player.invulnerableTimer == 0)
    {
        struct EntityHit *s = &entity->hit;

        s->hitBy[2] = 0;
        s->hitBy[1] = 0;
        s->hitBy[0] = 0;
        s->hitTimer[2] = 0;
        s->hitTimer[1] = 0;
        s->hitTimer[0] = 0;
    }
    if (gEwramData->gameState.stateFlags & 1)
    {
        u8 two = 2;
        gEwramData->playerState.player->ext.player.invulnerableTimer = two;
    }
    else if (!(entity->ext.player.frameCounter & 7) && gEwramData->playerStatus.equippedAccessory == 0x2A
        && gEwramData->startPauseEnabled && !(FLAGS(entity) & 0x40002) && (HELD & 0x30))
    {
        if (++gEwramData->playerStatus.currentExperience > 99999999)
            gEwramData->playerStatus.currentExperience = 99999999;
        if (Player_ApplyEarnedLevels(&gEwramData->playerStatus))
            Effect_CreatePlayerFlash(0);
    }
    if (PLAYER_BLINK_TIMER(entity) != 0)
    {
        PLAYER_BLINK_TIMER(entity)--;
        entity->drawSkip = 1;
        if (PLAYER_BLINK_TIMER(entity) == 0)
            entity->drawSkip = 0;
    }
    if (!(gEwramData->playerState.actionLockFlags & 2))
    {
        u32 ptr;

        if (entity->state == 3 || (FLAGS(entity) & 0x8000))
            ptr = (u32)(sUnk_080E12C8 + 0x20);
        else if (FLAGS(entity) & 0x800000)
            ptr = (u32)(sUnk_080E12C8 + 0x1C);
        else if (!(FLAGS(entity) & 2))
            ptr = (u32)(sUnk_080E12C8 + 0x14);
        else
        {
            if (entity->ext.player.onStairs != 0)
                gEwramData->playerState.wallProbeY0 = -12;
            else
            {
                u16 vx = (entity->velX.part16.integer < 0) ? -entity->velX.part16.integer : entity->velX.part16.integer;
                u16 vy = (entity->velY.part16.integer < 0) ? -entity->velY.part16.integer : entity->velY.part16.integer;

                if ((s16)vx <= 2 || entity->velY.part16.integer > 0)
                    gEwramData->playerState.wallProbeY0 = -(s16)vy / 2 - 4;
                else
                    gEwramData->playerState.wallProbeY0 = -12;
            }
            ptr = (u32)&gEwramData->playerState.wallProbeCount;
        }
        entity->ext.player.wallProbes = ptr;
    }
    if (!(gEwramData->playerState.actionLockFlags & 0x800) && REQ(entity) != 0xFF)
    {
        if (entity->animScriptIndex != REQ(entity))
        {
            Anim_StartIndexed(entity, REQ(entity), 3, 0);
            FLAGS(entity) &= ~0x200000;
        }
        else if (FLAGS(entity) & 0x200000)
            entity->ext.player.requestedAnim = 0xFF;
    }
    Anim_Update(entity);
    HitBox_UpdatePlayerRects();
    YellowSoul_CreateEquippedEntity(entity);
    Player_ApplyPreparedStats();
    Player_UpdateStatusAndRegen(entity);
    Player_RecalculateAttackDefense();
    gEwramData->playerState.collisionQueueCount = 0;
    if (gEwramData->playerState.actionLockFlags & 0x20)
    {
        entity->velX.whole = 0;
        entity->accelX.whole = 0;
    }
    if (gEwramData->playerStatus.inputOverrides != 0)
        gEwramData->playerStatus.inputOverrides = 0;
    if (gEwramData->playerStatus.soulStateFlags != 0 && (gEwramData->playerStatus.soulStateFlags & 0x10000)
        && (FLAGS(entity) & 0x80000))
        Player_RestoreHP(5);
    gEwramData->playerState.pendingHitDamage = 0;
    FLAGS(entity) &= ~0x4080000;
    gEwramData->playerState.actionLockFlags &= ~0x8FE;
    Camera_ScrollWithPriority(0, ((s16)entity->posX.part16.integer - gEwramData->playerState.cameraAnchorX) << 16,
        ((s16)entity->posY.part16.integer - gEwramData->playerState.cameraAnchorY) << 16);
    entity->ext.player.frameCounter++;
}

static inline struct EwramData_EntityData *ReadPlayer(struct EwramData *ew)
{
    register s32 offset asm("r1") = OFFSET_OF(struct EwramData, playerState.player); /* FAKEMATCH: pinned register */
    u8 *address = (u8 *)ew + offset;
    return *(struct EwramData_EntityData **)address;
}
/* Return a player-relative Y coordinate selected by the observed flags.
 * The getter's short address/offset bindings preserve the separate slot
 * loads and address calculations in the original instructions. They are
 * compiler constraints, not evidence for original source organization. */
/**
 * @brief Return the y position above the player head for damage numbers and
 * status icons (lower in the small and crouching states).
 */
s16 Player_GetOverheadY(void)
{
    struct EwramData *ew = gEwramData;
    s32 y;

    if (ew->playerStatus.soulStateFlags & 0x200)
    {
        y = (s16)ReadPlayer(ew)->posY.part16.integer - 0x10;
        goto end;
    }
    if ((ReadPlayer(ew))->ext.player.actionFlags & 0x400)
    {
        y = (s16)ReadPlayer(ew)->posY.part16.integer - 0x16;
        goto end;
    }
    y = (s16)ReadPlayer(ew)->posY.part16.integer - 0x22;
end:
    return y;
}

/* Player_StartScriptedAnimation: start a scripted player animation (Soma or other character) with velocity. */
/**
 * @brief Return one if the player is dead; otherwise select the character-dependent animation under existing guards and set scripted state, facing flags and velocity, returning zero.
 */
s32 Player_StartScriptedAnimation(u16 anim, u16 flags, s32 velX, s32 velY)
{
    u8 mode = 0;
    u16 left, right;

    if (gEwramData->playerStatus.currentHP <= 0)
        return 1;
    if (flags & 1)
        mode = 1;
    if (gEwramData->playerStatus.currentCharacter == 0)
    {
        u8 a = anim;
        struct EwramData_EntityData *player = gEwramData->playerState.player;
        struct PlayerBodyBox box = sSomaStandingBodyBox[0];
        s32 pal;

        if (!(gEwramData->playerState.actionLockFlags & 0x800) && player->animScriptIndex != a)
        {
            Anim_StartIndexed(player, a, 3, mode);
            HitBox_SetBody(player, &box);
            player->ext.player.requestedAnim = 0xFF;
            player->ext.player.actionFlags &= ~0x200000;
            pal = sSomaAnimPalettes[a];
            if (player->ext.player.animPalette != pal)
            {
                player->ext.player.animPalette = pal;
                ObjPalette_LoadRange(sSomaPalette, pal, 1, 0);
            }
        }
    }
    else
    {
        switch (anim)
        {
        case 0xE:
        case 0xF:
        case 0x10:
        case 0x11:
            anim = 0xC;
            break;
        case 0x20:
            anim = 0xB;
            break;
        default:
            anim = 0;
            break;
        }
        Player_TryStartAnimation(gEwramData->playerState.player, anim, mode);
    }
    if ((left = flags & 2))
        gEwramData->playerState.player->flipX = 1;
    right = flags & 4;
    if (right)
        gEwramData->playerState.player->flipX = 0;
    gEwramData->playerState.player->state = 0x11;
    gEwramData->playerState.player->ext.player.actionFlags |= 0x40000000;
    if (left)
        gEwramData->playerState.player->flipX = 1;
    else if (right)
        gEwramData->playerState.player->flipX = 0;
    gEwramData->playerState.player->velX.whole = velX;
    gEwramData->playerState.player->velY.whole = velY;
    return 0;
}
