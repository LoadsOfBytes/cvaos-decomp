/* Soma_UpdateVerticalMovement, decompiled from the original assembly (Soma variant of the
 * vertical physics; Julius uses Julius_UpdateVerticalMovement). 0x08018B98 is only the target
 * of a far jump inside this function. */
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

#define FUNCS(object) ((struct SolidObjectCallbacks *)object->ext.solidObject.callbacks.words)
#define FLAGS(e) (e->ext.player.actionFlags)
#define TILE_FLAGS (gUnk_03002CB0.collisionProbeFlags)
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

        if (player->velY.whole >= 0 && (!(player->ext.player.actionFlags & 2) || player->ext.player.airTime == 0x10))
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
extern struct PlayerBodyBox sSomaAirborneBodyBox[];
#define BOX(i) ((i) ? sSomaAirborneBodyBox[0] : sSomaStandingBodyBox[0])
extern u8 sSomaAnimPalettes[];
extern u8 sSomaPalette[];
extern s32 Soma_TryCeilingImpact(struct EwramData_EntityData *entity);

/* Animation switch with hitbox and palette, as in Soma_TryCeilingImpact. */
static inline void SetAnim(struct EwramData_EntityData *entity, u8 anim, s32 mode, s32 boxIndex)
{
    struct PlayerBodyBox box;

    box = BOX(boxIndex);
    if (!(gEwramData->playerState.actionLockFlags & 0x800) && entity->animScriptIndex != anim)
    {
        s32 pal;
        Anim_StartIndexed(entity, anim, 3, mode);
        HitBox_SetBody(entity, &box);
        entity->ext.player.requestedAnim = 0xFF;
        FLAGS(entity) &= ~0x200000;
        pal = sSomaAnimPalettes[anim];
        if (entity->ext.player.animPalette != pal)
        {
            entity->ext.player.animPalette = pal;
            ObjPalette_LoadRange(sSomaPalette, pal, 1, 0);
        }
    }
}

#define FC(dx, dy) CeilingCorrection(((x << 16) + (dx)) >> 16, ((y << 16) + (dy)) >> 16)
#define FP(dx) FloorProbe(((x << 16) + (dx)) >> 16, ((y << 16) + 0x10000) >> 16, 0)

/**
 * @brief Update Soma grounded/airborne vertical motion, gravity,
 * ceiling/floor and moving-platform probes, landing behavior,
 * animation, body box and palette under the existing flag guards.
 */
void Soma_UpdateVerticalMovement(struct EwramData_EntityData *entity)
{
    u16 corr = 0;
    s16 cs;
    u16 y = GetEntityRoomYPositionInteger(entity);
    u16 x = GetEntityRoomXPositionInteger(entity);
    u32 flags;
    u32 f2;
    u16 c1;
    s32 landed;
    u16 c;

    if (!(FLAGS(entity) & 0x100002))
    {
        FLAGS(entity) |= 2;
        entity->velY.whole = corr;
        entity->accelY.whole = -0x1000;
    }
    flags = FLAGS(entity);
    if ((flags & 0x800010) == 0x800000)
    {
        FLAGS(entity) = flags & 0xEFEFFFFF;
        if (FLAGS(entity) & 0x2000000)
        {
            if (entity->velY.whole < -0x10000)
            {
                entity->velY.whole += 0x4000;
                if (entity->velY.whole > -0x10000)
                {
                    entity->velY.whole = -0x10000;
                    FLAGS(entity) &= ~0x2000000;
                }
            }
            else if (entity->velY.whole > -0x10000)
            {
                entity->velY.whole -= 0x8000;
                if (entity->velY.whole < -0x10000)
                {
                    entity->velY.whole = -0x10000;
                    FLAGS(entity) &= ~0x2000000;
                }
            }
        }
        {
        u32 t = FLAGS(entity);
        u32 m = t & 0x08000800;
        f2 = t;
        if (!m)
        {
            if (!(f2 & 0x2000000))
            {
                entity->accelY.whole = 0;
                entity->velY.whole -= 0x8000;
                if (entity->velY.whole < -0x10000)
                    entity->velY.whole = -0x10000;
            }
            cs = FC(-0x50000, -0x200000);
            corr = cs;
            if (cs != 0)
            {
                entity->posY.whole += corr << 16;
                entity->velY.whole = -0x10000;
                f2 = FLAGS(entity);
            }
            else
            {
                cs = FC(0x50000, -0x200000);
                corr = cs;
                f2 = FLAGS(entity);
                if (cs != 0)
                {
                    entity->posY.whole += corr << 16;
                    entity->velY.whole = -0x10000;
                }
            }
        }
        else if (entity->velY.whole < 0 && !(f2 & 0x2000000))
            entity->velY.whole = 0;
        }
        if (f2 & 0x800)
        {
            cs = FC(-0x50000, -0x200000);
            corr = cs;
            if (cs == 0)
                cs = FC(0x50000, -0x200000);
            corr = cs;
            if (cs != 0)
                entity->posY.whole += corr << 16;
        }
        landed = 0;
        c = FP(0);
        {
            s32 d = (s16)c;
            if (d != 0)
            {
                entity->posY.whole += (d + 1) << 16;
                landed = 1;
            }
        }
        if (entity->velY.whole < 0)
        {
            c1 = FP(-0x50000);
            c = FP(0x50000);
            if (!landed)
            {
                {
                    s32 d = (s16)c1;
                    if (d != 0)
                    {
                        entity->posY.whole += (d + 1) << 16;
                        landed = 1;
                    }
                }
                if (!landed)
                {
                    s32 d = (s16)c;
                    if (d != 0)
                        entity->posY.whole += (d + 1) << 16;
                }
            }
        }
        if (FLAGS(entity) & 0x1E0)
            return;
        if ((u16)entity->ext.player.requestedAnim == 0x29)
            return;
        SetAnim(entity, 0x28, 1, 0);
    }
    else
    {
        if (!(flags & 0x1E))
            return;
        FLAGS(entity) = flags & 0xEFEFFBFF;
        if (FLAGS(entity) & 0x4000000)
        {
            entity->velY.whole += 0x6000;
            entity->accelY.whole = corr;
        }
        if (entity->ext.player.airTime <= 0xF)
            entity->ext.player.airTime++;
        if (entity->velY.whole <= 0 || entity->velX.whole != 0)
        {
            cs = FC(-0x50000, -0x200000);
            corr = cs;
            if (cs != 0)
            {
                entity->posY.part16.integer += ({ s32 t = (s16)corr; t; });
                if (entity->velY.whole < 0)
                {
                    if (Soma_TryCeilingImpact(entity))
                        return;
                    entity->velY.whole = 0x1000;
                    entity->accelY.whole = -0x2000;
                }
            }
            else
            {
                cs = FC(0x50000, -0x200000);
                corr = cs;
                if (cs != 0)
                {
                    entity->posY.part16.integer += ({ s32 t = (s16)corr; t; });
                    if (entity->velY.whole < 0)
                    {
                        if (Soma_TryCeilingImpact(entity))
                            return;
                        entity->velY.whole = 0x1000;
                        entity->accelY.whole = -0x2000;
                    }
                }
            }
        }
        if (entity->velY.whole <= 0x1FFF)
            entity->velY.whole += 0x2000;
        entity->velY.whole += 0x1A00;
        if (entity->velY.whole > 0)
        {
            entity->velY.whole += entity->accelY.whole;
            entity->accelY.whole += 0x400;
            if (entity->accelY.whole > 0x1000)
                entity->accelY.whole = 0x1000;
        }
        if (FLAGS(entity) & 0x10)
        {
            cs = FC(-0x50000, -0x180000);
            corr = cs;
            if (cs != 0)
            {
                entity->posY.part16.integer += ({ s32 t = (s16)corr; t; });
                if (entity->velY.whole < 0)
                {
                    if (Soma_TryCeilingImpact(entity))
                        return;
                    entity->velY.whole = 0x1000;
                    entity->accelY.whole = -0x2000;
                }
            }
            else
            {
                cs = FC(0x50000, -0x180000);
                corr = cs;
                if (cs != 0)
                {
                    entity->posY.part16.integer += ({ s32 t = (s16)corr; t; });
                    if (entity->velY.whole < 0)
                    {
                        if (Soma_TryCeilingImpact(entity))
                            return;
                        entity->velY.whole = 0x1000;
                        entity->accelY.whole = -0x2000;
                    }
                }
            }
            flags = FLAGS(entity);
            if (!(flags & 0x400000))
            {
                if (entity->velY.whole > 0x12000)
                    FLAGS(entity) = flags & ~0x10;
            }
            else if (entity->velY.whole > 0x8000)
                FLAGS(entity) = flags & ~0x10;
        }
        {
            u32 t = FLAGS(entity);
            u32 m = t & 0x1E0;
            f2 = t;
            if (m)
                return;
        }
        if (gEwramData->playerState.actionLockFlags & 4)
            return;
        if ((gEwramData->playerStatus.soulStateFlags & 0x400) && entity->velX.whole != 0)
        {
            SetAnim(entity, 3, 1, 0);
        }
        else if (entity->velY.whole <= 0x3FFF)
        {
            if ((f2 & 4) && (u16)entity->ext.player.requestedAnim != 0xFF)
                return;
            if (!(f2 & 0x10))
            {
                if (entity->velX.whole != 0)
                    SetAnim(entity, 0xB, 0, 1);
                else
                    SetAnim(entity, 0x32, 1, 1);
            }
            else
                SetAnim(entity, 0x24, 1, 1);
        }
        else
            SetAnim(entity, 0xC, 0, 1);
    }
}
