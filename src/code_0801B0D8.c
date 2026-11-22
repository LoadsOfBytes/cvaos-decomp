/* Player (Soma) main update: damage reactions, collision, state machine. */
#include "code_08009A0.h"
#include "player_entity.h"
#include "code_08001194.h"
#include "code_08032E4C.h"
#include "code_08032444.h"
#include "code_08033CAC.h"
#include "code_080D73B8.h"
#include "code_0800CB00.h"
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
extern struct PlayerBodyBox sSomaCrouchBodyBoxes[];
extern u8 sSomaAnimPalettes[];
extern u8 sSomaPalette[];

extern void Soma_ResolveBatExitCollision(struct EwramData_EntityData *entity);
extern void Soma_ResolveWorldCollision(struct EwramData_EntityData *entity);
extern void Soma_StartDeath(struct EwramData_EntityData *entity);
extern void Soma_CastEquippedRedSoul(struct EwramData_EntityData *entity, s32 noAnim);
extern void Soma_HandleWeaponAttackInput(struct EwramData_EntityData *entity);
extern void Soma_HandleJumpInput(struct EwramData_EntityData *entity);
extern void Soma_UpdateJumpControl(struct EwramData_EntityData *entity);
extern void Afterimage_Create(struct EwramData_EntityData *, s32, s32, s32);
extern void Soma_UpdateDiveAttack(struct EwramData_EntityData *entity);
extern void Soma_UpdateVerticalMovement(struct EwramData_EntityData *entity);
extern void Soma_HandleBlueSoulInput(struct EwramData_EntityData *entity);
extern void Soma_UpdateKnockback(struct EwramData_EntityData *entity);
extern void Soma_UpdateKnockbackFromImpulse(struct EwramData_EntityData *entity);
extern void Soma_UpdateStatusReaction(struct EwramData_EntityData *entity);
extern void Soma_UpdateBatForm(struct EwramData_EntityData *entity);
extern void Soma_UpdateTransformationDash(struct EwramData_EntityData *entity);
extern void Player_UpdateDeath(struct EwramData_EntityData *entity);
extern struct EwramData_EntityData *Effect_CreatePlayerFlash(s32 kind);
extern void HitBox_UpdatePlayerRects(void);
extern void YellowSoul_CreateEquippedEntity(struct EwramData_EntityData *entity);
extern u8 sUnk_080E12C8[];

#define FLAGS(e) (PLAYER_ACTION_FLAGS((e)))
#define HELD (gEwramData->inputData.playerHeldInput)
#define NEW (gEwramData->inputData.playerNewInput)
#define REQ(e) (*(u16 *)&PLAYER_REQUESTED_ANIM((e)))

static inline void SetPlayerAnimBox(struct EwramData_EntityData *entity, u8 anim, struct PlayerBodyBox *table, s32 flag)
{
    struct PlayerBodyBox box = table[0];
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

static inline void SetPlayerAnimBoxFlags(struct EwramData_EntityData *entity, u8 anim, struct PlayerBodyBox *table, s32 flag, u32 *playerFlags)
{
    struct PlayerBodyBox box = table[0];
    s32 pal;

    if (!(*playerFlags & 0x800) && entity->animScriptIndex != anim)
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

static inline void SetPlayerBox(struct EwramData_EntityData *entity, s32 value, struct PlayerBodyBox *table)
{
    struct PlayerBodyBox box = table[0];

    PLAYER_REQUESTED_ANIM(entity) = value;
    HitBox_SetBody(entity, &box);
}

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

    if (PLAYER_INVULNERABLE_TIMER(entity) == 0 && GetHitTimer(entity, 8) == 0)
    {
        SetHitTimer(entity, 8, 60);
        {
            s32 dmg = (gEwramData->playerState.equipmentDefenseBonus >> 1) + (gEwramData->playerState.equipmentDefenseBonus >> 2)
                + (gEwramData->playerStatus.maxHP >> 4);

            hit.attack = dmg;
        }
        hit.attributes = 1;
        hit.unk_4 = 0;
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
    switch ((u8)PLAYER_SLOPE_LEVEL(entity))
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
        PLAYER_REQUESTED_ANIM(entity) = value;
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

static inline s32 TryBackdash(struct EwramData_EntityData *entity)
{
    if ((NEW & gEwramData->playerStatus.abilityButton) && !(HELD & 0x40) && Player_HasAbility(0)
        && !(FLAGS(entity) & 0x10008402))
    {
        /* Keep the zero initialization before the box setup in the ROM order. */
        register u8 zero asm("r4") = 0; /* FAKEMATCH: pinned register */
        SetPlayerBox(entity, 0x15, sSomaStandingBodyBox);
        FLAGS(entity) = (FLAGS(entity) & ~0x200000) | 0x10000000;
        entity->timer = zero;
        if (!entity->flipX)
            entity->velX.whole = -0x3C000;
        else
            entity->velX.whole = 0x3C000;
        Afterimage_Create(entity, 0, 8, 0x1D);
        Afterimage_Create(entity, 0, 6, 0x1C);
        PlaySong(0xA9);
        return 1;
    }
    return 0;
}

static inline void TrySlide(struct EwramData_EntityData *entity)
{
    if (Player_HasAbility(1) && !(FLAGS(entity) & 0x1122) && (HELD & 0x80)
        && (NEW & gEwramData->playerStatus.jumpButton))
    {
        entity->state = 3;
        entity->subState = 0;
        PLAYER_REQUESTED_ANIM(entity) = 0xFF;
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

#define DUST_SIDE (!gEwramData->playerState.player->flipX)

/* These inline boundaries keep address temporaries separate during allocation.
 * A matching reconstruction does not establish the original helper layout. */
static inline u16 GetDamageType(struct EwramData *data)
{
    return data->playerState.hitReaction;
}

/* Preserve both frame addresses across animation/palette calls. The register
 * constraints and lock-mask destination reproduce agbcc allocation. */
static inline void RefreshPlayerAnim(struct EwramData_EntityData *entity)
{
    register u8 *framePtr asm("r5") = &entity->animStep; /* FAKEMATCH: pinned register */
    u8 frame = *framePtr;
    register u8 *timerPtr asm("r4") = &entity->animTimer; /* FAKEMATCH: pinned register */
    u8 timer = *timerPtr;
    u8 anim = Weapon_GetPlayerAttackAnim(0);

    struct PlayerBodyBox box = sSomaStandingBodyBox[0];
    s32 pal;
    register u32 lock asm("r1") = gEwramData->playerState.actionLockFlags; /* FAKEMATCH: pinned register */
    lock &= 0x800;

    if (!lock && entity->animScriptIndex != anim)
    {
        Anim_StartIndexed(entity, anim, 3, 0);
        HitBox_SetBody(entity, &box);
        PLAYER_REQUESTED_ANIM(entity) = 0xFF;
        PLAYER_ACTION_FLAGS(entity) &= ~0x200000;
        pal = sSomaAnimPalettes[anim];
        if (PLAYER_ANIM_PALETTE(entity) != pal)
        {
            PLAYER_ANIM_PALETTE(entity) = pal;
            ObjPalette_LoadRange(sSomaPalette, pal, 1, 0);
        }
        *framePtr = frame;
        *timerPtr = timer;
    }

}
static inline s32 GetHitX(struct EwramData *data)
{
    return data->playerState.hitSourceX;
}

static inline struct PlayerBodyBox *GetSlideBox(void)
{
    return &sSomaCrouchBodyBoxes[1];
}

/**
 * @brief Character-0 main entity update: process damage reactions,
 * world collision, input/state dispatch, animation and tracked hit
 * state. Entry 0 of sPlayerUpdateFuncs; retain all compiler constraints.
 */
void Soma_Update(struct EwramData_EntityData *entity)
{
    register struct EwramData *hurtData;
    u32 *hurtFlags;
    s32 maxAir;
    s32 maxSpeed;
    s32 speed;
    s32 count;
    u16 x, y;
    /* HI mode retains the temporary -1 and copy shared with the Julius twin. */
    register s16 none asm("r5"); /* FAKEMATCH: pinned register */
    u8 kind;
    u16 left;

    maxAir = 0x18000;
    if (gEwramData->playerState.standingPlatform != NULL)
        gEwramData->playerState.standingPlatform = NULL;
    FLAGS(entity) &= ~0x240000;
    if (entity->animFlags & 4)
    {
        FLAGS(entity) |= 0x200000;
        FLAGS(entity) &= ~0x10000000;
    }
    entity->hit.flags |= 2;
    Player_PruneTrackedEntities();
    PLAYER_DRAW_OFFSET_Y(entity) = 0;
    PLAYER_DRAW_OFFSET_X(entity) = 0;
    entity->posX.whole += entity->velX.whole + PLAYER_CONVEYOR_VELOCITY(entity);
    if (entity->velY.whole > 0x80000)
        entity->velY.whole = 0x80000;
    entity->posY.whole += entity->velY.whole;
    count = 2;
    PLAYER_CONVEYOR_VELOCITY(entity) = 0;
    if (PLAYER_FLOOR_KIND(entity) == 0xFF)
    {
        x = GetEntityRoomXPositionInteger(entity);
        y = GetEntityRoomYPositionInteger(entity) - 4;
        if ((FLAGS(entity) & 0x400) || (gEwramData->playerStatus.soulStateFlags & 0x200))
            count = 1;
        if (count != 0)
        {
        probe:
            {
                s32 tx = x << 16;
                s32 ty = (s16)y;
                u8 d = Collision_GetTile(tx >> 16, ty);

                if (SlopeKind(d) == 0 && !(d & 0xC0) && (d & 4))
                    kind = (d & 0x30) >> 4;
                else
                    kind = 0xFF;
                PLAYER_FLOOR_KIND(entity) = kind;
                if (kind == 0xFF)
                {
                    y = (u32)((y << 16) - 0x180000) >> 16;
                    count--;
                    if (count != 0)
                        goto probe;
                }
            }
        }
    }
    switch (PLAYER_FLOOR_KIND(entity))
    {
        case 0:
            SpikeDamage(entity, 1);
            break;
        case 1:
            SpikeDamage(entity, 0);
            break;
        case 2:
            PLAYER_CONVEYOR_VELOCITY(entity) = gEwramData->gameState.conveyorSpeed;
            break;
        case 3:
            PLAYER_CONVEYOR_VELOCITY(entity) = -gEwramData->gameState.conveyorSpeed;
            break;
    }
    none = -1;
    PLAYER_FLOOR_KIND(entity) = 0xFF;
    if (gEwramData->playerState.pendingHitDamage == 0)
        goto collide;
    if (gEwramData->playerStatus.currentHP <= 0)
    {
        gEwramData->playerState.actionLockFlags &= ~0x820;
        Soma_StartDeath(entity);
        goto collide;
    }
    if (gEwramData->playerStatus.statusEffects & 4)
    {
        if (entity->state == 8)
            Soma_ResolveBatExitCollision(entity);
        entity->palette = 0;
        entity->state = 0xF;
        FLAGS(entity) |= 0x80;
        if (FLAGS(entity) & 2)
        {
            s32 accel;

            if ((s32)gEwramData->playerState.hitSourceX > entity->posX.whole)
            {
                entity->velX.whole = -0x18000;
                accel = 0x200;
            }
            else
            {
                entity->velX.whole = 0x18000;
                accel = -0x200;
            }
            entity->accelX.whole = accel;
            entity->velY.whole = -0x20000;
            entity->accelY.whole = -0x1000;
            FLAGS(entity) |= 2;
        }
        goto knocked;
    }
    if (gEwramData->playerState.hitReaction == 3)
    {
        struct EwramData *ewram;
        struct EwramData_EntityData *player;

        if (entity->state == 8)
            Soma_ResolveBatExitCollision(entity);
        ewram = gEwramData;
        ewram->playerState.actionLockFlags &= ~0x820;
        ewram->playerState.player->velX.whole = ewram->playerState.knockbackVelocityX;
        ewram->playerState.player->velY.whole = ewram->playerState.knockbackVelocityY;
        player = ewram->playerState.player;
        player->accelX.whole = (player->velX.whole < 0) ? 0x1000 : -0x1000;
        {
            u8 state = 0xE;
            FLAGS(entity) |= 0x82;
            entity->state = state;
        }
        FLAGS(entity) &= ~0x200000;
        PlaySong(0x196);
        entity->palette = 0;
        goto collide;
    }
    if (gEwramData->playerState.hitReaction == 2)
        goto collide;
    if ((gEwramData->playerStatus.soulStateFlags & 0x20200) == 0x20000)
    {
        SetHitTimer(entity, 8, 30);
        goto collide;
    }
    if (entity->state == 8)
        Soma_ResolveBatExitCollision(entity);
    entity->palette = 0;
    hurtData = gEwramData;
    hurtFlags = &hurtData->playerState.actionLockFlags;
    hurtData->playerState.actionLockFlags &= ~0x820;
    if (GetDamageType(hurtData) == 0 && !(FLAGS(entity) & 2))
    {
        none |= entity->animScriptIndex;
        entity->animScriptIndex = none;
        if (!(FLAGS(entity) & 0x400))
        {
            if ((!entity->flipX && GetHitX(hurtData) > entity->posX.whole)
                || (entity->flipX && GetHitX(hurtData) < entity->posX.whole))
                SetPlayerAnimBox(entity, 0xE, sSomaStandingBodyBox, 0);
            else
                SetPlayerAnimBox(entity, 0x10, sSomaStandingBodyBox, 0);
        }
        else
        {
            SetPlayerAnimBoxFlags(entity, 0xF, sSomaCrouchBodyBoxes, 0, hurtFlags);
        }
        SetHitTimer(entity, 8, 50);
        FLAGS(entity) |= 0x80;
        entity->state = 0xC;
        PlaySong(0x19B);
    }
    else
    {
        s32 accel;

        if ((s32)gEwramData->playerState.hitSourceX > entity->posX.whole)
        {
            if (!entity->flipX)
                SetPlayerAnimBox(entity, 0x11, sSomaStandingBodyBox, 0);
            else
                SetPlayerAnimBox(entity, 0x10, sSomaStandingBodyBox, 0);
            entity->velX.whole = -0x18000;
            accel = 0x200;
        }
        else
        {
            if (entity->flipX)
                SetPlayerAnimBox(entity, 0x11, sSomaStandingBodyBox, 0);
            else
                SetPlayerAnimBox(entity, 0x10, sSomaStandingBodyBox, 0);
            entity->velX.whole = 0x18000;
            accel = -0x200;
        }
        entity->accelX.whole = accel;
        entity->velY.whole = -0x20000;
        entity->accelY.whole = -0x1000;
        FLAGS(entity) |= 0x82;
        entity->state = 0xD;
        PlaySong(0x195);
    }
knocked:
    FLAGS(entity) &= ~0x200000;
collide:
    if (!(gEwramData->playerState.actionLockFlags & 1))
        Soma_ResolveWorldCollision(entity);
    gEwramData->playerState.actionLockFlags &= ~1;
    gEwramData->playerState.cachedPlayerX = entity->posX.whole;
    gEwramData->playerState.cachedPlayerY = entity->posY.whole;

    switch (entity->state)
    {
        case 0:
            maxSpeed = 0x18000;
            speed = entity->velX.whole;
            if (speed < 0)
                speed = -speed;
            if (!(FLAGS(entity) & 0x10000002))
            {
                if (!(FLAGS(entity) & 0x1000000))
                {
                    if (speed > maxSpeed)
                    {
                        if (!(HELD & 0x30) || (HELD & 0x80))
                        {
                            if (!(entity->timer & 3))
                                Player_SpawnEffectSet(2, !gEwramData->playerState.player->flipX);
                        }
                        else if ((HELD & 0x20) && entity->velX.whole > maxSpeed)
                            Player_SpawnEffectSet(2, !gEwramData->playerState.player->flipX);
                        else if ((HELD & 0x10) && entity->velX.whole < -0x18000)
                            Player_SpawnEffectSet(2, !gEwramData->playerState.player->flipX);
                    }
                }
                else if (!(FLAGS(entity) & 0x400000))
                {
                    if (speed > maxSpeed)
                    {
                        if (!(entity->timer & 3))
                            Player_SpawnSurfaceEffect(1);
                    }
                    else if (speed > 0xC000)
                    {
                        if (!(entity->timer & 7))
                            Player_SpawnSurfaceEffect(1);
                    }
                }
            }
            if (gEwramData->playerStatus.soulStateFlags & 0x400)
                maxSpeed = 0x40000;
            TryBackdash(entity);
            if (!(FLAGS(entity) & 0x10008400))
            {
                if (((HELD & 0x10) && entity->flipX) || ((HELD & 0x20) && !entity->flipX))
                {
                    if (!(FLAGS(entity) & 2))
                        SetPlayerBox(entity, 0x18, sSomaStandingBodyBox);
                    else if ((FLAGS(entity) & 0x800010) == 0x800000)
                        SetPlayerBox(entity, 0x29, sSomaStandingBodyBox);
                }
                GroundControl(entity, maxSpeed);
                if (!(FLAGS(entity) & 2))
                {
                    if ((entity->velX.whole > 0 && (FLAGS(entity) & 0x4000))
                        || (entity->velX.whole < 0 && (FLAGS(entity) & 0x2000)))
                        SlopeSlowdown(entity, 0x18, 0x14, 0x12);
                }
            }
            else if (!(FLAGS(entity) & 0x10000000))
            {
                if (HELD & 0x20)
                    entity->flipX = 1;
                else if (HELD & 0x10)
                    entity->flipX = 0;
                entity->accelX.whole = (entity->velX.whole < 0) ? 0x4000 : -0x4000;
            }
            else
            {
                if (!(FLAGS(entity) & 2) && speed > 0x18000)
                {
                    if (!(entity->timer & 3))
                        Player_SpawnEffectSet(2, !gEwramData->playerState.player->flipX);
                    if (!(FLAGS(entity) & 2))
                    {
                        if ((entity->velX.whole > 0 && (FLAGS(entity) & 0x4000))
                            || (entity->velX.whole < 0 && (FLAGS(entity) & 0x2000)))
                            SlopeSlowdown(entity, 0x17, 0x13, 0x11);
                    }
                }
                entity->accelX.whole = (entity->velX.whole < 0) ? 0x2800 : -0x2800;
            }
            ApplyAccelX(entity);
            if (FLAGS(entity) & 0x100000)
            {
                entity->timer++;
                if (!(FLAGS(entity) & 0x1000001E))
                {
                    if ((HELD & 0x80) || (FLAGS(entity) & 0x8000) || (gEwramData->playerStatus.inputOverrides & 1))
                    {
                        if (!(FLAGS(entity) & 0x400))
                        {
                            FLAGS(entity) = (FLAGS(entity) | 0x400) & ~0x10000000;
                            SetPlayerBox(entity, 9, sSomaCrouchBodyBoxes);
                        }
                        if (REQ(entity) == 0xFF)
                            SetPlayerAnimBox(entity, 2, sSomaCrouchBodyBoxes, 1);
                    }
                    else if (HELD & 0x30)
                    {
                        if (!(FLAGS(entity) & 0x10000000) && REQ(entity) != 0x18)
                        {
                            if (!(gEwramData->playerStatus.soulStateFlags & 0x400))
                            {

                                if (entity->animScriptIndex != 1 && entity->animScriptIndex != 0x1A && entity->animScriptIndex != 0x18)
                                    SetPlayerAnimBox(entity, 0x1A, sSomaStandingBodyBox, 0);
                                if (FLAGS(entity) & 0x200000)
                                    SetPlayerAnimBox(entity, 1, sSomaStandingBodyBox, 1);
                            }
                            else
                                SetPlayerAnimBox(entity, 3, sSomaStandingBodyBox, 1);
                        }
                    }
                    else
                    {
                        if (!(FLAGS(entity) & 0x400) && (HELD & 0x40))
                        {
                            if (PLAYER_UP_HOLD_TIMER(entity) < 4)
                                PLAYER_UP_HOLD_TIMER(entity)++;
                            else
                            {
                                SetPlayerAnimBox(entity, 0x17, sSomaStandingBodyBox, 0);
                                goto ground_actions;
                            }
                        }
                        else
                            PLAYER_UP_HOLD_TIMER(entity) = 0;
                        if (!(FLAGS(entity) & 0x10000000))
                        {
                            u8 anim = entity->animScriptIndex;

                            if (anim != 0x1A && anim != 0 && anim != 0x18 && speed > 0xFFFF)
                                SetPlayerBox(entity, 0x19, sSomaStandingBodyBox);
                        }
                        if (REQ(entity) == 0xFF || entity->animScriptIndex == 0x1A)
                        {
                            FLAGS(entity) &= ~0x10000000;
                            SetPlayerAnimBox(entity, 0, sSomaStandingBodyBox, 1);
                        }
                    }
                    if ((FLAGS(entity) & 0x8400) == 0x400 && !(gEwramData->playerStatus.inputOverrides & 1)
                        && !(HELD & 0x80))
                    {
                        FLAGS(entity) &= ~0x400;
                        SetPlayerBox(entity, 0xA, sSomaStandingBodyBox);
                    }
                }
            }
        ground_actions:
            if ((HELD & 0x40) && (NEW & gEwramData->playerStatus.attackButton) && !(FLAGS(entity) & 0x540))
                Soma_CastEquippedRedSoul(entity, 0);
            Soma_HandleWeaponAttackInput(entity);
            TrySlide(entity);
            Soma_HandleJumpInput(entity);
            Soma_UpdateJumpControl(entity);
            Soma_HandleBlueSoulInput(entity);
            break;
        case 1:
            if (gEwramData->playerStatus.soulStateFlags & 0x400)
                maxAir = 0x40000;
            AirControl(entity, FLAGS(entity), maxAir);
            ApplyAccelX(entity);
            if ((gEwramData->playerStatus.soulStateFlags & 0x800000) && (HELD & 0x40)
                && (NEW & gEwramData->playerStatus.attackButton) && !(FLAGS(entity) & 0x540))
                Soma_CastEquippedRedSoul(entity, 0);
            if (Weapon_KeepsAttackOnLanding() && (FLAGS(entity) & 0x40))
            if (FLAGS(entity) & 0x100000)
            {
                RefreshPlayerAnim(entity);
                FLAGS(entity) &= ~0x40;
            }
            if (FLAGS(entity) & 0x200000)
            {
                u32 flags = FLAGS(entity) & ~0x200060;

                FLAGS(entity) = flags;
                if (!(flags & 2))
                {
                    if (!(flags & 0x400))
                        SetPlayerBoxIfFree(entity, Weapon_GetPlayerAttackAnim(3), sSomaStandingBodyBox);
                    else
                        SetPlayerBoxIfFree(entity, Weapon_GetPlayerAttackAnim(4), sSomaCrouchBodyBoxes);
                }
                entity->state = 0;
            }
            else if (TryBackdash(entity))
            {
                FLAGS(entity) &= ~0x200060;
                entity->state = 0;
            }
            Soma_UpdateJumpControl(entity);
            Soma_HandleBlueSoulInput(entity);
            break;
        case 2:
        {
            u32 flags;

            if (gEwramData->playerStatus.soulStateFlags & 0x400)
                maxAir = 0x40000;
            if (entity->subState && entity->animStep == 3)
            {
                PlaySong(0xEB);
                entity->subState = 0;
                if (!entity->flipX)
                    entity->velX.whole = 0x40000;
                else
                    entity->velX.whole = -0x40000;
            }
            flags = FLAGS(entity);
            AirControl(entity, FLAGS(entity), maxAir);
            ApplyAccelX(entity);
            if (FLAGS(entity) & 0x200000)
            {
                FLAGS(entity) &= ~0x200100;
                entity->state = 0;
            }
            Soma_UpdateJumpControl(entity);
            Soma_HandleBlueSoulInput(entity);
            break;
        }
        case 3:
        {
            /* Keep the short-lived source addresses separate from the saved
             * pointers. The timer uses r8; leave frame unconstrained. Binding
             * frame to r7 also changes allocation in earlier states. Both
             * expanded animation branches retain their own stack box. */
            register u8 *sourceFrame asm("r3") = &entity->animStep; /* FAKEMATCH: pinned register */
            u32 frame = *sourceFrame;
            u8 *sourceTimer = &entity->animTimer;
            u32 timer = *sourceTimer;
            u32 slope = FLAGS(entity) & 0x2000;
            register u8 *framePtr asm("r5") = sourceFrame; /* FAKEMATCH: pinned register */
            u8 *timerPtr = sourceTimer;
            s32 abs;
            u32 flags;
            if ((slope && (u8)PLAYER_SLOPE_LEVEL(entity) == 1 && !entity->flipX) || ((FLAGS(entity) & 0x4000) && (u8)PLAYER_SLOPE_LEVEL(entity) == 1 && entity->flipX))
            {
                struct PlayerBodyBox box = *GetSlideBox();
                s32 pal;

                if (!(gEwramData->playerState.actionLockFlags & 0x800) && entity->animScriptIndex != 0x2E)
                {
                    Anim_StartIndexed(entity, 0x2E, 3, 0);
                    HitBox_SetBody(entity, &box);
                    PLAYER_REQUESTED_ANIM(entity) = 0xFF;
                    PLAYER_ACTION_FLAGS(entity) &= ~0x200000;
                    pal = sSomaAnimPalettes[0x2E];
                    if (PLAYER_ANIM_PALETTE(entity) != pal)
                    {
                        PLAYER_ANIM_PALETTE(entity) = pal;
                        ObjPalette_LoadRange(sSomaPalette, pal, 1, 0);
                    }
                    *framePtr = frame;
                    *timerPtr = timer;
                }
            }
            else
            {
                struct PlayerBodyBox box = *GetSlideBox();
                s32 pal;

                if (!(gEwramData->playerState.actionLockFlags & 0x800) && entity->animScriptIndex != 0x1F)
                {
                    Anim_StartIndexed(entity, 0x1F, 3, 0);
                    HitBox_SetBody(entity, &box);
                    PLAYER_REQUESTED_ANIM(entity) = 0xFF;
                    PLAYER_ACTION_FLAGS(entity) &= ~0x200000;
                    pal = sSomaAnimPalettes[0x1F];
                    if (PLAYER_ANIM_PALETTE(entity) != pal)
                    {
                        PLAYER_ANIM_PALETTE(entity) = pal;
                        ObjPalette_LoadRange(sSomaPalette, pal, 1, 0);
                    }
                    *framePtr = frame;
                    *timerPtr = timer;
                }
            }
            entity->hit.flags &= ~2;
            abs = entity->velX.whole;
            if (abs < 0)
                abs = -abs;
            if (abs > 0x18000 && !(entity->timer & 3))
                Player_SpawnEffectSet(2, DUST_SIDE);
            ApplyAccelX(entity);
            flags = FLAGS(entity);
            if (flags & 2)
                entity->timer = 0x20;
            if (++entity->timer > 0x1F)
            {
                FLAGS(entity) &= ~0x20;
                entity->state = 0;
            }
            Soma_UpdateVerticalMovement(entity);
            Soma_HandleBlueSoulInput(entity);
            break;
        }
        case 4:
        {
            u32 flags;
            u8 t;

            Decel(entity, 0x3000);
            t = entity->timer;
            if (t <= 7)
            {
                if ((FLAGS(entity) & 0x80) && t == 6)
                {
                    s32 abs = entity->velX.whole;

                    if (abs < 0)
                        abs = -abs;
                    if (abs > 0x2000 && (RandomNumberGenerator() & 3))
                        PlaySong(0x199);
                }
                if (!(entity->timer & 3))
                    Player_SpawnEffectSet(0, DUST_SIDE);
                entity->timer++;
            }
            flags = FLAGS(entity);
            if (flags & 0x10000)
            {
                u32 f2 = flags | 0x400;

                FLAGS(entity) = f2;
                if ((f2 & 0x300000) != 0x100000)
                {
                    FLAGS(entity) = f2 & ~0x10080;
                    entity->state = 0;
                }
                else if (f2 & 0x80)
                {
                    Soma_HandleWeaponAttackInput(entity);
                    TrySlide(entity);
                    Soma_HandleJumpInput(entity);
                    Soma_UpdateJumpControl(entity);
                    if (FLAGS(entity) & 2)
                        entity->state = 0;
                    if (entity->state != 4)
                        FLAGS(entity) &= ~0x10080;
                }
            }
            Soma_HandleBlueSoulInput(entity);
            break;
        }
        case 5:
            if (entity->velY.whole > 0)
                entity->state = 0;
            Soma_UpdateJumpControl(entity);
            Soma_HandleBlueSoulInput(entity);
            break;
        case 6:
            if (FLAGS(entity) & 0x200000)
                entity->state = 0;
            break;
        case 7:
            Soma_UpdateDiveAttack(entity);
            Soma_UpdateVerticalMovement(entity);
            break;
        case 12:
        {
            u32 flags;

            FLAGS(entity) &= 0xEFFFFE9F;
            Decel(entity, 0x8000);
            flags = FLAGS(entity);
            if (flags & 0x200000)
            {
                FLAGS(entity) = flags & ~0x80;
                entity->state = 0;
            }
            Soma_UpdateVerticalMovement(entity);
            Soma_HandleBlueSoulInput(entity);
            break;
        }
        case 13:
            Soma_UpdateKnockback(entity);
            break;
        case 14:
            Soma_UpdateKnockbackFromImpulse(entity);
            break;
        case 15:
            Soma_UpdateStatusReaction(entity);
            break;
        case 8:
            Soma_UpdateBatForm(entity);
            break;
        case 9:
            Soma_UpdateTransformationDash(entity);
            break;
        case 10:
            if (gEwramData->playerState.activeBlueSoulEntities[0] == NULL)
            {
                PLAYER_REQUESTED_ANIM(entity) = 0xFF;
                entity->state = 0;
            }
            break;
        case 16:
        {
            u8 four = 4;
            PLAYER_INVULNERABLE_TIMER(gEwramData->playerState.player) = four;
        }
            Player_UpdateDeath(entity);
            break;
        case 17:
            FLAGS(gEwramData->playerState.player) &= 0x40000000;
            entity->state = 0;
            break;
    }

    if (PLAYER_INVULNERABLE_TIMER(entity) != 0 && --PLAYER_INVULNERABLE_TIMER(entity) == 0)
    {
        struct EntityHit *s = &entity->hit;

        s->hitBy[2] = 0;
        s->hitBy[1] = 0;
        s->hitBy[0] = 0;
        s->hitTimer[2] = 0;
        s->hitTimer[1] = 0;
        s->hitTimer[0] = 0;
    }
    if (gEwramData->gameState.stateFlags & 1)
    {
        u8 two = 2;
        PLAYER_INVULNERABLE_TIMER(gEwramData->playerState.player) = two;
    }
    else if (!(PLAYER_FRAME_COUNTER(entity) & 7) && gEwramData->playerStatus.equippedAccessory == 0x2A
        && gEwramData->startPauseEnabled && !(FLAGS(entity) & 0x40002) && (HELD & 0x30))
    {
        if (++gEwramData->playerStatus.currentExperience > 99999999)
            gEwramData->playerStatus.currentExperience = 99999999;
        if (Player_ApplyEarnedLevels(&gEwramData->playerStatus))
            Effect_CreatePlayerFlash(0);
    }
    if (PLAYER_BLINK_TIMER(entity) != 0)
    {
        PLAYER_BLINK_TIMER(entity)--;
        entity->drawSkip = 1;
        if (PLAYER_BLINK_TIMER(entity) == 0)
            entity->drawSkip = 0;
    }
    if (!(gEwramData->playerState.actionLockFlags & 2))
    {
        u32 ptr;

        if (entity->state == 3 || (FLAGS(entity) & 0x8000))
            ptr = (u32)(sUnk_080E12C8 + 0x20);
        else if (FLAGS(entity) & 0x800000)
            ptr = (u32)(sUnk_080E12C8 + 0x1C);
        else if (!(FLAGS(entity) & 2))
            ptr = (u32)(sUnk_080E12C8 + 0x14);
        else
        {
            if (PLAYER_ON_STAIRS(entity) != 0)
                gEwramData->playerState.wallProbeY0 = -12;
            else
            {
                u16 vx = (entity->velX.part16.integer < 0) ? -entity->velX.part16.integer : entity->velX.part16.integer;
                u16 vy = (entity->velY.part16.integer < 0) ? -entity->velY.part16.integer : entity->velY.part16.integer;

                if ((s16)vx <= 2 || entity->velY.part16.integer > 0)
                    gEwramData->playerState.wallProbeY0 = -(s16)vy / 2 - 4;
                else
                    gEwramData->playerState.wallProbeY0 = -12;
            }
            ptr = (u32)&gEwramData->playerState.wallProbeCount;
        }
        PLAYER_WALL_PROBES(entity) = ptr;
    }
    if (!(gEwramData->playerState.actionLockFlags & 0x800) && REQ(entity) != 0xFF)
    {
        if (entity->animScriptIndex != REQ(entity))
        {
            Anim_StartIndexed(entity, REQ(entity), 3, 0);
            FLAGS(entity) &= ~0x200000;
        }
        else if (FLAGS(entity) & 0x200000)
            PLAYER_REQUESTED_ANIM(entity) = 0xFF;
    }
    Anim_Update(entity);
    HitBox_UpdatePlayerRects();
    YellowSoul_CreateEquippedEntity(entity);
    Player_ApplyPreparedStats();
    Player_UpdateStatusAndRegen(entity);
    Player_RecalculateAttackDefense();
    gEwramData->playerState.collisionQueueCount = 0;
    if (gEwramData->playerState.actionLockFlags & 0x20)
    {
        entity->velX.whole = 0;
        entity->accelX.whole = 0;
    }
    if (gEwramData->playerStatus.inputOverrides != 0)
        gEwramData->playerStatus.inputOverrides = 0;
    if (gEwramData->playerStatus.soulStateFlags != 0 && (gEwramData->playerStatus.soulStateFlags & 0x10000)
        && (FLAGS(entity) & 0x80000))
        Player_RestoreHP(5);
    gEwramData->playerState.pendingHitDamage = 0;
    FLAGS(entity) &= ~0x4080000;
    gEwramData->playerState.actionLockFlags &= ~0x8FE;
    Camera_ScrollWithPriority(0, ((s16)entity->posX.part16.integer - gEwramData->playerState.cameraAnchorX) << 16,
        ((s16)entity->posY.part16.integer - gEwramData->playerState.cameraAnchorY) << 16);
    PLAYER_FRAME_COUNTER(entity)++;
}
