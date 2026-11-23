/*
 * Save point animation parts, orbit lights, sparkles and room objects 0x1D-0x1E.
 */
#include "core.h"
#include "save_point_entity.h"
#include "graphics.h"
#include "structs/ewram.h"
#include "structs/entity_render_flags.h"
#include "gameplay.h"
#include "room_object_entity.h"
#include "bg_and_collision.h"

extern void SavePoint_UpdateAnimPart(struct EwramData_EntityData *);
extern u8 sSaveWarpRoomSpriteSheet[];

struct PartInit
{
    s32 anim;
    s32 palette;
    void *update;
    s32 layer;
};

/**
 * @brief Spawn the save point part sprites 0 and 3 of the four-entry part
 * table (update SavePoint_UpdateAnimPart) at the parent position.
 * Return 1 when at least one part was created.
 */
s32 SavePoint_SpawnAnimParts(struct EwramData_EntityData *parent)
{
    struct EwramData_EntityData *entity;
    s32 created = 0;
    s32 i;
    struct PartInit parts[4] = {
        { 0, parent->ext.savePoint.frameBase + 2, SavePoint_UpdateAnimPart, 0x13 },
        { 2, parent->ext.savePoint.frameBase + 3, SavePoint_UpdateAnimPart, 0x14 },
        { 7, parent->ext.savePoint.frameBase + 3, SavePoint_UpdateAnimPart, 0x14 },
        { 4, parent->ext.savePoint.frameBase + 1, SavePoint_UpdateAnimPart, 0x14 },
    };

    for (i = 0; i <= 3; i++)
    {
        if ((u32)(i - 1) <= 1)
            continue;
        entity = EntityCreateInRange(0x1B, 0x48, (u32 *)EntityUpdateNothing);
        if (entity == NULL)
            continue;
        entity->posX.whole = parent->posX.whole;
        entity->posY.whole = parent->posY.whole;
        if (parts[i].anim == 2 || parts[i].anim == 7)
        {
            entity->posY.part16.integer -= 0x9F;
            if (parts[i].anim == 2)
                entity->posX.part16.integer -= 0x30;
            else
                entity->posX.part16.integer += 0x30;
        }
        entity->scrollWithBg = 1;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        Sprite_Attach(entity, sSaveWarpRoomSpriteSheet, parent->ext.savePoint.graphics, parts[i].palette);
        Anim_StartIndexed(entity, parts[i].anim, 0, 1);
        Anim_Update(entity);
        entity->spriteLayer = parts[i].layer;
        entity->updateFunc = (u32 *)parts[i].update;
        entity->ext.savePointPart.owner = parent;
        entity->timer = 4;
        created = 1;
    }
    return created;
}

extern void SavePoint_UpdateOrbitLight(struct EwramData_EntityData *);

static inline void AssignPalette(struct EwramData_EntityData *e, struct EwramData_EntityData *owner) { e->affineIndex = owner->ext.raw.unk_4F8.part8.unk_4FB; }
/**
 * @brief After a successful save, spawn eight lights (update
 * SavePoint_UpdateOrbitLight) around the save point. Return 1 when any
 * was created.
 */
s32 SavePoint_SpawnOrbitLights(struct EwramData_EntityData *parent)
{
    struct EwramData_EntityData *entity;
    s32 created = 0;
    struct PartInit parts[1] = {
        { 2, parent->ext.savePoint.frameBase + 3, SavePoint_UpdateOrbitLight, 0x11 },
    };
    s32 i;
    s32 j;

    j = 0;
    for (i = 0; i <= 7; i++)
    {
        entity = EntityCreateInRange(0x1B, 0x48, (u32 *)EntityUpdateNothing);
        if (entity == NULL)
            continue;
        entity->posX.whole = parent->posX.whole;
        entity->posY.whole = parent->posY.whole;
        entity->scrollWithBg = 1;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        Sprite_Attach(entity, sSaveWarpRoomSpriteSheet, parent->ext.savePoint.graphics, parts[j].palette);
        Anim_StartIndexed(entity, parts[j].anim, 0, 1);
        entity->spriteLayer = parts[j].layer;
        entity->updateFunc = (u32 *)parts[j].update;
        entity->ext.savePointPart.owner = parent;
        entity->ext.savePointLight.index = i;
        entity->ext.savePointLight.radius = 0x80000;
        entity->ext.savePointLight.phase = 0;
        SAVE_POINT_LIGHT_SPIN(entity) = 0;
        entity->state = 0;
        entity->timer = 2;
        AssignPalette(entity, parent);
        entity->affine = 1;
        created = 1;
    }
    return created;
}

/* Animation-driven completion and request flags for the object 0x1C effect. */
struct EffectOwnerState { u8 pad[0x24]; u16 flags; u8 pad26[10]; u8 done, request; };
/**
 * @brief Wait for owner flag 2, play the next animation, request the owner at
 * animation step 2 and delete itself when done.
 */
void SavePoint_UpdateAnimPart(struct EwramData_EntityData *entity)
{
    struct EffectOwnerState *owner = (struct EffectOwnerState *)entity->ext.savePointPart.owner;
    switch (entity->state)
    {
    case 0:
        if (owner->flags & 2) entity->state = 1;
        break;
    case 1:
        switch (entity->animScriptIndex)
        {
        case 0: Anim_StartIndexed(entity, 1, 0, 1); break;
        case 4: Anim_StartIndexed(entity, 5, 0, 1); break;
        }
        entity->state++;
        break;
    case 2:
        switch (entity->animScriptIndex)
        {
        case 2:
        case 7:
            if (owner->done == 1) entity->deleteRequested = 1;
            break;
        case 1:
        case 5:
            if (entity->animStep == 2) owner->request = 1;
            else if (entity->animDone)
            {
                if (--entity->timer == 0)
                {
                    entity->deleteRequested = 1;
                    owner->done = 1;
                }
            }
            break;
        }
        break;
    }
    Anim_Update(entity);
}

/* Orbiting child: spiral outward around a fixed point (state 0), then orbit and shrink around the owner (state 1). */
extern s32 Target_GetPixelX(struct EwramData_EntityData *);
extern s32 Target_GetPixelY(struct EwramData_EntityData *);
void SaveSparkle_SpawnAround(struct EwramData_EntityData *entity, u16 width, u16 height);

/**
 * @brief Spiral outwards, then orbit inwards onto the player; tell the owner
 * (SAVE_POINT_LIGHTS_OUT_DONE, SAVE_POINT_LIGHTS_IN_DONE) when each phase
 * ends. Emits sparkles.
 */
void SavePoint_UpdateOrbitLight(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner;
    s32 state;
    s32 s;
    s32 c;
    s32 r;
    u32 step;
    s32 rad;

    owner = entity->ext.savePointPart.owner;
    state = entity->state;
    switch (state)
    {
    case 0:
        s = Sin(entity->ext.savePointLight.index << 13);
        c = Sin((entity->ext.savePointLight.index << 13) + 0x4000);
        rad = entity->ext.savePointLight.radius;
        r = rad >> 16;
        s *= r;
        c *= r;
        entity->posY.whole = s + 0x500000;
        entity->posX.whole = c + 0x760000;
        if ((u32)entity->ext.savePointLight.phase <= 0x1DFFFF)
            step = (u32)entity->ext.savePointLight.phase >> 16;
        else
            step = ((u32)entity->ext.savePointLight.phase >> 16) * 3;
        entity->ext.savePointLight.radius = rad + (step << 13);
        entity->ext.savePointLight.phase += 0x10000;
        SAVE_POINT_LIGHT_SPIN(entity) += 0x800;
        if (*(vs32 *)&entity->ext.savePointLight.radius > 0xFFFFFF)
        {
            entity->state++;
            entity->timer = 2;
            entity->spriteLayer = 9;
            entity->ext.savePointLight.radius = 0x1000000;
            SAVE_POINT_LIGHTS_OUT_DONE(owner) = 1;
            entity->ext.savePointLight.phase = 0;
        }
        break;
    case 1:
        s = Sin((entity->ext.savePointLight.index << 13) + entity->ext.savePointLight.phase);
        c = Sin((entity->ext.savePointLight.index << 13) + ({ s32 t_ = entity->ext.savePointLight.phase; t_ + 0x4000; }));
        r = SAVE_POINT_LIGHT_RADIUS_PX(entity);
        s *= r;
        c *= r;
        entity->posX.whole = c + 0x780000;
        entity->posY.whole = s + 0x500000;
        entity->posX.whole += (Target_GetPixelX(entity) << 16) - owner->posX.whole;
        entity->posY.whole = entity->posY.whole + ((Target_GetPixelY(entity) << 16) - owner->posY.whole) + 0x200000;
        entity->ext.savePointLight.radius -= 0x30000;
        entity->ext.savePointLight.phase += 0x200;
        if (entity->ext.savePointLight.radius <= 0)
        {
            SAVE_POINT_LIGHTS_IN_DONE(entity->ext.savePointPart.owner) = state;
            entity->deleteRequested = 1;
        }
        break;
    }
    if ((++entity->timer & 3) == 0)
        SaveSparkle_SpawnAround(entity, 0x10, 0x10);
    Anim_Update(entity);
}

/* Particles 0x080552C4..0x080554BB. */
extern void SaveSparkle_Draw(struct EwramData_EntityData *entity);
void SaveSparkle_Update(struct EwramData_EntityData *entity);

extern u8 sSaveWarpRoomGfx[];
void SaveSparkle_Create(s32 x, s32 y, s32 accelX, s32 accelY, s32 phase, u8 palette, struct EwramData_EntityData *entity);

/* The entity that spawned the sparkle, stored as a word. */
#define SPARKLE_OWNER(e) ENTITY_WORD_518(e)

/**
 * @brief Set up the sparkle animation, fly 0xC frames with acceleration
 * accelX / accelY, then rise 8 frames and hide.
 */
void SaveSparkle_Update(struct EwramData_EntityData *entity)
{
    s32 gfx;

    switch (entity->state)
    {
        case 0:
            gfx = ObjGfx_Find(sSaveWarpRoomGfx);
            Sprite_Attach(entity, sSaveWarpRoomSpriteSheet, gfx, entity->palette);
            entity->drawFunc = (u32 *)SaveSparkle_Draw;
            Anim_StartIndexed(entity, 3, 0, 1);
            entity->timer = 0xC;
            entity->scrollWithBg = 1;
            entity->velY.whole = -0x10000;
            entity->state++;
            break;
        case 1:
            entity->posX.whole += entity->velX.whole;
            entity->posY.whole += entity->velY.whole;
            entity->velX.whole += entity->accelX.whole;
            entity->velY.whole += entity->accelY.whole;
            if (--entity->timer == 0)
            {
                entity->deleteRequested = 1;
                entity->state = 2;
                entity->timer = 0;
            }
            break;
        case 2:
            entity->posY.whole -= entity->velY.whole;
            if (++entity->timer > 8)
                entity->deleteRequested = 1;
            break;
    }
    Anim_Update(entity);
}

/**
 * @brief Spawn a sparkle at a random spot inside a width x height box around
 * the entity, with its velocity (accelX / accelY) and palette.
 */
void SaveSparkle_SpawnAround(struct EwramData_EntityData *entity, u16 width, u16 height)
{
    s32 x = entity->posX.whole + ((u16)((RandomNumberGenerator() & (width - 1)) - (width >> 1)) << 16);
    s32 y = entity->posY.whole + ((u16)((RandomNumberGenerator() & (height - 1)) - (height >> 1)) << 16);

    SaveSparkle_Create(x, y, entity->accelX.whole, entity->accelY.whole, 0, entity->palette, entity);
}

/**
 * @brief Create a sparkle (update SaveSparkle_Update) at (x, y) with velocity,
 * lifetime, a random drift, palette and owner, one layer below it.
 */
void SaveSparkle_Create(s32 x, s32 y, s32 accelX, s32 accelY, s32 phase, u8 palette, struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)SaveSparkle_Update);

    if (child != NULL)
    {
        child->posX.whole = x;
        child->posY.whole = y;
        child->accelX.whole = accelX;
        child->accelY.whole = accelY;
        child->ext.raw.unk_4FC.whole = phase;
        child->ext.particlePair.offsetX = ((RandomNumberGenerator() & 0x1F) << 16) - 0x100000;
        child->ext.particlePair.offsetY = ((RandomNumberGenerator() & 0x1F) << 16) - 0x100000;
        child->palette = palette;
        SPARKLE_OWNER(child) = (s32)entity;
        child->spriteLayer = entity->spriteLayer + 1;
        {
            s32 affineSlot = entity->affineIndex;

            child->affineIndex = affineSlot;
        }
    }
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   SaveSparkle_Draw = Effect_DrawAtOffsetPair */

/**
 * @brief Same as Effect_DrawAtOffsetPair.
 */
void SaveSparkle_Draw(struct EwramData_EntityData *entity)
{
    Sprite_Draw(entity);
    entity->posX.whole += entity->ext.particlePair.offsetX;
    entity->posY.whole += entity->ext.particlePair.offsetY;
    Sprite_Draw(entity);
    entity->posX.whole -= entity->ext.particlePair.offsetX;
    entity->posY.whole -= entity->ext.particlePair.offsetY;
}

extern u8 sWarpRoomGfx[], sWarpRoomPalette[], sWarpRoomSpriteSheet[];
extern u8 sSaveWarpRoomGfx[], sSaveWarpRoomPalette[], sSaveWarpRoomSpriteSheet[];

struct RoomExit { u8 pad[4]; s8 direction; u8 rest[11]; };
struct ExitRoom { u8 pad[0x18]; struct RoomExit *exits; u16 count; };

static inline s32 RoomX(void) { return (s16)gEwramData->bgInfo[1].xPos.part16.integer; }
static inline s32 RoomY(void) { return (s16)gEwramData->bgInfo[1].yPos.part16.integer; }

static inline s32 ExitSum(void)
{
    struct ExitRoom *room; s32 sum, i;
    { struct GameState *ctx = &gEwramData->gameState; room = (struct ExitRoom *)ctx->currentRoomDefinition; }
    sum = 0;
    for (i = 0; i < room->count; i++)
        sum += room->exits[i].direction;
    return sum;
}

void Object1DCreate(struct EwramData_EntityData *entity)
{
    u8 *gfx, *palette, *sprite;
    s32 layer, graphics, sum, i;
    struct ExitRoom *room;
    struct RoomExit *exit;
    if (entity->spawnParam0 == 0)
    {
        gfx = sWarpRoomGfx;
        palette = sWarpRoomPalette;
        sprite = sWarpRoomSpriteSheet;
        layer = 0xC;
    }
    else
    {
        gfx = sSaveWarpRoomGfx;
        palette = sSaveWarpRoomPalette;
        sprite = sSaveWarpRoomSpriteSheet;
        layer = 0xA;
    }
    graphics = ObjGfx_Find(gfx);
    if (graphics < 0)
        graphics = ObjGfx_LoadResource(gfx, 0);
    ObjPalette_LoadRange(palette, 0, 1, layer);
    Sprite_Attach(entity, sprite, graphics, layer);
    entity->spriteLayer = 0x11;
    sum = ExitSum();
    if (entity->spawnParam0 == 0)
    {
        entity->animFrame = 0xB;
        if (sum == 1)
        {
            entity->posX.part16.integer -= 0xD0;
            entity->ext.object1D.exitRight = 1;
            entity->flipX = 0;
        }
        else if (sum == -1)
        {
            entity->ext.object1D.exitRight = 0;
            entity->flipX = 1;
        }
        else if (sum == 0)
        {
            EntityDelete(entity);
            return;
        }
    }
    else
    {
        entity->animFrame = 0x14;
        if (sum == 1)
        {
            entity->posX.part16.integer = 0x10;
            entity->posY.part16.integer = 0x70;
            entity->ext.object1D.exitRight = 1;
            entity->flipX = 0;
        }
        else if (sum == -1)
        {
            entity->posX.part16.integer = 0xE0;
            entity->posY.part16.integer = 0x70;
            entity->ext.object1D.exitRight = 0;
            entity->flipX = 1;
        }
        else if (sum == 0)
        {
            EntityDelete(entity);
            return;
        }
    }
    for (i = 0; i <= 0x30; i += 0x10)
    {
        s32 x = (s16)entity->posX.part16.integer;
        struct BgLayerState *bg = &gEwramData->bgInfo[1];
        s32 y;
        x += (s16)bg->xPos.part16.integer;
        x -= entity->flipX * 16;
        y = (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer;
        Collision_SetBlock(x, y - i);
    }
}

/* Object 0x1D has no per-frame update. */
void Object1DUpdate(struct EwramData_EntityData *entity)
{
}

/* Unreferenced draw routine next to Object1DUpdate (warp room): draw the
 * object, then on alternating frame pairs a pulsing affine copy (frame 6) 28 px
 * above it. */
void Object1D_DrawPulsingOverlay(struct EwramData_EntityData *entity)
{
    u8 frame = entity->animFrame;
    entity->affine = 0;
    Sprite_Draw(entity);
    entity->affine = 1;
    if (gEwramData->frameCounter & 2)
    {
        s32 scale = (Sin(gEwramData->frameCounter << 11) >> 3) + 0xE000;
        ObjAffine_Set(0, scale, scale, 0, 0, (u8)entity->ext.raw.unk_4FC.part8.unk_4FC);
        entity->objMode = 1;
        entity->posY.part16.integer -= 28;
        entity->animFrame = 6;
        Sprite_Draw(entity);
        entity->objMode = 0;
        entity->posY.part16.integer += 28;
    }
    entity->animFrame = frame;
}

void Object1ECreate(struct EwramData_EntityData *entity)
{
}

void Object1EUpdate(struct EwramData_EntityData *entity)
{
}

/* Unreferenced: advance the animation (next to the empty Object1E callbacks). */
void Object1E_AnimUpdate(struct EwramData_EntityData *entity)
{
    Anim_Update(entity);
}
