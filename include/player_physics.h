#ifndef PLAYER_PHYSICS_H
#define PLAYER_PHYSICS_H

/* Collision and movement helpers shared by player/player_soma.c and
 * player/player_julius.c (both characters inline the same code).
 * Private to these two files: it defines short macros (FLAGS, X, Y ...).
 * The including file declares Anim_StartIndexed itself (the two files use
 * different prototypes). */

#include "types.h"
#include "structs/ewram.h"

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
#define HELD (gEwramData->inputData.playerHeldInput)
#define NEW (gEwramData->inputData.playerNewInput)
#define REQ(e) (*(u16 *)&e->ext.player.requestedAnim)
#define CanSnap(e, c) (FlagBit20(e) || !FlagBit12(e) || StepOk(e, c))

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

#endif // PLAYER_PHYSICS_H
