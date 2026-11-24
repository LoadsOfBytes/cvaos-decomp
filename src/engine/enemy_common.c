/*
 * Enemy / entity helpers: filtered box collision, slopes, stun and hit flash, fragment deaths, ground snap, template reset, room bounds, box update, animation viewers, hit effects and damage resolution, target direction / distance / range, heap and room reset.
 */
#include "enemy_common.h"
#include "bg_and_collision.h"
#include "entity_util.h"
#include "structs/ewram.h"
#include "core.h"
#include "enemy_status.h"
#include "structs/entity_template.h"
#include <string.h>
#include "graphics.h"
#include "bg_effects.h"
#include "sound.h"
#include "player_core.h"
#include "syscalls.h"
#include "gba/dma.h"
#include "music.h"
#include "player_level.h"
#include "map_flags.h"
#include "entity_common.h"
#include "entity_motion.h"
#include "hitbox.h"
#include "math_anim.h"
#include "obj_gfx.h"
#include "palette.h"
#include "red_souls_b.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"
#include "math_inline.h"
#include "damage_numbers.h"

extern const s8 sEntityVisibilityPattern[40];

s32 Enemy_UpdateHitFlash(struct EwramData_EntityData *entity);

/* Accelerate/move both axes and resolve collision at four box corners, with
 * a tile-type exclusion: a corner whose tile flags satisfy (flags & 3) == 1
 * is skipped. Otherwise the directional correction and the slope query use
 * the same cached room coordinates. xOffset/yOffset are interpreted as signed
 * halfwords; width/height are additive unsigned extents. A nonzero
 * landingVelocity replaces the Y velocity after a downward correction.
 * Query-specific flag bits retain numeric names.
 *
 * Compiler-sensitive constructs (default project flags; do not copy the
 * -fno-expensive-optimizations exception of Entity_MoveWithBoxCollision):
 * - The r1 tile locals and the ReadTileFlags helper reproduce the original
 *   zero extension, mask and compare registers.
 * - Final corners: the entity+camera sum is truncated to s16 directly, so
 *   agbcc adds the two halfword memory operands without extending them (the
 *   spilled camera pointer is then reloaded for every component).
 * - FINAL_X0/FINAL_Y0: the statement expression delays the offset's sign
 *   extension until after the sum, and its halfword result makes agbcc put
 *   the offset first in the addition.
 * Matching code does not establish the original source spelling. */

static inline s32 ReadTileFlags(s32 x, s32 y) { return Collision_GetTile(x, y); }

static inline s16 TruncatedSignedOffset(u16 v) { s32 x = (s16)v; return x; }
#define ROOM_X ((s16)camera->xPos.part16.integer + (s16)entity->posX.part16.integer)
#define ROOM_Y ((s16)camera->yPos.part16.integer + (s16)entity->posY.part16.integer)

#define FINAL_X0 ((s16)((s16)(camera->xPos.part16.integer + entity->posX.part16.integer) + ({ TruncatedSignedOffset(xOffset); })))
#define FINAL_Y0 ((s16)((s16)(camera->yPos.part16.integer + entity->posY.part16.integer) + ({ TruncatedSignedOffset(yOffset); })))
#define FINAL_X(extra) ((s16)((extra) + (SignExtendOffset(xOffset) + (s16)(camera->xPos.part16.integer + entity->posX.part16.integer))))
#define FINAL_Y(extra) ((s16)((extra) + (SignExtendOffset(yOffset) + (s16)(camera->yPos.part16.integer + entity->posY.part16.integer))))
/**
 * @brief Resolve four-corner box movement while excluding corners
 * whose tile flags satisfy (flags & 3) == 1. Preserve cached room
 * coordinates, query-specific slope bits and narrowing order.
 */
s32 Entity_MoveWithFilteredBoxCollision(struct EwramData_EntityData *entity, s32 landingVelocity,
                u16 xOffset, u16 yOffset, u16 width, u16 height, s32 movement)
{
    s32 corners[4];
    s32 x, y;
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    s32 flags = 0;

    EntityAccelerateAndMoveX(entity, movement);
    if (entity->velX.whole <= 0)
    {
        x = ROOM_X + (s16)xOffset;
        y = ROOM_Y + (s16)yOffset;
        {
            register s32 tile asm("r1") = ReadTileFlags(x, y); /* FAKEMATCH: pinned register */
            tile &= 3;
            if (tile != 1)
            {
            s16 correction = Collision_GetPushRight(x, y);
            if (correction != 0)
            {
                flags = -(Collision_IsSlopeTile((s16)x, (s16)y) != 0) & 0x800;
                entity->posX.whole += correction << 16;
                flags |= 2;
            }
        }
        }
        x = ROOM_X + (s16)xOffset;
        y = ROOM_Y + (s16)yOffset + height;
        {
            register s32 tile asm("r1") = ReadTileFlags(x, y); /* FAKEMATCH: pinned register */
            tile &= 3;
            if (tile != 1)
            {
            s16 correction = Collision_GetPushRight(x, y);
            if (correction != 0)
            {
                if (Collision_IsSlopeTile((s16)x, (s16)y)) flags |= 0x200;
                entity->posX.whole += correction << 16;
                flags |= 2;
            }
        }
        }
    }
    if (entity->velX.whole >= 0)
    {
        x = ROOM_X + (s16)xOffset + width;
        y = ROOM_Y + (s16)yOffset;
        {
            register s32 tile asm("r1") = ReadTileFlags(x, y); /* FAKEMATCH: pinned register */
            tile &= 3;
            if (tile != 1)
            {
            s16 correction = Collision_GetPushLeft(x, y);
            if (correction != 0)
            {
                if (Collision_IsSlopeTile((s16)x, (s16)y)) flags |= 0x400;
                entity->posX.whole += correction << 16;
                flags |= 1;
            }
        }
        }
        x = ROOM_X + (s16)xOffset + width;
        y = ROOM_Y + (s16)yOffset + height;
        {
            register s32 tile asm("r1") = ReadTileFlags(x, y); /* FAKEMATCH: pinned register */
            tile &= 3;
            if (tile != 1)
            {
            s16 correction = Collision_GetPushLeft(x, y);
            if (correction != 0)
            {
                if (Collision_IsSlopeTile((s16)x, (s16)y)) flags |= 0x100;
                entity->posX.whole += correction << 16;
                flags |= 1;
            }
        }
        }
    }
    EntityAccelerateAndMoveY(entity, movement);
    if (entity->velY.whole <= 0)
    {
        x = ROOM_X + (s16)xOffset;
        y = ROOM_Y + (s16)yOffset;
        {
            register s32 tile asm("r1") = ReadTileFlags(x, y); /* FAKEMATCH: pinned register */
            tile &= 3;
            if (tile != 1)
            {
            s16 correction = Collision_GetPushDown(x, y);
            if (correction != 0)
            {
                if (Collision_IsSlopeTile((s16)x, (s16)y)) flags |= 0x800;
                entity->posY.whole += correction << 16;
                flags |= 8;
            }
        }
        }
        x = ROOM_X + (s16)xOffset + width;
        y = ROOM_Y + (s16)yOffset;
        {
            register s32 tile asm("r1") = ReadTileFlags(x, y); /* FAKEMATCH: pinned register */
            tile &= 3;
            if (tile != 1)
            {
            s16 correction = Collision_GetPushDown(x, y);
            if (correction != 0)
            {
                if (Collision_IsSlopeTile((s16)x, (s16)y)) flags |= 0x400;
                entity->posY.whole += correction << 16;
                flags |= 8;
            }
        }
        }
    }
    if (entity->velY.whole >= 0)
    {
        x = ROOM_X + (s16)xOffset;
        y = ROOM_Y + (s16)yOffset + height;
        {
            register s32 tile asm("r1") = ReadTileFlags(x, y); /* FAKEMATCH: pinned register */
            tile &= 3;
            if (tile != 1)
            {
            s16 correction = Collision_GetPushUp(x, y);
            if (correction != 0)
            {
                if (Collision_IsSlopeTile((s16)x, (s16)y)) flags |= 0x200;
                entity->posY.whole += correction << 16;
                if (landingVelocity != 0) entity->velY.whole = landingVelocity;
                flags |= 4;
            }
        }
        }
        x = ROOM_X + (s16)xOffset + width;
        y = ROOM_Y + (s16)yOffset + height;
        {
            register s32 tile asm("r1") = ReadTileFlags(x, y); /* FAKEMATCH: pinned register */
            tile &= 3;
            if (tile != 1)
            {
            s16 correction = Collision_GetPushUp(x, y);
            if (correction != 0)
            {
                if (Collision_IsSlopeTile((s16)x, (s16)y)) flags |= 0x100;
                entity->posY.whole += correction << 16;
                if (landingVelocity != 0) entity->velY.whole = landingVelocity;
                flags |= 4;
            }
        }
        }
    }
    corners[0] = Tile_Classify(FINAL_X0, FINAL_Y0);
    corners[1] = Tile_Classify(FINAL_X(width), FINAL_Y0);
    corners[2] = Tile_Classify(FINAL_X0, FINAL_Y(height));
    corners[3] = Tile_Classify(FINAL_X(width), FINAL_Y(height));
    if (corners[0] == 3 && corners[1] == 3) flags |= 0x80;
    if (corners[2] == 3 && corners[3] == 3) flags |= 0x40;
    if (corners[0] == 3 && corners[2] == 3) flags |= 0x20;
    if (corners[1] == 3 && corners[3] == 3) flags |= 0x10;
    return flags;
}

/* Adjust a signed movement value for matching slope orientation. */

static inline s32 AdjustForSlope(struct EwramData_EntityData *entity, s32 speed, s32 flags)
{
    s32 slope = flags & 0xC0;
    if (slope)
    {
        u32 kind = slope >> 6;
        s32 reversed = flags & 4;
        if (!entity->flipX)
        {
            if (reversed != 0)
                 return speed;
        }
        else if (reversed == 0)
            return speed;
        switch (kind)
        {
        case 1:  return speed / 2;
        case 2:  return speed * 2 / 3;
        default: asm(""); return speed; /* FAKEMATCH: changes jump2 order (cross-jump of the inlined copies) */
        }
    }
    return speed;
}

/**
 * @brief Scale the supplied signed speed for the matching tile slope
 * and facing: half speed for kind 1 or two thirds for kind 2.
 * If the current tile is zero, inspect one pixel below.
 */
s32 Entity_AdjustSpeedForSlope(struct EwramData_EntityData *entity, s32 speed)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    s32 x = (s16)entity->posX.part16.integer + (s16)bg->xPos.part16.integer;
    s32 y = (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer;
    s32 flags = (u8)Collision_GetTile(x, y);
    s32 result;
    if (flags == 0)
        result = AdjustForSlope(entity, speed, (u8)Collision_GetTile(x, y + 1));
    else
         result = AdjustForSlope(entity, speed, flags);
    return result;
}

/* Common update gates and palette changes after an enemy takes damage. */

static inline s32 TemplateBlocksUpdate(struct EwramData_EntityData *entity)
{
    u32 base = (u32)sEnemyData;
    u8 index = entity->enemyId;
    struct EntityTemplate *entry = (struct EntityTemplate *)(index * sizeof(*entry) + base);
    if (entry->flags_1C & 0x400)
        return 0;
    return 1;
}

s32 Enemy_IsStunned(struct EwramData_EntityData *entity)
{
    if ((s16)entity->hp <= 0)
        return 0;
    if (Enemy_UpdateHitFlash(entity))
        return 1;
    if (!gEwramData->gameState.bestiaryPreview && gEwramData->gameState.timeStopTimer != 0)
    {
        if (TemplateBlocksUpdate(entity))
            return 1;
    }
    if (ENEMY_STATUS_PETRIFIED & ENEMY_STATUS(entity))
    {
        entity->palette = 14;
        entity->basePalette = entity->palette;
        return 1;
    }
    return 0;
}

s32 Enemy_IsStunnedNoFlash(struct EwramData_EntityData *entity)
{
    if ((s16)entity->hp <= 0)
        return 0;
    if (entity->hitFlashTimer != 0)
        return 1;
    if (!gEwramData->gameState.bestiaryPreview && gEwramData->gameState.timeStopTimer != 0)
    {
        if (TemplateBlocksUpdate(entity))
            return 1;
    }
    if (ENEMY_STATUS_PETRIFIED & ENEMY_STATUS(entity))
    {
        entity->palette = 14;
        entity->basePalette = entity->palette;
        return 1;
    }
    return 0;
}

/* Count down the hit flash, alternating between palette 4 and the saved
 * palette on bit 1 of the timer. */
s32 Enemy_UpdateHitFlash(struct EwramData_EntityData *entity)
{
    if (entity->hitFlashTimer != 0)
    {
        if (--entity->hitFlashTimer & 2)
            entity->palette = 4;
        else
            entity->palette = entity->basePalette;
        return 1;
    }
    return 0;
}

/* Emit a fading fragment every fourth tick and finish the source effect. */
static inline void SpawnFragment(s32 x, s32 y, s32 frame)
{
    struct EwramData_EntityData *fragment = EntityCreateInRange(0x49, 0xCF, RisingEffect_Update);
    if (fragment)
    {
        fragment->posX.whole = x;
        fragment->posY.whole = y;
        fragment->flipX = RandomNumberGenerator() % 2;
        fragment->spriteLayer = frame;
    }
}
static inline s32 FlashValue(s32 index)
{
    s8 values[40];
    s16 bound = index;
    /* Same inline as in this file: the loop note of the do-while stops
       regmove from moving the test onto the copy. */
    do { memcpy(values, sEntityVisibilityPattern, sizeof(values)); } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
    if (index > 39) return 0;
    return values[bound];
}
/**
 * @brief Emit a random fragment every fourth timer tick, apply the visibility table and request deletion when the signed byte timer expires.
 */
void Enemy_UpdateFragmentDeath(struct EwramData_EntityData *entity)
{
    if ((entity->timer & 3) == 0)
    {
        s32 frame = entity->spriteLayer;
        s32 x = entity->posX.whole;
        s32 y = entity->posY.whole - 0x60000;
        s32 fragmentFrame = frame - 1;
        s32 dx = RandomOffset(0x100000);
        s32 dy = RandomOffset(0xC0000);
        SpawnFragment(x + dx, y + dy, fragmentFrame);
    }
    {
        entity->hidden = FlashValue(entity->timer);
    }
    if ((s8)--entity->timer <= 0)
        entity->deleteRequested = 1;
}

/* Find the ground in eight-pixel steps, then apply its exact correction. */
static inline s16 GroundCorrection(struct EwramData_EntityData *entity, struct BgLayerState *background)
{
    return Collision_GetPushUp((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,
                       (s16)background->yPos.part16.integer + entity->posY.part16.integer);
}
void Entity_SnapToGround(struct EwramData_EntityData *entity)
{
    struct BgLayerState *background = &gEwramData->bgInfo[1];
    if (GroundCorrection(entity, background) < 0)
    {
        while (GroundCorrection(entity, background) != 0)
            entity->posY.part16.integer -= 8;
        entity->posY.part16.integer += 7;
    }
    else
    {
        while (GroundCorrection(entity, background) == 0)
            entity->posY.part16.integer += 8;
    }
    entity->posY.whole += GroundCorrection(entity, background) << 16;
}

/* Restore selected entity attributes from its object template. */
void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity)
{
    entity->spriteLayer = 18;
    entity->hp = sEnemyData[entity->enemyId].hp;
    entity->mp = sEnemyData[entity->enemyId].mp;
    entity->attack = sEnemyData[entity->enemyId].attack;
    entity->defense = sEnemyData[entity->enemyId].defense;
}

/* Check whether the entity lies beyond the background's tile dimensions. */
/**
 * @brief Return 1 when the entity position (plus the BG 1 scroll) lies outside the
 * BG 1 map of the room (30 x 26 tiles for a one-screen map, else 32 tiles
 * per screen), else 0.
 */
s32 Entity_IsOutsideRoomMap(struct EwramData_EntityData *entity)
{
    struct BgLayerState *initial = &gEwramData->bgInfo[1];
    struct BgMapMetadata *metadata = initial->pBgMetadata;
    struct BgLayerState *background = initial;
    s32 x = ((s16)entity->posX.part16.integer + (s16)background->xPos.part16.integer) / 8;
    s32 width = metadata->widthCells == 1 ? 30 : metadata->widthCells * 32;
    s32 y, height;
    if (x < 0 || x >= width) goto outside;
    y = (entity->posY.part16.integer + (s16)background->yPos.part16.integer) / 8;
    height = metadata->heightCells == 1 ? 26 : metadata->heightCells * 32;
    if (y < 0 || y >= height) goto outside;
    return 0;
outside:
    return 1;
}

/* Clamp the entity's X position to an inset measured in eight-pixel units. */
s32 Entity_ClampXToRoom(struct EwramData_EntityData *entity, s32 inset)
{
    struct BgLayerState *initial = &gEwramData->bgInfo[1];
    struct BgMapMetadata *metadata = initial->pBgMetadata;
    struct BgLayerState *background = initial;
    s32 x = ((s16)entity->posX.part16.integer + (s16)background->xPos.part16.integer) / 8;
    s32 width = metadata->widthCells == 1 ? 30 : metadata->widthCells * 32;
    if (x < inset)
    {
        entity->posX.part16.integer = (inset << 3) - background->xPos.part16.integer;
        entity->posX.part16.fraction = 0;
    }
    else if (x > width - inset)
    {
        entity->posX.part16.integer = ((width - inset + 1) << 3) - background->xPos.part16.integer;
        entity->posX.part16.fraction = 0;
    }
    else return 0;
    return 1;
}

/*
 * Walk check for a point at the feet one pixel ahead: 0 when the walker can
 * continue (empty point with floor 8 px below and headroom 7 px above, or a
 * solid point with an empty / slope (0xC0) tile 7 px above, i.e. a step), 1 for
 * a wall or a ledge. Used by Curly_UpdateWalk to turn around.
 */
s32 Collision_IsWalkBlocked(s16 x, s16 y)
{
    u8 tile;
    if (Collision_GetTile(x, y) == 0)
    {
        if (Collision_GetTile(x, y + 8) != 0)
        {
            if (Collision_GetTile(x, y - 7) == 0)
                goto empty;
        }
    }
    else
    {
        tile = Collision_GetTile(x, y - 7);
        if (tile == 0 || (tile & 0xC0))
            goto empty;
    }
    return 1;
empty:
    return 0;
}

/* Refresh the entity's collision boxes for the current sprite frame. */
void Entity_UpdateBoxes(struct EwramData_EntityData *entity)
{
    u8 *bank = &entity->spriteSlot;
    struct SpriteSheet *graphics = gEwramData->spriteSlots[*bank].sheet;
    u8 frameIndex = entity->animFrame;
    struct SpriteFrame *frames = graphics->frames;
    struct SpriteFrame *frame = (struct SpriteFrame *)((u32)frameIndex * sizeof(*frame) + (u32)frames);
    void *boxes = frame->boxes;
    switch (frame->boxCount)
    {
    case 1:
        entity->hit.flags &= ~4;
        entity->hit.flags &= ~2;
        HitBox_SetBody(entity, boxes);
        HitBox_SetAttack(entity, boxes);
        break;
    case 2:
        entity->hit.flags &= ~4;
        entity->hit.flags &= ~2;
        HitBox_SetBody(entity, boxes);
        HitBox_SetAttack(entity, (u8 *)boxes + 4);
        break;
    case 0:
        entity->hit.flags |= 4;
        entity->hit.flags |= 2;
        break;
    }
}

/* Animation viewer entity (AnimationViewer_Update); matched with a permuter
 * (the s32 temporary for the frame byte fixes the register allocation). */

/**
 * @brief Handle sequence selection/restart and switching to individual
 * frame selection with the existing input masks. Preserve the copied
 * 64-bit animation object, frame-byte temporary and animation mode 0.
 */
void AnimationViewer_Update(struct EwramData_EntityData *entity)
{
    union { u64 raw; struct AnimViewerFrame f; } anim;
    struct EwramData *ewram;

    anim.raw = *(const u64 *)&sAnimViewerInitialFrame;

    if ((s8)entity->ext.animViewer.frameMode == 0)
    {
        if ((gEwramData->inputData.playerNewInput & 0x40) && entity->animScriptIndex != 0)
            Anim_StartIndexed(entity, entity->animScriptIndex - 1, 0, 0);
        if (gEwramData->inputData.playerNewInput & 0x80)
            Anim_StartIndexed(entity, entity->animScriptIndex + 1, 0, 0);
        if (gEwramData->inputData.playerNewInput & 1)
            Anim_StartIndexed(entity, entity->animScriptIndex, 0, 0);
        Anim_Update(entity);
        if (gEwramData->inputData.playerNewInput & 0x104)
        {
            entity->ext.animViewer.frameMode = 1;
            entity->ext.animViewer.frame = 0;
            Anim_Start(entity, (struct AnimScript *)&anim, 0, 1);
        }
    }
    else
    {
        entity->animTimer = 0;
        if (gEwramData->inputData.playerNewInput & 0x40)
        {
            if ((s8)entity->ext.animViewer.frame != 0)
                entity->ext.animViewer.frame--;
            entity->animTimer = 100;
        }
        if (gEwramData->inputData.playerNewInput & 0x80)
        {
            entity->ext.animViewer.frame++;
            entity->animTimer = 100;
        }
        ewram = gEwramData;
        if (ewram->inputData.playerNewInput & 1)
        {
            entity->ext.animViewer.frame = 0;
            entity->animTimer = 100;
        }
        if (ewram->inputData.playerNewInput & 0x208)
        {
            entity->ext.animViewer.frameMode = 0;
            Anim_StartIndexed(entity, 0, 0, 0);
        }
        {
            s32 index = entity->ext.animViewer.frame;
            u8 frame = index;
            anim.f.frame = frame;
        }
        Anim_Update(entity);
    }
}

/* Animation viewer entity with palette selection (AnimationPaletteViewer_Update), sibling of
 * AnimationViewer_Update. */

/**
 * @brief Handle animation sequence/frame selection plus left/right
 * palette changes in animation mode 2. Preserve writing the frame
 * through the pointer obtained before Anim_Start.
 */
void AnimationPaletteViewer_Update(struct EwramData_EntityData *entity)
{
    union { u64 raw; struct AnimViewerFrame f; } anim;
    struct EwramData *ewram;

    anim.raw = *(const u64 *)&sAnimViewerInitialFrame;

    if ((s8)entity->ext.animViewer.frameMode == 0)
    {
        if ((gEwramData->inputData.playerNewInput & 0x40) && entity->animScriptIndex != 0)
            Anim_StartIndexed(entity, entity->animScriptIndex - 1, 2, 0);
        if (gEwramData->inputData.playerNewInput & 0x80)
            Anim_StartIndexed(entity, entity->animScriptIndex + 1, 2, 0);
        if (gEwramData->inputData.playerNewInput & 0x20)
            entity->palette--;
        if (gEwramData->inputData.playerNewInput & 0x10)
            entity->palette++;
        if (gEwramData->inputData.playerNewInput & 1)
            Anim_StartIndexed(entity, entity->animScriptIndex, 2, 0);
        Anim_Update(entity);
        if (gEwramData->inputData.playerNewInput & 0x104)
        {
            entity->ext.animViewer.frameMode = 1;
            entity->ext.animViewer.frame = 0;
            Anim_Start(entity, (struct AnimScript *)&anim, 2, 1);
        }
    }
    else
    {
        entity->animTimer = 0;
        if (gEwramData->inputData.playerNewInput & 0x40)
        {
            if ((s8)entity->ext.animViewer.frame != 0)
                entity->ext.animViewer.frame--;
            entity->animTimer = 100;
        }
        if (gEwramData->inputData.playerNewInput & 0x80)
        {
            entity->ext.animViewer.frame++;
            entity->animTimer = 100;
        }
        ewram = gEwramData;
        if (ewram->inputData.playerNewInput & 1)
        {
            entity->ext.animViewer.frame = 0;
            entity->animTimer = 100;
        }
        if (ewram->inputData.playerNewInput & 0x20)
            entity->palette--;
        if (ewram->inputData.playerNewInput & 0x10)
            entity->palette++;
        if (gEwramData->inputData.playerNewInput & 0x208)
        {
            entity->ext.animViewer.frameMode = 0;
            Anim_StartIndexed(entity, 0, 2, 0);
        }
        {
            /* the frame byte is written through a pointer taken before the call */
            struct AnimViewerFrame *frame = &anim.f;

            Anim_Start(entity, (struct AnimScript *)&anim, 2, 1);
            frame->frame = entity->ext.animViewer.frame;
        }
        Anim_Update(entity);
    }
}

/* Initialize and play a short shared effect animation. */
/**
 * @brief State 0: attach the shared object graphics with the palette and
 * animation of entry subState of sHitEffects (layer 3, scrolls with BG);
 * then animate and delete at the end of the animation.
 */
void HitEffect_Update(struct EwramData_EntityData *entity)
{
    if (entity->state == 0)
    {
        const struct HitEffectEntry *entry = &sHitEffects[entity->subState];
        s32 graphics = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, entry->palette);
        Anim_StartIndexed(entity, entry->animation, 0, 0);
        entity->spriteLayer = 3;
        entity->drawFunc = Sprite_DrawEntity;
        entity->scrollWithBg = 1;
        entity->state++;
    }
    if (Anim_Update(entity) == 3)
        entity->deleteRequested = 1;
}

struct HitRect {
    s16 left;
    s16 top;
    s16 right;
    s16 bottom;
};

/* Contact point of two entities: centre of the overlap of their boxes,
 * clamped to the second box. */
/**
 * @brief Estimate a contact point from the first attack box and second
 * body box, clamping the result to the second box. Y starts at
 * the first box center; missing boxes leave the outputs untouched.
 */
void Hit_GetContactPoint(struct EwramData_EntityData *a, struct EwramData_EntityData *b, s16 *outX, s16 *outY)
{
    struct HitRect ra;
    struct SpriteBox a0, a1;
    struct HitRect rb;
    struct SpriteBox b0, b1;
    struct HitRect *rect = &ra;
    struct HitRect *rect2;
    struct SpriteBox *box = &a0;

    if (!HitBox_GetBoxes(box, &a1, a))
        return;
    if (a->flipX)
    {
        { s32 dx = box->x; rect->right = a->posX.part16.integer - dx; }
        rect->left = rect->right - box->width;
    }
    else
    {
        rect->left = box->x + a->posX.part16.integer;
        rect->right = rect->left + box->width;
    }
    rect->top = box->y + a->posY.part16.integer;
    rect->bottom = rect->top + box->height;
    rect2 = &rb;
    if (!HitBox_GetBoxes(&b0, &b1, b))
        return;
    box = &b1;
    if (b->flipX)
    {
        { s32 dx = box->x; rect2->right = b->posX.part16.integer - dx; }
        rect2->left = rect2->right - box->width;
    }
    else
    {
        rect2->left = box->x + b->posX.part16.integer;
        rect2->right = rect2->left + box->width;
    }
    rect2->top = box->y + b->posY.part16.integer;
    rect2->bottom = rect2->top + box->height;
    if (ra.left < rb.left)
        *outX = ra.right - (ra.right - rb.left) / 2;
    else
        *outX = ra.left + (rb.right - ra.left) / 2;
    *outY = ra.top + (ra.bottom - ra.top) / 2;
    if (*outX < rb.left)
        *outX = rb.left;
    else if (*outX > rb.right)
        *outX = rb.right;
    if (*outY < rb.top)
        *outY = rb.top;
    else if (*outY > rb.bottom)
        *outY = rb.bottom;
}

/* Hit_ResolveDamage: matched (damage resolution as an inline whose
   working variables are parameters; template address passed as the last,
   written parameter). */
/* Resolve damage modifiers and create the shared hit effect. */

static inline void HighestEffect(u16 *out, u32 bits)
{
    s32 mask = 0x20;
    u16 result = 0;
    s32 i = 6;
    while (i > 0)
    {
        if (bits & mask)
        {
            result = i;
            break;
        }
        mask >>= 1;
        i--;
    }
    *out = result;
}

static inline void SpawnHitSpark(s16 x, s16 y, u16 effect)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, HitEffect_Update);

    if (child != NULL)
    {
        child->posX.part16.integer = x;
        child->posY.part16.integer = y;
        child->flipX = RandomNumberGenerator() & 1;
        child->ext.hitEffect.type = effect;
        child->subState = effect;
    }
}

static inline void ShowHit(struct EwramData_EntityData *other, struct EwramData_EntityData *entity, u16 effect)
{
    s16 x, y;

    Hit_GetContactPoint(other, entity, &x, &y);
    SpawnHitSpark(x, y, effect);
}

static inline u16 ResolveDamage(struct EwramData_EntityData *entity, s32 damage, u16 flags, u16 *out, u32 weakness, u32 attributes, const struct EntityTemplate *template)
{
    const struct EntityTemplate *t = template;

    if (flags & 2)
    {
        struct WeaponInfo *weapon = Weapon_GetEquippedInfo();

        if (weapon != NULL)
        {
            attributes = weapon->attackAttributes;
            weakness = 1;
            if (attributes & 0x1000)
                damage >>= 1;
        }
    }
    if (!gEwramData->gameState.linkPlay)
    {
        if (!weakness && (flags & 8))
        {
            const void *soul = RedSoul_GetLastUsedInfo();

            weakness = 1;
            attributes = ((const u16 *)soul)[6];
        }
    }
    else if (flags & 8)
    {
        struct WeaponInfo *weapon = Weapon_GetAttackerInfo(1);

        if (weapon != NULL)
        {
            attributes = weapon->attackAttributes;
            if (attributes & 0x1000)
                damage >>= 1;
        }
    }
    if (!weakness && (flags & 0x10))
        attributes = gEwramData->playerState.skillAttackAttributes;

    weakness = ((u16 *)t)[13] & attributes;
    if (weakness != 0)
    {
        if (ENEMY_STATUS(entity) == 0)
        {
            /* SImode mask register (a plain constant is narrowed to HImode). */
            ENEMY_STATUS(entity) = weakness & ({ u32 k_ = 0xFC0; k_; });
            if (ENEMY_STATUS(entity) & ENEMY_STATUS_PETRIFIED)
                return 0;
            if (ENEMY_STATUS(entity) & 0x800)
            {
                s16 old = entity->hp;
                s16 delta;

                entity->hp = entity->mp;
                entity->mp = old;
                delta = (s16)entity->hp - old;
                if (delta >= 0)
                    Entity_ShowDamage(entity, (s16)delta, 1);
                else
                    Entity_ShowDamage(entity, (s16)delta, 2);
            }
            if (ENEMY_STATUS(entity) & 0x40)
            {
                entity->defense = 0;
                entity->attack -= entity->attack >> 2;
            }
        }
        if (weakness & 0x80)
            damage += (s16)entity->hp >> 2;
        weakness &= 0x3F;
        if (weakness)
        {
            if (weakness == 1)
                damage += damage >> 2;
            else
                damage <<= 1;
            HighestEffect(out, weakness);
            return damage;
        }
    }
    weakness = attributes & 0x3F;
    if (weakness)
    {
        u32 resisted = t->flags_1C & weakness;

        if ((resisted & 0xFFFE) != 0)
        {
            if ((weakness & ~1) == (resisted & 0xFFFE))
                return damage - (damage >> 1);
        }
        else if (weakness == resisted)
            return damage - (damage >> 1);
    }
    /* Dead store: makes the parameter writable, so the inliner copies the
       argument through a temporary (the template address add lands after
       the other parameter copies, as in the original). */
    template = 0;
    HighestEffect(out, attributes);
    if (*out == 1)
        *out = 0;
    return damage;
}

s16 Hit_ResolveDamage(struct EwramData_EntityData *entity, struct EwramData_EntityData *other, u16 *hit, s16 inputDamage)
{
    u16 effect = 0;
    s32 damage;

    damage = ResolveDamage(entity, inputDamage, hit[0], &effect, 0, 0, &sEnemyData[entity->enemyId]);
    PlaySong(sHitEffects[effect].song);
    damage = ((damage - (entity->defense >> 1)) * (256 - entity->defense)) >> 8;
    if (damage <= 0)
        damage = 1;
    if (ENEMY_STATUS(entity) & ENEMY_STATUS_PETRIFIED)
    {
        damage <<= 1;
        effect = 0;
    }
    ShowHit(other, entity, effect);
    Hud_RequestEnemyName(entity);
    return damage;
}

/* Entity_GetTargetAngleAbove16: matched (s16 x/y locals: fold narrows to signed short subtraction,
   expand_binop loads posX before sign-extending x). */
/* Aim at the selected target, with a sixteen-pixel vertical offset. */
static inline s32 TargetX(struct EwramData_EntityData *entity)
{
    if (!gEwramData->gameState.linkPlay)
    {
        struct EwramData_EntityData *target = gEwramData->playerState.enemyTarget;
        if (target == NULL) return (s16)entity->posX.part16.integer;
        return (s16)target->posX.part16.integer;
    }
    return (s16)gEwramData->playerState.linkTargetProxy->posX.part16.integer;
}
static inline s16 TargetY(struct EwramData_EntityData *entity)
{
    if (!gEwramData->gameState.linkPlay)
    {
        struct EwramData_EntityData *target = gEwramData->playerState.enemyTarget;
        if (target == NULL) return entity->posY.part16.integer;
        return target->posY.part16.integer;
    }
    return gEwramData->playerState.linkTargetProxy->posY.part16.integer;
}
/**
 * @brief Return ArcTan2 toward the selected target with target Y reduced by 16 pixels; preserve signed-halfword coordinate subtraction and target fallback.
 */
u16 Entity_GetTargetAngleAbove16(struct EwramData_EntityData *entity)
{
    s16 x, y;
    if (entity->enemyFlags & 4)
    {
        struct EwramData_EntityData *player = gEwramData->playerState.player;
        x = player->posX.part16.integer;
        y = player->posY.part16.integer;
    }
    else
    {
        x = TargetX(entity);
        y = TargetY(entity);
    }
    return ArcTan2(x - (s16)entity->posX.part16.integer, y - entity->posY.part16.integer - 16);
}

/* Target-relative direction helpers, decompiled from original assembly. */
/* 1 when the target is to the right, -1 to the left, 0 at the same x. */
s32 Entity_GetTargetDirection(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *target;
    s32 targetX;
    s32 selfX;

    if (entity->enemyFlags & 4)
        target = gEwramData->playerState.player;
    else if (!gEwramData->gameState.linkPlay)
    {
        target = gEwramData->playerState.enemyTarget;
        if (target == NULL)
        {
            targetX = entity->posX.whole;
            selfX = targetX;
            goto compare;
        }
    }
    else
        target = gEwramData->playerState.linkTargetProxy;
    targetX = target->posX.whole;
    selfX = entity->posX.whole;
compare:
    if (selfX < targetX)
        return 1;
    if (selfX > targetX)
        return -1;
    return 0;
}

/* Face the selected target, retaining the facing when their x is equal. */
void Entity_FaceTarget(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *target;
    s32 targetX;
    s32 selfX;

    if (entity->enemyFlags & 4)
        target = gEwramData->playerState.player;
    else if (!gEwramData->gameState.linkPlay)
    {
        target = gEwramData->playerState.enemyTarget;
        if (target == NULL)
        {
            targetX = entity->posX.whole;
            selfX = targetX;
            goto compare;
        }
    }
    else
        target = gEwramData->playerState.linkTargetProxy;
    targetX = target->posX.whole;
    selfX = entity->posX.whole;
compare:
    if (targetX != selfX)
    {
        s32 facing = 0;
        if (selfX < targetX)
            facing = 1;
        entity->flipX = facing;
    }
}

/* Check the entity facing against the selected target position.
 * The comparison works on a copy of selfX (x); the remaining pin keeps
 * selfX in r3 so the copy is not coalesced. */

/* Whether the target lies in the direction the entity currently faces. */
/**
 * @brief Return 1 when the selected target lies strictly ahead of the
 * entity facing. Honor forced-player bit enemyFlags & 4, alternate
 * target mode and primary-target fallback; equal X returns 0.
 */
s32 Entity_IsFacingTarget(struct EwramData_EntityData *entity)
{
    s32 result = 0;
    struct EwramData_EntityData *target;
    s32 targetX;
    register s32 selfX asm("r3"); /* FAKEMATCH: pinned register */

    if (entity->enemyFlags & 4)
    {
        target = gEwramData->playerState.player;
        targetX = target->posX.whole;
        selfX = entity->posX.whole;
    }
    else if (!gEwramData->gameState.linkPlay)
    {
        target = gEwramData->playerState.enemyTarget;
        if (target == NULL)
        {
            targetX = entity->posX.whole;
            selfX = targetX;
        }
        else
        {
            targetX = target->posX.whole;
            selfX = entity->posX.whole;
        }
    }
    else
    {
        target = gEwramData->playerState.linkTargetProxy;
        targetX = target->posX.whole;
        selfX = entity->posX.whole;
    }
    {
        s32 x = selfX;
        if (targetX != x && ((x < targetX && entity->flipX) || (selfX > targetX && !entity->flipX)))
            result = 1;
    }
    return result;
}

/* Signed pixel distance from the selected target. */

s32 Entity_GetTargetDistanceX(struct EwramData_EntityData *entity)
{
    u16 targetX;
    if (entity->enemyFlags & 4)
        targetX = gEwramData->playerState.player->posX.part16.integer;
    else
    {
        struct EwramData_EntityData *target;
        s32 x;
        if (!gEwramData->gameState.linkPlay)
        {
            target = gEwramData->playerState.enemyTarget;
            if (target == NULL)
            {
                x = (s16)entity->posX.part16.integer;
                goto convert;
            }
        }
        else
            target = gEwramData->playerState.linkTargetProxy;
        x = (s16)target->posX.part16.integer;
    convert:
        targetX = x;
    }
    {
        s32 distance = entity->posX.part16.integer - (s16)targetX;
        distance <<= 16;
        return distance >> 16;
    }
}

/**
 * @brief Return signed 16-bit pixel distance entity Y minus selected target Y; missing default target gives zero.
 */
s32 Entity_GetTargetDistanceY(struct EwramData_EntityData *entity)
{
    u16 targetY;
    if (entity->enemyFlags & 4)
        targetY = gEwramData->playerState.player->posY.part16.integer;
    else
    {
        struct EwramData_EntityData *target;
        s32 y;
        if (!gEwramData->gameState.linkPlay)
        {
            target = gEwramData->playerState.enemyTarget;
            if (target == NULL)
            {
                y = (s16)entity->posY.part16.integer;
                goto convert;
            }
        }
        else
            target = gEwramData->playerState.linkTargetProxy;
        y = (s16)target->posY.part16.integer;
    convert:
        targetY = y;
    }
    {
        s32 distance = (u16)entity->posY.part16.integer - (s16)targetY;
        distance <<= 16;
        return distance >> 16;
    }
}

/* Emit a fading fragment every fourth tick and finish the source effect. */
static inline s32 FlashValue2(s32 index)
{
    s8 values[40];
    s16 bound = index;
    /* The loop note of the do-while stops regmove from moving the test onto the copy. */
    do { memcpy(values, sEntityVisibilityPattern, sizeof(values)); } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
    if (index > 39) return 0;
    return values[bound];
}
/**
 * @brief Run fragment death with caller-supplied emission interval and fixed-point width/height spread.
 */
void Enemy_UpdateFragmentDeathWithSpread(struct EwramData_EntityData *entity, s32 interval, s32 width, s32 height)
{
    if (entity->timer % interval == 0)
    {
        s32 frame = entity->spriteLayer;
        s32 x = entity->posX.whole;
        s32 y = entity->posY.whole - 0x60000;
        s32 fragmentFrame = frame - 1;
        s32 dx = RandomOffset(width);
        s32 dy = RandomOffset(height);
        SpawnFragment(x + dx, y + dy, fragmentFrame);
    }
    entity->hidden = FlashValue2(entity->timer);
    if ((s8)--entity->timer <= 0)
        entity->deleteRequested = 1;
}

/* Is the selected target within RANGE of the entity on both axes? */
/* Position of the selected target (playerState.player when enemyFlags bit 2 is set,
 * otherwise linkTargetProxy or enemyTarget; without a target the entity itself).
 * The two spellings give the two code shapes of the original. */
#define TARGET_POS_A(entity, AXIS)                                   \
    ({                                                               \
        struct EwramData_EntityData *t_;                             \
        s32 v_;                                                      \
        if ((entity)->enemyFlags & 4)                                   \
            v_ = gEwramData->playerState.player->AXIS.whole;        \
        else if (!gEwramData->gameState.linkPlay)                      \
        {                                                            \
            t_ = gEwramData->playerState.enemyTarget;                    \
            if (t_ == NULL)                                          \
                v_ = (entity)->AXIS.whole;                           \
            else                                                     \
                v_ = t_->AXIS.whole;                                 \
        }                                                            \
        else                                                         \
            v_ = gEwramData->playerState.linkTargetProxy->AXIS.whole;        \
        v_;                                                          \
    })

#define TARGET_POS_B(entity, AXIS)                                   \
    ({                                                               \
        struct EwramData_EntityData *t_;                             \
        s32 v_;                                                      \
        if ((entity)->enemyFlags & 4)                                   \
            v_ = gEwramData->playerState.player->AXIS.whole;        \
        else if (!gEwramData->gameState.linkPlay)                      \
        {                                                            \
            t_ = gEwramData->playerState.enemyTarget;                    \
            v_ = (entity)->AXIS.whole;                               \
            if (t_ != NULL)                                          \
                v_ = t_->AXIS.whole;                                 \
        }                                                            \
        else                                                         \
            v_ = gEwramData->playerState.linkTargetProxy->AXIS.whole;        \
        v_;                                                          \
    })

/* |pos - target| along AXIS; the ?: inside the condition makes each arm
 * compare against RANGE on its own. */
#define TARGET_DIST(entity, AXIS)                                    \
    ((entity)->AXIS.whole - TARGET_POS_A(entity, AXIS) < 0           \
         ? TARGET_POS_B(entity, AXIS) - (entity)->AXIS.whole         \
         : (entity)->AXIS.whole - TARGET_POS_B(entity, AXIS))

/**
 * @brief Test both absolute fixed-point axis distances against the inclusive supplied range; missing default target falls back to the entity itself.
 */
u8 Entity_IsTargetWithinRange(struct EwramData_EntityData *entity, u32 range)
{
    if (TARGET_DIST(entity, posX) <= range && TARGET_DIST(entity, posY) <= range)
        return 1;
    return 0;
}

/* Target box tests, decompiled from the original assembly. */

/**
 * @brief Test the selected target in a centered width/height pixel rectangle; the alternate-target flag overrides player selection and a null target returns zero.
 */
s32 Entity_IsTargetInCenteredBox(struct EwramData_EntityData *entity, u16 width, u16 height)
{
    struct EwramData_EntityData *target;
    u16 x;
    u16 y;
    if (entity->enemyFlags & 4)
        target = gEwramData->playerState.player;
    else
        target = gEwramData->playerState.enemyTarget;
    x = entity->posX.part16.integer - (width >> 1);
    y = entity->posY.part16.integer - (height >> 1);
    if (gEwramData->gameState.linkPlay)
        target = gEwramData->playerState.linkTargetProxy;
    if (target == NULL)
        return 0;
    else
        return Entity_IsInRect(target, (s16)x, (s16)y, (u16)width, (u16)height);
}

/* Headers for the word-sized blocks in the shared 0x800-byte arena.
 * A free block has tag 0; words excludes the header itself. */
struct EntityHeapBlock {
    u16 tag;
    u16 words;
};

/* Free an allocation, then coalesce the first adjacent run of free blocks. */
void Heap_Free(void *allocation)
{
    struct EntityHeapBlock *block = (struct EntityHeapBlock *)gEwramData->entityScratchBuffer;
    s32 index;
    if (allocation == NULL)
        return;
    ((struct EntityHeapBlock *)allocation)[-1].tag = 0;
    for (index = 0; index <= 510; )
    {
        u16 words = block->words;
        if (block->tag == 0 && words + index <= 511)
        {
            struct EntityHeapBlock *next = block;
            s32 span = 0;
            while (next->tag == 0)
            {
                span += next->words + 1;
                next = block + span;
                if (index + span > 510)
                    break;
            }
            if (span > block->words + 1)
            {
                block->words = span - 1;
                return;
            }
        }
        words++;
        index += words;
        block += words;
    }
}

/* Allocate words from the first free block with room for a new header. */
void *Heap_Alloc(u16 words)
{
    struct EntityHeapBlock *block = (struct EntityHeapBlock *)gEwramData->entityScratchBuffer;
    s32 index = 0;
    s32 limit = 508;
    for (; index <= 510; )
    {
        u16 size = block->words;
        s32 tag = block->tag;
        if (tag == 0 && size >= words + 2)
        {
            void *data;
            s32 bytes;
            s32 end;
            block->tag = index + 1;
            block->words = words;
            end = index + words;
            bytes = words << 2;
            if (end <= limit)
            {
                struct EntityHeapBlock *tail = block + words;
                tail[1].tag = tag;
                tail[1].words = size - words - 1;
            }
            data = block + 1;
            DMA_FILL_32(3, tag, data, bytes);
            return data;
        }
        size++;
        index += size;
        block += size;
    }
    return NULL;
}

/* Reset entity-related state and initialize the small allocation arena. */
/**
 * @brief Per-room reset: clear tracked sounds and the music override, stop
 * finished colour fades, clear the selected target, solid object and
 * shared palettes, reset the entity heap, screen shake and message
 * window.
 */
void Room_ResetTransientState(void)
{
    struct EntityHeapBlock *heap;
    s32 zero;
    gEwramData->gameState.conveyorSpeed = -0x8000;
    gEwramData->gameState.cachedPickupIcon = 0xFF;
    gEwramData->gameState.stateFlags &= ~0x440800;
    Sound_ClearTracked();
    Music_StopOverride();
    if (!BgPalette_GetPendingFades())
        BgPalette_StopColorFade();
    if (!ObjPalette_GetPendingFades())
        ObjPalette_StopColorFade();
    Player_SetSelectedTarget(NULL);
    Player_ClearSolidObject();
    SharedObjPalette_ResetSlots();
    heap = (struct EntityHeapBlock *)gEwramData->entityScratchBuffer;
    zero = 0;
    DMA_FILL_32(3, zero, heap, 0x800);
    heap->tag = zero;
    heap->words = 511;
    ScreenShake_Clear();
    Hud_CloseMessageWindow();
}
