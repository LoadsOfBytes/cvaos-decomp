/*
 * Shared effects: timed / attached particles, player flash, afterimages, effect tables, shared animations and pickup sources.
 */
#include "core.h"
#include "graphics.h"
#include "sound.h"
#include "structs/ewram.h"
#include "in_game.h"
#include "bg_effects.h"
#include "structs/entity_render_flags.h"
#include "soul_inventory.h"
#include "entity_util.h"
#include "types.h"
#include "gameplay.h"
#include "effects.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "hitbox.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "palette.h"
#include "pickup.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"
/* from bg_and_collision.h (not included: see the module notes) */
extern struct DmaQueueInfo* DmaQueue_IndirectCopy(u32 size, u32 *src, u32 *dst);

/* Effects with an entry of table 085251A8[unk_4FC] (frame, lifetime,
 * animation), graphics 081C15F4 / 0820ED60 with their own palette
 * (palette). */
struct Unk_085251A8 { u8 frame; u8 life; u16 pad; struct AnimScript *anim; };
extern struct Unk_085251A8 sUnk_085251A8[];
extern void Effect_DrawAtOffsetPair(struct EwramData_EntityData *);

/* Particle: random unk_500 / unk_504 in [-16, 16) px, drawn by
 * Effect_DrawAtOffsetPair, rising and accelerating until its lifetime runs out. */
/**
 * @brief Initialize a table-selected frame, random secondary draw offset and lifetime, then move with acceleration until the countdown expires.
 */
void Effect_UpdateTimedDoubleParticle(struct EwramData_EntityData *entity)
{
    struct Unk_085251A8 *info = &sUnk_085251A8[entity->ext.particleTable.index];
    switch (entity->state)
    {
    case 0:
    {
        s32 graphics = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, entity->palette);
        entity->ext.particlePair.offsetX = ((RandomNumberGenerator() & 0x1F) << 16) - 0x100000;
        entity->ext.particlePair.offsetY = ((RandomNumberGenerator() & 0x1F) << 16) - 0x100000;
        entity->drawFunc = (u32 *)Effect_DrawAtOffsetPair;
        entity->animFrame = info->frame;
        entity->velY.whole = -0x10000;
        entity->timer = info->life;
        entity->state = 1;
        entity->scrollWithBg = 1;
        break;
    }
    case 1:
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        entity->velX.whole += entity->accelX.whole;
        entity->velY.whole += entity->accelY.whole;
        if (--entity->timer == 0)
            entity->deleteRequested = 1;
        break;
    }
}

/* Animated effect moving with acceleration until the animation ends. */
/**
 * @brief Initialize the table-selected animation, move with acceleration in state one and request deletion when Anim_Update returns three.
 */
void Effect_UpdateTableAnimation(struct EwramData_EntityData *entity)
{
    struct Unk_085251A8 *info = &sUnk_085251A8[entity->ext.particleTable.index];
    switch (entity->state)
    {
    case 0:
    {
        s32 graphics = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, entity->palette);
        Anim_Start(entity, info->anim, 0, 0);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->state = 1;
        entity->scrollWithBg = 1;
        break;
    }
    case 1:
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        entity->velX.whole += entity->accelX.whole;
        entity->velY.whole += entity->accelY.whole;
        break;
    }
    if (Anim_Update(entity) == 3)
        entity->deleteRequested = 1;
}

/* Effect pinned to the entity at unk_4FC at offset (accelX, accelY) with
 * sound 0xBF and animation unk_4F8 until the animation ends; removed at
 * once without a parent. */
/**
 * @brief Follow the parent at stored offsets and animate until completion; request deletion immediately for a NULL parent.
 */
void Effect_UpdateAttachedAnimation(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.particleAttach.parent;
    if (parent == NULL)
    {
        entity->deleteRequested = 1;
        return;
    }
    switch (entity->state)
    {
    case 0:
    {
        s32 graphics;
        PlaySong(0xBF);
        graphics = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, entity->palette);
        Anim_StartIndexed(entity, (u8)entity->ext.particleAttach.anim, 0, 0);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->scrollWithBg = 1;
        entity->state++;
    }
    case 1:
        entity->posX.whole = parent->posX.whole + entity->accelX.whole;
        entity->posY.whole = parent->posY.whole + entity->accelY.whole;
        break;
    }
    if (Anim_Update(entity) == 3)
        entity->deleteRequested = 1;
}

/* Particle living unk_4FD frames: moves with acceleration, either rises
 * (unk_4FE == 0) or every 8 frames steps back through the frame table
 * 08525138 (subState). */
/**
 * @brief Move and accelerate, optionally rise or step backward through the frame table, draw and expire after the stored timer limit.
 */
void Effect_DrawTimedParticle(struct EwramData_EntityData *entity)
{
    u8 timer = entity->timer;
    if (timer > entity->ext.particleTimed.lifetime)
    {
        entity->deleteRequested = 1;
        return;
    }
    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
    entity->velX.whole += entity->accelX.whole;
    entity->velY.whole += entity->accelY.whole;
    entity->timer = timer + 1;
    if (entity->ext.particleTimed.stepFrames == 0)
        entity->velY.whole -= 0x1000;
    else if (((timer + 1) & 7) == 0 && entity->subState != 0)
    {
        entity->subState--;
        entity->animFrame = sTimedParticleFrames[entity->subState];
    }
    Sprite_Draw(entity);
}

/* Emit a particle (Effect_DrawTimedParticle, frame and palette given) six pixels above
 * the player, in a random direction at 1.5 px/frame decelerating by 1/64
 * of the speed per frame, for 20 frames. */
/**
 * @brief Spawn a short-lived particle six pixels above the player with a random direction and acceleration opposing its initial velocity.
 */
void Effect_SpawnPlayerOutwardParticle(u8 frame, u8 palette)
{
    struct EwramData **ewram = &gEwramData;
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)EntityUpdateNothing);
    s32 graphics, angle, vy;

    if (entity == NULL)
        return;
    entity->spriteLayer = 2;
    graphics = ObjGfx_Find(sCommonObjectGfx);
    Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, palette);
    entity->drawFunc = (u32 *)Effect_DrawTimedParticle;
    entity->animFrame = frame;
    entity->spriteLayer = 3;
    entity->ext.particleTimed.lifetime = 0x14;
    {
        struct PlayerState *player = &(*ewram)->playerState;
        entity->posX.whole = player->player->posX.whole;
        entity->posY.whole = player->player->posY.whole - 0x60000;
    }
    angle = (RandomNumberGenerator() & 0xFF) << 8;
    entity->velX.whole = Sin(angle + 0x4000) * 24 / 16;
    vy = Sin(angle) * 24 / 16;
    entity->velY.whole = vy;
    entity->accelX.whole = -entity->velX.whole / 64;
    entity->accelY.whole = -vy / 64;
}

/* Emit a particle (Effect_DrawTimedParticle, given palette) on a 3 px circle around
 * the point 16 px above the player, moving inwards and stepping through the
 * frame table from entry 3, for 20 frames. */
/**
 * @brief Spawn a particle around the point sixteen pixels above the player, moving inward and stepping backward through the shared frame table.
 */
void Effect_SpawnPlayerInwardParticle(u8 palette)
{
    struct EwramData **ewram = &gEwramData;
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)EntityUpdateNothing);
    s32 graphics, angle, c, s, vx, vy;

    if (entity == NULL)
        return;
    entity->ext.particleTimed.stepFrames = 1;
    entity->subState = 3;
    entity->spriteLayer = 2;
    graphics = ObjGfx_Find(sCommonObjectGfx);
    Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, palette);
    entity->drawFunc = (u32 *)Effect_DrawTimedParticle;
    entity->animFrame = sTimedParticleFrames[entity->subState];
    entity->spriteLayer = 3;
    entity->ext.particleTimed.lifetime = 0x14;
    angle = (RandomNumberGenerator() & 0xFF) << 8;
    c = Sin(angle + 0x4000);
    s = Sin(angle);
    {
        struct PlayerState *player = &(*ewram)->playerState;
        entity->posX.whole = player->player->posX.whole + c * 48;
        entity->posY.whole = player->player->posY.whole + s * 48 - 0x100000;
    }
    vx = -(c * 20) / 16;
    entity->velX.whole = vx;
    vy = -(s * 20) / 16;
    entity->velY.whole = vy;
    entity->accelX.whole = vx / 16;
    entity->accelY.whole = vy / 16;
}

/* Effect_DrawPlayerFlash: growing flash effect that follows the player and toggles the blend. */
static inline s32 GrowX(s32 t)
{
    s32 r;

    if (t <= 15)
        r = 0x10000 - Sin((t << 10) + 0x4000);
    else
        r = 0x10000;
    return r;
}

static inline s32 GrowY(s32 t)
{
    s32 r;

    if (t <= 7)
        r = 0x2000;
    else if (t <= 15)
        r = 0x10000 - Sin(t << 11);
    else
        r = 0x10000;
    return r;
}

struct AffineSlot16 { u16 slot:5; u16 rest:3; };
/**
 * @brief Grow and draw the player-following flash, emit particles, pause at the existing timer point and release pause/affine state at expiration.
 */
void Effect_DrawPlayerFlash(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *player = gEwramData->playerState.player;
    s32 scaleX, scaleY;

    scaleX = GrowX(entity->timer);
    scaleY = GrowY(entity->timer);
    {
        u8 *flags = (u8 *)&entity->accelY + 5;
        *flags |= 4;
    }
    if (entity->timer <= entity->ext.playerFlash.duration)
        entity->timer++;
    if (entity->timer <= 2)
    {
        if (entity->timer == 2)
        {
            Game_SetMenusEnabled(0);
            gEwramData->paused = 1;
        }
        return;
    }
    if (entity->timer <= 0x27 && (entity->timer & 1))
        Effect_SpawnPlayerOutwardParticle(0x49, 6);
    if (entity->timer > entity->ext.playerFlash.duration)
    {
        ObjAffine_Free(entity->affineIndex);
        if (!(gEwramData->gameState.stateFlags & 0x100))
            Game_SetMenusEnabled(1);
        gEwramData->paused = 0;
        entity->deleteRequested = 1;
        return;
    }
    if (entity->timer <= 0x34 || !(entity->timer & 1))
    {
        entity->posY.part16.integer = player->posY.part16.integer - 0x20;
        entity->posX.part16.integer = player->posX.part16.integer;
        ObjAffine_Set(0, scaleX, scaleY, 0, 0, entity->affineIndex);
        Sprite_Draw(entity);
    }
}

/* Effect_CreatePlayerFlash. */
void Effect_DrawPlayerFlash(struct EwramData_EntityData *entity);

/**
 * @brief Create the table-selected player flash; kind zero allocates an affine slot and plays sound 0xBC; retain the allocation assumption.
 */
struct EwramData_EntityData *Effect_CreatePlayerFlash(s32 kind)
{
    u8 *info = sPlayerFlashInfo[kind];
    struct EwramData_EntityData *entity = EntityCreateInRange(0x16, 0x19, (u32 *)EntityUpdateNothing);
    s32 gfx;

    entity->spriteLayer = 1;
    gfx = ObjGfx_Find(sCommonObjectGfx);
    Sprite_Attach(entity, sCommonObjectSpriteSheet, gfx, 6);
    entity->drawFunc = (u32 *)Effect_DrawPlayerFlash;
    entity->animFrame = info[0];
    entity->ext.playerFlash.duration = info[1];
    if (kind == 0)
    {
        {
            u16 matrix = ObjAffine_Alloc(1);

            entity->affineIndex = matrix;
        }
        entity->affine = 1;
        PlaySong(0xBC);
    }
    entity->ext.playerFlash.info = (s32)info;
    entity->ext.playerFlash.kind = kind;
    return entity;
}

/* Spawn an effect (update Effect_UpdateAttachedAnimation) attached to parent at offset (dx, dy). */
static inline void CreateAttachedEffect(struct EwramData_EntityData *parent, u8 animation, u8 palette, s32 dx, s32 dy)
{
    struct EwramData_EntityData *child;
    if (parent == NULL)
        return;
    child = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateAttachedAnimation);
    if (child == NULL)
        return;
    child->ext.particleAttach.anim = animation;
    child->ext.particleAttach.parent = parent;
    child->palette = palette;
    child->accelX.whole = dx;
    child->accelY.whole = dy;
    child->posX.whole = parent->posX.whole + dx;
    child->posY.whole = parent->posY.whole + dy;
    child->spriteLayer = animation;
}

/* Wait for the timer and a button press (A/B), then release the player and
 * spawn the effect above them. */
/**
 * @brief Pause and emit inward particles, then wait for A/B after the configured delay before releasing pause and spawning the attached effect.
 */
void SoulAcquisition_DrawAndWait(struct EwramData_EntityData *entity)
{
    UpdateInGameTimer();
    if (entity->timer == 0)
    {
        Game_SetMenusEnabled(0);
        gEwramData->paused = 1;
    }
    entity->roomActive = 1;
    if (entity->timer < 48)
        Effect_SpawnPlayerInwardParticle(entity->palette);
    if (entity->timer < entity->ext.soulAcquire.duration)
        entity->timer++;
    else
    {
        if (gEwramData->inputData.newInput & 3)
        {
            struct EwramData_EntityData *player;
            Game_SetMenusEnabled(1);
            gEwramData->paused = 0;
            entity->deleteRequested = 1;
            player = gEwramData->playerState.player;
            {
                /* the arguments live in registers across the null check */
                u8 animation = 12;
                s32 dy = -0x100000;
                CreateAttachedEffect(player, animation, 6, 0, dy);
            }
        }
        Hud_DrawWindowPrompt();
    }
}

struct TrailTiming { u16 unused, timer; };
struct TrailSample { s16 x, y; u8 frame, flip; u8 pad[2]; };
extern s32 GetEntityRoomXPositionInteger(struct EwramData_EntityData *);
extern s32 GetEntityRoomYPositionInteger(struct EwramData_EntityData *);

/**
 * @brief Record owner position, frame and facing in 24 history entries and display the selected entry; free history and delete on expiration or invalid owner.
 */
void Afterimage_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = entity->ext.afterimage.owner;
    struct TrailSample *history = (struct TrailSample *)entity->ext.afterimage.history.ptr;
    s32 i;
    struct BgLayerState *bg;
    if (!owner || !owner->updateFunc || ((entity->ext.afterimage.flags & 0x20) && (s16)owner->hp <= 0))
        entity->state = 3;
    switch (entity->state)
    {
    case 0:
        if ((u16)((struct TrailTiming *)&entity->ext.raw.unk_500)->timer != 0)
            entity->state = 1;
        else
            entity->state = 2;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        break;
    case 1:
        if ((u16)((struct TrailTiming *)&entity->ext.raw.unk_500)->timer)
            ((struct TrailTiming *)&entity->ext.raw.unk_500)->timer--;
        else
            goto destroy;
        break;
    case 3:
    destroy:
        Heap_Free(entity->ext.afterimage.history.ptr);
        EntityDelete(entity);
        return;
    }
    entity->hidden = owner->hidden;
    for (i = 23; i > 0; i--)
    {
        history[i].x = history[i - 1].x;
        history[i].y = history[i - 1].y;
        history[i].frame = history[i - 1].frame;
        history[i].flip = history[i - 1].flip;
    }
    history[0].x = GetEntityRoomXPositionInteger(owner);
    history[0].y = GetEntityRoomYPositionInteger(owner);
    history[0].frame = owner->animFrame;
    history[0].flip = owner->flipX;
    entity->timer++;
    if ((entity->ext.afterimage.flags & 0x10) && (entity->timer & 3) == 0)
    {
        if (entity->subState == 0 || --entity->subState == 0)
        {
            entity->state = 3;
            return;
        }
    }
    if (entity->ext.afterimage.flags & 4)
        entity->animFrame = owner->animFrame;
    else
        entity->animFrame = history[entity->subState].frame;
    entity->flipX = history[entity->subState].flip;
    if (entity->ext.afterimage.flags & 0x80)
        entity->flipX ^= 1;
    {
        s32 offset = entity->subState * 8; struct TrailSample *sample; offset += (u32)history; sample = (struct TrailSample *)offset;
        bg = &gEwramData->bgInfo[1];
        entity->posX.part16.integer = sample->x - bg->xPos.part16.integer;
    }
    entity->posY.part16.integer = history[entity->subState].y - bg->yPos.part16.integer;
}

/* Afterimage_Create: matched (halfword bitfield view for affine; anim pinned to r6
   so the copy anim2 survives - FAKEMATCH). */
/* The canonical field now supplies the halfword mode; the animation pin remains. */

struct AfterimageFrame
{
    s16 x;
    s16 y;
    u8 frame;
    u8 flipX;
    u8 pad[2];
};

/**
 * @brief Create a history-based afterimage with the supplied countdown, history index and flags; return NULL if entity, sheet or history allocation is unavailable.
 */
struct EwramData_EntityData *Afterimage_Create(struct EwramData_EntityData *entity, u16 delay, u8 count, s32 flags)
{
    s32 gfx;
    void *anim2;
    struct EwramData_EntityData *e;
    struct AfterimageFrame *p;
    s32 i;

    gfx = Sprite_GetTileSlot(entity);
    { register void *anim asm("r6"); /* FAKEMATCH: anim in r6 */
    { u8 *ix_ = &entity->spriteSlot; anim = gEwramData->spriteSlots[*ix_].sheet; }
    anim2 = anim;
    e = EntityCreateInRange(0x49, 0xCF, (u32 *)Afterimage_Update);
    if (e == NULL)
        return NULL;
    if (anim == NULL || (e->ext.afterimage.history.value = (s32)Heap_Alloc(0x31)) == 0)
    {
        EntityDelete(e);
        return NULL;
    }
    Sprite_Attach(e, anim2, gfx, entity->palette); }
    e->animFrame = entity->animFrame;
    e->spriteLayer = entity->spriteLayer + 1;
    e->posX.part16.integer = (s16)GetEntityRoomXPositionInteger(entity);
    e->posY.part16.integer = (s16)GetEntityRoomYPositionInteger(entity);
    e->ext.afterimage.owner = entity;
    e->ext.afterimage.delay = delay;
    if (count > 0x17)
        count = 0x17;
    e->subState = count;
    if (!(flags & 8))
        e->objMode = 1;
    e->flipX = entity->flipX;
    if (flags & 1)
        e->drawSkip = 1;
    if (flags & 2)
    {
        e->affine = entity->affine;
        e->affineIndex = entity->affineIndex;
    }
    p = (struct AfterimageFrame *)e->ext.afterimage.history.value;
    for (i = 0; i < 24; i++, p++)
    {
        p->x = e->posX.part16.integer;
        p->y = e->posY.part16.integer;
        p->frame = entity->animFrame;
        p->flipX = entity->flipX;
    }
    e->ext.afterimage.flags = flags;
    if (flags & 0x20)
        e->animMode = 2;
    if (flags & 0x40)
        e->animMode = 1;
    return e;
}

/* Spawn a terminated list of particle templates at the given position. */
struct ParticleTemplate {
    u8 animation, palette, layer, lifetime;
    s16 x, y;
    s32 vx, vy, ax, ay;
};
extern struct ParticleTemplate *sParticleTemplateLists[];
extern void Effect_UpdateDelayedParticle(struct EwramData_EntityData *);
/**
 * @brief Spawn the selected particle-template list until palette 0xFF or entity allocation failure, copying offsets, motion and start delay.
 */
void Effect_SpawnParticleTemplateList(s16 x, s16 y, s32 index)
{
    struct ParticleTemplate *info = sParticleTemplateLists[index];
    for (; info->palette != 0xFF; info++)
    {
        s32 graphics;
        struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateDelayedParticle);
        if (entity == NULL) return;
        graphics = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, info->palette);
        Anim_StartIndexed(entity, info->animation, 0, 0);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = info->layer;
        entity->posX.part16.integer = (s16)x + info->x;
        entity->posY.part16.integer = (s16)y + info->y;
        entity->velX.whole = info->vx;
        entity->velY.whole = info->vy;
        entity->accelX.whole = info->ax;
        entity->accelY.whole = info->ay;
        entity->timer = info->lifetime;
        entity->scrollWithBg = 1;
    }
}

/**
 * @brief Create a frame/lifetime or animated effect selected by the shared table, with supplied position, acceleration and palette; return NULL on entity allocation failure.
 */
struct EwramData_EntityData *Effect_CreateFromTable(s32 x, s32 y, s32 ax, s32 ay, s32 index, u8 palette)
{
    struct EwramData_EntityData *entity;
    struct Unk_085251A8 *info = &sUnk_085251A8[index];
    if (info->anim == NULL) entity = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateTimedDoubleParticle);
    else entity = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateTableAnimation);
    if (entity != NULL)
    {
        entity->spriteLayer = 12;
        entity->posX.whole = x;
        entity->posY.whole = y;
        entity->accelX.whole = ax;
        entity->accelY.whole = ay;
        entity->ext.particleTable.index = index;
        entity->palette = palette;
    }
    return entity;
}

/* Spawners 0x08045BC8..0x08045C9B. */

/**
 * @brief Spawn an animated child at the supplied parent-relative offsets and layer 12; do nothing if parent or entity allocation is NULL.
 */
void Effect_SpawnAttachedAnimation(struct EwramData_EntityData *parent, u8 anim, u8 palette, s32 offsetX, s32 offsetY)
{
    struct EwramData_EntityData *child;

    if (parent == NULL)
        return;
    child = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateAttachedAnimation);
    if (child == NULL)
        return;
    child->ext.particleAttach.anim = anim;
    child->ext.particleAttach.parent = parent;
    child->palette = palette;
    child->accelX.whole = offsetX;
    child->accelY.whole = offsetY;
    child->posX.whole = parent->posX.whole + offsetX;
    child->posY.whole = parent->posY.whole + offsetY;
    child->spriteLayer = 0xC;
}

/**
 * @brief 45C34 | Spawn an effect entity (update SoulAcquisition_DrawAndWait) using the
 * shared graphics 081C15F4/0820ED60 and play sound 0xBE.
 */
/* Declared non-void: the original epilogue returns through r1, but no value
 * is set (callers ignore it). */
/**
 * @brief Create the hidden acquisition effect with the soul-kind palette and 64-frame wait, then play sound 0xBE; preserve the unset return value and allocation assumption.
 */
s32 SoulAcquisition_CreateEffect(s32 kind)
{
    struct EwramData_EntityData *entity;
    s32 gfx;
    u8 *anim;

    entity = EntityCreateInRange(0x16, 0x19, (u32 *)EntityUpdateNothing);
    gfx = ObjGfx_Find(sCommonObjectGfx);
    anim = sCommonObjectSpriteSheet;
    Sprite_Attach(entity, anim, gfx, Soul_GetPickupPalette(kind));
    entity->hidden = 1;
    entity->drawFunc = (u32 *)SoulAcquisition_DrawAndWait;
    entity->ext.soulAcquire.duration = 0x40;
    PlaySong(0xBE);
}

/**
 * @brief Install the sprite draw callback in state zero, move with acceleration and request deletion when the animation returns three.
 */
void Effect_UpdateSharedAnimation(struct EwramData_EntityData *entity)
{
    if (entity->state == 0)
    {
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->state = 1;
    }
    entity->posX.whole += entity->velX.whole;
    entity->velX.whole += entity->accelX.whole;
    entity->posY.whole += entity->velY.whole;
    entity->velY.whole += entity->accelY.whole;
    if (Anim_Update(entity) == 3) entity->deleteRequested = 1;
}

/**
 * @brief 45CEC | Spawn a Effect_UpdateSharedAnimation particle at (x, y) with the shared
 * graphics 081C15F4/0820ED60, animation script and palette palette.
 */
struct EwramData_EntityData *Effect_SpawnShared(s32 x, s32 y, s32 script, u32 palette)
{
    struct EwramData_EntityData *entity;
    s32 gfx;

    entity = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateSharedAnimation);
    if (entity != NULL)
    {
        gfx = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, gfx, palette);
        Anim_StartIndexed(entity, script, 0, 0);
        entity->posX.whole = x;
        entity->posY.whole = y;
        entity->spriteLayer = 0xD;
        entity->scrollWithBg = 1;
    }
    return entity;
}

/**
 * @brief Move with acceleration and request deletion outside the existing screen rectangle or when a nonzero countdown expires.
 */
void Effect_UpdateBoundedParticle(struct EwramData_EntityData *entity)
{
    entity->posX.whole += entity->velX.whole;
    entity->velX.whole += entity->accelX.whole;
    entity->posY.whole += entity->velY.whole;
    entity->velY.whole += entity->accelY.whole;
    if (Entity_IsInRect(entity, -0x10, 0, 0x110, 0xB0) == 0)
        entity->deleteRequested = 1;
    if (entity->timer != 0 && --entity->timer == 0)
        entity->deleteRequested = 1;
}

/**
 * @brief Create a shared-graphics frame particle at the supplied position and palette; return NULL on entity allocation failure.
 */
struct EwramData_EntityData *Effect_CreateSharedFrame(s32 x, s32 y, s32 frame, u32 palette)
{
    struct EwramData_EntityData *entity;
    s32 gfx;

    entity = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateBoundedParticle);
    if (entity != NULL)
    {
        gfx = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, gfx, palette);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->animFrame = frame;
        entity->posX.whole = x;
        entity->posY.whole = y;
        entity->spriteLayer = 0xD;
        entity->scrollWithBg = 1;
    }
    return entity;
}

/* 5E38. */
/* Put a live entity into state 3. */
/**
 * @brief Put a live entity into state 3.
 */
void Entity_EndIfActive(struct EwramData_EntityData *entity)
{
    if (entity->updateFunc != NULL)
        entity->state = 3;
}

/* Effect_DrawAtOffsetPair. */
/**
 * @brief Draw once at the entity position and once at the stored secondary offset, then restore the position.
 */
void Effect_DrawAtOffsetPair(struct EwramData_EntityData *entity)
{
    Sprite_Draw(entity);
    entity->posX.whole += entity->ext.particlePair.offsetX;
    entity->posY.whole += entity->ext.particlePair.offsetY;
    Sprite_Draw(entity);
    entity->posX.whole -= entity->ext.particlePair.offsetX;
    entity->posY.whole -= entity->ext.particlePair.offsetY;
}

/**
 * @brief Stay hidden through the initial countdown, then accelerate and move on both axes and request deletion when the animation returns three.
 */
void Effect_UpdateDelayedParticle(struct EwramData_EntityData *entity)
{
    if (entity->state == 0)
    {
        entity->hidden = 1;
        if (entity->timer != 0)
        {
            entity->timer--;
            return;
        }
        entity->hidden = 0;
        entity->state++;
    }
    EntityAccelerateAndMoveX(entity, 0x80000);
    EntityAccelerateAndMoveY(entity, 0x80000);
    if (Anim_Update(entity) == 3)
        entity->deleteRequested = 1;
}

s32 Pickup_TrySpawnOnHit(struct EwramData_EntityData *entity, s32 offsetY, u8 kind, s32 amount);

/**
 * @brief Hide off screen, spawn a reward on a qualifying player hit, then hide, emit the existing particles and expire the timer.
 */
void PickupSource_Update(struct EwramData_EntityData *entity)
{
    if (Entity_IsInRect(entity, -0x10, -0x10, 0x110, 0xC0) == 0)
    {
        entity->hidden = 1;
        return;
    }
    switch (entity->state)
    {
        case 0:
            entity->hidden = 0;
            if (Pickup_TrySpawnOnHit(entity, 0, entity->enemyId, entity->spawnParam1) != 0)
            {
                PlaySong(0xA2);
                entity->hidden = 1;
                entity->timer = 0x28;
                entity->state++;
            }
            break;
        case 1:
            if (entity->timer > 0x1E && (entity->timer & 3) == 0)
                Effect_SpawnRandom(entity->posX.whole, entity->posY.whole, 0x80000, 0x80000, entity->spriteLayer - 1);
            if (--entity->timer == 0xFF)
                entity->deleteRequested = 1;
            break;
    }
}

/* Pickup_TrySpawnOnHit. */
/* Same callee as Enemy_PreUpdate's, whose result is ignored there; here the
 * low halfword is tested against 0x1E. */
extern s32 Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);
extern void Pickup_CreateMana(s32 x, s32 y, s32 large, s32 jump);
extern void Pickup_CreateGold(s32 x, s32 y, s32 index);

/**
 * @brief On hit-mask 0x1E select and spawn the existing mana/gold/item reward, with character and full-MP substitutions; return one for the hit even if allocation fails.
 */
s32 Pickup_TrySpawnOnHit(struct EwramData_EntityData *entity, s32 offsetY, u8 kind, s32 amount)
{
    u8 collision[16];
    s32 x = (s16)entity->posX.part16.integer;
    s32 y = (s16)entity->posY.part16.integer + offsetY;
    u16 touch;

    touch = Hit_CheckWithPlayer(collision, entity);
    if (touch & 0x1E)
    {
        if (gEwramData->playerStatus.currentCharacter != 0 && kind != 0)
        {
            kind = 0;
            amount = 0;
        }
        switch (kind)
        {
            case 0:
                if (gEwramData->playerStatus.currentMP != gEwramData->playerStatus.maxMP || gEwramData->playerStatus.currentCharacter != 0)
                    Pickup_CreateMana(x, y, amount, 0);
                else
                    Pickup_CreateGold(x, y, (RandomNumberGenerator() & 1) + amount);
                break;
            case 1:
                Pickup_CreateGold(x, y, amount);
                break;
            case 2:
                Pickup_CreateItem(x, y, 2, amount, 0);
                break;
        }
        return 1;
    }
    return 0;
}

extern void PickupSource_LoadIcon(s32);
/**
 * @brief Load the source icon, attach shared graphics and animation, configure its body box and install the source draw callback.
 */
void PickupSource_Init(struct EwramData_EntityData *entity)
{
    s32 graphics;
    PickupSource_LoadIcon(entity->spawnParam0);
    entity->spriteLayer = 0x14;
    entity->flipX = 0;
    graphics = ObjGfx_Find(sCommonObjectGfx);
    Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, 7);
    Anim_StartIndexed(entity, 3, 0, 1);
    Anim_Update(entity);
    Hit_Enable(entity, 0, NULL, 0);
    entity->hit.flags |= 2;
    HitBox_SetBodyRaw(entity, 0x140CF4FA);
    entity->drawFunc = (u32 *)PickupSource_Draw;
}

/**
 * @brief Advance animation and draw the source, optionally draw frame 0x31 six pixels above it under frame-counter bit one, then restore frame and position.
 */
void PickupSource_Draw(struct EwramData_EntityData *entity)
{
    Anim_Update(entity);
    Sprite_Draw(entity);
    if (gEwramData->frameCounter & 2)
    {
        u8 saved = entity->animFrame;
        entity->posY.part16.integer -= 6;
        entity->animFrame = 0x31;
        Sprite_Draw(entity);
        entity->animFrame = saved;
        entity->posY.part16.integer += 6;
    }
}

/**
 * @brief Upload two icon rows to the shared OBJ graphics unless the icon index matches the cached value; retain the cache and index assumptions.
 */
void PickupSource_LoadIcon(s32 icon)
{
    struct GameState *data = &gEwramData->gameState;
    u8 *src;
    s32 tile;

    if (data->cachedPickupIcon == icon)
        return;
    data->cachedPickupIcon = icon;
    src = sPickupIconTiles;
    tile = ObjTiles_GetSlotTile((u8)ObjGfx_Find(sCommonObjectGfx)) << 5;
    src += (icon & 1) * 0x100 + (icon / 2) * 0x400;
    DmaQueue_IndirectCopy(0xC0, (u32 *)src, (u32 *)(tile + 0x06010800));
    src += 0x200;
    DmaQueue_IndirectCopy(0xC0, (u32 *)src, (u32 *)(tile + 0x06010C00));
}
