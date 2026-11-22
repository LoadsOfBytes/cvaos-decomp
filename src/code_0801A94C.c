/* Soma transformation-dash state 9 (Devil/Manticore/Curly souls: charge,
 * sparks, dash with ceiling probes, stop). Helpers copied from
 * code_08016DE4.c. */
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
        if (x >= px && x <= right && y >= py - up && y <= py + (extra + 8))
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

struct PlayerBodyBox {
    u8 x;
    u8 y;
    u8 width;
    u8 height;
};

extern struct PlayerBodyBox sSomaStandingBodyBox[];
extern u8 sSomaAnimPalettes[];
extern u8 sSomaPalette[];
extern u8 *sUnk_084F10A0[];
extern void Soma_UpdateVerticalMovement(struct EwramData_EntityData *entity);
extern void BlueSoul_StopAll(void);

#define B53E(e) (*((u8 *)(e) + 0x5A))

static inline void SetAnim(struct EwramData_EntityData *entity, u8 anim, s32 flag)
{
    struct PlayerBodyBox box = sSomaStandingBodyBox[0];
    s32 pal;

    if (!(gEwramData->playerState.actionLockFlags & 0x800) && entity->animScriptIndex != anim)
    {
        Anim_StartIndexed(entity, anim, 3, flag);
        HitBox_SetBody(entity, &box);
        PLAYER_REQUESTED_ANIM(entity) = 0xFF;
        PLAYER_ACTION_FLAGS(entity) &= ~0x200000;
        pal = sSomaAnimPalettes[anim];
        if (PLAYER_ANIM_PALETTE(entity) != pal)
        {
            PLAYER_ANIM_PALETTE(entity) = pal;
            ObjPalette_LoadRange(sSomaPalette, pal, 1, 0);
        }
    }
}

/**
 * @brief Player state 9 entered by Skill_Devil_Manticore_Curly_Use
 * (soulStateFlags bit 0x1000): charge animation with sparks and flashing,
 * per-form animations from sUnk_084F10A0, accelerating dash to
 * +-0x60000, ceiling checks that stop blue souls, then deceleration.
 */
void Soma_UpdateTransformationDash(struct EwramData_EntityData *entity)
{
    u8 *anims;

    {
        u8 **tbl = sUnk_084F10A0;
        anims = tbl[entity->unk_508.part8.unk_50B];
    }

    if (entity->subState <= 3 && !(gEwramData->playerStatus.soulStateFlags & 0x1000))
        entity->subState = 4;

    switch (entity->subState)
    {
    case 0:
        entity->stage = 0;
        entity->velX.whole = 0;
        PLAYER_REQUESTED_ANIM(entity) = 0xFF;
        SetAnim(entity, 0x20, 1);
        entity->timer = 0x2D;
        entity->subState++;
        FLAGS(entity) &= ~0x400;
        break;
    case 1:
    {
        u8 c = entity->timer;

        if (entity->timer != 0)
        {
            s32 t;

            if (entity->timer <= 0x1D && !(t = c & 3))
            {
                s32 px = entity->posX.whole + (RandomSigned(8) << 16);
                s32 py = entity->posY.whole - (((RandomNumberGenerator() & 7) + 8) << 16);

                Effect_CreateFromTable(px, py, 0, ((RandomNumberGenerator() & 0x1F) << 4) - 0x3000, t, 6);
            }
            if (entity->timer <= 0xF)
            {
                if (entity->timer & ({ s32 k = 2; k; }))
                    entity->palette = 4;
                else
                    entity->palette = 0;
            }
            if (entity->timer == 0x23)
                PlaySong(0x1AB);
            entity->timer--;
            break;
        }
        if ((u16)PLAYER_REQUESTED_ANIM(entity) == 0xFF)
        {
            SetAnim(entity, anims[0], 0);
            entity->stage = 1;
            entity->subState++;
        }
    }
    case 2:
        if (FLAGS(entity) & 0x200000)
        {
            entity->subState = 3;
            SetAnim(entity, anims[1], 1);
        }
    case 3:
    {
        u16 x, y;

        {
            u8 two = 2;
            PLAYER_INVULNERABLE_TIMER(gEwramData->playerState.player) = two;
        }
        PLAYER_DRAW_OFFSET_Y(entity) = 0xF0;
        if (entity->subState == 3)
        {
            s32 speed;

            entity->timer++;
            if (entity->flipX)
            {
                entity->velX.whole += -0x1000;
                if (entity->velX.whole < -0x60000)
                    entity->velX.whole = -0x60000;
            }
            else
            {
                entity->velX.whole += 0x1000;
                if (entity->velX.whole > 0x60000)
                    entity->velX.whole = 0x60000;
            }
            speed = entity->velX.whole;
            if (speed < 0)
                speed = -speed;
            if ((FLAGS(entity) & 0x1400000) == 0x1000000 && speed > 0xC000 && !(entity->timer & 3))
                Player_SpawnSurfaceEffect(1);
            PLAYER_WALL_PROBES(entity) = 0x080E12EF;
            Soma_UpdateVerticalMovement(entity);
        }
        x = GetEntityRoomXPositionInteger(entity);
        y = GetEntityRoomYPositionInteger(entity);
        if ((s16)CeilingCorrection(X, Y - 0x37) != 0
            || (s16)CeilingCorrection(({ s32 t_ = X; t_; }) + 8, ({ s32 t_ = Y; t_; }) - 0x32) != 0
            || (s16)CeilingCorrection(({ s32 t_ = X; t_; }) - 8, ({ s32 t_ = Y; t_; }) - 0x32) != 0)
            BlueSoul_StopAll();
        break;
    }
    case 4:
        FLAGS(entity) &= ~0x200000;
        entity->timer = 0;
        if (entity->stage)
            SetAnim(entity, anims[2], 0);
        else
            FLAGS(entity) |= 0x200000;
        entity->subState++;
    case 5:
    {
        s32 old, new;

        entity->accelX.whole = (entity->velX.whole < 0) ? 0x6000 : -0x6000;
        old = entity->velX.whole;
        new = old + entity->accelX.whole;
        entity->velX.whole = new;
        if (old < 0)
            goto check_x;
        if (new < 0)
            goto clear_x;
        if (old > 0)
            goto done_x;
    check_x:
        if (new <= 0)
            goto done_x;
    clear_x:
        entity->velX.whole = 0;
        entity->accelX.whole = 0;
    done_x:
        if (!(FLAGS(entity) & 0x200000))
        {
            {
                u8 two = 2;
                PLAYER_INVULNERABLE_TIMER(gEwramData->playerState.player) = two;
            }
            gEwramData->playerState.actionLockFlags |= 6;
            entity->timer++;
            entity->palette = 4;
            PLAYER_DRAW_OFFSET_Y(entity) = 0xF0;
            break;
        }
        if (entity->timer)
            PLAYER_DRAW_OFFSET_Y(entity) = 0xF0;
        entity->velX.whole = 0;
        entity->velY.whole = 0;
        entity->accelX.whole = 0;
        entity->accelY.whole = 0;
        FLAGS(entity) &= 0xEFFFFE9F;
        entity->palette = 0;
        entity->state = 0;
        break;
    }
    }
}
