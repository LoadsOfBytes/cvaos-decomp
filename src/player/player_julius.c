/*
 * Player (Julius mode): collision corrections, movement, whip attack, dive attack, jumps, death, knockback, ability dash, Grand Cross and the shared player update.
 */
#include "core.h"
#include "player_entity.h"
#include "bg_and_collision.h"
#include "player_effects.h"
#include "sound.h"
#include "player_core.h"
#include "map_flags.h"
#include "soul_inventory.h"
#include "player_level.h"
#include "bg_effects.h"
#include "blue_souls.h"
#include "effects.h"
#include "enemy_common.h"
#include "hitbox.h"
#include "math_anim.h"
#include "obj_gfx.h"
#include "palette.h"
#include "player_soma.h"
#include "red_souls_b.h"
#include "data/data_0E0B68.h"
#include "data/data_118B48.h"
/* from graphics.h (not included: see the module notes) */
#include "player_physics.h"


void Julius_TryStartDiveAttack(struct EwramData_EntityData *entity);
void Julius_UpdateVerticalMovement(struct EwramData_EntityData *entity);
void Julius_UpdateVerticalMovement(struct EwramData_EntityData *entity);
void Julius_UpdateVerticalMovement(struct EwramData_EntityData *entity);

/* Julius collision with the tile map, the carrying object and moving platforms.
 * Twin of the Soma version in player/player_soma.c. */


static inline void SetJuliusAnimF(struct EwramData_EntityData *entity, u8 anim, s32 flag)
{
    if (!(gEwramData->playerState.actionLockFlags & 0x800) && entity->animScriptIndex != anim)
    {
        Anim_StartIndexed(entity, anim, 1, flag);
        entity->ext.player.requestedAnim = 0xFF;
        FLAGS(entity) &= ~0x200000;
    }
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
    offsets = (s8 *)entity->ext.player.wallProbes;
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
    entity->ext.player.onStairs = 0;
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
                    entity->ext.player.onStairs = 1;
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
    entity->ext.player.slopeLevel = 0;
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
    /* Matching reconstruction of the landing tail, not an original-source claim.
     * The ROM has "bne zero; b next; <no-floor block>; next:" although the code
     * around it was optimized as one straight run (shared zero / EWRAM registers,
     * reload scratch r1 then r2). The only form found that reproduces it lets the
     * final cross-jump pass create the "b next":
     *  - "(s32)entity > N" are opaque tests that never fire; they only keep the
     *    duplicated code reachable until the last jump pass and then disappear;
     *  - the first "if (statusEffects & 4)" is a copy that is merged into the one at
     *    "next", leaving just the jump;
     *  - "dup" is a copy of the actionLockFlags test placed between them so that the
     *    reload scratch registers (which rotate r1, r2 in insn order) come out the
     *    same in both copies; it is merged back into the real test;
     *  - z / ew / g / b are explicit variables because CSE cannot carry the values
     *    across the extra labels. */
    {
    s32 z;
    struct EwramData **ew;
    struct EwramData *g;

    if (c != 0 || left != 0 || right != 0)
    {
        s32 sc = c;

        if (!CanLand(entity, sc))
            return;
        if (!IsLanding(entity, sc))
            return;
        FLAGS(entity) |= 0x100000;
        {
        register u8 b asm("r1"); /* FAKEMATCH: pinned register */

        b = 0;
        z = 0;
        entity->ext.player.airTime = 0;
        FLAGS(entity) &= 0xFFDFFFE1;
        entity->timer = b;
        }
        PlaySong(0xBB);
        entity->posY.part16.fraction = 0;
        ew = &gEwramData;
        g = *ew;
        if ((s32)entity > 1)
            goto dup;
        if (g->playerState.actionLockFlags & 8)
            goto zero;
        if ((s32)entity > 0)
            goto next;
        if (g->playerStatus.statusEffects & 4)
        {
            entity->velY.whole = z;
            entity->accelY.whole = z;
            return;
        }
        goto cont;
    dup:
        if (g->playerState.actionLockFlags & 8)
            goto zero;
        goto next;
    }
    else
    {
        FLAGS(entity) &= ~0x100000;
        return;
    }
next:
    if (g->playerStatus.statusEffects & 4)
        {
zero:
            entity->velY.whole = z;
            entity->accelY.whole = z;
            return;
        }
cont:
    {
        if (entity->velY.whole > 0x64000 || (FLAGS(entity) & 0x80))
        {
            FLAGS(entity) |= 0x10000;
            FLAGS(entity) &= 0xFFFFFE9F;
            PlaySong(0xBB);
            if (!((*ew)->playerState.actionLockFlags & 0x800) && entity->animScriptIndex != 0x13)
            {
                Anim_StartIndexed(entity, 0x13, 1, 0);
                entity->ext.player.requestedAnim = 0xFF;
                FLAGS(entity) &= ~0x200000;
            }
            entity->state = 4;
        }
        else if (!(FLAGS(entity) & 0x160))
        {
            if (!(g->inputData.playerHeldInput & 0xF0)
             && !((g->playerStatus.jumpButton | g->playerStatus.attackButton | g->playerStatus.abilityButton | g->playerStatus.guardianButton) & g->inputData.playerNewInput))
                entity->ext.player.requestedAnim = 0x12;
            if (gEwramData->inputData.playerHeldInput & 0x30)
                SetJuliusAnimF(entity, 1, 1);
            entity->state = 0;
        }
    }
    entity->velY.whole = 0;
    entity->accelY.whole = 0;
    }
}

/**
 * @brief Resolve Julius collision against the tile map, carrying-object
 * callbacks and moving platforms through the existing inline
 * reconstruction. Preserve character-specific animation behavior.
 */
void Julius_ResolveWorldCollision(struct EwramData_EntityData *entity)
{
    PlayerCollision(entity);
}

/* Julius_HandleWhipAttackInput. */

/* Same block as written out in Julius_UpdateKnockbackFromImpulse / Julius_TryStartAbilityDash / Julius_StartDeath. */
static inline void SetPlayerAnim(struct EwramData_EntityData *entity, s32 anim)
{
    if (!(gEwramData->playerState.actionLockFlags & 0x800) && entity->animScriptIndex != anim)
    {
        Anim_StartIndexed(entity, anim, 1, 0);
        entity->ext.player.requestedAnim = 0xFF;
        entity->ext.player.actionFlags &= 0xFFDFFFFF;
    }
}

/**
 * @brief On a new attack press (or playerStatus.inputOverrides bit 1), unless actionLockFlags bit
 * 0x80 or flags 0x160 block it, select animation 0x18 (0x1A when
 * crouching flag 0x400 is set), play sound 0x122, set flag 0x20,
 * enter state 1 and start the whip helper.
 */
void Julius_HandleWhipAttackInput(struct EwramData_EntityData *entity)
{
    if (gEwramData->playerState.actionLockFlags & 0x80)
        return;
    if (!(gEwramData->inputData.playerNewInput & gEwramData->playerStatus.attackButton)
        && !(gEwramData->playerStatus.inputOverrides & 2))
        return;
    if (entity->ext.player.actionFlags & 0x160)
        return;
    if (!(entity->ext.player.actionFlags & 0x400))
        SetPlayerAnim(entity, 0x18);
    else
        SetPlayerAnim(entity, 0x1A);
    PlaySong(0x122);
    entity->ext.player.actionFlags |= 0x20;
    entity->state = 1;
    entity->subState = 0;
    Julius_CreateWhipAttackEntity(entity);
}

/* Julius_TryStartDiveAttack
 * (dive attack as in Soma_TryStartDiveAttack without hitboxes). */

static inline void SetPlayerAnim2(struct EwramData_EntityData *entity, s32 anim)
{
    if (!(gEwramData->playerState.actionLockFlags & 0x800) && entity->animScriptIndex != anim)
    {
        Anim_StartIndexed(entity, anim, 1, 1);
        entity->ext.player.requestedAnim = 0xFF;
        entity->ext.player.actionFlags &= 0xFFDFFFFF;
    }
}

/**
 * @brief When the entity is airborne (flag 2) without flags 0x400000 and
 * 0x800000 and down + new jump is pressed, enter state 7 with downward
 * velY 0x4E000, optional +-0x40000 X, animation 9/0x1E by speed,
 * sounds 0x1EC/0xA9 and two afterimages. Julius counterpart of
 * Soma_TryStartDiveAttack, without attack-box setup.
 */
void Julius_TryStartDiveAttack(struct EwramData_EntityData *entity)
{
    struct EwramData *ew;
    u16 key;
    s32 speed;

    if ((entity->ext.player.actionFlags & 0xC00002) != 2)
        return;
    ew = gEwramData;
    if (!(ew->inputData.playerHeldInput & 0x80))
        return;
    if (!(ew->inputData.playerNewInput & ew->playerStatus.jumpButton))
        return;
    entity->state = 7;
    entity->velY.whole = 0x4E000;
    entity->accelY.whole = 0x2000;
    if ((key = ew->inputData.playerHeldInput & 0x20))
    {
        entity->flipX = 1;
        entity->velX.whole = -0x40000;
        entity->accelX.whole = 0;
    }
    else if (ew->inputData.playerHeldInput & 0x10)
    {
        entity->flipX = 0;
        entity->velX.whole = 0x40000;
        entity->accelX.whole = key;
    }
    else
        entity->accelX.whole = (entity->velX.whole < 0) ? 0x4000 : -0x4000;
    speed = entity->velX.whole;
    if (speed < 0)
        speed = -speed;
    if (speed > 0x10000)
        SetPlayerAnim2(entity, 9);
    else
        SetPlayerAnim2(entity, 0x1E);
    gEwramData->playerState.actionLockFlags |= 4;
    PlaySong(0x1EC);
    PlaySong(0xA9);
    Afterimage_Create(entity, 0, 6, 0xD5);
    Afterimage_Create(entity, 0, 3, 0xD4);
}

/* Julius_UpdateDiveAttack. */
/**
 * @brief Update state 7: end on flag 0x400000; on contact flag 0x80000
 * rebound upward (velY -0x3C000), set flag 8, clear flag 4, play
 * sound 0xB9 and return to state 0; otherwise set actionLockFlags |= 4.
 */
void Julius_UpdateDiveAttack(struct EwramData_EntityData *entity)
{
    if (entity->ext.player.actionFlags & 0x400000)
    {
        entity->state = 0;
        return;
    }
    {
        u8 two = 2;
        entity->hit.type = two;
    }
    entity->hit.flags &= ~2;
    if (entity->ext.player.actionFlags & 0x80000)
    {
        entity->ext.player.actionFlags = (entity->ext.player.actionFlags | 8) & ~4;
        entity->ext.player.requestedAnim = 3;
        PlaySong(0xB9);
        entity->velY.whole = -0x3C000;
        entity->accelY.whole = -0x2000;
        entity->state = 0;
    }
    else
        gEwramData->playerState.actionLockFlags |= 4;
}

/* Soma_TryCastRedSoulUnused (Soma variant
 * of Julius_HandleSubweaponInput in src/player/player_level.c). */

/**
 * @brief 1F2C8 | Cast the equipped red soul on up + attack: switching soul
 * frees the old graphics (redSoulGfx); checks MP, the soul limit and a free
 * slot, spawns the soul entity, pays MP and plays animation 0x17 (state 2).
 */
void Soma_TryCastRedSoulUnused(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child;
    s8 slot;
    const struct RedSoulInfo *info;

    if (entity->ext.player.actionFlags & 0x540)
        return;
    if (!(gEwramData->inputData.playerHeldInput & 0x40))
        return;
    if (!(gEwramData->inputData.playerNewInput & gEwramData->playerStatus.attackButton))
        return;
    info = RedSoul_GetInfo(gEwramData->playerStatus.equippedRedSoul);
    if (gEwramData->playerState.nbrActiveRedSoulEntities == 0)
    {
        if (gEwramData->playerState.lastUsedRedSoul != gEwramData->playerStatus.equippedRedSoul)
        {
            gEwramData->playerState.lastUsedRedSoul = gEwramData->playerStatus.equippedRedSoul;
            if (gEwramData->playerState.redSoulGfx != -1)
            {
                ObjGfx_Unload(gEwramData->playerState.redSoulGfx);
                gEwramData->playerState.redSoulGfx = -1;
            }
        }
    }
    if (gEwramData->playerState.lastUsedRedSoul != gEwramData->playerStatus.equippedRedSoul)
        return;
    if (info->updateFunc == 0)
        return;
    if (info->mpCost >= gEwramData->playerStatus.currentMP)
        return;
    if (RedSoul_GetActiveLimit() <= gEwramData->playerState.nbrActiveRedSoulEntities)
        return;
    slot = RedSoul_FindFreeEntitySlot();
    if (slot < 0)
        return;
    child = EntityCreateInRange(2, 0x10, info->updateFunc);
    if (child == NULL)
        return;
    gEwramData->playerStatus.currentMP -= info->mpCost;
    gEwramData->playerState.activeRedSoulEntities[slot] = child;
    if (!(gEwramData->playerState.actionLockFlags & 0x800) && entity->animScriptIndex != 0x17)
    {
        Anim_StartIndexed(entity, 0x17, 1, 0);
        entity->ext.player.requestedAnim = 0xFF;
        entity->ext.player.actionFlags &= 0xFFDFFFFF;
    }
    entity->ext.player.actionFlags |= 0x100;
    entity->state = 2;
    child->ext.redSoul.owner = entity;
    child->ext.redSoul.slot = slot;
    child->ext.redSoul.id = gEwramData->playerStatus.equippedRedSoul;
    child->ext.redSoul.param = info->param;
    child->roomActive = 1;
    entity->ext.player.actionFlags &= ~0x200060;
}

/* Julius_HandleJumpInput. */

/**
 * @brief Process the ability-button high jump (state 5, requires flag 2),
 * the ordinary jump and both additional-jump variants (flag 4 or
 * flag 0x2000000 marks use), then fall through to the dive-attack
 * start check unless the 0x800000 variant was taken.
 */
void Julius_HandleJumpInput(struct EwramData_EntityData *entity)
{
    u32 flags = entity->ext.player.actionFlags;

    if (flags & 0x20000000)
        return;
    if (flags & 0x160)
        return;
    if (gEwramData->inputData.playerNewInput & gEwramData->playerStatus.abilityButton)
    if (flags & 2)
    if (!(flags & 0x10))
    {
        entity->ext.player.requestedAnim = 0xFF;
        entity->state = 5;
        PlaySong(0xBA);
        entity->ext.player.actionFlags = (entity->ext.player.actionFlags | 0x12) & 0xFFEFFBFB;
        entity->accelY.whole = 0x2000;
        if (!(entity->ext.player.actionFlags & 0x4000000))
            entity->velY.whole = -0x90000;
        else
            entity->velY.whole = -0x24000;
        Afterimage_Create(entity, 0, 8, 0xD5);
        Afterimage_Create(entity, 0, 4, 0xD4);
    }
    if ((u16)entity->ext.player.dropThroughTimer != 0)
        return;
    flags = entity->ext.player.actionFlags;
    if (flags & 0x10)
        return;
    if (!(flags & 0x800000))
    {
        if (!(flags & 4) && (gEwramData->inputData.playerNewInput & gEwramData->playerStatus.jumpButton))
        {
            PlaySong(0xB9);
            if (entity->ext.player.actionFlags & 2)
                entity->ext.player.actionFlags |= 4;
            entity->ext.player.actionFlags = (entity->ext.player.actionFlags | 2) & 0xFFEFFBFF;
            entity->ext.player.airTime = 0x10;
            entity->accelY.whole = 0;
            if (!(entity->ext.player.actionFlags & 0x4000000))
                entity->velY.whole = -0x50000;
            else
                entity->velY.whole = -0x13C00;
        }
    }
    else if (!(flags & 0x2000000) && (gEwramData->inputData.playerNewInput & gEwramData->playerStatus.jumpButton))
    {
        entity->accelY.whole = 0;
        entity->ext.player.actionFlags = (flags | 0x02000002) & 0xFFEFFBF7;
        if (entity->velY.whole > 0x10000)
            entity->velY.whole += -0x4E000;
        else
            entity->velY.whole = -0x4E000;
        return;
    }
    Julius_TryStartDiveAttack(entity);
}

/* Julius_UpdateVerticalMovement. */

/* Function table of the object in solidObject (pointer at its unk_4F4). */
struct SolidObjectCallbacks2 {
    void *flags;
    s16 (*floorCorrection)(struct EwramData_EntityData *object, s32 x, s32 y);
};

/* Animation switch without hitbox, as in this file (flags 1, 1). */

/* Floor correction at room pixel (x, y): the tile map value (Collision_GetPushDown),
 * or the larger one reported by the object in solidObject. */
static inline s16 FloorCorrection(s16 x, s16 y)
{
    u16 c = Collision_GetPushDown(x, y);
    struct EwramData_EntityData *object = gEwramData->playerState.solidObject;

    if (object != NULL)
    {
        s16 d = ((struct SolidObjectCallbacks2 *)object->ext.solidObject.callbacks.words)->floorCorrection(object, x, y);
        return ((s16)c < d) ? d : c;
    }
    return c;
}

/**
 * @brief Update Julius airborne gravity, floor correction, landing
 * feedback and air animation through the existing flag guards.
 * Preserve the original asymmetric probe/velocity conditions.
 */
void Julius_UpdateVerticalMovement(struct EwramData_EntityData *entity)
{
    s16 corr = 0;
    u16 y = GetEntityRoomYPositionInteger(entity);
    u16 x = GetEntityRoomXPositionInteger(entity);
    u32 flags;

    if (!(entity->ext.player.actionFlags & 0x100002))
    {
        entity->ext.player.actionFlags |= 2;
        entity->velY.whole = corr;
        entity->accelY.whole = -0x2000;
    }
    if (!(entity->ext.player.actionFlags & 0x1E))
        return;
    entity->ext.player.actionFlags &= 0xEFEFFBFF;
    if (entity->ext.player.actionFlags & 0x4000000)
    {
        entity->velY.whole += 0x6000;
        entity->accelY.whole = corr;
    }
    if (entity->ext.player.airTime <= 0xF)
        entity->ext.player.airTime++;
    if (entity->velY.whole <= 0 || entity->velX.whole != 0)
    {
        /* (x - 5, y - 0x20) and (x + 5, y - 0x20), written in 16.16 as in the
         * original arithmetic. */
        corr = FloorCorrection(((x << 16) - 0x50000) >> 16, ((y << 16) - 0x200000) >> 16);
        if (corr == 0)
            corr = FloorCorrection(((x << 16) + 0x50000) >> 16, ((y << 16) - 0x200000) >> 16);
        if (corr != 0)
        {
            entity->posY.part16.integer += corr;
            if (entity->velY.whole <= -0x4F000 && (entity->ext.player.actionFlags & 0x10))
            {
                PlaySong(0xB8);
                entity->state = 0;
                ScreenShake_Request(1, 0x10, 0, 7);
            }
            if (entity->velY.whole < 0)
            {
                entity->velY.whole = 0x1000;
                entity->accelY.whole = -0x2000;
            }
        }
    }
    if (entity->velY.whole <= 0x1FFF)
        entity->velY.whole += 0x2000;
    entity->velY.whole += 0x1800;
    if (entity->velY.whole > 0)
    {
        entity->velY.whole += entity->accelY.whole;
        entity->accelY.whole += 0x400;
        if (entity->accelY.whole > 0x1000)
            entity->accelY.whole = 0x1000;
    }
    flags = entity->ext.player.actionFlags;
    if (flags & 0x10)
    {
        if (!(flags & 0x400000))
        {
            if (entity->velY.whole > 0x12000)
                entity->ext.player.actionFlags = flags & ~0x10;
        }
        else if (entity->velY.whole > 0x2000)
            entity->ext.player.actionFlags = flags & ~0x10;
    }
    flags = entity->ext.player.actionFlags;
    if (flags & 0x1E0)
        return;
    if (gEwramData->playerState.actionLockFlags & 4)
        return;
    if (flags & 0x10)
        SetPlayerAnim2(entity, 0x16);
    else if (entity->velY.whole < -0x8000)
        SetPlayerAnim2(entity, 3);
    else if (entity->velY.whole <= 0x7FFF)
        SetPlayerAnim2(entity, 0x10);
    else
        SetPlayerAnim2(entity, 0x11);
}

/* Julius_UpdateJumpControl. The flags and
 * velocity are re-read from the entity each time (the CSE copies match only
 * that way). */

/**
 * @brief Unless actionLockFlags bit 0x10 is set, cut rising speed when jump is
 * released, add held-jump lift near the apex and slow falling under
 * flag 0x400000, then update Julius vertical movement.
 */
void Julius_UpdateJumpControl(struct EwramData_EntityData *entity)
{
    struct EwramData *ew = gEwramData;

    if (ew->playerState.actionLockFlags & 0x10)
        return;
    if ((entity->ext.player.actionFlags & 0x1E) && !(entity->ext.player.actionFlags & 0x18))
    {
        if (entity->velY.whole < -0x4000 && !(ew->inputData.playerHeldInput & ew->playerStatus.jumpButton))
        {
            entity->velY.whole = -0x4000;
            entity->accelY.whole = -0x2000;
        }
        if (entity->velY.whole > -0x20000 && entity->velY.whole < 0x2000 && (gEwramData->inputData.playerHeldInput & gEwramData->playerStatus.jumpButton))
        {
            entity->accelY.whole -= 0x800;
            if (entity->accelY.whole < -0x2000)
                entity->accelY.whole = -0x2000;
        }
        if ((entity->ext.player.actionFlags & 0x400000) && entity->velY.whole > 0x1800)
            entity->velY.whole -= 0x1800;
    }
    Julius_UpdateVerticalMovement(entity);
}

/* Julius_StartDeath. */

/**
 * @brief Enter death state 16 from the Julius main update at nonpositive
 * HP: set scene flags, animation 0xC, stop blue souls and apply
 * the existing recoil velocity, player flags and sound.
 */
void Julius_StartDeath(struct EwramData_EntityData *entity)
{
    gEwramData->gameState.subMode = 2;
    gEwramData->gameState.subModeStep = 0;
    if (!(gEwramData->playerState.actionLockFlags & 0x800) && entity->animScriptIndex != 0xC)
    {
        Anim_StartIndexed(entity, 0xC, 1, 0);
        entity->ext.player.requestedAnim = 0xFF;
        entity->ext.player.actionFlags &= 0xFFDFFFFF;
    }
    BlueSoul_StopAll();
    entity->ext.player.actionFlags &= 0xEFFFFE9F;
    gEwramData->playerState.actionLockFlags |= 0xE;
    gEwramData->playerState.pendingHitDamage = 0;
    if (!entity->flipX)
        entity->velX.whole = -0x10000;
    else
        entity->velX.whole = 0x10000;
    entity->velY.whole = -0x18000;
    entity->accelY.whole = 0x2000;
    entity->state = 0x10;
    {
        u8 two = 2;
        gEwramData->playerState.player->ext.player.invulnerableTimer = two;
    }
    PlaySong(0x1E9);
}

/* Julius_UpdateKnockback. */

/**
 * @brief Update damage-reaction state 13: damp horizontal motion by
 * +-0x1000 on the tile check without crossing zero, return to state 0
 * when the airborne flag clears, then update vertical movement.
 */
void Julius_UpdateKnockback(struct EwramData_EntityData *entity)
{
    s32 old, new;
    u16 x;
    s16 y;

    entity->ext.player.actionFlags &= 0xEFFFFE9F;
    x = GetEntityRoomXPositionInteger(entity);
    y = GetEntityRoomYPositionInteger(entity);
    if (Collision_IsPointSolid((s16)x, y))
        entity->accelX.whole = (entity->velX.whole < 0) ? 0x1000 : -0x1000;
    old = entity->velX.whole;
    new = old + entity->accelX.whole;
    entity->velX.whole = new;
    if (old < 0)
        goto check_pos;
    if (new < 0)
        goto clear;
    if (old > 0)
        goto done;
check_pos:
    if (new <= 0)
        goto done;
clear:
    entity->velX.whole = 0;
    entity->accelX.whole = 0;
done:
    if (!(entity->ext.player.actionFlags & 2))
    {
        entity->ext.player.actionFlags &= ~0x80;
        entity->state = 0;
    }
    Julius_UpdateVerticalMovement(entity);
}

/* Julius_UpdateKnockbackFromImpulse (variant of Julius_UpdateKnockback with an animation switch). */

/**
 * @brief Update damage-reaction state 14, entered with stored external
 * impulse velocities: select animation 0xC, then damp and recover as
 * Julius_UpdateKnockback and update vertical movement.
 */
void Julius_UpdateKnockbackFromImpulse(struct EwramData_EntityData *entity)
{
    s32 old, new;
    u16 x;
    s16 y;

    entity->ext.player.actionFlags &= 0xEFFFFE9F;
    if (!(gEwramData->playerState.actionLockFlags & 0x800) && entity->animScriptIndex != 0xC)
    {
        Anim_StartIndexed(entity, 0xC, 1, 0);
        entity->ext.player.requestedAnim = 0xFF;
        entity->ext.player.actionFlags &= 0xFFDFFFFF;
    }
    x = GetEntityRoomXPositionInteger(entity);
    y = GetEntityRoomYPositionInteger(entity);
    if (Collision_IsPointSolid((s16)x, y))
        entity->accelX.whole = (entity->velX.whole < 0) ? 0x1000 : -0x1000;
    old = entity->velX.whole;
    new = old + entity->accelX.whole;
    entity->velX.whole = new;
    if (old < 0)
        goto check_pos;
    if (new < 0)
        goto clear;
    if (old > 0)
        goto done;
check_pos:
    if (new <= 0)
        goto done;
clear:
    entity->velX.whole = 0;
    entity->accelX.whole = 0;
done:
    if (!(entity->ext.player.actionFlags & 2))
    {
        entity->ext.player.actionFlags &= ~0x80;
        entity->state = 0;
    }
    Julius_UpdateVerticalMovement(entity);
}

/* Julius_TryStartAbilityDash. */

/**
 * @brief When flags 2, 0x20 and 0x100 are clear and the ability button is
 * held, enter state 11 with animation 3, facing velocity +-0x28000
 * and opposing deceleration 0x800, two afterimages and sound 0x12B.
 */
void Julius_TryStartAbilityDash(struct EwramData_EntityData *entity)
{
    s32 bits = entity->ext.player.actionFlags & 0x122;

    if (bits == 0 && (gEwramData->inputData.playerHeldInput & gEwramData->playerStatus.abilityButton))
    {
        entity->state = 0xB;
        entity->subState = bits;
        entity->ext.player.requestedAnim = 0xFF;
        if (!(gEwramData->playerState.actionLockFlags & 0x800) && entity->animScriptIndex != 3)
        {
            Anim_StartIndexed(entity, 3, 1, 0);
            entity->ext.player.requestedAnim = 0xFF;
            entity->ext.player.actionFlags &= 0xFFDFFFFF;
        }
        if (!entity->flipX)
            entity->velX.whole = 0x28000;
        else
            entity->velX.whole = -0x28000;
        entity->accelX.whole = (entity->velX.whole < 0) ? 0x800 : -0x800;
        entity->timer = 0;
        Afterimage_Create(entity, 0, 0xC, 0xD5);
        Afterimage_Create(entity, 0, 6, 0xD4);
        PlaySong(0x12B);
    }
}

/* Julius_UpdateAbilityDash. */
/**
 * @brief Update state 11: set drawSkip, decelerate X without crossing zero,
 * stop vertical motion, flip facing at timer 8 and return to state 0
 * after 0x40 updates.
 */
void Julius_UpdateAbilityDash(struct EwramData_EntityData *entity)
{
    s32 old, new;

    {
        u8 two = 2;
        gEwramData->playerState.player->ext.player.invulnerableTimer = two;
    }
    entity->drawSkip = 1;
    old = entity->velX.whole;
    new = old + entity->accelX.whole;
    entity->velX.whole = new;
    if (old < 0)
        goto check_pos;
    if (new < 0)
        goto clear;
    if (old > 0)
        goto done;
check_pos:
    if (new <= 0)
        goto done;
clear:
    entity->velX.whole = 0;
    entity->accelX.whole = 0;
done:
    entity->velY.whole = 0;
    entity->accelY.whole = 0;
    if (entity->timer == 8)
        entity->flipX ^= 1;
    if (++entity->timer > 0x3F)
        entity->state = 0;
}

/* Julius_UpdateGrandCrossCast. */

/* Animation switch without hitbox, as in this file (flags 1, 1). */

/**
 * @brief Julius state 10, entered for the sub-weapon entry with unk_4 set
 * (entry 3, Grand Cross): converging particles and animation 0xA,
 * then animation 0xB for 0x10 updates, then 0xA until return to
 * state 0. Vertical motion is held at zero.
 */
void Julius_UpdateGrandCrossCast(struct EwramData_EntityData *entity)
{
    {
        u8 four = 4;
        gEwramData->playerState.player->ext.player.invulnerableTimer = four;
    }
    gEwramData->playerState.actionLockFlags |= 0x20;
    entity->velY.whole = 0;
    entity->accelY.whole = 0;
    switch (entity->subState)
    {
    case 0:
        if (!(entity->timer & 1))
        {
            s16 angle = RandomNumberGenerator();
            s32 radius = (RandomNumberGenerator() & 0x3F) + 0x40;
            s32 dx = radius * Sin(angle + 0x4000);
            s32 dy = radius * Sin(angle);
            s32 x = entity->posX.whole + dx;
            s32 y = dy - 0x100000;
            struct EwramData_EntityData *effect = Effect_CreateSharedFrame(x, entity->posY.whole + y, 0x40, 6);

            if (effect != NULL)
            {
                s32 vx, vy;

                effect->timer = 0x20;
                effect->velX.whole = vx = -dx / 64;
                effect->velY.whole = vy = -dy / 64;
                effect->accelX.whole = vx / 32;
                effect->accelY.whole = vy / 32;
            }
        }
        SetPlayerAnim2(entity, 0xA);
        if (entity->timer == 0)
        {
            entity->timer = 0x60;
            entity->subState++;
        }
        else
            entity->timer--;
        break;
    case 1:
        SetPlayerAnim2(entity, 0xB);
        if (entity->timer == 0)
        {
            entity->timer = 0x10;
            entity->subState++;
        }
        else
            entity->timer--;
        break;
    case 2:
        SetPlayerAnim2(entity, 0xA);
        if (entity->timer != 0)
            entity->timer--;
        else
        {
            entity->state = 0;
            entity->ext.player.actionFlags &= 0xFFDFFEFF;
        }
        break;
    }
}

/* Julius main update: damage reactions, collision, whip input and state machine.
 * The helper boundaries and narrow temporaries preserve the original ROM. */


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

#define DUST_SIDE (!gEwramData->playerState.player->flipX)

#define SetJuliusAnim(e, a) SetJuliusAnimF(e, a, 0)

/* Keep the inline boundary: it changes the offset pseudo's lifetime and the
 * subsequent reload allocation in agbcc. This is not an original-source claim. */
static inline struct EwramData_EntityData *GetWhip(void)
{
    return gEwramData->playerState.whipChain;
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
            Anim_StartIndexed(entity, (u16)(REQ(entity)), 1, 0);
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
