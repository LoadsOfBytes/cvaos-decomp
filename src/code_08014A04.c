/* Player collision with the tile map, the carrying object and moving platforms. */
#include "code_08009A0.h"
#include "player_entity.h"
#include "code_08001194.h"
#include "code_08032E4C.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"

struct PlayerBodyBox {
    u8 x;
    u8 y;
    u8 width;
    u8 height;
};

extern struct PlayerBodyBox sSomaStandingBodyBox[];
extern struct PlayerBodyBox sSomaAirborneBodyBox[];
extern struct PlayerBodyBox sSomaCrouchBodyBoxes[];
extern u8 sSomaAnimPalettes[];
extern u8 sSomaPalette[];

/* Function table of the object in solidObject (pointer at its unk_4F4). */
struct SolidObjectCallbacks {
    void *flags;
    s16 (*ceiling)(struct EwramData_EntityData *object, s32 x, s32 y);
    s16 (*floor)(struct EwramData_EntityData *object, s32 x, s32 y);
    s16 (*wallRight)(struct EwramData_EntityData *object, s32 x, s32 y);
    s16 (*wallLeft)(struct EwramData_EntityData *object, s32 x, s32 y);
    s32 (*push)(struct EwramData_EntityData *object, s16 x, s16 y);
};

#define FUNCS(object) ((struct SolidObjectCallbacks *)(object)->unk_4F4.unk_4F4_p32)
#define FLAGS(e) ((e)->unk_4F4.whole)
#define TILE_FLAGS (gUnk_03002CB0.collisionProbeFlags)
#define TIMER(e) (*(u16 *)&(e)->unk_4F8.part16.unk_4FA)
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
        entity->unk_504.part16.unk_504 = 0xFF;
        entity->unk_4F4.whole &= ~0x200000;
        pal = sSomaAnimPalettes[anim];
        if (PLAYER_ANIM_PALETTE(entity) != pal)
        {
            PLAYER_ANIM_PALETTE(entity) = pal;
            ObjPalette_LoadRange(sSomaPalette, pal, 1, 0);
        }
    }
}

static inline void SetPlayerBox(struct EwramData_EntityData *entity, s32 value, struct PlayerBodyBox *table)
{
    struct PlayerBodyBox box = table[0];

    entity->unk_504.part16.unk_504 = value;
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
    if (c != 0 && PLAYER_FLOOR_KIND(gEwramData->playerState.player) == 0xFF)
    {
        s32 kind;
        u8 d = TILE_FLAGS;

        kind = SlopeKind(d);
        if (kind == 0 && !(d & 0xC0) && (d & 4))
            kind = (d & 0x30) >> 4;
        else
            kind = 0xFF;
        PLAYER_FLOOR_KIND(gEwramData->playerState.player) = kind;
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
    if ((u8)entity->unk_500.part8.unk_501 < (TILE_FLAGS >> 6))
        entity->unk_500.part8.unk_501 = TILE_FLAGS >> 6;
}

static inline s16 PlatformFloor(s16 x, s16 y, u16 c)
{
    s32 i;

    for (i = 0; i < gEwramData->playerState.collisionQueueCount; i++)
    {
        struct HitBoxRecord a;
        struct HitBoxRecord b;
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

        if (player->velY.whole >= 0 && (!(player->unk_4F4.whole & 2) || player->unk_4F8.part16.unk_4F8 == 0x10))
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
    offsets = (s8 *)entity->unk_4FC.whole;
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
    entity->unk_500.part8.unk_500 = 0;
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
                    entity->unk_500.part8.unk_500 = 1;
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
    entity->unk_500.part8.unk_501 = 0;
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
        entity->unk_4F8.part16.unk_4F8 = 0;
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
