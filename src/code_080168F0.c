/* Soma bat-form exit collision (floor probe at y + 7, two ceiling probes,
 * walls from the offset table at 0x080E12F4 with bounce). Helpers copied
 * from code_08016DE4.c. */
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

#define FUNCS(object) ((struct SolidObjectCallbacks *)(object)->unk_4F4.unk_4F4_p32)
#define FLAGS(e) ((e)->unk_4F4.whole)
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

        if (player->velY.whole >= 0 && (!(player->unk_4F4.whole & 2) || player->unk_4F8.part16.unk_4F8 == 0x10))
            c = PlatformFloor(x, y, c);
    }
    UpdateFloorKind(c);
    return c;
}

/**
 * @brief Push the player out of terrain when leaving bat state 8:
 * floor probe at +7 px, ceiling probes at -15/-31 px and walls
 * at +-8 px from the 0x080E12F4 offsets. A wall bounce reverses and
 * quarters velX unless flag 0x80 is set; sets wall-contact bit 0x40000.
 */
void Soma_ResolveBatExitCollision(struct EwramData_EntityData *entity)
{
    u16 x = GetEntityRoomXPositionInteger(entity);
    u16 y = GetEntityRoomYPositionInteger(entity);
    s16 c;
    s32 i;
    s8 *offsets;
    s32 count;
    u16 probe;
    s32 r;
    u32 flags;

    s16 d;

    c = FloorProbe(X, Y + 7, 0);
    if ((s16)c != 0)
    {
        r = c + 7;
        r <<= 16;
        goto adjust;
    found:
        r = d << 16;
    adjust:
        entity->posY.whole += r;
        entity->velY.whole = 0;
        y = GetEntityRoomYPositionInteger(entity);
        goto walls;
    zero_left:
        entity->accelX.whole = 0;
        entity->velX.whole = 0;
        goto hit_left;
    zero_right:
        entity->accelX.whole = 0;
        entity->velX.whole = 0;
        goto hit_right;
    }
    for (i = 15; i <= 31; i += 16)
    {
        d = CeilingCorrection(X, ({ s32 t_ = Y; t_; }) - i);

        if (d != 0)
            goto found;
    }

walls:
    probe = 0;
    offsets = (s8 *)0x080E12F4;
    count = offsets[0];
    for (i = 0; i < count; i++)
    {
        s16 d = WallLeftCorrection(({ s32 t_ = X; t_; }) - 8, ({ s32 o_ = offsets[i + 1]; (Y) + o_; }));

        probe = d;
        if (d != 0)
        {
            entity->posX.whole += d << 16;
            flags = FLAGS(entity);
            if (flags & 0x80)
                goto zero_left;
            entity->velX.whole = -entity->velX.whole;
            entity->velX.whole /= 4;
            entity->accelX.whole = -entity->accelX.whole;
        hit_left:
            FLAGS(entity) = flags | 0x40000;
            break;
        }
    }
    if (probe == 0)
    {
        for (i = 0; i < count; i++)
        {
            s16 d = WallRightCorrection(({ s32 t_ = X; t_; }) + 8, ({ s32 o_ = offsets[i + 1]; (Y) + o_; }));

            if (d != 0)
            {
                entity->posX.whole += d << 16;
                flags = FLAGS(entity);
                if (flags & 0x80)
                    goto zero_right;
                entity->velX.whole = -entity->velX.whole;
                entity->velX.whole /= 4;
                entity->accelX.whole = -entity->accelX.whole;
            hit_right:
                FLAGS(entity) = flags | 0x40000;
                break;
            }
        }
    }
}
