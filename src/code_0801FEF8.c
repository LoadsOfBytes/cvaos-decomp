/* Julius main update: damage reactions, collision, whip input and state machine.
 * The helper boundaries and narrow temporaries preserve the original ROM. */
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

#define FLAGS(e) (e->ext.player.actionFlags)
#define HELD (gEwramData->inputData.playerHeldInput)
#define NEW (gEwramData->inputData.playerNewInput)
#define REQ(e) (*(u16 *)&e->ext.player.requestedAnim)

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

static inline void SetPlayerAnimBoxKeep(struct EwramData_EntityData *entity, u8 anim, struct PlayerBodyBox *table,
    s32 flag, u8 frame, u8 timer)
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
        entity->animStep = frame;
        entity->animTimer = timer;
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

static inline s32 TryBackdash(struct EwramData_EntityData *entity)
{
    if ((NEW & gEwramData->playerStatus.abilityButton) && !(HELD & 0x40) && Player_HasAbility(0)
        && !(FLAGS(entity) & 0x10008402))
    {
        SetPlayerBox(entity, 0x15, sSomaStandingBodyBox);
        FLAGS(entity) = (FLAGS(entity) & ~0x200000) | 0x10000000;
        entity->timer = 0;
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

#define DUST_SIDE (!gEwramData->playerState.player->flipX)

extern void Whip_DeactivateChain(struct EwramData_EntityData *entity);
extern void Julius_StartDeath(struct EwramData_EntityData *entity);
extern void Julius_ResolveWorldCollision(struct EwramData_EntityData *entity);
extern void Julius_HandleSubweaponInput(struct EwramData_EntityData *entity);
extern void Julius_HandleWhipAttackInput(struct EwramData_EntityData *entity);
extern void Julius_HandleJumpInput(struct EwramData_EntityData *entity);
extern void Julius_TryStartAbilityDash(struct EwramData_EntityData *entity);
extern void Julius_UpdateGrandCrossCast(struct EwramData_EntityData *entity);
extern void Julius_UpdateAbilityDash(struct EwramData_EntityData *entity);
extern void Julius_UpdateJumpControl(struct EwramData_EntityData *entity);
extern void Julius_UpdateVerticalMovement(struct EwramData_EntityData *entity);
extern void Julius_UpdateDiveAttack(struct EwramData_EntityData *entity);
extern void Julius_UpdateKnockback(struct EwramData_EntityData *entity);
extern void Julius_UpdateKnockbackFromImpulse(struct EwramData_EntityData *entity);
extern void Whip_SwingChainRight(struct EwramData_EntityData *entity);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *entity);
#define SetJuliusAnim(e, a) SetJuliusAnimF(e, a, 0)

/* Keep the inline boundary: it changes the offset pseudo's lifetime and the
 * subsequent reload allocation in agbcc. This is not an original-source claim. */
static inline struct EwramData_EntityData *GetWhip(void)
{
    return gEwramData->playerState.whipChain;
}

static inline void SetJuliusAnimF(struct EwramData_EntityData *entity, u8 anim, s32 flag)
{
    if (!(gEwramData->playerState.actionLockFlags & 0x800) && entity->animScriptIndex != anim)
    {
        Anim_StartIndexed(entity, anim, 1, flag);
        entity->ext.player.requestedAnim = 0xFF;
        FLAGS(entity) &= ~0x200000;
    }
}

/* Matching reconstruction; this does not establish the original source form. */
/**
 * @brief Character-1 main entity update: process damage reactions,
 * world collision, movement and whip/state input, animation and
 * hit state. Entry 1 of sPlayerUpdateFuncs; retain narrow temporaries.
 */
void Julius_Update(struct EwramData_EntityData *entity)
{
    s32 count;
    /* Matching register constraint; retain explicit u16 narrowing below.
     * Matching this form would not establish the original declaration. */
    register s32 x asm("r8"); /* FAKEMATCH: pinned register */
    u16 y;
    /* HI mode creates the -1 temporary and copy seen in both player updates. */
    register s16 none asm("r5"); /* FAKEMATCH: pinned register */
    u8 kind;

    entity->drawSkip = 0;
    Whip_DeactivateChain(gEwramData->playerState.whipChain);
    {
        u8 one = 1;
        entity->hit.type = one;
    }
    entity->hit.flags |= 2;
    gEwramData->playerState.standingPlatform = NULL;
    FLAGS(entity) &= ~0x240000;
    if (entity->animFlags & 4)
    {
        FLAGS(entity) |= 0x200000;
        FLAGS(entity) &= ~0x10000000;
    }
    Player_PruneTrackedEntities();
    entity->posX.whole += entity->velX.whole + PLAYER_CONVEYOR_VELOCITY(entity);
    if (entity->velY.whole > 0x80000)
        entity->velY.whole = 0x80000;
    entity->posY.whole += entity->velY.whole;
    count = 2;
    PLAYER_CONVEYOR_VELOCITY(entity) = 0;
    if (entity->ext.player.floorKind == 0xFF)
    {
        x = (u16)GetEntityRoomXPositionInteger(entity);
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
                entity->ext.player.floorKind = kind;
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
    switch (entity->ext.player.floorKind)
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
    entity->ext.player.floorKind = 0xFF;
    if (gEwramData->playerState.pendingHitDamage == 0)
        goto collide;
    if (gEwramData->playerStatus.currentHP <= 0)
    {
        gEwramData->playerState.actionLockFlags &= ~0x820;
        Julius_StartDeath(entity);
        goto collide;
    }
    if (gEwramData->playerState.hitReaction == 3)
    {
        struct EwramData *ewram = gEwramData;
        struct EwramData_EntityData *player;

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
        PlaySong(0x1E9);
        goto collide;
    }
    if (gEwramData->playerState.hitReaction == 2)
        goto collide;
    entity->palette = 0;
    gEwramData->playerState.actionLockFlags &= ~0x820;
    if (gEwramData->playerState.hitReaction == 0 && !(FLAGS(entity) & 2))
    {
        entity->animScriptIndex |= none;
        if (!(gEwramData->playerState.actionLockFlags & 0x800) && entity->animScriptIndex != 0xC)
        {
            Anim_StartIndexed(entity, 0xC, 1, 0);
            entity->ext.player.requestedAnim = 0xFF;
            FLAGS(entity) &= ~0x200000;
        }
        SetHitTimer(entity, 8, 50);
        FLAGS(entity) |= 0x80;
        entity->state = 0xC;
        PlaySong(0x1EA);
    }
    else
    {
        s32 accel;

        if ((s32)gEwramData->playerState.hitSourceX > entity->posX.whole)
        {
            SetJuliusAnim(entity, 0xC);
            entity->velX.whole = -0x18000;
            accel = 0x200;
        }
        else
        {
            SetJuliusAnim(entity, 0xC);
            entity->velX.whole = 0x18000;
            accel = -0x200;
        }
        entity->accelX.whole = accel;
        entity->velY.whole = -0x28000;
        entity->accelY.whole = -0x1000;
        FLAGS(entity) |= 0x82;
        entity->state = 0xD;
        PlaySong(0x1EA);
    }
    FLAGS(entity) &= ~0x200000;
collide:
    Julius_ResolveWorldCollision(entity);
    gEwramData->playerState.cachedPlayerX = entity->posX.whole;
    gEwramData->playerState.cachedPlayerY = entity->posY.whole;
    switch (entity->state)
    {
        case 0:
        {
            s32 speed = entity->velX.whole;
            u32 mask = 0;

            if (speed < 0)
                speed = -speed;
            if (!(FLAGS(entity) & 0x10000002) && ((FLAGS(entity) & 0x1000000) != mask) && !(FLAGS(entity) & 0x400000))
            {
                if (speed > 0x18000)
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
            mask = FLAGS(entity) & 0x10008400;
            if (!mask)
            {
                u32 left = HELD & 0x20;

                if (left)
                {
                    entity->flipX = 1;
                    entity->velX.whole = -0x20000;
                    entity->accelX.whole = mask;
                }
                else if (HELD & 0x10)
                {
                    entity->flipX = 0;
                    entity->velX.whole = 0x20000;
                    entity->accelX.whole = left;
                }
                else
                    entity->accelX.whole = (entity->velX.whole < 0) ? 0x4000 : -0x4000;
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
            ApplyAccelX(entity);
            if (FLAGS(entity) & 0x100000)
            {
                entity->timer++;
                if ((FLAGS(entity) & 0x1000) && (HELD & 0x80) && (NEW & gEwramData->playerStatus.jumpButton))
                {
                    entity->ext.player.dropThroughTimer = 0x10;
                    if (FLAGS(entity) & 0x1000000)
                        entity->ext.player.dropThroughTimer = 0x20;
                    FLAGS(entity) = (FLAGS(entity) | 2) & ~0x1000;
                    entity->velX.whole = 0;
                    entity->velY.whole = 0;
                    entity->accelY.whole = -0x1000;
                }
                if (!(FLAGS(entity) & 0x1000001E))
                {
                    if ((HELD & 0x80) || (FLAGS(entity) & 0x8000) || (gEwramData->playerStatus.inputOverrides & 1))
                    {
                        if (!(FLAGS(entity) & 0x400))
                        {
                            FLAGS(entity) = (FLAGS(entity) | 0x400) & ~0x10000000;
                            entity->ext.player.requestedAnim = 0xE;
                        }
                        if (REQ(entity) == 0xFF)
                            SetJuliusAnimF(entity, 4, 1);
                    }
                    else if (HELD & 0x30)
                        SetJuliusAnimF(entity, 1, 1);
                    else
                    {
                        if (!(FLAGS(entity) & 0x10000000) && entity->animScriptIndex != 0 && speed > 0xFFFF)
                            entity->ext.player.requestedAnim = 2;
                        if (REQ(entity) == 0xFF)
                        {
                            FLAGS(entity) &= ~0x10000000;
                            SetJuliusAnimF(entity, 0, 1);
                        }
                    }
                    if ((FLAGS(entity) & 0x8400) == 0x400 && !(gEwramData->playerStatus.inputOverrides & 1)
                        && !(HELD & 0x80))
                    {
                        FLAGS(entity) &= ~0x400;
                        entity->ext.player.requestedAnim = 0xF;
                    }
                }
            }
            Julius_HandleSubweaponInput(entity);
            Julius_HandleWhipAttackInput(entity);
            if (!(FLAGS(entity) & 0x1122) && (HELD & 0x80) && (NEW & gEwramData->playerStatus.jumpButton))
            {
                entity->state = 3;
                entity->subState = 0;
                entity->ext.player.requestedAnim = 0xFF;
                SetJuliusAnim(entity, 8);
                if (!entity->flipX)
                    entity->velX.whole = 0x38000;
                else
                    entity->velX.whole = -0x38000;
                entity->accelX.whole = (entity->velX.whole < 0) ? 0x1800 : -0x1800;
                entity->timer = 0;
                FLAGS(entity) = (FLAGS(entity) | 0x20000420) & ~0x10000000;
                PlaySong(0xBD);
            }
            Julius_HandleJumpInput(entity);
            Julius_TryStartAbilityDash(entity);
            Julius_UpdateJumpControl(entity);
            break;
        }
        case 1:
            if (FLAGS(entity) & 2)
            {
                if (entity->subState <= 1)
                    AirControl(entity, 2, 0x20000);
                else
                    AirControl(entity, 2, 0x10000);
            }
            else
                entity->accelX.whole = (entity->velX.whole < 0) ? 0x4000 : -0x4000;
            ApplyAccelX(entity);
            switch (entity->subState)
            {
                case 0:
                {
                    u32 attackFlags = FLAGS(entity);

                    if (attackFlags & 0x200000)
                    {
                        if (HELD & gEwramData->playerStatus.attackButton)
                        {
                            if (entity->flipX)
                                Whip_LayChainStraight(gEwramData->playerState.whipChain, 1);
                            else
                                Whip_LayChainStraight(gEwramData->playerState.whipChain, 0);
                            entity->subState = 2;
                        }
                        else
                        {
                            attackFlags &= 0x400;
                            if (!attackFlags)
                                SetJuliusAnim(entity, 0x19);
                            else
                                SetJuliusAnim(entity, 0x1B);
                            FLAGS(entity) &= ~0x20;
                            entity->subState = 1;
                        }
                    }
                    break;
                }
                case 1:
                    Julius_HandleWhipAttackInput(entity);
                    if (FLAGS(entity) & 0x200000)
                    {
                        FLAGS(entity) &= ~0x200060;
                        entity->state = 0;
                    }
                    break;
                case 2:
                    if (!(FLAGS(entity) & 0x400))
                        SetJuliusAnim(entity, 0x1C);
                    else
                        SetJuliusAnim(entity, 0x1D);
                    entity->timer = 8;
                    Whip_ActivateChain(gEwramData->playerState.whipChain);
                    entity->subState = 3;
                    PlaySong(0x123);
                    break;
                case 3:
                    Whip_ActivateChain(gEwramData->playerState.whipChain);
                    if (!(HELD & gEwramData->playerStatus.attackButton))
                    {
                        FLAGS(entity) &= ~0x200060;
                        entity->state = 0;
                    }
                    if (entity->timer != 0)
                        entity->timer--;
                    else
                    {
                        struct EwramData_EntityData *whip = GetWhip();

                        if (NEW & 0xF0)
                        {
                            PlaySong(0x123);
                            if (NEW & 0x20)
                                Whip_SwingChainLeft(whip);
                            if (NEW & 0x10)
                                Whip_SwingChainRight(whip);
                            if (NEW & 0x40)
                                Whip_SwingChainUp(whip);
                            if (NEW & 0x80)
                                Whip_SwingChainDown(whip);
                        }
                    }
                    break;
            }
            {
                struct EwramData_EntityData *whip = gEwramData->playerState.whipChain;

                if (!(FLAGS(entity) & 0x400))
                    whip->posY.whole = entity->posY.whole - 0x130000;
                else
                    whip->posY.whole = entity->posY.whole - 0xB0000;
                if (entity->flipX)
                    whip->posX.whole = entity->posX.whole - 0x170000;
                else
                    whip->posX.whole = entity->posX.whole + 0x180000;
            }
            Julius_UpdateJumpControl(entity);
            break;
        case 2:
        {
            AirControl(entity, FLAGS(entity), 0x20000);
            ApplyAccelX(entity);
            if (FLAGS(entity) & 0x200000)
            {
                FLAGS(entity) &= ~0x200100;
                entity->state = 0;
            }
            Julius_UpdateJumpControl(entity);
            break;
        }
        case 3:
        {
            s32 abs;
            u32 flags;

            {
                u8 four = 4;
                entity->hit.type = four;
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
            Julius_UpdateVerticalMovement(entity);
            break;
        }
        case 10:
            Julius_UpdateGrandCrossCast(entity);
            break;
        case 11:
            Julius_UpdateAbilityDash(entity);
            Julius_HandleWhipAttackInput(entity);
            break;
        case 4:
        {
            u32 flags;

            Decel(entity, 0x3000);
            if (entity->timer <= 7)
            {
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
                    Julius_HandleWhipAttackInput(entity);
                    Julius_HandleJumpInput(entity);
                    Julius_UpdateJumpControl(entity);
                    if (FLAGS(entity) & 2)
                    {
                        entity->state = 0;
                        FLAGS(entity) &= ~0x10080;
                    }
                    if (entity->state == 1)
                        FLAGS(entity) &= ~0x10080;
                }
            }
            break;
        }
        case 5:
            if (entity->velY.whole > 0)
                entity->state = 0;
            {
                u8 four = 4;
                entity->hit.type = four;
            }
            entity->hit.flags &= ~2;
            Julius_UpdateJumpControl(entity);
            break;
        case 7:
            Julius_UpdateDiveAttack(entity);
            Julius_UpdateVerticalMovement(entity);
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
            Julius_UpdateVerticalMovement(entity);
            break;
        }
        case 13:
            Julius_UpdateKnockback(entity);
            break;
        case 14:
            Julius_UpdateKnockbackFromImpulse(entity);
            break;
        case 16:
        {
            u8 four = 4;

            gEwramData->playerState.player->ext.player.invulnerableTimer = four;
            Player_UpdateDeath(entity);
            break;
        }
        case 17:
            FLAGS(gEwramData->playerState.player) &= 0x40000000;
            entity->state = 0;
            break;
    }

    if (entity->ext.player.invulnerableTimer != 0 && --entity->ext.player.invulnerableTimer == 0)
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

        gEwramData->playerState.player->ext.player.invulnerableTimer = two;
    }
    if (PLAYER_BLINK_TIMER(entity) != 0)
    {
        entity->drawSkip = 1;
        if (--PLAYER_BLINK_TIMER(entity) == 0)
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
            if (entity->ext.player.onStairs != 0)
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
        entity->ext.player.wallProbes = ptr;
    }
    if (!(gEwramData->playerState.actionLockFlags & 0x800) && REQ(entity) != 0xFF)
    {
        if (entity->animScriptIndex != REQ(entity))
        {
            Anim_StartIndexed(entity, REQ(entity), 1, 0);
            FLAGS(entity) &= ~0x200000;
        }
        else if (FLAGS(entity) & 0x200000)
            entity->ext.player.requestedAnim = 0xFF;
    }
    Anim_Update(entity);
    Entity_UpdateBoxes(entity);
    entity->flipX ^= 1;
    HitBox_UpdatePlayerRects();
    entity->flipX ^= 1;
    Player_ApplyPreparedStats();
    Player_RecalculateAttackDefense();
    gEwramData->playerState.collisionQueueCount = 0;
    if (gEwramData->playerState.actionLockFlags & 0x20)
    {
        entity->velX.whole = 0;
        entity->accelX.whole = 0;
    }
    gEwramData->playerState.pendingHitDamage = 0;
    FLAGS(entity) &= ~0x4080000;
    gEwramData->playerState.actionLockFlags &= ~0x8FE;
    gEwramData->playerStatus.inputOverrides = 0;
    Camera_ScrollWithPriority(0, ((s16)entity->posX.part16.integer - gEwramData->playerState.cameraAnchorX) << 16,
        ((s16)entity->posY.part16.integer - gEwramData->playerState.cameraAnchorY) << 16);
    entity->ext.player.frameCounter++;
    if (NEW & gEwramData->playerStatus.guardianButton)
    {
        gEwramData->playerStatus.equippedRedSoul++;
        if (gEwramData->playerStatus.equippedRedSoul > 3)
            gEwramData->playerStatus.equippedRedSoul = 0;
    }
}

