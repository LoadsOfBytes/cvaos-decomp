/*
 * Room object 0x14: terrain movement, wall / floor / ceiling corrections, debris and its three variants.
 */
#include "roller.h"
#include "player_core.h"
#include "syscalls.h"
#include "structs/ewram.h"
#include "core.h"
#include "sound.h"
#include "gameplay.h"
#include "structs/entity_render_flags.h"
#include "music.h"
#include "entity_util.h"
#include "entity_common.h"
#include "hitbox.h"
#include "oam.h"
#include "obj_gfx.h"
#include "data/data_0E5364.h"
#include "data/data_118B48.h"
#include "entity_motion.h"
#include "bg_and_collision.h"

extern void (*const sRollerVariantFuncs[])(struct EwramData_EntityData *entity);

void RollerDebris_Update(struct EwramData_EntityData *);
void Roller_Draw(struct EwramData_EntityData *);

/* Move an entity, settle it on the floor and tilt it along the slope
 * (Roller_MoveOnTerrain). The do/while (0) around the posY update doubles the weight
 * of those references so global-alloc orders off > d > dx (r5, r6, r7). */

/**
 * @brief Move with velocity and gravity, snap onto the floor below and turn the
 * angle towards the slope ahead; return the floor push.
 */
s32 Roller_MoveOnTerrain(struct EwramData_EntityData *entity, s32 mode)
{
    s16 x, y, off, d, dx; s32 d2;

    off = 0;
    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
    entity->velY.whole += entity->accelY.whole;
    Player_QueueEntity(entity, 7);
    x = (s16)GetEntityRoomXPositionIntegerWide(entity);
    y = (s16)GetEntityRoomYPositionIntegerWide(entity);
    if (entity->velY.whole == 0)
        off = 8;
    d = Collision_GetPushUpFiltered(x, y + off, mode);
    if (d != 0)
        do { entity->posY.whole += (d + off) << 16; } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
    dx = 8;
    if (entity->velX.whole < 0)
        dx = -dx;
    d2 = Collision_GetPushUpFiltered(x + dx, y + off, mode);
    if (d2 != 0)
        { s32 angle = ArcTan2(dx, off + d2); Angle_TurnTowards((u16 *)&entity->ext.roller.angle, 0x80, angle); }
    return d;
}

/* Spawns two debris pieces (animFrame = 3, mirrored) and four random ones (animFrame 4-5)
 * around the parent. */

/**
 * @brief Spawn the debris pieces (RollerDebris_Update) of object 0x14 around
 * the parent.
 */
void Roller_SpawnDebris(struct EwramData_EntityData *parent)
{
    struct EwramData_EntityData *entity;
    s32 i;

    for (i = 0; i <= 1; i++)
    {
        entity = EntityCreateInRange(0x49, 0xCF, RollerDebris_Update);
        if (entity != NULL)
        {
            Entity_LoadGraphics(entity, sRollerGfx, sRollerPalette, 0, sRollerSpriteSheet);
            entity->animFrame = 3;
            entity->posX.part16.integer = (s16)parent->posX.part16.integer + (i * 16 - 16);
            entity->posY.part16.integer = parent->posY.part16.integer;
            entity->velX.whole = (i << 17) + ((RandomSigned(8) << 12) + ({ s32 k_ = 0x10000; -k_; }));
            entity->velY.whole = (RandomSigned(0x10) << 12) - 0x30000;
            entity->accelY.whole = 0x2000;
            entity->spriteLayer = parent->spriteLayer;
            entity->flipX = i;
            entity->scrollWithBg = 1;
        }
    }
    for (i = 0; i < 4; i++)
    {
        entity = EntityCreateInRange(0x49, 0xCF, RollerDebris_Update);
        if (entity != NULL)
        {
            Entity_LoadGraphics(entity, sRollerGfx, sRollerPalette, 0, sRollerSpriteSheet);
            { register u32 k_ asm("r1"); s32 rn_ = RandomNumberGenerator(); k_ = 1; entity->animFrame = (rn_ & k_) + 4; } /* FAKEMATCH: pinned register */
            entity->posX.part16.integer = parent->posX.part16.integer + RandomSigned(0x10);
            entity->posY.part16.integer = parent->posY.part16.integer + RandomSigned(0x10);
            entity->velX.whole = RandomSigned(0x10) << 12;
            entity->velY.whole = (RandomSigned(0x20) << 12) - 0x30000;
            entity->accelY.whole = 0x2000;
            entity->spriteLayer = parent->spriteLayer;
            entity->flipX = RandomNumberGenerator() & 1;
            entity->flipY = RandomNumberGenerator() & 1;
            entity->scrollWithBg = 1;
        }
    }
}

/* Entity with rise/fall states whose affine sprite is scaled and turned by the player distance. */
static inline struct EwramData_EntityData *Player(struct EwramData **ew)
{
    return (*ew)->playerState.player;
}

static inline s32 PackY(s32 value, u8 mask)
{
    return ((value < 0 ? -(-value >> 16) : value >> 16) & mask) << 8;
}

/**
 * @brief Update of object 0x14 variant 0 (sRollerVariantFuncs[0]): affine sprite that
 * rolls along the terrain from its start position.
 */
void RollerVariant0_Update(struct EwramData_EntityData *entity)
{
    s32 length, x, y, radius;
    if (gEwramData->gameState.timeStopTimer != 0) {
        if (entity->state > 1)
            Player_QueueEntity(entity, 0);
        return;
    }
    switch (entity->state) {
    case 0: {
        s16 slot = ObjAffine_Alloc(1);
        if (slot < 0) {
            EntityDelete(entity);
            return;
        }
        entity->hidden = 0;
        entity->spriteLayer = 12;
        entity->affineIndex = slot;
        entity->affine = 1;
        entity->drawFunc = Roller_Draw;
        entity->ext.roller.roomX = GetEntityRoomXPositionIntegerWide(entity);
        entity->ext.roller.roomY = GetEntityRoomYPositionIntegerWide(entity);
        entity->ext.roller.prevX = entity->ext.roller.roomX;
        entity->ext.roller.prevY = entity->ext.roller.roomY;
        entity->velX.whole = 0x8000;
        Hit_Enable(entity, 8, 0, 0);
        HitBox_SetBodyRaw(entity, 0x103000E8);
        entity->hit.flags |= 2;
        entity->hit.flags |= 4;
        entity->state = 1;
    }
    case 1:
        entity->animFrame = 0;
        entity->ext.roller.scale += 0x400;
        if (entity->ext.roller.scale > 0xFFFF) {
            entity->animFrame = 2;
            entity->ext.roller.scale = 0x10000;
            entity->state = 2;
        }
        break;
    case 2:
        if (!Roller_MoveOnTerrain(entity, 2)) {
            entity->state = 3;
            entity->accelY.whole = 0x2000;
        }
        break;
    case 3:
        if (Roller_MoveOnTerrain(entity, 1)) {
            if (entity->velY.whole > 0x40000)
                entity->state = 4;
            else {
                s32 zero = 0;
                entity->state = 2;
                entity->velY.whole = zero;
                entity->accelY.whole = zero;
            }
        }
        break;
    case 4: {
        struct BgLayerState *bg;
        if (Entity_IsInRect(entity, -16, 0, 0x110, 0xB0)) {
            Roller_SpawnDebris(entity);
            PlaySong(0xAC);
        }
        entity->ext.roller.scale = 0;
        entity->ext.roller.angle = 0;
        entity->velY.whole = 0;
        entity->accelY.whole = 0;
        bg = &gEwramData->bgInfo[1];
        entity->posX.part16.integer = entity->ext.roller.roomX - bg->xPos.part16.integer;
        entity->posY.part16.integer = entity->ext.roller.roomY - bg->yPos.part16.integer;
        entity->state = 1;
        break;
    }
    }
    length = Entity_DistanceToPoint(entity, Player(&gEwramData)->posX.whole, Player(&gEwramData)->posY.whole);
    if (length > 0xC0000)
        length = 0xC0000;
    if (Player(&gEwramData)->posX.whole < entity->posX.whole)
        length = -length;
    radius = entity->ext.roller.angle == 0 ? 10 : 12;
    x = (length < 0 ? -(-length >> 16) : length >> 16) * Sin(entity->ext.roller.angle + 0x4000);
    y = (length < 0 ? -(-length >> 16) : length >> 16) * Sin(entity->ext.roller.angle);
    { s32 left = -radius;
      length = radius << 16;
      if (x < 0) left -= -x >> 16; else left += x >> 16;
      left &= 0xFF;
      { register u8 mask asm("r3") = 0xFF; /* FAKEMATCH: pinned register */
      length <<= 1;
      { s32 sum = left + PackY(y, mask) + length; { s32 k = 0x14000000; asm("" :: "r"(k)); left = sum + k; } HitBox_SetBodyRaw(entity, left); } } } /* FAKEMATCH: k stays a pseudo (no combine/equiv fold), sum chain stays in r0 */
    ObjAffine_Set(entity->ext.roller.angle, entity->ext.roller.scale, entity->ext.roller.scale,
                entity->flipX, 0, entity->affineIndex);
}

/* Roller_WallLeftCorrection. */
static inline s32 Roller_WallLeftCorrectionInline(struct EwramData_EntityData *entity, s16 x, s16 y)
{
    u32 band;
    s32 roomX;
    s32 roomY;
    struct BgLayerState *bg;
    s32 px = x;

    bg = &gEwramData->bgInfo[1];
    roomX = (px - (s16)bg->xPos.part16.integer) << 16;
    roomY = ((y - (s16)bg->yPos.part16.integer) << 16) + 0x40000;
    if ((u32)(entity->posY.whole - roomY) > 0x7FFFF)
        return 0;
    band = entity->posX.whole - roomX + 0x180000;
    if (band > 0x300000)
        return 0;
    if (band > 0x80000)
        return 0;
    entity->ext.roller.flags |= 2;
    {
        s32 v = band;
        return (v < 0) ? (s16)(-(-v >> 0x10)) : (s16)(v >> 0x10);
    }
}

/**
 * @brief 520FC | Contact test against a point given in screen pixels: when
 * the entity is within 8 px below it and at most 8 px to the right of the
 * 24 px left band edge, set bit 1 of unk_4F8 and return the band offset in
 * pixels; otherwise 0.
 */
s16 Roller_WallLeftCorrection(struct EwramData_EntityData *entity, s16 x, s16 y)
{
    return Roller_WallLeftCorrectionInline(entity, x, y);
}

/* Roller_WallRightCorrection. */
static inline s32 Roller_WallRightCorrectionInline(struct EwramData_EntityData *entity, s16 x, s16 y)
{
    s32 dx;
    u32 band;
    s32 roomX;
    s32 roomY;
    struct BgLayerState *bg;
    s32 px = x;

    bg = &gEwramData->bgInfo[1];
    roomX = (px - (s16)bg->xPos.part16.integer) << 16;
    roomY = ((y - (s16)bg->yPos.part16.integer) << 16) + 0x40000;
    if ((u32)(entity->posY.whole - roomY) > 0x7FFFF)
        return 0;
    dx = entity->posX.whole - roomX;
    band = dx + 0x180000;
    if (band > 0x300000)
        return 0;
    if (band <= 0x27FFFF)
        return 0;
    entity->ext.roller.flags |= 1;
    {
        s32 v = dx - 0x180000;
        return (v < 0) ? (s16)(-(-v >> 0x10)) : (s16)(v >> 0x10);
    }
}

/**
 * @brief 52180 | Right-hand counterpart of Roller_WallLeftCorrection: within the last
 * 8 px of the 48 px band set bit 0 of unk_4F8 and return the signed offset
 * from the band centre in pixels; otherwise 0.
 */
s16 Roller_WallRightCorrection(struct EwramData_EntityData *entity, s16 x, s16 y)
{
    return Roller_WallRightCorrectionInline(entity, x, y);
}

/* Move an entity, settle it on the ground and turn it toward the slope ahead. */
/**
 * @brief Variant-1 terrain movement: move with gravity, snap onto the floor and
 * follow the slope.
 */
s16 Roller_MoveOnTerrainSolid(struct EwramData_EntityData *entity, s32 layer)
{
    u16 fall = entity->velY.part16.integer;
    u16 x;
    u16 y;
    s16 ground;
    u16 dir;
    u16 off;
    s32 hit;
    s32 i;

    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
    entity->velY.whole += entity->accelY.whole;
    Player_QueueEntity(entity, 7);
    x = (s16)GetEntityRoomXPositionIntegerWide(entity);
    y = (s16)GetEntityRoomYPositionIntegerWide(entity) + 8;
    if (entity->velY.whole == 0)
        fall = 8;
    ground = Collision_GetPushUpFiltered((s16)x, (s16)y + (s16)fall, layer);
    if ((s16)ground != 0)
    {
        entity->posY.whole += (s16)ground << 16;
        if (entity->velY.whole == 0)
            entity->posY.whole += (s16)fall << 16;
    }
    dir = 8;
    if (entity->velX.whole < 0)
        dir *= -1;
    y = (s16)GetEntityRoomYPositionIntegerWide(entity) + 8;
    for (off = 0, i = 0; i <= 1; i++)
    {
        s16 so = (s16)off;
        hit = (u16)Collision_GetPushUpFiltered((s16)x + (s16)dir, (s16)y + so, layer);
        if ((s16)hit != 0)
        {
            hit = ((u32)(((s16)hit + so) << 16)) >> 16;
            break;
        }
        asm("" :: "r"(off)); /* FAKEMATCH: keeps off live across the call (callee-saved reg) */
        off = so + 8;
    }
    if (hit << 16)
    {
        u32 h = (u32)(hit << 16) >> 16;
        asm("" :: "r"(hit)); /* FAKEMATCH: hit overlaps h, so it avoids r1 */
        if (entity->velY.whole == 0)
            h = ((h << 16) + 0xFFF80000) >> 16; 
        {
             s32 a = ArcTan2(dir, h);
            Angle_TurnTowards((u16 *)&entity->ext.roller.angle, 0x400, a);
        }
    }
    return ground;
}

/**
 * @brief Update of object 0x14 variant 1 (sRollerVariantFuncs[1]): rolling affine
 * sprite with a body hit box, registered as solid object
 * (sRollerVariant1SolidCallbacks).
 */
void RollerVariant1_Update(struct EwramData_EntityData *entity)
{
    s32 length, x, y;
    switch (entity->state) {
    case 0: {
        s16 slot = ObjAffine_Alloc(1);
        if (slot < 0)
            { EntityDelete(entity); return; }
        entity->hidden = 0;
        entity->spriteLayer = 12;
        entity->affineIndex = slot;
        entity->affine = 1;
        entity->drawFunc = Roller_Draw;
        entity->animFrame = 2;
        entity->ext.roller.roomX = GetEntityRoomXPositionIntegerWide(entity);
        entity->ext.roller.roomY = GetEntityRoomYPositionIntegerWide(entity);
        entity->ext.roller.prevX = entity->ext.roller.roomX;
        entity->ext.roller.prevY = entity->ext.roller.roomY;
        Hit_Enable(entity, 8, 0, 0);
        HitBox_SetBodyRaw(entity, 0x102EFEE8);
        entity->hit.flags |= 2;
        entity->hit.flags |= 4;
        Player_RegisterSolidObject(entity, sRollerVariant1SolidCallbacks);
        entity->state = 1;
    }
    case 1:
        if (entity->ext.roller.flags & 1) {
            if (entity->velX.whole == 0)
                Sound_PlayTracked(0x136);
            entity->stage++;
            entity->velX.whole = 0x4000;
        } else {
            if (entity->velX.whole != 0)
                Sound_StopTracked(0x136);
            entity->velX.whole = 0;
        }
        Roller_MoveOnTerrainSolid(entity, 0);
        if (entity->ext.roller.angle != 0) {
            Sound_PlayTracked(0x136);
            entity->ext.roller.angle = 0;
            entity->state = 2;
            entity->timer = 8;
        }
        break;
    case 2:
        if (entity->timer && (u8)--entity->timer == 0)
            Player_ClearSolidObject();
        entity->stage++;
        if (entity->velX.whole <= 0x7FFFF)
            entity->velX.whole += entity->ext.roller.angle / 2;
        entity->velY.whole = entity->velX.whole;
        Roller_MoveOnTerrainSolid(entity, 0);
        if (entity->ext.roller.angle < 0) {
            Sound_StopTracked(0x136);
            entity->state = 3;
            entity->accelY.whole = 0x400;
            entity->velY.whole += -0x74000;
        }
        break;
    case 3: {
        s32 x, y;
        if (entity->velX.whole <= 0x7FFFF)
            entity->velX.whole += 0x800;
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        entity->velY.whole += entity->accelY.whole;
        Player_QueueEntity(entity, 7);
        Angle_TurnTowards(&entity->ext.roller.angle, 0x80, 0xE000);
        x = GetEntityRoomXPositionIntegerWide(entity) + 26;
        y = GetEntityRoomYPositionIntegerWide(entity);
        if (Collision_GetPushLeftFilteredWide(x, y, 0)) {
            Roller_SpawnDebris(entity);
            PlaySong(0xAC);
            ObjAffine_Free(entity->affineIndex);
            EntityDelete(entity);
            return;
        }
        break;
    }
    }
    entity->ext.roller.flags = 0;
    ObjAffine_Set(entity->ext.roller.angle, 0x10000, 0x10000, entity->flipX, 0, entity->affineIndex);
    if (entity->state > 1) {
        length = Entity_DistanceToPoint(entity, Player(&gEwramData)->posX.whole,
                            Player(&gEwramData)->posY.whole);
        if (length > 0xC0000)
            length = 0xC0000;
        if (Player(&gEwramData)->posX.whole < entity->posX.whole)
            length = -length;
        x = (length < 0 ? -(-length >> 16) : length >> 16) * Sin(entity->ext.roller.angle + 0x4000);
        y = Sin(entity->ext.roller.angle) * (length < 0 ? -(-length >> 16) : length >> 16);
        HitBox_SetBodyRaw(entity, (((x < 0 ? -(-x >> 16) : x >> 16) - 8) & 0xFF) + (((y < 0 ? -(-y >> 16) : y >> 16) & 0xFF) << 8) + 0x14140000);
    }
}

/* RollerVariant2_Update. */

static inline u16 RoomX(struct EwramData_EntityData *entity)
{
    u16 x = entity->posX.part16.integer;
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    return bg->xPos.part16.integer + x;
}

static inline u16 RoomY(struct EwramData_EntityData *entity)
{
    u16 y = entity->posY.part16.integer;
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    return bg->yPos.part16.integer + y;
}

/**
 * @brief Update of object 0x14 variant 2 (sRollerVariantFuncs[2]): carries the player
 * on top while moving.
 */
void RollerVariant2_Update(struct EwramData_EntityData *entity)
{
    s16 x;
    s16 y;
    struct BgLayerState *bg;
    s32 probeX;
    s32 probeY;
    s32 speed;

    if (Entity_IsInRect(entity, -0x80, -0x40, 0x1F0, 0x120) != 0)
    {
        Player_QueueEntity(entity, 3);
        if (gEwramData->playerState.standingPlatform == entity)
            Player_SetXFraction((s16)entity->posX.part16.fraction);
    }
    switch (entity->state)
    {
    case 0:
        entity->hidden = 0;
        entity->spriteLayer = 0xC;
        entity->animFrame = 2;
        entity->drawFunc = Roller_Draw;
        entity->ext.roller.roomX = (s16)GetEntityRoomXPositionIntegerWide(entity);
        entity->ext.roller.roomY = (s16)GetEntityRoomYPositionIntegerWide(entity);
        entity->ext.roller.prevX = entity->ext.roller.roomX;
        entity->ext.roller.prevY = entity->ext.roller.roomY;
        Hit_Enable(entity, 8, NULL, 0);
        HitBox_SetBodyRaw(entity, 0x103000E8);
        entity->hit.flags |= 2;
        entity->hit.flags |= 4;
        if (RandomNumberGenerator() & 1)
            entity->ext.rollerSwing.angle = 0x8000;
        entity->ext.rollerSwing.speed = (RandomNumberGenerator() & 0x3F) + 0x40;
        entity->state = 1;
    case 1:
        entity->ext.rollerSwing.angle += entity->ext.rollerSwing.speed;
        entity->velX.whole = Sin(entity->ext.rollerSwing.angle) / 3;
        entity->posX.whole += entity->velX.whole;
        break;
    case 2:
        x = entity->velX.part16.integer + RoomX(entity);
        if (entity->velX.whole < 0)
            x -= 0x18;
        else
            x += 0x18;
        { u32 tmp_ = RoomY(entity) << 16; y = (s32)tmp_ >> 16; }
        if (Collision_GetGroundOffsetWide(x, y) && !Collision_IsPointSolid(x, y))
            entity->velX.whole = -entity->velX.whole / 2;
        EntityDampVelocityX(entity, 0x50);
        EntityAccelerateAndMove(entity, 0x20000);
        speed = entity->velX.whole;
        if (speed < 0)
            speed = -speed;
        if (speed <= 0x7FF)
            entity->state = 1;
        break;
    }
}

/* Object 0x14 and helpers 0x080527E4..0x080528DF. */

/* Graphics of object 0x14 and its update functions by variant (spawnParam0). */

/**
 * @brief 527E4 | Draw at the room position in unk_500 / unk_502 (made
 * screen relative to BG 1), then store the current room position there.
 */
void Roller_Draw(struct EwramData_EntityData *entity)
{
    s32 oldX = entity->posX.whole;
    s32 oldY = entity->posY.whole;
    struct BgLayerState *bg = &gEwramData->bgInfo[1];

    entity->posX.part16.integer = entity->ext.roller.prevX - bg->xPos.part16.integer;
    entity->posY.part16.integer = entity->ext.roller.prevY - bg->yPos.part16.integer;
    Sprite_DrawEntity(entity);
    entity->posX.whole = oldX;
    entity->posY.whole = oldY;
    entity->ext.roller.prevX = (s16)GetEntityRoomXPositionIntegerWide(entity);
    entity->ext.roller.prevY = (s16)GetEntityRoomYPositionIntegerWide(entity);
}

/**
 * @brief 52834 | Slow down sideways, fall (max 0x80000), end (0x53D bit 3)
 * outside the screen.
 */
void RollerDebris_Update(struct EwramData_EntityData *entity)
{
    EntityDampVelocityX(entity, 0x80);
    EntityAccelerateAndMove(entity, 0x80000);
    if (EntityIsOutsideScreen(entity, 4))
        entity->deleteRequested = 1;
}

/**
 * @brief Ceiling correction slot of the solid table sRollerVariant1SolidCallbacks (object 0x14
 * variant 1): none, return 0.
 */
s32 Roller_CeilingCorrection(void)
{
    return 0;
}

/**
 * @brief Floor correction slot of the solid table sRollerVariant1SolidCallbacks: none, return
 * 0.
 */
s32 Roller_FloorCorrection(void)
{
    return 0;
}

/**
 * @brief 52870 | Object 0x14 create: load the graphics (delete the object
 * when that fails), hidden.
 */
void RollerCreate(struct EwramData_EntityData *entity)
{
    if (!Entity_LoadGraphics(entity, sRollerGfx, sRollerPalette, 0, sRollerSpriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    entity->flipX = 0;
    entity->hidden = 1;
    entity->scrollWithBg = 1;
}

/**
 * @brief 528C4 | Object 0x14 update: run the update of its variant.
 */
void RollerUpdate(struct EwramData_EntityData *entity)
{
    sRollerVariantFuncs[entity->spawnParam0](entity);
}
