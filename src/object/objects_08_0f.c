/*
 * Room objects 0x08-0x0F: debris pieces and bursts, breakable blocks (collision, objects 0x08 / 0x09 and their dust), launch triggers, objects 0x0A-0x0D and the falling pieces of object 0x0E.
 */
#include "objects_08_0f.h"
#include "core.h"
#include "graphics.h"
#include "entity_util.h"
#include "structs/ewram.h"
#include "syscalls.h"
#include "player_core.h"
#include "gameplay.h"
#include "map_flags.h"
#include "sound.h"
#include "structs/entity_template.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "hitbox.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "effects.h"
#include "data/data_0E5364.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"
#include "entity_hit.h"

extern s32 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void (*sObject0CTriggerFuncs[])(struct EwramData_EntityData *, struct Object0CTrigger *);
extern s32 Pickup_TrySpawnOnHit(struct EwramData_EntityData *, s32, s32, s32);

/* from bg_and_collision.h (not included: see the module notes) */
extern s16 GetEntityRoomXPositionInteger(struct EwramData_EntityData *entity);
extern s16 GetEntityRoomYPositionInteger(struct EwramData_EntityData *entity);
extern void Collision_SetBlock(s32 arg0, s32 arg1);
extern void Collision_ClearBlock(s32 arg0, s32 arg1);
/* Declared s16 in bg_and_collision.h; the caller here uses the full register. */
extern s32 Collision_GetGroundOffset(s32, s32);

void BreakableBlockDust_Update(struct EwramData_EntityData *entity);
void Object0E_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker);
void Object0F_Draw(struct EwramData_EntityData *entity);

/**
 * @brief Burst debris piece: start with a random velocity away from the player,
 * fall under gravity, bounce once at half speed (new frame) when the point
 * below it is solid, then keep falling; deleted outside the screen margin.
 */
void DebrisPiece_Update(struct EwramData_EntityData *entity)
{
    struct EwramData **ewram = &gEwramData;
    struct EwramData_EntityData *player = (*ewram)->playerState.player;
    switch (entity->state)
    {
    case 0:
        entity->velX.whole = ((RandomNumberGenerator() & 0x1FFF) << 4) - 0x8000;
        if (entity->posX.whole < player->posX.whole) entity->velX.whole = -entity->velX.whole;
        entity->velY.whole = -(RandomNumberGenerator() & 0x3FFF) << 4;
        entity->state++;
        break;
    case 1:
    {
        s32 x = (s16)entity->posX.part16.integer;
        struct BgLayerState *camera = &(*ewram)->bgInfo[1];
        if ((u16)Collision_GetGroundOffset(x + (s16)camera->xPos.part16.integer,
                         (s16)entity->posY.part16.integer + (s16)camera->yPos.part16.integer + 4))
        {
            entity->animFrame = (RandomNumberGenerator() & 1) + 0x40;
            entity->velY.whole = -(entity->velY.whole >> 1);
            entity->state++;
        }
        else
        {
            entity->posX.whole += entity->velX.whole;
            entity->posY.whole += entity->velY.whole;
            entity->velY.whole += 0x3000;
        }
        break;
    }
    case 2:
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        entity->velY.whole += 0x3000;
        break;
    }
    if (!Entity_IsInRect(entity, -16, -16, 272, 192)) entity->deleteRequested = 1;
}

/* Bouncing debris using graphics/palette information from two tables. */
/**
 * @brief Object 0x09 debris piece: load the graphics of its kind from
 * sBreakableBlockVariants, fly ballistically and delete off screen.
 */
void Object09Debris_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
    case 0:
    {
        struct BreakableBlockVariant *kind = &sBreakableBlockVariants[entity->spawnParam0];
        struct BreakableBlockGraphics *graphics = &sBreakableBlockGraphics[kind->graphicsIndex];
        if (!(u8)Entity_LoadGraphics(entity, graphics->graphics, graphics->palette, kind->paletteIndex, graphics->spriteSheet))
        {
            EntityDelete(entity);
            return;
        }
        entity->animFrame = (RandomNumberGenerator() & 1) + 1;
        entity->velX.whole = ((RandomNumberGenerator() & 0x1FFF) << 4) + 0x20000;
        entity->velY.whole = -0x10000 - ((RandomNumberGenerator() & 0x3FFF) << 4);
        entity->accelY.whole = 0x3000;
        entity->state++;
    }
    /* fall through */
    case 1:
        EntityAccelerateAndMove(entity, 0x80000);
        break;
    }
    if (!Entity_IsInRect(entity, -16, -16, 272, 192)) entity->deleteRequested = 1;
}

/**
 * @brief 4FA88 | Burst of 3 x 3 debris pieces (DebrisPiece_Update) at the owner's
 * position, rows dy - 16, dy, dy + 16, with random frame 0x40..0x42 and
 * random flips; stops a row when no entity slot is free.
 */
void Debris_SpawnBurst(struct EwramData_EntityData *owner, s32 dy)
{
    s32 graphics = ObjGfx_Find((void *)sCommonObjectGfx);
    s32 y;
    s32 n;
    struct EwramData_EntityData *piece;
    s8 palette = 6;

    for (y = -0x10; y <= 0x10; y += 0x10)
    {
        for (n = 3; n > 0; n--)
        {
            piece = EntityCreateInRange(0x49, 0xCF, DebrisPiece_Update);
            if (piece == NULL)
                break;
            Sprite_Attach(piece, (void *)sCommonObjectSpriteSheet, graphics, palette);
            piece->animFrame = Mod(RandomNumberGenerator() >> 4, 3) + 0x40;
            piece->posX.part16.integer = owner->posX.part16.integer;
            piece->posY.part16.integer = owner->posY.part16.integer + dy + y;
            piece->flipX = (RandomNumberGenerator() >> 7) & 1;
            piece->flipY = (RandomNumberGenerator() >> 7) & 1;
            piece->spriteLayer = 0x15;
            piece->scrollWithBg = 1;
            piece->drawFunc = Sprite_DrawEntity;
        }
    }
}

/* Spawn falling pieces around the entity, then finish. (flipX written through an s8 byte pointer so the
 * mask stays before the address; drawSkip = 1 shares the flipY constant). */

/**
 * @brief Child of object 0x08 (breakable block) when hit: for 32 frames spawn
 * dust pieces around the block (spread by its orientation), then
 * delete itself.
 */
void BreakableBlock_SpawnDust(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *piece;

    switch (entity->state)
    {
    case 0:
        entity->ext.object08Dust.gfx = ObjGfx_Find((u8 *)sCommonObjectGfx);
        entity->timer = 0x20;
        entity->state++;
    case 1:
        if ((entity->timer & 3) == 0)
        {
            piece = EntityCreateInRange(0x49, 0xCF, BreakableBlockDust_Update);
            if (piece != NULL)
            {
                Sprite_Attach(piece, (void *)sCommonObjectSpriteSheet, entity->ext.object08Dust.gfx, 6);
                Anim_StartIndexed(piece, 5, 0, 0);
                Anim_Update(piece);
                piece->spriteLayer = entity->spriteLayer;
                switch ((u8)entity->ext.object08.kind)
                {
                case 1:
                case 2:
                    piece->posX.part16.integer = entity->posX.part16.integer + RandomSigned(0x20);
                    piece->posY.part16.integer = entity->posY.part16.integer + RandomSigned(0x40);
                    break;
                case 3:
                case 4:
                    piece->posX.part16.integer = entity->posX.part16.integer + RandomSigned(0x40);
                    piece->posY.part16.integer = entity->posY.part16.integer + RandomSigned(0x20);
                    break;
                }
                { s32 f = (RandomNumberGenerator() >> 7) & 1; s8 *p = (s8 *)piece + 0x58; s32 t = f << 6; *p = (*p & ~0x40) | t; }
                { s32 f = (RandomNumberGenerator() >> 7) & 1; piece->flipY = f; }
                piece->scrollWithBg = 1;
                piece->drawFunc = Sprite_DrawEntity;
                piece->drawSkip = 1;
            }
        }
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        entity->deleteRequested = 1;
        break;
    }
}

/* BreakableBlock_SetCollision (breakable block tiles). */
/* Per variant: collisionOffsetX/collisionOffsetY and collisionWidth x collisionHeight in pixels. */

/**
 * @brief 4FCA0 | Call Collision_SetBlock for every 16 px tile of the area of
 * variant spawnParam0 around the entity: a column (mode 1, mode 2 shifted 16 px
 * left), a row (modes 3 / 4) or the whole rectangle (mode 0); the mode is
 * unk_4F8.
 */
void BreakableBlock_SetCollision(struct EwramData_EntityData *entity)
{
    struct BreakableBlockVariant *def = &sBreakableBlockVariants[entity->spawnParam0];
    s32 offsetX = def->collisionOffsetX;
    s32 offsetY = def->collisionOffsetY;
    s32 i;
    s32 j;

    switch ((u8)entity->ext.object08.kind)
    {
        case 2:
            offsetX -= 0x10;
        case 1:
            for (i = -def->collisionHeight / 2; i < def->collisionHeight / 2; i += 0x10)
            {
                s32 x = (s16)entity->posX.part16.integer;
                struct BgLayerState *bg = &gEwramData->bgInfo[1];

                Collision_SetBlock(x + (s16)bg->xPos.part16.integer + offsetX,
                             (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer + offsetY - i);
            }
            break;
        case 3:
        case 4:
            for (i = -def->collisionWidth / 2; i < def->collisionWidth / 2; i += 0x10)
            {
                s32 x = (s16)entity->posX.part16.integer;
                struct BgLayerState *bg = &gEwramData->bgInfo[1];

                Collision_SetBlock(x + (s16)bg->xPos.part16.integer + offsetX - i,
                             (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer + offsetY);
            }
            break;
        case 0:
            for (i = -def->collisionWidth / 2; i < def->collisionWidth / 2; i += 0x10)
            {
                for (j = -def->collisionHeight / 2; j < def->collisionHeight / 2; j += 0x10)
                {
                    s32 x = (s16)entity->posX.part16.integer;
                    struct BgLayerState *bg = &gEwramData->bgInfo[1];

                    Collision_SetBlock(x + (s16)bg->xPos.part16.integer + offsetX + i,
                                 (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer + offsetY + j);
                }
            }
            break;
    }
}

/* BreakableBlock_ClearCollision (breakable block tiles, Collision_ClearBlock variant). */
/* Per variant: collisionOffsetX/collisionOffsetY and collisionWidth x collisionHeight in pixels. */

/**
 * @brief 4FE20 | Call Collision_ClearBlock for every 16 px tile of the area of
 * variant spawnParam0 around the entity: a column (mode 1, mode 2 shifted 16 px
 * left), a row (modes 3 / 4) or the whole rectangle (mode 0); the mode is
 * unk_4F8.
 */
void BreakableBlock_ClearCollision(struct EwramData_EntityData *entity)
{
    struct BreakableBlockVariant *def = &sBreakableBlockVariants[entity->spawnParam0];
    s32 offsetX = def->collisionOffsetX;
    s32 offsetY = def->collisionOffsetY;
    s32 i;
    s32 j;

    switch ((u8)entity->ext.object08.kind)
    {
        case 2:
            offsetX -= 0x10;
        case 1:
            for (i = -def->collisionHeight / 2; i < def->collisionHeight / 2; i += 0x10)
            {
                s32 x = (s16)entity->posX.part16.integer;
                struct BgLayerState *bg = &gEwramData->bgInfo[1];

                Collision_ClearBlock(x + (s16)bg->xPos.part16.integer + offsetX,
                             (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer + offsetY - i);
            }
            break;
        case 3:
        case 4:
            for (i = -def->collisionWidth / 2; i < def->collisionWidth / 2; i += 0x10)
            {
                s32 x = (s16)entity->posX.part16.integer;
                struct BgLayerState *bg = &gEwramData->bgInfo[1];

                Collision_ClearBlock(x + (s16)bg->xPos.part16.integer + offsetX - i,
                             (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer + offsetY);
            }
            break;
        case 0:
            for (i = -def->collisionWidth / 2; i < def->collisionWidth / 2; i += 0x10)
            {
                for (j = -def->collisionHeight / 2; j < def->collisionHeight / 2; j += 0x10)
                {
                    s32 x = (s16)entity->posX.part16.integer;
                    struct BgLayerState *bg = &gEwramData->bgInfo[1];

                    Collision_ClearBlock(x + (s16)bg->xPos.part16.integer + offsetX + i,
                                 (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer + offsetY + j);
                }
            }
            break;
    }
}

/* Object 08: persistent flag, per-kind graphics and a centred hit box. */
void Object08Create(struct EwramData_EntityData *entity)
{
    struct BreakableBlockVariant *kind;
    struct BreakableBlockGraphics *graphics;
    u32 box;
    if (entity->spawnParam1 != 0 && (gEwramData->gameState.gameFlagsTail[(entity->spawnParam1) >> 5] & (1 << ((entity->spawnParam1) & 31)))) goto fail;
    kind = &sBreakableBlockVariants[entity->spawnParam0];
    graphics = &sBreakableBlockGraphics[kind->graphicsIndex];
    entity->flipX = 0;
    entity->spriteLayer = kind->spriteLayer;
    if (!(u8)Entity_LoadGraphics(entity, graphics->graphics, graphics->palette, kind->paletteIndex, graphics->spriteSheet))
    {
fail:
        EntityDelete(entity);
        return;
    }
    entity->animFrame = kind->animFrame;
    box = ((-kind->collisionWidth / 2) & 255) + (((-kind->collisionHeight / 2) & 255) << 8) + (kind->collisionWidth << 16) + (kind->collisionHeight << 24);
    entity->ext.object08.kind = kind->collisionMode;
    BreakableBlock_SetCollision(entity);
    Hit_Enable(entity, 8, NULL, 0);
    HitBox_SetBodyRaw(entity, box);
    entity->hit.flags |= 2;
    entity->hp = 3;
}

/**
 * @brief 5008C | Breakable object (spawnParam1 = flag index): while
 * Player_TestSoulStateFlags holds it sheds dust Effect_SpawnShared every 16th frame; when hit
 * (contact bits 0x1A) a debris object BreakableBlock_SpawnDust appears, each hit costs
 * one of hp (song 0xD3); the last one breaks it (Debris_SpawnBurst, song
 * 0xAB, BreakableBlock_ClearCollision), sets its flag in gameFlagsTail, updates the map
 * (GameFlag_FindRoomCoordinate / GameFlag_Set) and finishes it.
 */
void Object08Update(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    struct EwramData_EntityData *child;
    s32 r;
    u16 x, y;
    struct EwramData_EntityData *dust;

    if (entity->spawnParam1 == 0)
        return;
    if (Player_TestSoulStateFlags(0x1000000))
    {
        entity->timer++;
        if ((entity->timer & 0xF) == 0)
        {
            dust = Effect_SpawnShared(entity->posX.whole, entity->posY.whole, 12, 6);
            if (dust != NULL)
                dust->spriteLayer = entity->spriteLayer - 1;
        }
    }
    if (entity->state != 0)
        return;
    Hit_CheckWithPlayer(&result, entity);
    if (!(result.flags & 0x1A))
        return;
    ObjGfx_Find(sCommonObjectGfx);
    child = EntityCreateInRange(0x49, 0xCF, BreakableBlock_SpawnDust);
    if (child != NULL)
    {
        child->ext.object08.kind = entity->ext.object08.kind;
        child->spriteLayer = entity->spriteLayer - 1;
        child->posX.part16.integer = entity->posX.part16.integer;
        child->posY.part16.integer = entity->posY.part16.integer;
        child->scrollWithBg = 1;
    }
    if ((s16)entity->hp != 0)
    {
        PlaySong(0xD3);
        entity->hp--;
        return;
    }
    Debris_SpawnBurst(entity, 0);
    PlaySong(0xAB);
    BreakableBlock_ClearCollision(entity);
    gEwramData->gameState.gameFlagsTail[entity->spawnParam1 >> 5] |= 1 << (entity->spawnParam1 & 0x1F);
    x = GetEntityRoomXPositionInteger(entity);
    y = GetEntityRoomYPositionInteger(entity);
    r = GameFlag_FindRoomCoordinate((s16)x, (s16)y, (u8)entity->ext.object08.kind);
    if (r >= 0)
        GameFlag_Set(r);
    entity->deleteRequested = 1;
}

/* Object09Create (breakable block). */

/* Per variant (see also this file): graphics set, palette, frame,
 * area offset / size, layer and mode. */

/* Event flag param_1 of the bit array gameFlagsTail (as in Object35Create). */
#define OBJECT_FLAG(ewram, flag) ((ewram)->gameState.gameFlagsTail[(flag) >> 5] & (1 << ((flag) & 0x1F)))

/**
 * @brief 501D0 | Object 0x09 create: deleted when its flag spawnParam1 is set or
 * the graphics cannot be loaded; otherwise frame / mode from its variant
 * and BreakableBlock_SetCollision.
 */
void Object09Create(struct EwramData_EntityData *entity)
{
    struct EwramData *ewram = gEwramData;
    struct BreakableBlockVariant *def;
    struct BreakableBlockGraphics *gfxDef;

    if (OBJECT_FLAG(ewram, entity->spawnParam1))
    {
        EntityDelete(entity);
        return;
    }
    def = &sBreakableBlockVariants[entity->spawnParam0];
    gfxDef = &sBreakableBlockGraphics[def->graphicsIndex];
    entity->flipX = 0;
    entity->spriteLayer = def->spriteLayer;
    if (!(u8)Entity_LoadGraphics(entity, gfxDef->graphics, gfxDef->palette, (u8)def->paletteIndex, gfxDef->spriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    entity->animFrame = def->animFrame;
    entity->ext.object09.collisionMode = def->collisionMode;
    BreakableBlock_SetCollision(entity);
}

/**
 * @brief 50270 | Breakable wall that crumbles once its flag (spawnParam1 in
 * gameFlagsTail) is set: dust Effect_SpawnShared every 16th frame while Player_TestSoulStateFlags
 * holds; when the room entry sBreakableBlockVariants[spawnParam0].collisionMode is set it just
 * opens (Debris_SpawnBurst(0)), otherwise up to 3 x 7 debris pieces
 * Object09Debris_Update scatter over the wall and it opens 32 px up; then song
 * 0xAB, BreakableBlock_ClearCollision, the map update (GameFlag_FindRoomCoordinate / GameFlag_Set) and it
 * finishes.
 */
void Object09Update(struct EwramData_EntityData *entity)
{
    u8 unused[16];
    struct EwramData_EntityData *dust;
    struct EwramData_EntityData *piece;
    s32 i, j, r;
    u16 x, y;
    const struct BreakableBlockVariant *info;

    if (Player_TestSoulStateFlags(0x1000000))
    {
        entity->timer++;
        if ((entity->timer & 0xF) == 0)
        {
            dust = Effect_SpawnShared(entity->posX.whole, entity->posY.whole, 12, 6);
            if (dust != NULL)
                dust->spriteLayer = entity->spriteLayer - 1;
        }
    }
    if (entity->state != 0)
        return;
    if (!(gEwramData->gameState.gameFlagsTail[entity->spawnParam1 >> 5] & (1 << (entity->spawnParam1 & 0x1F))))
        return;
    info = &sBreakableBlockVariants[entity->spawnParam0];
    if (info->collisionMode != 0)
        Debris_SpawnBurst(entity, 0);
    else
    {
        for (i = -1; i <= 1; i++)
        {
            for (j = -3; j <= 3; j++)
            {
                piece = EntityCreateInRange(0x49, 0xCF, Object09Debris_Update);
                if (piece == NULL)
                    break;
                piece->scrollWithBg = 1;
                piece->posX.part16.integer = entity->posX.part16.integer - (i << 3) + RandomSigned(4);
                piece->posY.part16.integer = entity->posY.part16.integer - ((j << 3) + 0x20) + RandomSigned(4);
                piece->spawnParam0 = entity->spawnParam0;
            }
        }
        Debris_SpawnBurst(entity, -0x20);
    }
    PlaySong(0xAB);
    BreakableBlock_ClearCollision(entity);
    x = GetEntityRoomXPositionInteger(entity);
    y = GetEntityRoomYPositionInteger(entity);
    r = GameFlag_FindRoomCoordinate((s16)x, (s16)y, (u8)entity->ext.object09.collisionMode);
    if (r >= 0)
        GameFlag_Set(r);
    entity->deleteRequested = 1;
}

/**
 * @brief Rise slowly and delete at the end of the animation or off screen.
 */
void BreakableBlockDust_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
    case 0:
        entity->velY.whole = -0xC000 - ((RandomNumberGenerator() & 0xFF) << 4);
        entity->state++;
        break;
    case 1:
        entity->posY.whole += entity->velY.whole;
        break;
    }
    if (!Entity_IsInRect(entity, -16, -16, 272, 192)) entity->deleteRequested = 1;
    if (Anim_Update(entity) == 3) entity->deleteRequested = 1;
}

/* Conditional creation from the common entity template table. */
void Object0ACreate(struct EwramData_EntityData *entity)
{
    if (((gEwramData->gameState.bossEventFlags >> entity->spawnParam1) & 1) == 0)
        EntityDelete(entity);
    else
    {
        struct EntityTemplate *entry;
        entity->enemyId = entity->spawnParam0;
        entry = (struct EntityTemplate *)(entity->enemyId * sizeof(*entry) + (u32)sEnemyData);
        entity->hp = entry->hp;
        entity->spawnParam0 = 0;
        entity->spawnParam1 = 0;
        entity->updateFunc = entry->updateFunc;
        entry->createFunc(entity);
    }
}

void Object0AUpdate(struct EwramData_EntityData *entity)
{
}

void Object0BCreate(struct EwramData_EntityData *entity)
{
    if (!GameFlag_GetMask(entity->spawnParam1))
        EntityDelete(entity);
    else
    {
        struct EntityTemplate *entry;
        entity->enemyId = entity->spawnParam0;
        entry = (struct EntityTemplate *)(entity->enemyId * sizeof(*entry) + (u32)sEnemyData);
        entity->hp = entry->hp;
        entity->spawnParam0 = 0;
        entity->spawnParam1 = 0;
        entity->updateFunc = entry->updateFunc;
        entry->createFunc(entity);
    }
}

void Object0BUpdate(struct EwramData_EntityData *entity)
{
}

/* Object0C_LaunchPlayerTrigger, decompiled from the original assembly. The store to
 * hitSourceX goes through a pointer type: with strict aliasing it may alias
 * the player pointer, which is then reloaded as in the target. */
/* Object 0x0C: rectangular interaction regions with a callback table. */

/**
 * @brief Trigger type 2: when bit 9 of soulStateFlags or player flag 0x400000 is set,
 * set player flag 0x4000000 and either mark ext.player.invulnerableTimer or start the scripted
 * player motion (hitReaction.. fields) with speed 0x30000 left (value 0) or
 * right, copying that speed to the entity velX.
 */
void Object0C_LaunchPlayerTrigger(struct EwramData_EntityData *entity, struct Object0CTrigger *trigger)
{
    s32 speed = 0x30000;
    struct EwramData *ewram = gEwramData;
    if ((ewram->playerStatus.soulStateFlags & 0x200)
        || (ewram->playerState.player->ext.player.actionFlags & 0x400000))
    {
        struct EwramData_EntityData **player = &ewram->playerState.player;
        (*player)->ext.player.actionFlags |= 0x4000000;
        if ((*player)->ext.player.invulnerableTimer != 0)
            (*player)->ext.player.invulnerableTimer = 1;
        else
        {
            ewram->playerState.pendingHitDamage = 1;
            ewram->playerState.hitReaction = 3;
            *(struct EwramData_EntityData **)&ewram->playerState.hitSourceX = (void *)((*player)->posX.whole + 0x80000);
            ewram->playerState.hitSourceY = (*player)->posY.whole;
            if (trigger->value == 0)
            {
                s32 negative = -speed;
                Player_SetKnockbackVelocity(negative, negative);
                entity->velX.whole = negative;
            }
            else
            {
                Player_SetKnockbackVelocity(speed, -0x30000);
                entity->velX.whole = speed;
            }
        }
    }
}

/* Object 0x0C: rectangular interaction regions with a callback table. */
static inline s32 TriggerContainsPoint(s16 x, s16 y, struct Object0CTrigger *trigger)
{
    if (x < trigger->x)
        return 0;
    if (x > trigger->x + trigger->width)
        return 0;
    if (y < trigger->y)
        return 0;
    if (y > trigger->y + trigger->height)
        return 0;
    return 1;
}

void Object0CUpdate(struct EwramData_EntityData *entity)
{
    struct Object0CTrigger *trigger = sObject0CTriggerLists[entity->spawnParam0];
    u16 x = GetEntityRoomXPositionInteger(gEwramData->playerState.player);
    u16 y = GetEntityRoomYPositionInteger(gEwramData->playerState.player);
    entity->velX.whole = 0;
    while (trigger->type != 3)
    {
        if (TriggerContainsPoint(x, y, trigger))
            sObject0CTriggerFuncs[trigger->type](entity, trigger);
        trigger++;
    }
    if (!entity->roomActive)
    {
        entity->deleteRequested = 1;
        Player_ClearSolidObject();
    }
}

/* Object helpers decompiled from assembly. */

/* Copy the second word of the supplied data to velX when the player's
 * unk_4F4 flag 0x400000 is active. */
void Object0C_ApplyTriggerVelocity(struct EwramData_EntityData *entity, s32 *data)
{
    if (gEwramData->playerState.player->ext.player.actionFlags & 0x400000)
        entity->velX.whole = data[1];
}

/**
 * @brief Trigger type 1: when the player has flag 0x400000 in unk_4F4, also set
 * flag 0x4000000.
 */
void Object0C_SetPlayerTriggerFlag(void)
{
    struct EwramData_EntityData *player = gEwramData->playerState.player;
    if (player->ext.player.actionFlags & 0x400000)
        player->ext.player.actionFlags |= 0x4000000;
}

/**
 * @brief Carry callback (slot 5) of the object 0x0C solid table sObject0CSolidCallbacks:
 * return its x velocity.
 */
s32 Object0C_GetCarryVelocity(struct EwramData_EntityData *entity)
{
    return entity->velX.whole;
}

void Object0CCreate(struct EwramData_EntityData *entity)
{
    Player_RegisterSolidObject(entity, sObject0CSolidCallbacks);
    entity->roomActive = 1;
}

void Object0DCreate(struct EwramData_EntityData *entity)
{
    u16 slot;
    if (!Entity_LoadGraphics(entity, sObject0DGfx, sObject0DPalette, 0, sObject0DSpriteSheet)) goto remove;
    slot = ObjAffine_Alloc(1);
    if ((s16)slot < 0)
    {
remove:
        EntityDelete(entity);
        return;
    }
    entity->affineIndex = slot;
    entity->affine = 1;
    entity->spriteLayer = 0x16;
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    Hit_Enable(entity, 8, NULL, 0);
    HitBox_SetBodyRaw(entity, 0x122006F0);
    entity->hit.flags |= 2;
    entity->hit.flags &= 0xFB;
    entity->scrollWithBg = 1;
}

/**
 * @brief 507BC | Swinging object (object 0x0D, hangs from the ceiling):
 * hidden below y 0xA8; when the player lands on it (standingPlatform) or hits it
 * (Hit_CheckWithPlayer flags 0x1A) it gets a swing impulse of 0x100 away from the
 * player (sound 0x112, amplitude unk_504 = 0x100). The angle unk_4FC swings
 * back towards 0 with acceleration 0x10, losing 0x20 of amplitude at each
 * zero crossing; speed and angle (+-0x1000) are clamped and drawn with
 * ObjAffine_Set.
 */
void Object0DUpdate(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    s32 angle;
    s32 impulse;

    if ((s16)entity->posY.part16.integer > 0xA8)
        entity->hidden = 1;
    else
        entity->hidden = 0;
    Player_QueueEntity(entity, 0);
    if (entity->subState == 0)
    {
        if (gEwramData->playerState.standingPlatform != entity)
            goto hit;
        impulse = 0x100;
        PlaySong(0x112);
        if (gEwramData->playerState.player->posX.whole < entity->posX.whole)
            impulse = -impulse;
        entity->ext.object0D.speed = impulse;
        entity->ext.object0D.amplitude = 0x100;
        entity->subState = 1;
    }
    else if (gEwramData->playerState.standingPlatform != entity)
        entity->subState = 0;
hit:
    if ((u16)Hit_CheckWithPlayer(&result, entity) && (result.flags & 0x1A))
    {
        impulse = 0x100;
        PlaySong(0x112);
        if (gEwramData->playerState.player->posX.whole < entity->posX.whole)
            impulse = -impulse;
        entity->ext.object0D.speed = impulse;
        entity->ext.object0D.amplitude = 0x100;
    }
    angle = entity->ext.object0D.angle;
    if (angle > 0)
    {
        entity->ext.object0D.speed -= 0x10;
        if ((s32)(entity->ext.object0D.angle = angle + entity->ext.object0D.speed) > 0)
            goto clamp;
        goto slow;
    }
    else if (angle < 0)
    {
        entity->ext.object0D.speed += 0x10;
        if ((s32)(entity->ext.object0D.angle = angle + entity->ext.object0D.speed) < 0)
            goto clamp;
    slow:
        if (entity->ext.object0D.amplitude != 0)
            entity->ext.object0D.amplitude -= 0x20;
    }
    else
        entity->ext.object0D.angle = angle + entity->ext.object0D.speed;
clamp:
    {
        s32 v = entity->ext.object0D.speed;
        s32 lim = entity->ext.object0D.amplitude;
        if (v > lim || v < (lim = -lim))
            entity->ext.object0D.speed = lim;
    }
    {
        if ((s32)entity->ext.object0D.angle > 0x1000)
            entity->ext.object0D.angle = 0x1000;
        else if ((s32)entity->ext.object0D.angle < -0x1000)
            entity->ext.object0D.angle = -0x1000;
    }
    ObjAffine_Set(entity->ext.object0D.angle, 0x10000, 0x10000, 0, 0, entity->affineIndex);
}

/* Object0EPiece_Update (falling object hitting the ground). */

/**
 * @brief 50920 | Fall; on hitting solid ground (Collision_GetGroundOffset) spawn a
 * bouncing piece (Effect_SpawnShared) and dust (Effect_SpawnRandom), play song 0x158
 * or 0x159 at random and finish (0x53D bit 3); also finish when far off
 * screen.
 */
void Object0EPiece_Update(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg;
    s32 x;

    EntityDampVelocityX(entity, 0x3C);
    EntityAccelerateAndMove(entity, 0x80000);
    x = (s16)entity->posX.part16.integer;
    bg = &gEwramData->bgInfo[1];
    if (Collision_GetGroundOffset(x + (s16)bg->xPos.part16.integer,
                     (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer))
    {
        struct EwramData_EntityData *effect = Effect_SpawnShared(entity->posX.whole, entity->posY.whole - 0x40000, 4, 6);

        if (effect != NULL)
        {
            s32 vy;

            effect->drawSkip = 1;
            vy = RandomSigned(0x1000);
            effect->velY.whole = vy - 0x4000;
            effect->accelY.whole = effect->velY.whole / 32;
            effect->velX.whole = RandomSigned(0x2000) - entity->velX.whole;
            effect->accelX.whole = -effect->velX.whole / 64;
        }
        Effect_SpawnRandom(entity->posX.whole, entity->posY.whole - 0x10000, 0x40000, 0x40000, entity->spriteLayer - 1);
        if ((RandomNumberGenerator() & 0x3FF) <= 0x1FF)
            PlaySong(0x158);
        else
            PlaySong(0x159);
    }
    else if (Entity_IsInRect(entity, -0x20, -0x20, 0x130, 0xE0))
    {
        return;
    }
    entity->deleteRequested = 1;
}

/**
 * @brief Spawn the variant's count of falling pieces (Object0EPiece_Update) with
 * consecutive frames, loading their graphics; stop when no entity slot or
 * graphics are available.
 */
void Object0E_SpawnPieces(struct EwramData_EntityData *owner)
{
    const struct Object0EVariant *info = &sObject0EVariants[owner->spawnParam0];
    const struct Object0EGraphics *gfx = &sObject0EGraphics[info->graphics];
    s32 frame = info->firstPieceFrame;
    s32 i;
    struct { s16 x, y; } pos;
    for (i = 0; i < info->pieceCount; i++, frame++)
    {
        struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, Object0EPiece_Update);
        if (!entity)
            return;
        if (!(u8)Entity_LoadGraphics(entity, gfx->graphics, gfx->palette, (u8)info->palette, gfx->sprites))
        {
            EntityDelete(entity);
            return;
        }
        entity->animFrame = frame;
        entity->spriteLayer = 0x14;
        entity->flipX = owner->flipX;
        Sprite_GetPartCenterOffset(owner, i, (s16 *)&pos);
        entity->posX.part16.integer = owner->posX.part16.integer + pos.x;
        entity->posY.part16.integer = owner->posY.part16.integer + pos.y;
        Sprite_GetPartOriginOffset(entity, 0, (s16 *)&pos);
        entity->posX.part16.integer += pos.x / 4;
        entity->posY.part16.integer -= pos.y / 4;
        entity->velX.whole = owner->velX.whole * ((RandomNumberGenerator() & 0xFFFF) + 0x10000);
        entity->velY.whole = -0x20000 - RandomSigned(0x20000);
        entity->accelY.whole = 0x2000;
        entity->scrollWithBg = 1;
    }
}

/* Draw the entity one pixel lower, then its alternate frame every other pair of frames. */
/**
 * @brief Unless hidden, animate and draw one pixel lower; every other two frames
 * also draw the variant's overlay frame when it has one.
 */
void Object0E_Draw(struct EwramData_EntityData *entity)
{
    struct Object0EVariant *info;
    if (entity->hidden) return;
    info = &sObject0EVariants[entity->spawnParam0];
    Anim_Update(entity);
    entity->posY.part16.integer++;
    Sprite_Draw(entity);
    entity->posY.part16.integer--;
    if (info->overlayFrame != 0 && (gEwramData->frameCounter & 2))
    {
        u8 frame = entity->animFrame;
        entity->animFrame = info->overlayFrame;
        Sprite_Draw(entity);
        entity->animFrame = frame;
    }
}

void Object0ECreate(struct EwramData_EntityData *entity)
{
    struct Object0EVariant *variant = &sObject0EVariants[entity->spawnParam0];
    struct Object0EGraphics *graphics = &sObject0EGraphics[variant->graphics];
    if (!(u8)Entity_LoadGraphics(entity, graphics->graphics, graphics->palette, (u8)variant->palette, graphics->sprites))
    {
        EntityDelete(entity);
        return;
    }
    entity->spriteLayer = 0x14;
    Anim_StartIndexed(entity, variant->animation, 0, 1);
    Anim_Update(entity);
    entity->drawFunc = Object0E_Draw;
    Hit_Enable(entity, 0, NULL, Object0E_OnHit);
    entity->hit.flags |= 2;
    HitBox_SetBodyRaw(entity, variant->box);
    if (variant->skipGroundSnap == 0) Entity_SnapToGround(entity);
    entity->scrollWithBg = 1;
}

void Object0EUpdate(struct EwramData_EntityData *entity)
{
    u8 *entry;
    if (!Entity_IsInRect(entity, -0x20, -0x20, 0x130, 0x120))
    {
        entity->hidden = 1;
        return;
    }
    switch (entity->state)
    {
    case 0:
        entity->hidden = 0;
        entry = sUnk_08526370[entity->spawnParam1];
        if (Pickup_TrySpawnOnHit(entity, -20, entry[0], entry[1]))
        {
            Object0E_SpawnPieces(entity);
            PlaySong(0xA2);
            entity->hidden = 1;
            entity->timer = 0x28;
            entity->state++;
        }
        break;
    case 1:
        if (entity->timer > 30 && (entity->timer & 3) == 0)
            Effect_SpawnRandom(entity->posX.whole, entity->posY.whole - 0x100000, 0x80000, 0x100000, entity->spriteLayer - 1);
        if (--entity->timer == 0xFF) entity->deleteRequested = 1;
        break;
    }
}

/* Object0E_OnHit. */
/**
 * @brief 50D98 | velX = 1, or -1 when entity is left of attacker.
 */
void Object0E_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker)
{
    s32 speed = 1;

    entity->velX.whole = speed;
    if (entity->posX.whole < attacker->posX.whole)
        entity->velX.whole = -speed;
}

/* Object 0x0F, reconstructed from the original ROM. */

/* Select palette from the high byte of the placement parameter (default 7),
 * retain its low byte, and install drawing/collision data. */
void Object0FCreate(struct EwramData_EntityData *entity)
{
    s32 palette = (u16)entity->spawnParam0 >> 8;
    s32 graphics;
    if (palette == 0)
        palette = 7;
    entity->spawnParam0 &= 0xFF;
    entity->spriteLayer = 0x14;
    entity->flipX = 0;
    graphics = ObjGfx_Find(sCommonObjectGfx);
    Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, palette);
    Anim_StartIndexed(entity, 0xE, 0, 1);
    Anim_Update(entity);
    Hit_Enable(entity, 0, 0, 0);
    entity->hit.flags |= 2;
    /* Packed box {-6, -8, 12, 8}, passed by value. */
    HitBox_SetBodyRaw(entity, 0x080CF8FA);
    entity->drawFunc = Object0F_Draw;
}

/* Activation starts a 40-frame countdown, with particles during its first
 * ten frames. The object is marked for removal when the counter wraps. */
void Object0FUpdate(struct EwramData_EntityData *entity)
{
    s32 x;
    s32 y;
    if (!Entity_IsInRect(entity, -0x10, -0x10, 0x110, 0xC0))
        entity->hidden = 1;
    else
    {
        switch (entity->state)
        {
        case 0:
            entity->hidden = 0;
            if (Pickup_TrySpawnOnHit(entity, 0, entity->spawnParam0, entity->spawnParam1))
            {
                PlaySong(0xA2);
                entity->hidden = 1;
                entity->timer = 0x28;
                entity->state++;
            }
            break;
        case 1:
            if (entity->timer > 0x1E && !(entity->timer & 3))
            {
                x = entity->posX.whole + RandomSigned(0x80000);
                y = entity->posY.whole + RandomSigned(0x80000);
                Effect_SpawnShared(x, y, 0, entity->palette);
            }
            if (--entity->timer == 0xFF)
                entity->deleteRequested = 1;
            break;
        }
    }
}

/* Object0F_Draw (same code as SaveSelect_Open). */
/**
 * @brief 50F08 | Animate, then Sprite_Draw.
 */
void Object0F_Draw(struct EwramData_EntityData *entity)
{
    Anim_Update(entity);
    Sprite_Draw(entity);
}
