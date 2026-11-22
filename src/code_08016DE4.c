/* Soma bat-form flight collision with the tile map and the carrying object. */
#include "code_08009A0.h"
#include "player_entity.h"
#include "code_08001194.h"
#include "code_08032E4C.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"

/* Function table of the object in solidObject (pointer at its unk_4F4). */
struct SolidObjectCallbacks {
    void *flags;
    s16 (*ceiling)(struct EwramData_EntityData *object, s32 x, s32 y);
    s16 (*floor)(struct EwramData_EntityData *object, s32 x, s32 y);
    s16 (*wallRight)(struct EwramData_EntityData *object, s32 x, s32 y);
    s16 (*wallLeft)(struct EwramData_EntityData *object, s32 x, s32 y);
    s32 (*push)(struct EwramData_EntityData *object, s16 x, s16 y);
};

#define FUNCS(object) ((struct SolidObjectCallbacks *)SOLID_OBJECT_CALLBACKS((object)))
#define FLAGS(e) (PLAYER_ACTION_FLAGS((e)))
#define TILE_FLAGS (gUnk_03002CB0.collisionProbeFlags)
#define X ((s16)x)
#define Y ((s16)y)
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

        if (player->velY.whole >= 0 && (!(PLAYER_ACTION_FLAGS(player) & 2) || PLAYER_AIR_TIME(player) == 0x10))
            c = PlatformFloor(x, y, c);
    }
    UpdateFloorKind(c);
    return c;
}

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
    PLAYER_ON_STAIRS(entity) = side;
    x = GetEntityRoomXPositionInteger(entity);
    y = GetEntityRoomYPositionInteger(entity);
    if (Collision_IsPointSolid(X, Y - 8) != 0)
        FLAGS(entity) |= 0x1000000;
    offsets = (s8 *)PLAYER_WALL_PROBES(entity);
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
