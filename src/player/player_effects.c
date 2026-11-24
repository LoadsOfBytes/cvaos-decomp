#include "player_effects.h"
#include "core.h"
#include "bg_and_collision.h"
#include "player_core.h"
#include "graphics.h"
#include "entity_util.h"
#include "sound.h"
#include "gba.h"
#include "macros.h"

#include "constants/main.h"

#include "data/data_0E0334.h"
#include "data/data_0E3464.h"

#include "structs/agb_sram.h"
#include "structs/ewram.h"
#include "structs/main.h"
#include "entity_common.h"
#include "math_anim.h"
#include "obj_gfx.h"
#include "music.h"
#include "data/data_118B48.h"

/**
 * @brief Wait for the delay, then move with the velocity and delete itself
 * when the animation ends.
 */
void PlayerEffectSetPart_Update(struct EwramData_EntityData *entity)
{
    s32 animResult;
    s32 gfx;

    switch (entity->state)
    {
        case 0:
            gfx = ObjGfx_Find((u8 *)sCommonObjectGfx);
            Sprite_Attach(entity, (u8 *)sCommonObjectSpriteSheet, gfx, 6);
            entity->drawFunc = NULL;
            entity->spriteLayer = 0xD;
            Anim_StartIndexed(entity, (u8) entity->ext.playerEffectSet.anim, 0, 0);
            Anim_Update(entity);
            entity->scrollWithBg = 1;
            entity->state = 1;
            entity->drawSkip = 1;
            break;

        case 1:
            if (entity->timer == 0)
            {
                entity->state = 2;
                entity->drawFunc = Sprite_DrawEntity;
            }
            else
            {
                entity->timer -= 1;
            }
            break;

        case 2:
            entity->posX.whole += entity->velX.whole;
            entity->posY.whole += entity->velY.whole;
            animResult = Anim_Update(entity);
            if (animResult == 3)
            {
                entity->deleteRequested = 1;
                entity->state = 3;
            }
            return;
    }
}

struct PlayerEffectPartDefinition {
    u8 animationAndPlacementMode;
    u8 delay;
    s16 offsetX;
    s16 offsetY;
    s16 velocityX;
    s16 velocityY;
    u8 pad_A[0xC - 0xA];
};
struct PlayerEffectSetDefinition {
    const struct PlayerEffectPartDefinition *parts;
    s32 partCount;
};

const struct PlayerEffectPartDefinition sPlayerEffectParts0[] = {
    [0] = {
        .animationAndPlacementMode = 0x40,
        .delay = 0x0,
        .offsetX = -0x200,
        .offsetY = -0x300,
        .velocityX = -0x20,
        .velocityY = 0x0
    },
    [1] = {
        .animationAndPlacementMode = 0x40,
        .delay = 0x0,
        .offsetX = 0x100,
        .offsetY = -0x200,
        .velocityX = 0x20,
        .velocityY = 0x0
    }
};

const struct PlayerEffectPartDefinition sPlayerEffectParts1[] = {
    [0] = {
        .animationAndPlacementMode = 0x40,
        .delay = 0x0,
        .offsetX = -0x200,
        .offsetY = -0x200,
        .velocityX = -0x30,
        .velocityY = -0x8
    },
    [1] = {
        .animationAndPlacementMode = 0x40,
        .delay = 0x0,
        .offsetX = 0x280,
        .offsetY = -0x200,
        .velocityX = -0x20,
        .velocityY = -0x4
    }
};

const struct PlayerEffectPartDefinition sPlayerEffectParts2[] = {
    [0] = {
        .animationAndPlacementMode = 0x41,
        .delay = 0x0,
        .offsetX = -0x500,
        .offsetY = 0x0,
        .velocityX = -0x80,
        .velocityY = 0x0
    },
    [1] = {
        .animationAndPlacementMode = 0x41,
        .delay = 0x0,
        .offsetX = 0x500,
        .offsetY = 0x0,
        .velocityX = -0x80,
        .velocityY = 0x0
    }
};

const struct PlayerEffectPartDefinition sPlayerEffectParts3[] = {
    [0] = {
        .animationAndPlacementMode = 0x40,
        .delay = 0x0,
        .offsetX = -0x800,
        .offsetY = -0x800,
        .velocityX = -0x40,
        .velocityY = 0x0
    },
    [1] = {
        .animationAndPlacementMode = 0x40,
        .delay = 0x0,
        .offsetX = 0x800,
        .offsetY = -0x800,
        .velocityX = 0x40,
        .velocityY = 0x0
    },
    [2] = {
        .animationAndPlacementMode = 0x40,
        .delay = 0x1,
        .offsetX = -0xA00,
        .offsetY = -0x1000,
        .velocityX = -0x80,
        .velocityY = 0x0
    },
    [3] = {
        .animationAndPlacementMode = 0x40,
        .delay = 0x1,
        .offsetX = 0xA00,
        .offsetY = -0x1000,
        .velocityX = 0x80,
        .velocityY = 0x0
    },
    [4] = {
        .animationAndPlacementMode = 0x40,
        .delay = 0x0,
        .offsetX = -0x800,
        .offsetY = -0x1800,
        .velocityX = -0x80,
        .velocityY = 0x0
    },
    [5] = {
        .animationAndPlacementMode = 0x40,
        .delay = 0x0,
        .offsetX = 0x800,
        .offsetY = -0x1800,
        .velocityX = 0x80,
        .velocityY = 0x0
    }
};

const struct PlayerEffectSetDefinition sPlayerEffectSets[] = {
    [0] = {
        .parts = sPlayerEffectParts0,
        .partCount = ARRAY_SIZE(sPlayerEffectParts0)
    },
    [1] = {
        .parts = sPlayerEffectParts1,
        .partCount = ARRAY_SIZE(sPlayerEffectParts1)
    },
    [2] = {
        .parts = sPlayerEffectParts2,
        .partCount = ARRAY_SIZE(sPlayerEffectParts2)
    },
    [3] = {
        .parts = sPlayerEffectParts3,
        .partCount = ARRAY_SIZE(sPlayerEffectParts3)
    }
};

/**
 * @brief Spawn the effect sprites of set n from sPlayerEffectSets around the
 * player, mirrored by facing, each with its delay and velocity.
 */
void Player_SpawnEffectSet(s32 set, s32 facing)
{
    s32 remaining;
    s32 baseY;
    s32 dir;
    struct EwramData_EntityData *child;
    const struct PlayerEffectPartDefinition *parts;
    const struct PlayerEffectPartDefinition *part;
    s32 baseX;
    s32 baseY2;
    
    if (facing == 1)
    {
        dir = 1;
    }
    else
    {
        dir = -1;
    }

    parts = sPlayerEffectSets[set].parts;
    if (sPlayerEffectSets[set].partCount <= 0)
    {
        return;
    }

    part = parts;
    remaining = sPlayerEffectSets[set].partCount;
    do
    {
        child = EntityCreateInRange(0x49, 0xCF, PlayerEffectSetPart_Update);
        baseX = gEwramData->playerState.player->posX.whole;
        child->posX.whole = baseX + ((part->offsetX << 8) * dir);

        if ((part->animationAndPlacementMode & 0xF) == 1)
        {
            baseY = gEwramData->playerState.player->posY.whole;
            child->posY.whole = (baseY + (part->offsetY << 8)) - ((RandomNumberGenerator() & 3) << 0x10);
        }
        else
        {
            baseY2 = gEwramData->playerState.player->posY.whole;
            child->posY.whole = baseY2 + (part->offsetY << 8);
        }

        child->ext.playerEffectSet.anim = part->animationAndPlacementMode >> 4;
        child->velX.whole = (part->velocityX << 8) * dir;
        child->velY.whole = part->velocityY << 8;
        child->timer = part->delay;

        part++;
        remaining--;
    }
    while (remaining != 0);
}

/**
 * @brief Place the effect on the filtered surface below the player (two
 * probes, else delete) and play its animation once.
 */
void Effect_UpdatePlayerSurfaceEffect(struct EwramData_EntityData *entity)
{
    s32 i;
    s16 push;
    struct BgLayerState *bgLayer;
    struct BgLayerState *bg;
    s16 x;
    s16 y;
    s32 gfx;
    s32 playerX;
    s32 playerY;
    s32 surfaceY;

    if (entity->state == 0)
    {
        playerX = (s16) gEwramData->playerState.player->posX.part16.integer;
        entity->posX.part16.integer = playerX;
        playerY = (s16) gEwramData->playerState.player->posY.part16.integer;
        entity->posY.part16.integer = playerY;

        bg = &gEwramData->bgInfo[1];
        x = bg->xPos.part16.integer + entity->posX.part16.integer;
        y = bg->yPos.part16.integer + playerY - 8;
        for (i = 0; i < 2; i++)
        {
            push = Collision_GetPushUpFiltered(x, y + (i * 0x10), 2);
            if (push != 0)
            {
                surfaceY = y + push;
                bgLayer = &gEwramData->bgInfo[1];
                entity->posY.part16.integer = (surfaceY - (bgLayer->yPos.part16.integer - 1)) + (i * 0x10);
                break;
            }
        }

        if (push == 0)
        {
            entity->deleteRequested = 1;
            return;
        }

        gfx = ObjGfx_Find((u8 *)sCommonObjectGfx);
        Sprite_Attach(entity, (u8 *)sCommonObjectSpriteSheet, gfx, 6);
        Anim_StartIndexed(entity, entity->ext.surfaceEffect.anim, 0, 0);
        entity->drawFunc = Sprite_DrawEntity;
        entity->spriteLayer = 0xF;
        entity->scrollWithBg = 1;
        entity->state += 1;
    }

    if (Anim_Update(entity) == 3)
    {
        entity->deleteRequested = 1;
    }
}

/**
 * @brief Debris particle: attach frame 0x40 with random horizontal flip,
 * then move with gravity 0x2000 until it leaves the screen.
 */
void Effect_UpdateFallingDebris(struct EwramData_EntityData *entity)
{
    s32 gfx;
    s32 frame;

    switch (entity->state)
    {
        case 0:
            gfx = ObjGfx_Find((u8 *)sCommonObjectGfx);
            Sprite_Attach(entity, (u8 *)sCommonObjectSpriteSheet, gfx, 6);
            entity->drawFunc = Sprite_DrawEntity;
            frame = 0x40;
            entity->animFrame = frame;
            entity->flipX = RandomNumberGenerator() & 1;
            entity->spriteLayer = 0xD;
            entity->scrollWithBg = 1;
            entity->state += 1;
            break;

        case 1:
            entity->posX.whole += entity->velX.whole;
            entity->posY.whole += entity->velY.whole;
            entity->velY.whole += 0x2000;
            break;
    }

    if (EntityIsOutsideScreen(entity, 3) != 0)
    {
        entity->deleteRequested = 1;
    }
}

const u8 sPlayerSurfaceEffects[][2] = {
    [0] =  { [0] = 0x0A, [1] = 0xC0 },
    [1] =  { [0] = 0x0B, [1] = 0x00 },
    [2] =  { [0] = 0x0A, [1] = 0xC1 },
    [3] =  { [0] = 0x00, [1] = 0x00 },
    // [4] =  { [0] = 0x01, [1] = 0x00 },
    // [5] =  { [0] = 0x00, [1] = 0x00 },
    // [6] =  { [0] = 0xE0, [1] = 0x00 },
    // [7] =  { [0] = 0xBF, [1] = 0x00 }, 
    // [8] =  { [0] = 0x03, [1] = 0x00 },
    // [9] =  { [0] = 0x00, [1] = 0x00 },
    // [10] = { [0] = 0x80, [1] = 0x00 },
    // [11] = { [0] = 0xC0, [1] = 0x00 },
    // [12] = { [0] = 0x80, [1] = 0x00 },
    // [13] = { [0] = 0xA0, [1] = 0x00 },
    // [14] = { [0] = 0x00, [1] = 0x01 },
    // [15] = { [0] = 0xC0, [1] = 0x00 }, 
};

/**
 * @brief Create the surface effect of the given kind (animation and optional
 * sound from sPlayerSurfaceEffects). Kinds 0 and 2 are used when the player
 * enters or leaves flag-0x1000000 tiles at speed, kind 1 while
 * moving fast within them.
 */
void Player_SpawnSurfaceEffect(s32 kind)
{
    struct EwramData_EntityData *child;

    child = EntityCreateInRange(0x49, 0xCF, Effect_UpdatePlayerSurfaceEffect);
    if (child != NULL)
    {
        child->ext.surfaceEffect.anim = sPlayerSurfaceEffects[kind][0];
        if (sPlayerSurfaceEffects[kind][1] != 0)
        {
            PlaySong(sPlayerSurfaceEffects[kind][1]);
        }
    }
}

/**
 * @brief Spawn up to five falling debris particles spread horizontally
 * 16 px above the entity. Used by Soma_TryCeilingImpact.
 */
void Effect_SpawnDebrisBurst(struct EwramData_EntityData *entity)
{
    s32 i;
    struct EwramData_EntityData *child;

    for (i = -2; i < 3; i++)
    {
        child = EntityCreateInRange(0x49, 0xCF, Effect_UpdateFallingDebris);
        if (child != NULL)
        {
            child->posX.whole = entity->posX.whole + (i << 0xF) + RandomSigned(0x8000);
            child->posY.whole = entity->posY.whole -  0x100000 + (RandomSigned(0x1000) << 8);
            child->velX.whole = (i << 0xF) + RandomSigned(0x4000);
            child->velY.whole = RandomSigned(0x4000) - 0x18000;
            continue;
        }
        break;
    }
}

struct PushBlockGroupLayout {
    u8 blockCount;
    u8 pad_1[0x4 - 0x1];
    u16 roomCoordinates[2];
};
struct PushBlockLayoutTable {
    u8 maxGroupIndex;
    u8 pad_1[0x4 - 0x1];
    const struct PushBlockGroupLayout *layouts[2];
};

const struct PushBlockGroupLayout sPushBlockGroupLayout0 = {
    .blockCount = 1,
    .roomCoordinates = {
        [0] = 0xE0,
        [1] = 0xBF
    }
};

const struct PushBlockGroupLayout sPushBlockGroupLayout1 = {
    .blockCount = 3,
    .roomCoordinates = {
        [0] = 0x80,
        [1] = 0xC0
    }
};

// TODO: where does this data come from?
const u8 sUnk_080E1DA0[] = {
    0x80, 0x00, 0xA0, 0x00, 0x00, 0x01, 0xC0, 0x00
};

const struct PushBlockLayoutTable sPushBlockGroupLayouts = {
    .maxGroupIndex = 2,
    .layouts = {
        [0] = &sPushBlockGroupLayout0,
        [1] = &sPushBlockGroupLayout1
    }
};

const struct SolidObjectCallbackStorage sPushBlockSolidCallbacks = {
    .flags = 0,
    .ceilingCorrection = PushBlockGroup_CeilingCorrection,
    .floorCorrection = PushBlockGroup_FloorCorrection,
    .wallRightCorrection = PushBlockGroup_WallRightCorrection,
    .wallLeftCorrection = PushBlockGroup_WallLeftCorrection,
    .push = SolidObject_NoCarry
};

/**
 * @brief Object01 creation: register the group as the player solid object
 * and, for a valid variant (spawnParam0), create its 1 or 3 blocks at
 * the table room positions, storing their slots in unk_4F8.
 */
void PushBlockGroup_Create(struct EwramData_EntityData *entity)
{
    s32 tileSlot;
    s32 group;
    s32 i;
    s32 paletteIndex;
    struct EwramData_EntityData *child;
    const struct PushBlockGroupLayout *layout;
    s32 count;
    struct BgLayerState *bg;

    group = entity->spawnParam0;
    if ((Player_RegisterSolidObject(entity, (struct SolidObjectCallbackStorage*)&sPushBlockSolidCallbacks) != 0) && (group <= sPushBlockGroupLayouts.maxGroupIndex))
    {
        layout = sPushBlockGroupLayouts.layouts[group];
        count = layout->blockCount;
        tileSlot = ObjGfx_LoadResource((u8 *)sPushBlockGroupGfx, 0);
        paletteIndex = ObjPalette_FindOrLoad((void *)sPushBlockGroupPalette);
        for (i = 0; i < count; i++)
        {
            child = EntityCreateInRange(0x49, 0xCF, PushBlock_Update);
            Sprite_Attach(child, (u8 *)sPushBlockGroupSpriteSheet, tileSlot, (s32) paletteIndex);
            child->animFrame = 0;
            child->drawFunc = Sprite_DrawEntity;
            child->scrollWithBg = 1;

            bg = &gEwramData->bgInfo[1];
            child->posX.part16.integer = layout->roomCoordinates[i * 2] - bg->xPos.part16.integer;
            child->posY.part16.integer = layout->roomCoordinates[i * 2 + 1] - bg->yPos.part16.integer;
            child->accelY.part16.integer = 0;
            child->velY.part16.integer = 0;

            child->spriteLayer = 0x11;
            child->ext.pushBlock.flags = 0;
            ENTITY_OWNER(child) = entity;
            child->roomActive = 1;
            i[entity->ext.pushBlock.groupSlots] = EntityGetSlot(child);
        }
        entity->roomActive = 1;
    }
}

/* Overlap with the nearest block of the group on the left (> 0: push right). */
static inline s32 OverlapLeft(struct EwramData_EntityData *block)
{
    struct EwramData_EntityData *spawner = ENTITY_OWNER(block);
    s32 push = 0;
    s32 count = sPushBlockGroupLayouts.layouts[spawner->spawnParam0]->blockCount;
    s32 i;

    for (i = 0; i < count; i++)
    {
        struct EwramData_EntityData *other = &gEwramData->entityData[*(spawner->ext.pushBlock.groupSlots + i)];
        s32 dy = block->posY.whole - other->posY.whole;

        if (other != block && (dy >= -0x1F0000 && dy < 0x1F0000))
        {
            u32 dx = block->posX.whole - other->posX.whole;

            if (dx < 0x200000)
            {
                s32 overlap = 0x200000 - dx;

                if (push < overlap)
                    push = overlap;
            }
        }
    }
    return push;
}

/* Overlap with the nearest block of the group on the right (< 0: push left). */
static inline s32 OverlapRight(struct EwramData_EntityData *block)
{
    struct EwramData_EntityData *spawner = ENTITY_OWNER(block);
    s32 push = 0;
    s32 count = sPushBlockGroupLayouts.layouts[spawner->spawnParam0]->blockCount;
    s32 i;

    for (i = 0; i < count; i++)
    {
        struct EwramData_EntityData *other = &gEwramData->entityData[*(spawner->ext.pushBlock.groupSlots + i)];
        s32 dy = block->posY.whole - other->posY.whole;

        if (other != block && (dy >= -0x1F0000 && dy < 0x1F0000))
        {
            u32 dx = other->posX.whole - block->posX.whole;

            if (dx < 0x200000)
            {
                s32 overlap = dx - 0x200000;

                if (push > overlap)
                    push = overlap;
            }
        }
    }
    return push;
}

/* Overlap with a block of the group right below (< 0: push up); marks that block as carrying one. */
static inline s32 OverlapBelow(struct EwramData_EntityData *block)
{
    struct EwramData_EntityData *spawner = ENTITY_OWNER(block);
    s32 push = 0;
    s32 x = block->posX.whole;
    s32 y = block->posY.whole + 0x10000;
    s32 count = sPushBlockGroupLayouts.layouts[spawner->spawnParam0]->blockCount;
    s32 i;

    for (i = 0; i < count; i++)
    {
        struct EwramData_EntityData *other = &gEwramData->entityData[*(spawner->ext.pushBlock.groupSlots + i)];
        s32 dx = other->posX.whole - x;
        u32 dy = other->posY.whole - y;

        if (other != block && (dx >= -0x1F0000 && dx < 0x1F0000) && dy < 0x200000)
        {
            s32 overlap;

            other->ext.pushBlock.flags |= 4;
            overlap = dy - 0x200000;
            if (push > overlap)
                push = overlap;
        }
    }
    if (push != 0)
        push += 0x10000;
    return push;
}

/**
 * @brief Block of a push-block group: move 0.5 px per update while marked
 * by the wall callbacks or its own player/wall probes, unless it is
 * falling or carrying a block;
 * resolve overlap with group blocks and walls, fall with gravity
 * onto blocks or floor, and play the landing and scrape sounds.
 */
void PushBlock_Update(struct EwramData_EntityData *block)
{
    s32 falling;
    s32 startX;
    s32 x;
    s32 dx;
    s32 push;
    struct EwramData_EntityData *player;
    s16 roomY;
    s16 roomX;
    u32 dy;

    player = gEwramData->playerState.player;
    x = block->posX.whole;
    dx = x - player->posX.whole;
    dy = block->posY.whole - player->posY.whole;
    startX = x;

    roomX = GetEntityRoomXPositionInteger(block);
    roomY = GetEntityRoomYPositionInteger(block);
    if (dy < 0x1E0000)
    {
        if (Collision_GetPushLeft(roomX + 0x1E, roomY - 0x10) != 0)
        {
            if (dx > -0x190000 && dx < 0)
                block->ext.pushBlock.flags |= 1;
        }
        else if (Collision_GetPushRight(roomX - 0x1E, roomY - 0x10) != 0)
        {
            if (dx > 0 && dx < 0x190000)
                block->ext.pushBlock.flags |= 2;
        }
    }

    if (!(block->ext.pushBlock.flags & 0xC))
    {
        if (block->ext.pushBlock.flags & 1)
            block->posX.whole -= 0x8000;
        if (block->ext.pushBlock.flags & 2)
            block->posX.whole += 0x8000;
    }

    block->velY.whole += block->accelY.whole;
    if (block->velY.whole > 0x60000)
        block->velY.whole = 0x60000;
    block->posY.whole += block->velY.whole;

    if (block->ext.pushBlock.flags & 1)
    {
        push = OverlapLeft(block);
        if (push != 0)
            block->posX.whole += push;

        roomX = GetEntityRoomXPositionInteger(block);
        roomY = GetEntityRoomYPositionInteger(block);
        if (push = Collision_GetPushRight(roomX - 0x11, roomY), push != 0)
        {
            block->posX.part16.integer -= 1;
            block->posX.part16.integer += push;
        }
        else if (push = Collision_GetPushRight(roomX - 0x11, roomY - 0x18), push != 0)
        {
            block->posX.part16.integer -= 1;
            block->posX.part16.integer += push;
        }
    }
    if (block->ext.pushBlock.flags & 2)
    {
        push = OverlapRight(block);
        if (push != 0)
            block->posX.whole += push;

        roomX = GetEntityRoomXPositionInteger(block);
        roomY = GetEntityRoomYPositionInteger(block);
        if (push = Collision_GetPushLeft(roomX + 0x11, roomY), push != 0)
        {
            block->posX.part16.integer += 1;
            block->posX.part16.integer += push;
        }
        else if (push = Collision_GetPushLeft(roomX + 0xF, roomY - 0x18), push != 0)
        {
            block->posX.part16.integer -= 1;
            block->posX.part16.integer += push;
        }
    }

    falling = TRUE;
    push = OverlapBelow(block);
    if (push != 0)
    {
        block->posY.whole += push;
        block->accelY.whole = 0;
        block->velY.whole = 0;
        falling = FALSE;
    }

    roomX = GetEntityRoomXPositionInteger(block);
    roomY = GetEntityRoomYPositionInteger(block);
    if (push = Collision_GetPushUp(roomX + 0xF, roomY + 1), push != 0)
    {
        block->posY.whole += (push + 1) << 0x10;
        block->accelY.whole = 0;
        block->velY.whole = 0;
        falling = FALSE;
    }
    else if (push = Collision_GetPushUp(roomX - 0xF, roomY + 1), push != 0)
    {
        block->posY.whole += (push + 1) << 0x10;
        block->accelY.whole = 0;
        block->velY.whole = 0;
        falling = FALSE;
    }
    else if (push = Collision_GetPushUp(roomX, roomY + 1), push != 0)
    {
        block->posY.whole += (push + 1) << 0x10;
        block->accelY.whole = 0;
        block->velY.whole = 0;
        falling = FALSE;
    }

    if (falling)
    {
        block->accelY.whole = 0x2000;
        block->ext.pushBlock.flags |= 8;
    }
    else
    {
        if (block->ext.pushBlock.flags & 8)
            PlaySong(0x14D);
        block->ext.pushBlock.flags &= ~8;
    }

    if (startX != block->posX.whole)
    {
        if (!(block->ext.pushBlock.flags & 0x10))
        {
            Sound_PlayTracked(0x136);
            block->ext.pushBlock.flags |= 0x10;
        }
    }
    else
    {
        if (block->ext.pushBlock.flags & 0x10)
            Sound_StopTracked(0x136);
        block->ext.pushBlock.flags &= ~0x10;
    }

    block->ext.pushBlock.flags &= ~7;
    block->roomActive = 1;
}

static inline s32 PushBlockGroup_CeilingCorrectionInline(struct EwramData_EntityData *entity, s16 x, s16 y)
{
    s32 i;
    s32 correction;
    struct EwramData_EntityData *block;
    u32 dy;
    s32 count;
    struct BgLayerState *bg;
    s32 screenY;
    s32 screenX;
    s32 dx;
    s32 roomX;
    s32 offset;

    correction = 0;
    roomX = x;
    bg = &gEwramData->bgInfo[1];
    screenX = (roomX - (s16) bg->xPos.part16.integer) << 0x10;
    screenY = (y - (s16) bg->yPos.part16.integer) << 0x10;
    
    if (entity == NULL)
    {
        return 0;
    }
    else if (count = sPushBlockGroupLayouts.layouts[entity->spawnParam0]->blockCount, count == 0)
    {
        return 0;
    }

    for (i = 0; i < count; i++)
    {
        block = &gEwramData->entityData[i[entity->ext.pushBlock.groupSlots]];
        dx = block->posX.whole - screenX;
        if (dx >= -0x100000 && dx < 0x100000)
        {
            dy = block->posY.whole - screenY;
            if (dy < 0x200000)
            {
                offset = dy - 0;
                if (correction < offset)
                {
                    correction = offset;
                }
            }
        }
    }

    return (correction < 0) ? (s16)(-(-correction >> 0x10)) : (s16)(correction >> 0x10);
}

/**
 * @brief Solid-object ceiling callback: return the downward push needed to
 * move the room point out of the bottom of any group block.
 */
s16 PushBlockGroup_CeilingCorrection(struct EwramData_EntityData *entity, s16 x, s16 y)
{
    return PushBlockGroup_CeilingCorrectionInline(entity, x, y);
}

static inline s32 PushBlockGroup_FloorCorrectionInline(struct EwramData_EntityData *entity, s16 x, s16 y)
{
    s32 i;
    s32 correction;
    struct EwramData_EntityData *block;
    u32 dy;
    s32 count;
    struct BgLayerState *bg;
    s32 screenY;
    s32 screenX;
    s32 dx;
    s32 roomX;
    s32 offset;

    correction = 0;
    roomX = x;
    bg = &gEwramData->bgInfo[1];
    screenX = (roomX - (s16) bg->xPos.part16.integer) << 0x10;
    screenY = (y - (s16) bg->yPos.part16.integer) << 0x10;
    
    if (entity == NULL)
    {
        return 0;
    }
    else if (count = sPushBlockGroupLayouts.layouts[entity->spawnParam0]->blockCount, count == 0)
    {
        return 0;
    }

    for (i = 0; i < count; i++)
    {
        block = &gEwramData->entityData[i[entity->ext.pushBlock.groupSlots]];
        dx = block->posX.whole - screenX;
        if (dx >= -0x100000 && dx < 0x100000)
        {
            dy = block->posY.whole - screenY;
            if (dy < 0x200000)
            {
                offset = dy - 0x200000;
                if (correction > offset)
                {
                    correction = offset;
                }
            }
        }
    }

    return (correction < 0) ? (s16)(-(-correction >> 0x10)) : (s16)(correction >> 0x10);
}

/**
 * @brief Solid-object floor callback: return the upward push needed to
 * place the room point on top of any group block.
 */
s16 PushBlockGroup_FloorCorrection(struct EwramData_EntityData *entity, s16 x, s16 y)
{
    return PushBlockGroup_FloorCorrectionInline(entity, x, y);
}

static inline s32 PushBlockGroup_WallRightCorrectionInline(struct EwramData_EntityData *entity, s16 x, s16 y)
{
    s32 i;
    s32 correction;
    struct EwramData_EntityData *block;
    struct EwramData_EntityData *playerEntity;
    struct EwramData_EntityData *nearest;
    s32 dx;
    s32 count;
    struct BgLayerState *bg;
    s32 screenY;
    s32 screenX;
    u32 dy;
    s32 roomX;
    s32 offset;
    struct EwramData_EntityData *player;
    s32 pushing;

    player = gEwramData->playerState.player;
    pushing = player->ext.player.actionFlags & 0x100000;
    nearest = NULL;
    correction = 0;
    roomX = x;
    bg = &gEwramData->bgInfo[1];
    screenX = (roomX - (s16) bg->xPos.part16.integer) << 0x10;
    screenY = (y - (s16) bg->yPos.part16.integer) << 0x10;
    
    if (entity == NULL)
    {
        return 0;
    }
    else if (count = sPushBlockGroupLayouts.layouts[entity->spawnParam0]->blockCount, count == 0)
    {
        return 0;
    }

    for (i = 0; i < count; i++)
    {
        block = &gEwramData->entityData[i[entity->ext.pushBlock.groupSlots]];
        dy = block->posY.whole - screenY;
        if (dy < 0x200000)
        {
            dx = block->posX.whole - screenX;
            if (dx >= -0x100000 && dx < 0x100000)
            {
                offset = dx - 0x100000;
                if (correction > offset)
                {
                    correction = offset;
                    nearest = block;
                }
            }
        }
    }

    playerEntity = gEwramData->playerState.player;
    if ((playerEntity->ext.player.actionFlags & 0x400) && (playerEntity->state != 3))
    {
        correction = 0;
    }
    if ((pushing) && (nearest != NULL))
    {
        nearest->ext.pushBlock.flags |= 2;
    }

    return (correction < 0) ? (s16)(-(-correction >> 0x10)) : (s16)(correction >> 0x10);
}

/**
 * @brief Solid-object right-wall callback: return the leftward push out of
 * the nearest group block and mark it to move right. No push
 * while the player is crouching outside state 3.
 */
s16 PushBlockGroup_WallRightCorrection(struct EwramData_EntityData *entity, s16 x, s16 y)
{
    return PushBlockGroup_WallRightCorrectionInline(entity, x, y);
}

static inline s32 PushBlockGroup_WallLeftCorrectionInline(struct EwramData_EntityData *entity, s16 x, s16 y)
{
    s32 i;
    s32 correction;
    struct EwramData_EntityData *block;
    struct EwramData_EntityData *playerEntity;
    struct EwramData_EntityData *nearest;
    s32 dx;
    s32 count;
    struct BgLayerState *bg;
    s32 screenY;
    s32 screenX;
    u32 dy;
    s32 roomX;
    s32 offset;
    struct EwramData_EntityData *player;
    s32 pushing;

    player = gEwramData->playerState.player;
    pushing = player->ext.player.actionFlags & 0x100000;
    nearest = NULL;
    correction = 0;
    roomX = x;
    bg = &gEwramData->bgInfo[1];
    screenX = (roomX - (s16) bg->xPos.part16.integer) << 0x10;
    screenY = (y - (s16) bg->yPos.part16.integer) << 0x10;
    
    if (entity == NULL)
    {
        return 0;
    }
    else if (count = sPushBlockGroupLayouts.layouts[entity->spawnParam0]->blockCount, count == 0)
    {
        return 0;
    }

    for (i = 0; i < count; i++)
    {
        block = &gEwramData->entityData[i[entity->ext.pushBlock.groupSlots]];
        dy = block->posY.whole - screenY;
        if (dy < 0x200000)
        {
            dx = block->posX.whole - screenX;
            if (dx >= -0x100000 && dx < 0x100000)
            {
                offset = dx + 0x100000;
                if (correction <= offset)
                {
                    correction = offset - 1;
                    nearest = block;
                }
            }
        }
    }

    playerEntity = gEwramData->playerState.player;
    if ((playerEntity->ext.player.actionFlags & 0x400) && (playerEntity->state != 3))
    {
        correction = 0;
    }
    if ((pushing) && (nearest != NULL))
    {
        nearest->ext.pushBlock.flags |= 1;
    }

    return (correction < 0) ? (s16)(-(-correction >> 0x10)) : (s16)(correction >> 0x10);
}

/**
 * @brief Solid-object left-wall callback: return the rightward push out of
 * the nearest group block and mark it to move left. No push
 * while the player is crouching outside state 3.
 */
s16 PushBlockGroup_WallLeftCorrection(struct EwramData_EntityData *entity, s16 x, s16 y)
{
    return PushBlockGroup_WallLeftCorrectionInline(entity, x, y);
}

/**
 * @brief Unreferenced: delete the push-block group unless its 0x53D bit 2
 * (keep-alive) is set.
 */
void PushBlockGroup_DeleteIfOffscreen(struct EwramData_EntityData *entity)
{
    if (!(entity->roomActive))
    {
        PushBlockGroup_Delete(entity);
    }
}

/**
 * @brief Request deletion of all group blocks and the group, and clear
 * the player solid-object slot.
 */
void PushBlockGroup_Delete(struct EwramData_EntityData *entity)
{
    s32 i;
    s32 count;
    struct EwramData_EntityData *block;

    if (entity != NULL)
    {
        count = sPushBlockGroupLayouts.layouts[entity->spawnParam0]->blockCount;

        for (i = 0; i < count; i++)
        {
            block = &gEwramData->entityData[i[entity->ext.pushBlock.groupSlots]];
            block->deleteRequested = 1;
        }
        entity->deleteRequested = 1;
        Player_ClearSolidObject();
    }
}

/**
 * @brief 33C84 | To document
 * 
 * @param entity To document
 */
void Object01Create(struct EwramData_EntityData *entity)
{
    PushBlockGroup_Create(entity);
}

/**
 * @brief 33C90 | To document
 * 
 * @param entity To document
 */
void Object01Update(struct EwramData_EntityData *entity)
{
    if (!(entity->roomActive))
    {
        PushBlockGroup_Delete(entity);
    }
}
