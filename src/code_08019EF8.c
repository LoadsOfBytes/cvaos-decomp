/* Soma bat-form state 8 and its exit path (three floor probes at x, x - 8,
 * x + 8). Helpers copied from code_08016DE4.c. */
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

struct PlayerBodyBox {
    u8 x;
    u8 y;
    u8 width;
    u8 height;
};

extern struct PlayerBodyBox sSomaBatFormBodyBoxes[];
extern u8 sSomaAnimPalettes[];
extern u8 sSomaPalette[];
extern void Soma_ResolveBatExitCollision(struct EwramData_EntityData *entity);
extern void Soma_CastEquippedRedSoul(struct EwramData_EntityData *entity, s32 noAnim);
extern void Soma_HandleBatFlightInput(struct EwramData_EntityData *entity);
extern void Soma_ResolveBatFormCollision(struct EwramData_EntityData *entity);

static inline void StartAnim(struct EwramData_EntityData *entity, u16 anim)
{
    struct PlayerBodyBox box2 = sSomaBatFormBodyBoxes[0];

    entity->unk_504.part16.unk_504 = anim;
    HitBox_SetBody(entity, &box2);
}

/**
 * @brief Player state 8 entered by SkillGiantBatUse (soulStateFlags bit 0x200).
 * While the bat flag remains, process flight input, optional red-soul
 * cast and bat-form collision; once cleared, enter substate 2, resolve
 * exit collision with horizontal-only control and return to state 0.
 */
void Soma_UpdateBatForm(struct EwramData_EntityData *entity)
{
    if (entity->subState != 2 && !(gEwramData->playerStatus.soulStateFlags & 0x200))
    {
        struct PlayerBodyBox box;

        entity->velX.whole = 0;
        entity->accelX.whole = 0;
        entity->velY.whole = 0;
        entity->accelY.whole = 0;
        FLAGS(entity) &= ~0x200000;
        FLAGS(entity) &= 0xEFFFFE9F;
        entity->unk_4FC.whole = 0x080E12EA;
        Soma_ResolveBatExitCollision(entity);
        if (FLAGS(entity) & 0x01000080)
            entity->state = 0;
        entity->subState = 2;
        box = sSomaBatFormBodyBoxes[0];
        if (!(gEwramData->playerState.actionLockFlags & 0x800) && entity->animScriptIndex != 0x1E)
        {
            s32 pal;

            Anim_StartIndexed(entity, 0x1E, 3, 0);
            HitBox_SetBody(entity, &box);
            entity->unk_504.part16.unk_504 = 0xFF;
            entity->unk_4F4.whole &= ~0x200000;
            pal = sSomaAnimPalettes[0x1E];
            if (PLAYER_ANIM_PALETTE(entity) != pal)
            {
                PLAYER_ANIM_PALETTE(entity) = pal;
                ObjPalette_LoadRange(sSomaPalette, pal, 1, 0);
            }
        }
    }

    switch (entity->subState)
    {
    case 0:
        StartAnim(entity, 0x1D);
        entity->subState = 1;
    case 1:
        if (gEwramData->inputData.playerNewInput & gEwramData->playerStatus.attackButton)
        {
            if (gEwramData->playerStatus.equippedRedSoul == 2
                || (*(u32 *)&gEwramData->playerStatus.equippedWeapon & 0xFF00FF00) == 0x1E000800
                || (*(u32 *)&gEwramData->playerStatus.equippedWeapon & 0xFF00FF00) == 0x07002C00)
                Soma_CastEquippedRedSoul(entity, 1);
        }
        Soma_HandleBatFlightInput(entity);
        Soma_ResolveBatFormCollision(entity);
        break;
    case 2:
    {
        u16 key;
        s32 old, new;
        u16 x, y;

        gEwramData->playerState.actionLockFlags |= 3;
        if ((key = gEwramData->inputData.playerHeldInput & 0x20))
        {
            entity->flipX = 1;
            entity->velX.whole = -0x20000;
            entity->accelX.whole = 0;
        }
        else if (gEwramData->inputData.playerHeldInput & 0x10)
        {
            entity->flipX = 0;
            entity->velX.whole = 0x20000;
            entity->accelX.whole = key;
        }
        else
            entity->accelX.whole = (entity->velX.whole < 0) ? 0x4000 : -0x4000;
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
        entity->velY.whole = 0;
        entity->accelY.whole = 0;
        entity->unk_4FC.whole = 0x080E12EA;
        Soma_ResolveBatExitCollision(entity);
        if (!(FLAGS(entity) & 0x200000))
            return;
        x = GetEntityRoomXPositionInteger(entity);
        y = GetEntityRoomYPositionInteger(entity);
        if ((s16)FloorProbe(X, Y + 1, 0) != 0
            || (s16)FloorProbe(({ s32 t_ = X; t_; }) - 8, ({ s32 t_ = Y; t_; }) + 1, 0) != 0
            || (s16)FloorProbe(({ s32 t_ = X; t_; }) + 8, ({ s32 t_ = Y; t_; }) + 1, 0) != 0)
            FLAGS(entity) = (FLAGS(entity) & ~0x1E) | 0x100000;
        entity->state = 0;
        break;
    }
    }
}
