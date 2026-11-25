/*
 * Room objects 0x15-0x18: orbiters, effects, the background overlay of Object 0x17 and Object 0x18 debris (Object 0x19 Create has its own CFLAGS: next module).
 */
#include "elevator_portal.h"
#include "core.h"
#include "graphics.h"
#include "sound.h"
#include "structs/ewram.h"
#include "in_game.h"
#include "map_screen.h"
#include "map_flags.h"
#include "entity_util.h"
#include "syscalls.h"
#include "player_core.h"
#include "gameplay.h"
#include "blue_souls.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "hitbox.h"
#include "math_anim.h"
#include "palette.h"
#include "soul_pickup.h"
#include "effects.h"
#include "data/data_0E5364.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"
#include "structs/entity_template.h"
#include "math_inline.h"
#include "music.h"
#include "pickup.h"
#include "entity_hit.h"
#include "bg_and_collision.h"
#include "constants/hit_flags.h"
#include "constants/songs.h"

void ElevatorFollower_Update(struct EwramData_EntityData *);
void ElevatorEffect_Update(struct EwramData_EntityData *);
void BreakableCoinContainer_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker);

/**
 * @brief Sprite of object 0x15 placed around its owner at the owner angle
 * unk_4FC and radius unk_500.
 */
void ChaoticRealmPortalOrbiter_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);
    if (entity->state == 0)
    {
        if (!Entity_LoadGraphics(entity, sChaoticRealmPortalGfx, sUnk_0820A1C4, 8, sChaoticRealmPortalSpriteSheet)) { EntityDelete(entity); return; }
        entity->animFrame = 1;
        entity->spriteLayer = 31;
        entity->state = 1;
    }
    entity->velX.whole = FixedMulFast(Sin(owner->ext.chaoticRealmPortal.angle + 0x4000), owner->ext.chaoticRealmPortal.radius);
    entity->velY.whole = FixedMulFast(Sin(owner->ext.chaoticRealmPortal.angle), owner->ext.chaoticRealmPortal.radius);
    entity->posX.whole = owner->posX.whole + entity->velX.whole;
    entity->posY.whole = owner->posY.whole + entity->velY.whole;
}

void ChaoticRealmPortalCreate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child;
    if (!Entity_LoadGraphics(entity, sChaoticRealmPortalGfx, sUnk_0820A1C4, 8, sChaoticRealmPortalSpriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    entity->animFrame = 0;
    entity->spriteLayer = 31;
    child = EntityCreateInRange(ENTITY_SLOT_OBJECT_FIRST, ENTITY_SLOT_OBJECT_LAST, ChaoticRealmPortalOrbiter_Update);
    if (child == NULL)
    {
        EntityDelete(entity);
        return;
    }
    child->scrollWithBg = 1;
    ENTITY_OWNER(child) = entity;
    entity->ext.chaoticRealmPortal.angle = 0x6000;
    entity->ext.chaoticRealmPortal.radius = 0x200000;
    if ((gEwramData->gameState.gameFlagsTail[1] & 0x0A000000) == 0x02000000)
    {
        entity->state = 4;
        gEwramData->gameState.stateFlags |= STATE_EVENT_RUNNING;
        Game_SetMenusEnabled(0);
        gEwramData->gameState.gameFlagsTail[1] |= 0x08000000;
    }
    else entity->state = 1;
}

/* Object 15 interaction: capture player input, animate, show one of two
 * messages, switch UI state and restore the palette transition. */
static inline struct EwramData_EntityData *Player(struct EwramData *e) { return e->playerState.player; }
void ChaoticRealmPortalUpdate(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
    case 1:
        if (!Entity_IsInRect(gEwramData->playerState.player, 0x60, 0x7F, 0x20, 0x10)) break;
        if (!(gEwramData->inputData.playerHeldInput & 0x40)) break;
        if (Player_IsBusy()) break;
        entity->state = 2;
        gEwramData->gameState.stateFlags |= STATE_EVENT_RUNNING;
        Game_SetMenusEnabled(0);
        SetPlayerInput(1, 0);
        break;
    case 2:
        if (entity->ext.chaoticRealmPortal.radius > 0x90000) SetPlayerInput(1, 0);
        else SetPlayerInput(1, gEwramData->playerStatus.jumpButton);
        entity->ext.chaoticRealmPortal.angle += 0x40;
        if (entity->ext.chaoticRealmPortal.radius != 0)
        {
            if (entity->ext.chaoticRealmPortal.radius == 0x100000)
            {
                BgPalette_StartColorFade(31, 7, 7, 0x10000, 0, 64);
                ObjPalette_StartColorFade(31, 7, 7, 0x10000, 0, 64);
            }
            entity->ext.chaoticRealmPortal.radius -= 0x4000;
        }
        else entity->state = 3;
        break;
    case 3:
        SetPlayerInput(1, gEwramData->playerStatus.jumpButton);
        if (entity->spawnParam0 == 0) RoomJump_SetDestination(sChaoticRealmPortalDestinationRoom1, 0, 0, 0x78, 0x50);
        else RoomJump_SetDestination(sChaoticRealmPortalDestinationRoom0, 0, 0, 0x78, 0x50);
        RoomJump_Begin(&gEwramData->gameState);
        gEwramData->gameState.stateFlags &= ~STATE_EVENT_RUNNING;
        BgPalette_StartColorFade(31, 31, 31, 0, 0x10000, 8);
        ObjPalette_StartColorFade(31, 31, 31, 0, 0x10000, 8);
        break;
    case 4:
    {
        s32 input, playerX, entityX;
        Game_SetMenusEnabled(0);
        playerX = Player(gEwramData)->posX.whole;
        entityX = entity->posX.whole;
        input = 0x20;
        if (entityX > playerX) input = 0x10;
        SetPlayerInput(1, input);
        if (Entity_IsInRect(Player(gEwramData), 0x70, 0x7F, 0x10, 0x10)) entity->state = 2;
        break;
    }
    }
}

/* Animated fragment: play the selected animation, then expire after 8 frames. */
/**
 * @brief One-shot effect of object 0x16: play its animation, then delete after
 * 8 frames.
 */
void ChaoticRealmVortexEffect_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
    case 0:
    {
        struct AnimScript *anim;
        s32 variant;
        if (!Entity_LoadGraphics(entity, sChaoticRealmVortexGfx, sChaoticRealmVortexPalette, 0, sChaoticRealmVortexSpriteSheet))
        { EntityDelete(entity); return; }
        variant = entity->subState;
        anim = sChaoticRealmVortexEffectAnimScript;
        if (variant == 0) anim = sChaoticRealmVortexEffectAnimScriptVariant0;
        Anim_Start(entity, anim, 0, 0);
        entity->spriteLayer = 0x11 - entity->subState;
        entity->scrollWithBg = 1;
        entity->state++;
        break;
    }
    case 1:
        if (Anim_Update(entity) == 3)
        {
            entity->timer = 8;
            entity->drawSkip = 1;
            entity->state++;
        }
        break;
    case 2:
        if (entity->timer != 0) entity->timer--;
        else entity->deleteRequested = 1;
        break;
    }
    EntityAccelerateAndMove(entity, 0x80000);
}

/* ChaoticRealmVortexUpdate: affine zoom-out cutscene object with rising dust particles; matched. */

static inline struct EwramData_EntityData *Player2(void) { return gEwramData->playerState.player; }

void ChaoticRealmVortexUpdate(struct EwramData_EntityData *entity)
{
    struct BgAffineSrcData src;
    struct EwramData_EntityData *child;
    struct EwramData_EntityData *player;
    s32 input;
    s32 t;
    u32 r;

    src.texX = 0x8000;
    src.texY = 0x8000;
    src.scrX = 0x80;
    src.scrY = 0x50;
    src.alpha = entity->ext.chaoticRealmVortex.angle;
    entity->ext.chaoticRealmVortex.angle += entity->ext.chaoticRealmVortex.spin;
    switch (entity->state)
    {
    case 0:
        if (gEwramData->gameState.stateFlags & STATE_EVENT_RUNNING)
        {
            gEwramData->gameState.stateFlags |= STATE_EVENT_RUNNING;
            Game_SetMenusEnabled(0);
            SetPlayerInput(1, 0);
            entity->state = 1;
        }
        break;
    case 1:
        {
            s32 px = gEwramData->playerState.player->posX.whole;
            SetPlayerInput(1, entity->posX.whole > px ? 0x10 : 0x20);
        }
        Game_SetMenusEnabled(0);
        if (Entity_IsInRect(entity, (s16)(Player2()->posX.part16.integer - 8), (s16)(Player2()->posY.part16.integer), 0x10, 2))
            entity->state = 2;
        break;
    case 2:
        SetPlayerInput(1, 0);
        RedSoul_CancelAll();
        BlueSoul_StopAll();
        entity->state = 3;
        PlaySong(0x157);
        break;
    case 3:
        SetPlayerInput(1, 0);
        if (entity->ext.chaoticRealmVortex.timer > 0)
        {
            entity->ext.chaoticRealmVortex.scale = 0x1000 - Sin((entity->ext.chaoticRealmVortex.timer << 6) + 0x4000) / 16;
            entity->ext.chaoticRealmVortex.timer--;
            entity->ext.chaoticRealmVortex.spin += 2;
            if (entity->ext.chaoticRealmVortex.timer == 0x40)
            {
                PlaySong(0x1AE);
                t = entity->ext.chaoticRealmVortex.timer;
                if (t == 0x40)
                {
                    BgPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0x10000, 0, t);
                    ObjPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0x10000, 0, t);
                }
            }
        }
        else
        {
            entity->state = 4;
        }
        break;
    case 4:
        SetPlayerInput(1, 0);
        gEwramData->gameState.stateFlags &= ~STATE_EVENT_RUNNING;
        RoomJump_SetDestination((void *)sDebugExitOptionsRoom11, 0, 0, 0x78, 0x6F);
        RoomJump_Begin(&gEwramData->gameState);
        gEwramData->gameState.stateFlags |= STATE_ROOM_JUMP_NO_PAUSE;
        break;
    }
    src.sx = entity->ext.chaoticRealmVortex.scale;
    src.sy = entity->ext.chaoticRealmVortex.scale;
    BgAffineSet(&src, (struct BgAffineDstData *)&gDisplayRegisters.bg2PA, 1);
    entity->timer++;
    if (entity->timer & 1)
    {
        child = EntityCreateInRange(ENTITY_SLOT_EFFECT_FIRST, ENTITY_SLOT_EFFECT_LAST, ChaoticRealmVortexEffect_Update);
        if (child != NULL)
        {
            child->posX.whole = entity->posX.whole;
            child->posY.whole = entity->posY.whole + ((RandomSigned(8) - 4) << 16);
            child->velY.whole = RandomSigned(0x1000) - 0x8000;
            if ((RandomNumberGenerator() & 0x3F) <= 0x27)
            {
                if ((RandomNumberGenerator() & 0x3F) <= 0xF)
                {
                    child->subState = 1;
                    child->velY.whole += -0x1C000;
                }
                child->posX.whole += RandomSigned(4) << 16;
            }
            else
            {
                r = RandomNumberGenerator();
                child->posX.whole += (((r & 7) >> 1) + 2) * ((RandomNumberGenerator() & 1) * 2 - 1) << 16;
            }
            child->accelY.whole = -entity->velY.whole / 128;
        }
    }
}

/* Object 0x16 helpers 0x08052F6C..0x0805305B. */

/**
 * @brief 52F6C | Object 0x16 create: load the graphics, palette
 * sChaoticRealmVortexPaletteAnim, layer 0x12, unk_500 = 0xF00, unk_504 = 0, unk_508 = 0x40,
 * unk_4F8 = 0x100.
 */
void ChaoticRealmVortexCreate(struct EwramData_EntityData *entity)
{
    if (!Entity_LoadGraphics(entity, sChaoticRealmVortexGfx, sChaoticRealmVortexPalette, 0, sChaoticRealmVortexSpriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    PaletteAnim_Start(sChaoticRealmVortexPalette, sChaoticRealmVortexPaletteAnim, entity->basePalette);
    entity->animFrame = 0;
    entity->spriteLayer = 0x12;
    entity->ext.chaoticRealmVortex.scale = 0xF00;
    entity->ext.chaoticRealmVortex.angle = 0;
    entity->ext.chaoticRealmVortex.spin = 0x40;
    entity->ext.chaoticRealmVortex.timer = 0x100;
}

/**
 * @brief Follow the parent and mirror its states: show with animation 2 when
 * it enters state 1, animation 3 at state 3, hide again at state 0.
 */
void ElevatorOverlay_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);

    entity->posX.whole = owner->posX.whole;
    entity->posY.whole = owner->posY.whole;
    switch (entity->state)
    {
        case 0:
            if (owner->state == 1)
            {
                entity->hidden = 0;
                Anim_StartIndexed(entity, 2, 0, 0);
                entity->state = 1;
            }
            break;
        case 1:
            if (owner->state == 3)
            {
                Anim_StartIndexed(entity, 3, 0, 0);
                entity->state = 2;
            }
            break;
        case 2:
            if (owner->state == 0)
            {
                entity->hidden = 1;
                entity->state = 0;
            }
            break;
    }
    Anim_Update(entity);
}

/* ElevatorCreate: spawns the object and its child effects; matched. */

static inline struct BgLayerState *Bg1(void)
{
    return &gEwramData->bgInfo[1];
}

void ElevatorCreate(struct EwramData_EntityData *entity)
{
    s32 i;
    s32 j;
    u32 py;
    s32 t;
    const s16 *tbl;
    struct EwramData_EntityData *e;
    struct BgLayerState *bg;

    if (!Entity_LoadGraphics(entity, sElevatorGfx, sUnk_08209FD8, 4, sElevatorSpriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    entity->spriteLayer = 0x11;
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    t = GetEntityRoomYPositionInteger(gEwramData->playerState.player) << 16;
    i = 0;
    if (sElevatorFloorY[0] + 0x20 >= ({ py = (u32)t >> 16; t >> 16; }))
    {
        entity->subState = i;
        do { tbl = sElevatorFloorY; } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
    }
    else
    {
    next:
        i++;
        do { tbl = sElevatorFloorY; } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
        if (i <= 2)
        {
            if (tbl[i] + 0x20 < (s16)py)
                goto next;
            entity->subState = i;
        }
    }
    bg = &gEwramData->bgInfo[1];
    entity->posX.part16.integer = 0x78 - bg->xPos.part16.integer;
    entity->posY.part16.integer = tbl[entity->subState] - bg->yPos.part16.integer;
    Player_RegisterSolidObject(entity, (void *)sElevatorSolidCallbacks);
    entity->roomActive = 1;
    e = EntityCreateInRange(ENTITY_SLOT_EFFECT_FIRST, ENTITY_SLOT_EFFECT_LAST, ElevatorFollower_Update);
    if (e != NULL)
    {
        if (!Entity_LoadGraphics(e, sElevatorGfx, sUnk_08209FD8, 4, sElevatorSpriteSheet))
        {
            EntityDelete(e);
            return;
        }
        Anim_StartIndexed(e, 1, 0, 1);
        Anim_Update(e);
        ENTITY_OWNER(e) = entity;
        e->scrollWithBg = 1;
        e->posX.whole = entity->posX.whole;
        e->posY.whole = entity->posY.whole;
        e->spriteLayer = 0x12;
    }
    for (i = 0; i <= 1; i++)
    {
        e = EntityCreateInRange(ENTITY_SLOT_EFFECT_FIRST, ENTITY_SLOT_EFFECT_LAST, ElevatorOverlay_Update);
        if (e == NULL)
            continue;
        if (!Entity_LoadGraphics(e, sElevatorGfx, sUnk_08209FD8, 4, sElevatorSpriteSheet))
        {
            EntityDelete(e);
            return;
        }
        Anim_StartIndexed(e, 3, 0, 0);
        Anim_Update(e);
        ENTITY_OWNER(e) = entity;
        e->scrollWithBg = 1;
        e->flipX = i;
        e->hidden = 1;
        e->posX.whole = entity->posX.whole;
        e->posY.whole = entity->posY.whole;
        e->spriteLayer = 0x11;
    }
    for (i = 0; i <= 1; i++)
    {
        for (j = 0; j <= 7; j++)
        {
            e = EntityCreateInRange(ENTITY_SLOT_EFFECT_FIRST, ENTITY_SLOT_EFFECT_LAST, ElevatorEffect_Update);
            if (e == NULL)
                break;
            e->scrollWithBg = 1;
            {
                const u16 *tx = sElevatorXs;
                e->posX.part16.integer = tx[i] - ({ Bg1(); })->xPos.part16.integer;
            }
            {
                const u16 *ty = sElevatorYs;
                e->posY.part16.integer = ty[j] - ({ Bg1(); })->yPos.part16.integer;
            }
            e->flipX = i;
            e->subState = i + 4;
        }
    }
}

/* Elevator movement and floor input. The r0 timer locals retain the original
 * load/copy sequence; case 1 keeps its zero copy across the sound call. */

static inline s32 AddBgY(s32 v)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    return v + bg->yPos.whole;
}

void ElevatorUpdate(struct EwramData_EntityData *entity)
{
    s32 y;
    u16 yi;
    u8 t;
    u8 t3;
    u8 x;
    struct BgLayerState *bg;
    s16 *tbl;
    struct EwramData_EntityData *player;

    if (!entity->roomActive)
    {
        entity->deleteRequested = 1;
        Player_ClearSolidObject();
        return;
    }
    switch (entity->state)
    {
    case 0:
        if (Player_IsBusy())
        {
            entity->ext.elevator.holdFrames = 0;
            break;
        }
        player = gEwramData->playerState.player;
        if (!Entity_IsInRect(entity, (s16)(player->posX.part16.integer - 0x20), (s16)(player->posY.part16.integer - 0x20), 0x40, 0x10))
        {
            entity->ext.elevator.holdFrames = 0;
            break;
        }
        if (gEwramData->inputData.playerHeldInput & 0xC0)
            entity->ext.elevator.holdFrames++;
        else
            entity->ext.elevator.holdFrames = 0;
        if ((u8)entity->ext.elevator.holdFrames <= 2)
            break;
        if ((gEwramData->inputData.playerHeldInput & 0x80) && entity->subState <= 1)
        {
            entity->subState++;
            entity->stage = 0;
            entity->state = 1;
            entity->timer = 0x20;
            SetPlayerInput(1, 0);
        }
        if ((gEwramData->inputData.playerHeldInput & 0x40) && entity->subState != 0)
        {
            entity->subState--;
            entity->stage = 1;
            entity->state = 1;
            entity->timer = 0x40;
            SetPlayerInput(1, 0);
        }
        entity->velY.whole = 0;
        break;
    case 1: {
        s16 timer;
        u32 copy;
        SetPlayerInput(1, 0);
        timer = entity->timer;
        copy = timer;
        if (copy) { entity->timer = timer - 1; break; }
        Sound_PlayTracked(0x116);
        entity->state = 2;
        entity->velY.whole = copy;
        break;
    }
    case 2:
        SetPlayerInput(1, 0);
        entity->posY.whole += entity->velY.whole;
        if (entity->stage == 0)
        {
            if (entity->velY.whole <= 0x2FFFF)
                entity->velY.whole += 0x1000;
            y = AddBgY(entity->posY.whole + entity->velY.whole);
            yi = y < 0 ? -(-y >> 16) : y >> 16;
            if ((s16)yi < sElevatorFloorY[entity->subState])
                break;
        }
        else
        {
            if (entity->velY.whole > -0x30000)
                entity->velY.whole -= 0x1000;
            y = AddBgY(entity->posY.whole + entity->velY.whole);
            yi = y < 0 ? -(-y >> 16) : y >> 16;
            if ((s16)yi > sElevatorFloorY[entity->subState])
                break;
        }
        Sound_StopTracked(0x116);
        PlaySong(0x117);
        ScreenShake_Request(0, 0x10, 0, 7);
        {
            const s16 *tbl = sElevatorFloorY;
            struct BgLayerState *bg2;
            s32 fy = tbl[entity->subState];
            bg2 = &gEwramData->bgInfo[1];
            entity->posY.whole = (fy - (s16)bg2->yPos.part16.integer) << 16;
        }
        entity->velY.whole = 0;
        entity->state = 3;
        entity->timer = 0x40;
        break;
    case 3: {
        s16 timer = entity->timer;
        u32 copy = timer;
        if (copy) { entity->timer = timer - 1; break; }
        entity->state = copy;
        break;
    }
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
}

/* Elevator_CeilingCorrection, Elevator_FloorCorrection. */
/**
 * @brief 53540 | Whether the point (x, y) lies on top of entity (within
 * -0x40..0x40 horizontally of its screen position and up to 0x10 below
 * its top edge y - 0x18, using the integer part of velY): the depth below
 * the edge, else 0.
 */
s32 Elevator_CeilingCorrection(struct EwramData_EntityData *entity, s16 x, s16 y)
{
    struct BgLayerState *bg;
    s32 sx = x;
    s32 px = (s16)entity->posX.part16.integer;
    s32 dx;
    s32 dy;
    s32 bottom;
    s32 py;

    bg = &gEwramData->bgInfo[1];
    dx = sx - (px + (s16)bg->xPos.part16.integer) + 0x40;
    py = (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer + entity->velY.part16.integer;
    bottom = y + 0x18;
    dy = py - bottom;
    if ((u32)dx > 0x80 || (u32)dy > 0x10)
        return 0;
    return (s16)dy;
}

/**
 * @brief 535A0 | As Elevator_CeilingCorrection with the next 16.16 y position (posY +
 * velY + camera) and the edge 0x1F above it; returns minus the offset.
 */
s32 Elevator_FloorCorrection(struct EwramData_EntityData *entity, s16 x, s16 y)
{
    struct BgLayerState *bg;
    s32 sx = x;
    s32 px = (s16)entity->posX.part16.integer;
    s32 dx;
    s32 dy;
    s32 v;
    s32 sy;

    bg = &gEwramData->bgInfo[1];
    dx = sx - (px + (s16)bg->xPos.part16.integer) + 0x40;
    sy = y;
    v = entity->posY.whole + entity->velY.whole + bg->yPos.whole;
    if (v < 0)
        dy = sy + (-v >> 16);
    else
        dy = sy - (v >> 16);
    dy -= 0x1F;
    if ((u32)dx > 0x80 || (u32)dy > 0x10)
        return 0;
    return (s16)-dy;
}

/* Object effect helpers decompiled from assembly. */

/**
 * @brief Wall-left correction slot of the object 0x17 solid table (sElevatorSolidCallbacks):
 * none, return 0.
 */
s32 Elevator_WallLeftCorrection(void)
{
    return 0;
}

/**
 * @brief Wall-right correction slot of the object 0x17 solid table
 * (sElevatorSolidCallbacks): none, return 0.
 */
s32 Elevator_WallRightCorrection(void)
{
    return 0;
}

/* Load the effect once, then advance its animation on every update. */
/**
 * @brief Load the effect once (animation subState), then advance it every
 * update.
 */
void ElevatorEffect_Update(struct EwramData_EntityData *entity)
{
    if (entity->state == 0)
    {
        if (!(u8)Entity_LoadGraphics(entity, sElevatorGfx, sUnk_08209FD8, 4, sElevatorSpriteSheet))
        {
            EntityDelete(entity);
            return;
        }
        entity->spriteLayer = 0x11;
        Anim_StartIndexed(entity, entity->subState, 0, 1);
        entity->state++;
    }
    Anim_Update(entity);
}

/* Follow the entity stored in unk_4F8 and advance the animation. */
/**
 * @brief Follow the entity in unk_4F8 and advance the animation.
 */
void ElevatorFollower_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = ENTITY_OWNER(entity);
    entity->posX.whole = parent->posX.whole;
    entity->posY.whole = parent->posY.whole;
    Anim_Update(entity);
}

/* BreakableCoinContainerDebris_Update: variant of an already decompiled function, generated with
 * a cloning script (constants adjusted where needed), checked byte for
 * byte. */

/**
 * @brief Same as BreakablePropPiece_Update: slide, fall and burst with dust when
 * it lands.
 */
void BreakableCoinContainerDebris_Update(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg;
    s32 x;

    EntityDampVelocityX(entity, 0x3C);
    EntityAccelerateAndMove(entity, 0x80000);
    x = (s16)entity->posX.part16.integer;
    bg = &gEwramData->bgInfo[1];
    if (Collision_GetGroundOffsetWide(x + (s16)bg->xPos.part16.integer,
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
 * @brief Spawn the four debris pieces (frames 1..4, BreakableCoinContainerDebris_Update) of
 * the parent at its part offsets, flung with a share of its speed.
 */
void BreakableCoinContainer_SpawnDebris(struct EwramData_EntityData *parent)
{
    struct EwramData_EntityData *entity;
    s32 frame;
    s32 i;
    s16 offset[2];
    s16 *p;

    frame = 1;
    for (i = 0; i <= 3; i++, frame++)
    {
        entity = EntityCreateInRange(ENTITY_SLOT_EFFECT_FIRST, ENTITY_SLOT_EFFECT_LAST, BreakableCoinContainerDebris_Update);
        if (entity == NULL)
            return;
        if (!Entity_LoadGraphics(entity, sBreakableCoinContainerGfx, sBreakableCoinContainerPalette, 0, sBreakableCoinContainerSpriteSheet))
        {
            EntityDelete(entity);
            return;
        }
        entity->animFrame = frame;
        entity->spriteLayer = 0x14;
        entity->flipX = parent->flipX;
        p = offset;
        Sprite_GetPartCenterOffset(parent, i, p);
        entity->posX.part16.integer = parent->posX.part16.integer + offset[0];
        entity->posY.part16.integer = parent->posY.part16.integer + p[1];
        Sprite_GetPartOriginOffset(entity, 0, offset);
        entity->posX.part16.integer += offset[0] / 4;
        entity->posY.part16.integer -= p[1] / 4;
        entity->velX.whole = parent->velX.whole * ((RandomNumberGenerator() & 0xFFFF) + 0x10000);
        entity->velY.whole = -0x20000 - RandomSigned(0x20000);
        entity->accelY.whole = 0x2000;
        entity->scrollWithBg = 1;
    }
}

void BreakableCoinContainerCreate(struct EwramData_EntityData *entity)
{
    if (!Entity_LoadGraphics(entity, sBreakableCoinContainerGfx, sBreakableCoinContainerPalette, 0, sBreakableCoinContainerSpriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    entity->spriteLayer = 0x14;
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    Hit_Enable(entity, 0, NULL, BreakableCoinContainer_OnHit);
    entity->hit.flags |= HIT_NO_ATTACK;
    HitBox_SetBodyRaw(entity, 0x1820E8F0);
    Entity_SnapToGround(entity);
    entity->scrollWithBg = 1;
}

/**
 * @brief 53968 | Breakable object (object 0x18): hidden off screen; when hit
 * (Hit_CheckWithPlayer flags 0x1E) it breaks (BreakableCoinContainer_SpawnDebris, sound 0xA2) and drops
 * up to eight coins (Pickup_CreateGold, Soma only; Julius gets Pickup_CreateMana), then
 * smokes for 40 frames; with spawnParam0 set it instead releases enemy 0x4A
 * (sEnemyData[0x4A]) every 4th frame for 32 frames.
 */
void BreakableCoinContainerUpdate(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    u16 hit;
    s32 x;
    s32 y;
    s32 i;
    struct EwramData_EntityData *e;
    const struct EntityTemplate *info;

    hit = Hit_CheckWithPlayer(&result, entity);
    x = (s16)entity->posX.part16.integer;
    y = (s16)entity->posY.part16.integer - 8;
    if (!Entity_IsInRect(entity, -0x20, -0x20, 0x130, 0xE0))
    {
        entity->hidden = 1;
        return;
    }
    switch (entity->state)
    {
    case 0:
        entity->hidden = 0;
        if (!(hit & 0x1E))
            break;
        if (entity->spawnParam0)
        {
            entity->state = 2;
            entity->timer = 0x20;
            BreakableCoinContainer_SpawnDebris(entity);
            PlaySong(SE_ITEM_DROP);
            entity->hidden = 1;
            break;
        }
        if (!gEwramData->playerStatus.currentCharacter)
        {
            for (i = 0; i <= 7; i++)
            {
                e = Pickup_CreateGold(x + RandomSigned(0x20), y - (RandomNumberGenerator() & 0xF), i / 2);
                if (e == NULL)
                    break;
                e->updateFunc = Pickup_UpdateBouncing;
                e->velX.whole = RandomSigned(0x100) << 10;
                e->velY.whole = RandomSigned(0x8000) - 0x20000;
                e->accelY.whole = 0x2000;
            }
        }
        else
        {
            Pickup_CreateMana(x, y, 1, 0);
        }
        BreakableCoinContainer_SpawnDebris(entity);
        PlaySong(SE_ITEM_DROP);
        entity->hidden = 1;
        entity->timer = 0x28;
        entity->state++;
        break;
    case 1:
        if (entity->timer > 0x1E && !(entity->timer & 3))
            Effect_SpawnRandom(entity->posX.whole, entity->posY.whole - 0x100000, 0x80000, 0x100000, entity->spriteLayer - 1);
        if (--entity->timer == 0xFF)
            entity->deleteRequested = 1;
        break;
    case 2:
        if (!(entity->timer & 3))
        {
            info = &sEnemyData[0x4A];
            e = EntityCreateInRange(ENTITY_SLOT_OBJECT_FIRST, ENTITY_SLOT_OBJECT_LAST, EntityUpdateNothing);
            if (e != NULL)
            {
                e->posX.part16.integer = entity->posX.part16.integer + RandomSigned(0x20);
                {
                    s32 dy = RandomSigned(0x10);
                    dy -= 0x18;
                    e->posY.part16.integer = dy + entity->posY.part16.integer;
                }
                e->enemyId = 0x4A;
                e->hp = info->hp;
                e->updateFunc = info->updateFunc;
                e->scrollWithBg = 1;
                info->createFunc(e);
            }
        }
        if (--entity->timer == 0xFF)
            entity->deleteRequested = 1;
        break;
    }
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   BreakableCoinContainer_OnHit = BreakableProp_OnHit
 *
 * SpinningBlade_OnPlayerContact decompiled locally. */

/**
 * @brief Hit callback of object 0x18: set velX to +1/-1 away from the attacker.
 */
void BreakableCoinContainer_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker)
{
    s32 speed = 1;

    entity->velX.whole = speed;
    if (entity->posX.whole < attacker->posX.whole)
        entity->velX.whole = -speed;
}

/**
 * @brief Player contact callback of object 0x19: contact damage of 2/3 of the
 * strength-based value used by CrusherCreate, at the entity position.
 */
void SpinningBlade_OnPlayerContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;
    s32 power;

    contactDamage.attributes = 1;
    contactDamage.unused = 0;
    power = (gEwramData->playerState.equipmentDefenseBonus >> 1) + (gEwramData->playerState.equipmentDefenseBonus >> 2);
    power += gEwramData->playerStatus.maxHP >> 4;
    power = (u16)power;
    contactDamage.attack = power * 2 / 3;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}
