/*
 * Death: the weapon (boss arm segment), trails, blink / intro effects, floating weapons, blades and the fight.
 */
#include "death.h"
#include "core.h"
#include "music.h"
#include "sound.h"
#include "player_core.h"
#include "graphics.h"
#include "gameplay.h"
#include "structs/ewram.h"
#include "structs/entity_render_flags.h"
#include "structs/main.h"
#include "syscalls.h"
#include "entity_hit.h"
#include "in_game.h"
#include "boss_defeat.h"
#include "effects.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "entity_motion.h"
#include "hitbox.h"
#include "killer_mantle.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "palette.h"
#include "damage_numbers.h"
#include "player_level.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"
#include "hit_shared_history.h"

void DeathWeapon_DamagePlayerOnContact(struct EwramData_EntityData *);

#define SPIN(e) (e->ext.deathWeapon.angle)
#define SPEED(e) (e->ext.deathWeapon.spin)
#define FLAGS(e) (e->ext.death.flags)
#define TO_PIXELS(d) ((d) < 0 ? -(-(d) >> 16) : (d) >> 16)


void DeathFloatingWeaponHitBox_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
void DeathFloatingWeapon_OnHit(struct EwramData_EntityData *, s32, u16 *);
void DeathBlade_OnHit(struct EwramData_EntityData *);
void DeathPillar_DamagePlayerOnContact(struct EwramData_EntityData *);
void Death_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);

/**
 * @brief Affine weapon controller: follow owner poses, perform throw/return
 * or wall traversal from owner flags, and fly away on owner defeat.
 * A secondary instance mirrors the primary weapon and shares its lifetime.
 */
void DeathWeapon_Update(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    struct EwramData_EntityData *owner = &gEwramData->entityData[(u8)entity->ext.deathHelper.ownerIndex];
    struct ArmPose *pose = &sDeathWeaponArmPoses[(u8)owner->ext.death.dashStep];
    s32 dx, dy, angle;
    s32 zero;
    u8 t;

    if ((u8)(entity->state - 3) <= 4 && (owner->enemyFlags & 2))
    {
        Sound_StopTracked(0x105);
        entity->attack = 0x28;
        entity->state = 9;
        entity->velX.whole = 0x18000;
        entity->accelX.whole = 0;
        entity->velY.whole = -0x40000;
        entity->accelY.whole = 0x2000;
        entity->hidden = 0;
    }
    switch (entity->state)
    {
    case 0:
    {
        u16 slot;
        if (!Entity_LoadGraphics(entity, sDeathGfx, sDeathPalette, 1, sDeathSpriteSheet))
            goto remove;
        slot = ObjAffine_Alloc(1);
        if ((s16)slot < 0)
            goto remove;
        entity->affineIndex = slot;
        entity->affine = 1;
        entity->attack = owner->attack;
        entity->enemyId = owner->enemyId;
        entity->spriteLayer = owner->spriteLayer;
        Anim_StartIndexed(entity, 0x11, 2, 0);
        Anim_Update(entity);
        Hit_Enable(entity, 8, DeathWeapon_DamagePlayerOnContact, 0);
        entity->hit.flags |= 4;
        entity->attack = 0x3C;
        SPIN(entity) = 0x4000;
        if (entity->ext.deathWeapon.mirror)
        {
            entity->state = 8;
            goto tail;
        }
        entity->state = 1;
        entity->timer = 8;
    }
    case 1:
        entity->posX.whole = owner->posX.whole;
        entity->posY.whole = owner->posY.whole - 0x280000;
        if (entity->timer)
        {
            if ((u8)--entity->timer > 3)
                goto tail;
            if (entity->timer & 1)
                entity->palette = owner->palette;
            else
                entity->palette = entity->basePalette;
            goto tail;
        }
        entity->timer = 4;
        entity->palette = owner->palette;
        entity->state = 2;
    case 2:
        SPIN(entity) = (entity->timer << 10) + pose->angle;
        entity->posX.part16.integer = owner->posX.part16.integer + pose->dx;
        { u16 v_ = owner->posY.part16.integer + pose->dy - entity->timer; register u16 *p_ asm("r3") = &entity->posY.part16.integer; *p_ = v_; } /* FAKEMATCH: pinned register */
        if ((t = entity->timer) != 0)
            goto count;
        goto idle;
    case 3:
    {
        u32 flags = (u16)(FLAGS(owner) & 8);
        if (flags)
        {
            s32 speed, vx, ax;
            { u8 *p_ = &entity->attack; u8 v_; flags = 0; v_ = 0x41; *p_ = v_; }
            entity->hidden = 0;
            SPEED(entity) = speed = 0x7A0;
            entity->velX.whole = vx = 0x48000;
            entity->accelX.whole = ax = -0x1000;
            entity->velY.whole = flags;
            entity->accelY.whole = flags;
            entity->timer = 0x10;
            if (!owner->flipX)
            {
                SPEED(entity) = -speed;
                entity->velX.whole = -vx;
                entity->accelX.whole = -ax;
            }
            entity->state = 5;
            goto tail;
        }
        if (FLAGS(owner) & 0x10)
        {
            entity->attack = 0x46;
            entity->hidden = 0;
            entity->state = 4;
            entity->stage = flags;
            goto tail;
        }
        entity->attack = 0x3C;
        entity->hidden = pose->flag;
        if (!owner->flipX)
        {
            SPIN(entity) = pose->angle;
            entity->posX.part16.integer = owner->posX.part16.integer + pose->dx;
        }
        else
        {
            s32 an = pose->angle;
            SPIN(entity) = -0x8000 - an;
            entity->posX.part16.integer = owner->posX.part16.integer - pose->dx;
        }
        entity->posY.part16.integer = owner->posY.part16.integer + pose->dy;
        entity->spriteLayer = owner->spriteLayer + pose->layer;
        entity->flipX = owner->flipX;
        goto tail;
    }
    case 5:
        SPIN(entity) += SPEED(entity);
        if (!(entity->subState & 0x1F))
            PlaySong(0x135);
        if (FLAGS(owner) & 4)
            entity->state = 6;
        goto tail;
    case 6:
    {
        if (!(entity->subState & 0x1F))
            PlaySong(0x135);
        SPIN(entity) += SPEED(entity);
        EntityAccelerateAndMove(entity, 0x20000);
        if ((t = entity->timer) != 0)
            goto count;
        zero = owner->posX.whole - entity->posX.whole;
        if (zero < 0)
            zero = entity->posX.whole - owner->posX.whole;
        if (zero > 0x1FFFF)
            goto tail;
        FLAGS(owner) &= ~0xC;
        entity->timer = 0x34;
        entity->state = 7;
        goto tail;
    }
    case 7:
        SPIN(entity) += SPEED(entity);
        if (SPEED(entity) >= 0)
            SPEED(entity) = SPEED(entity) * 11 / 12;
        else
            SPEED(entity) = SPEED(entity) * 15 / 16;
        entity->posX.part16.integer = owner->posX.part16.integer;
        { u16 v_ = owner->posY.part16.integer - 0x26; register u16 *p_ asm("r3") = &entity->posY.part16.integer; *p_ = v_; } /* FAKEMATCH: pinned register */
        if ((t = entity->timer) == 0)
            goto idle;
    count:
        entity->timer = t - 1;
        goto tail;
    case 4:
        if (DeathWeapon_UpdateWallTraversal(entity, owner))
        {
            { u16 f_ = FLAGS(owner); register u32 m_ asm("r1") = 0xFFEF; m_ &= f_; FLAGS(owner) = m_; } /* FAKEMATCH: pinned register */
            goto idle;
        }
        goto tail;
    idle:
        entity->state = 3;
        goto tail;
    case 9:
        SPIN(entity) += 0x790;
        EntityAccelerateAndMove(entity, 0x80000);
        if (EntityIsOutsideScreen(entity, 3))
        {
            entity->state = 10;
            Player_SetSelectedTarget(NULL);
        }
        goto tail;
    case 8:
    default:
    tail:
    if ((zero = entity->ext.deathWeapon.mirror) == 0)
    {
        ObjAffine_Set(SPIN(entity), 0x10000, 0x10000, entity->flipX, zero, entity->affineIndex);
    }
    else
    {
        owner = &gEwramData->entityData[entity->ext.deathWeapon.primaryIndex];
        if (owner->state == 10)
        {
    case 10:
            ObjAffine_Free(entity->affineIndex);
        remove:
            EntityDelete(entity);
            return;
        }
        entity->hidden = owner->hidden;
        entity->palette = owner->palette;
        { u8 v_ = owner->spriteLayer; u8 *p_ = &entity->spriteLayer; zero = 0; *p_ = v_; }
        entity->flipX = owner->flipX;
        entity->posX.whole = owner->posX.whole;
        entity->posY.whole = owner->posY.whole;
        SPIN(entity) = -0x8000 + SPIN(owner);
        ObjAffine_Set(SPIN(entity), 0x10000, 0x10000, entity->flipX, zero, entity->affineIndex);
    }
    }
    if (entity->hidden)
        return;
    entity->subState++;
    if (entity->subState & 1)
    {
        struct EwramData_EntityData *fx;
        s32 x, y;
        s32 a;
        a = (s16)(SPIN(entity) - 0x4000);
        x = Sin(a + 0x4000);
        y = 0x80000;
        do { do { x = x * 48 + RandomSigned(y); } while (0); } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
        do { do { y = Sin(a) * 48 + RandomSigned(y); } while (0); } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
        fx = Effect_SpawnShared(entity->posX.whole + x, entity->posY.whole + y, 0, 6);
        if (fx != NULL)
            fx->accelY.whole = -0x800;
    }
    {
        s32 ang = (s16)(-0x4000 + SPIN(entity));
        s32 q = 0x4000;
        dx = Sin(ang + q) * 24;
        dy = Sin(ang) * 24;
        {
            u16 a = SPIN(entity);
            if (entity->flipX)
                a = (a << 16) + 0x80000000 >> 16;
            angle = (s16)a;
        }
        dx += Sin(angle + q) << 4;
        dy += Sin(angle) << 4;
    }
    HitBox_SetAttackRaw(entity, ((TO_PIXELS(dx) - 16) & 0xFF) + (((TO_PIXELS(dy) - 16) & 0xFF) << 8) + 0x20200000);
    Hit_CheckWithPlayer(&result, entity);
}

/* Rotating one-shot effect. */

/**
 * @brief BCECC | Affine variant of DeathBlinkEffect_Update's effect (same graphics,
 * palette 0): allocates a matrix (affineIndex, affine) or is deleted,
 * priority 0x14, animation 0xC and a 10-frame counter; at 8 objMode = 1,
 * at 4 drawSkip = 1, at 0 (or state 2) the matrix is freed and the entity
 * deleted.  While alive the matrix rotates by (copied from the weapon; scale
 * 1.0, mirrored with flipX)->ext.deathWeapon.angle.
 */
void DeathWeaponTrail_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
    case 0:
        if (Entity_LoadGraphics(entity, sDeathGfx, sDeathPalette, 0, sDeathSpriteSheet))
        {
            u16 matrix = ObjAffine_Alloc(1);

            if ((s16)matrix >= 0)
            {
                entity->affineIndex = matrix;
                entity->affine = 1;
                entity->spriteLayer = 0x14;
                Anim_StartIndexed(entity, 0xC, 2, 0);
                Anim_Update(entity);
                entity->timer = 10;
                entity->state = 1;
                goto active;
            }
        }
        goto delete;
    case 1:
    active:
        if (entity->timer == 8)
            entity->objMode = 1;
        if (entity->timer == 4)
            entity->drawSkip = 1;
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
    case 2:
        ObjAffine_Free(entity->affineIndex);
    delete:
        EntityDelete(entity);
        return;
    }
    ObjAffine_Set(entity->ext.deathWeapon.angle, 0x10000, 0x10000, entity->flipX, 0, entity->affineIndex);
}

/* Short blinking effect over an owner slot. */
/**
 * @brief BCFC8 | One-shot effect drawn just above its owner (entity slot
 * unk_4F8, priority + 1).  State 0 loads graphics sDeathGfx /
 * sDeathPalette / sDeathSpriteSheet (palette 2), plays animation subState and
 * starts a 12-frame counter (timer) with drawSkip = 1; at 8 it sets
 * objMode = 1 and clears drawSkip, at 4 sets drawSkip again, and at 0 (or
 * in state 2) the entity is deleted.
 */
void DeathBlinkEffect_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = &gEwramData->entityData[(u8)entity->ext.deathHelper.ownerIndex];

    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sDeathGfx, sDeathPalette, 2, sDeathSpriteSheet))
        {
            EntityDelete(entity);
            return;
        }
        entity->spriteLayer = owner->spriteLayer + 1;
        Anim_StartIndexed(entity, entity->subState, 2, 0);
        Anim_Update(entity);
        entity->drawSkip = 1;
        entity->timer = 12;
        entity->state = 1;
    case 1:
        if (entity->timer == 8)
        {
            entity->objMode = 1;
            entity->drawSkip = 0;
        }
        if (entity->timer == 4)
            entity->drawSkip = 1;
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
    case 2:
        EntityDelete(entity);
        break;
    }
}

/* Screen-fixed effect: wait for the owner's flag, then rise and leave a trail
 * of DeathBlinkEffect_Update sparks before deleting itself. */

/* The spark pointer is copied to r1 for the position writes, as in the
 * original allocation; this does not establish the original source form. */
void DeathIntroEffect_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = &gEwramData->entityData[(u8)entity->ext.deathHelper.ownerIndex];

    switch (entity->state)
    {
        case 0:
            if (!Entity_LoadGraphics(entity, sDeathGfx, sDeathPalette, 2, sDeathSpriteSheet))
                goto delete;
            entity->spriteLayer = 0x19;
            Anim_StartIndexed(entity, 0x18, 2, 1);
            entity->scrollWithBg = 0;
            entity->velX.whole = 0x800000;
            entity->velY.whole = 0x600000;
            entity->state = 1;
            /* Fall through. */
        case 1:
            {
                struct BgLayerState *bg = &gEwramData->bgInfo[1];
                entity->posX.whole = entity->velX.whole - bg->xPos.whole / 2;
                entity->posY.whole = entity->velY.whole - bg->yPos.whole / 2;
            }
            if (owner->ext.death.flags & 0x20)
            {
                entity->timer = 0x30;
                entity->state = 2;
            }
            break;
        case 2:
            {
                struct BgLayerState *bg = &gEwramData->bgInfo[1];
                entity->posX.whole = entity->velX.whole - bg->xPos.whole / 2;
                entity->posY.whole = entity->velY.whole - bg->yPos.whole / 2;
            }
            if (entity->timer == 0)
            {
                entity->state = 3;
                entity->timer = 0x20;
                entity->velY.whole = -0x10000;
                entity->accelY.whole = -0x2000;
                break;
            }
            entity->timer--;
            break;
        case 3:
            EntityAccelerateAndMoveY(entity, 0x80000);
            if (!(entity->timer & 3))
            {
                struct EwramData_EntityData *spark = Entity_CreateObject(DeathBlinkEffect_Update);
                if (spark)
                {
                    spark->ext.deathHelper.ownerIndex = ENTITY_INDEX(entity);
                    spark->scrollWithBg = 1;
                    {
                        register struct EwramData_EntityData *placed asm("r1") = spark; /* FAKEMATCH: pinned register */
                        placed->posX.whole = entity->posX.whole;
                        placed->posY.whole = entity->posY.whole;
                        placed->subState = 0x18;
                    }
                }
            }
            if (entity->timer == 0)
            {
            delete:
                EntityDelete(entity);
                return;
            }
            entity->timer--;
            break;
    }
    Anim_Update(entity);
}

/* OBJ-window sprite bound to an owner slot. */

/**
 * @brief BD20C | Sprite used as OBJ window, owned by entity slot unk_4F8.
 * Keeps DISPCNT bit 15 set every frame.  State 0 sets WINOUT to 0x2F3F,
 * loads the graphics (sCommonObjectGfx / sCommonObjectSpriteSheet, palette 6), priority
 * 0x12, frame 0x3F, draw callback Sprite_DrawEntity, objMode = 2 and places
 * itself at the owner + (0x20, 0x18).  It is deleted once the owner's
 * state exceeds 2.
 */
void DeathFloatingWeaponWindow_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = &gEwramData->entityData[(u8)entity->ext.deathHelper.ownerIndex];
    s32 slot;

    gUnk_03002CB0.dispCnt |= 0x8000; /* OBJ window on */
    switch (entity->state)
    {
    case 0:
        gDisplayRegisters.winOut_L = 0x3F;
        gDisplayRegisters.winOut_H = 0x2F;
        slot = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, slot, 6);
        entity->spriteLayer = 0x12;
        entity->animFrame = 0x3F;
        entity->drawFunc = Sprite_DrawEntity;
        entity->objMode = 2;
        entity->posX.part16.integer = owner->posX.part16.integer + 0x20;
        entity->posY.part16.integer = owner->posY.part16.integer + 0x18;
        entity->state = 1;
    case 1:
        if (owner->state > 2)
            EntityDelete(entity);
        break;
    }
}

/* Hitbox entity bound to an owner slot. */

/**
 * @brief BD2D0 | Attached hitbox: the owner is entity slot unk_4F8 (byte).
 * Deleted as soon as the owner's HP (hp) drops to 0.  State 0 registers
 * box slot 8 with callback DeathFloatingWeaponHitBox_OnHit (Hit_Enable) and sets bit 1 of
 * flags; state 1 follows the owner's position, copies its current box
 * (Hit_GetEntityBoxes) with HitBox_SetBodyRaw and runs the collision check
 * Hit_CheckWithPlayerSharedHistory against the owner.
 */
void DeathFloatingWeaponHitBox_Update(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    struct EwramData_EntityData *owner = &gEwramData->entityData[(u8)entity->ext.deathHelper.ownerIndex];

    if ((s16)owner->hp <= 0)
    {
        EntityDelete(entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
        Hit_Enable(entity, 8, NULL, DeathFloatingWeaponHitBox_OnHit);
        entity->hit.flags |= 2;
        entity->state++;
        break;
    case 1:
        entity->posX.whole = owner->posX.whole;
        entity->posY.whole = owner->posY.whole;
        HitBox_SetBodyRaw(entity, *(s32 *)Hit_GetEntityBoxes(owner));
        Hit_CheckWithPlayerSharedHistory(&result, entity, owner);
        break;
    }
}

/* DeathFloatingWeapon_Update: matched (case 4 posY value pinned to r2 keeps r3 out of the spill
   register set; operand order of mul/and/or as in the target). */

#define MOVE_TO_A(x0, y0, tx, ty, shift, ANGLE)                                                         \
    {                                                                                    \
        u32 dist;                                                                        \
        u16 angle;                                                                       \
        x0 = entity->posX.whole;                                                         \
        y0 = entity->posY.whole;                                                         \
        dist = Sqrt(({ s32 d = (x0 - (tx)) >> 16; d * d; }) + ({ s32 d = (y0 - (ty)) >> 16; d * d; })) << 16; \
        angle = ANGLE(tx, ty); \
        dist >>= (shift);                                                                \
        Entity_AccelerateAlongAngle(entity, angle, dist, 0, 0, 0);                                      \
        entity->velX.whole = entity->posX.whole - x0;                                    \
        entity->velY.whole = entity->posY.whole - y0;                                    \
    }

#define MOVE_TO(x0, y0, tx, ty, shift) MOVE_TO_A(x0, y0, tx, ty, shift, ANGLE)
#define DIV16(x) ((x) < 0 ? -(-(x) >> 16) : (x) >> 16)
#define BOX(x, y) (((DIV16(x) - 8) & 0xFF) + (((DIV16(y) - 8) & 0xFF) << 8) + 0x10100000)

#define SPAWN(fn)                                                                        \
    {                                                                                    \
        struct EwramData_EntityData *child = Entity_CreateObject(fn);                           \
        if (child != NULL)                                                               \
        {                                                                                \
            child->ext.deathHelper.ownerIndex = ENTITY_INDEX(entity);        \
            child->scrollWithBg = 1;                                                        \
        }                                                                                \
    }

#define ANGLE(tx, ty) ArcTan2(DIV16(tx) - (s16)entity->posX.part16.integer, DIV16(ty) - ({ s32 py = (s16)entity->posY.part16.integer; py; }))
/* Case 4 copy: posY value pinned to r2, so dx takes r3 and the ldrsh scratch is r1; otherwise r3
   joins the function-wide spill register set and the reload rotation drifts. */
#define ANGLE3(tx, ty) ArcTan2(DIV16(tx) - (s16)entity->posX.part16.integer, DIV16(ty) - ({ register s32 py asm("r2") = (s16)entity->posY.part16.integer; /* FAKEMATCH: posY value in r2 */ asm("" : "+r"(py)); /* FAKEMATCH */ py; }))
/**
 * @brief Create the destructible affine weapon with an OBJ window and
 * shared-history hitbox. Follow the boss, spin and approach the player
 * from owner flags; on defeat notify the boss and remove with effects.
 * Preserve the pinned-register coordinate helpers and expression ordering.
 */
void DeathFloatingWeapon_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = &gEwramData->entityData[(u8)entity->ext.deathHelper.ownerIndex];
    struct HitResult res;
    s32 b;
    s32 sx, sy;

    switch (entity->state)
    {
    case 0:
    {
        u16 slot;
        if (!Entity_LoadGraphics(entity, sDeathGfx, sDeathPalette, 0, sDeathSpriteSheet))
            goto delete;
        slot = ObjAffine_Alloc(1);
        if ((s16)slot < 0)
            goto delete;
        entity->enemyId = parent->enemyId;
        Enemy_ResetFromTemplate(entity);
        entity->hp = 0x7D0;
        Hit_Enable(entity, 8, Enemy_OnContactMask1, DeathFloatingWeapon_OnHit);
        entity->affineIndex = slot;
        entity->affine = 1;
        entity->spriteLayer = 0x12;
        Anim_StartIndexed(entity, 0xC, 2, 0);
        Anim_Update(entity);
        if (parent->spawnParam0 != 0)
        {
            struct BgLayerState *bg = &gEwramData->bgInfo[1];
            entity->posX.part16.integer = 0xE8 - bg->xPos.part16.integer;
            entity->posY.part16.integer = 0x64 - bg->yPos.part16.integer;
            entity->state = 1;
            entity->ext.deathWeapon.angle = 0x1000;
            SPAWN(DeathFloatingWeaponWindow_Update);
            entity->hit.flags |= 4;
        }
        else
        {
            entity->posX.whole = parent->posX.whole;
            entity->posY.whole = parent->posY.whole;
            entity->state = 4;
        }
        SPAWN(DeathFloatingWeaponHitBox_Update);
        break;
    }
    case 1:
        if (parent->ext.death.flags & 0x20)
        {
            entity->state = 2;
            entity->ext.deathFloatingWeapon.timer = 0x20;
            PlaySong(0x124);
        }
        break;
    case 2:
    {
        s32 x0, y0;
        s32 tx = parent->posX.whole + 0x500000;
        MOVE_TO(x0, y0, tx, 0x200000, 5);
        entity->ext.deathWeapon.angle -= 0x66;
        if (entity->ext.deathFloatingWeapon.timer != 0)
        {
            entity->ext.deathFloatingWeapon.timer--;
            break;
        }
        entity->state = 3;
        entity->ext.deathFloatingWeapon.timer = 0x60;
        break;
    }
    case 3:
    {
        s32 x0, y0;
        s32 tx = parent->posX.whole + 0x200000;
        MOVE_TO(x0, y0, tx, 0x300000, 4);
        entity->ext.deathWeapon.angle -= 0x616;
        if (entity->ext.deathFloatingWeapon.timer != 0)
        {
            entity->ext.deathFloatingWeapon.timer--;
            break;
        }
        entity->hit.flags &= ~4;
        entity->state = 4;
        break;
    }
    case 4:
    {

        s32 off, y0;
        s32 tx, ty;
        if (!(parent->ext.death.flags & 0x40) && (s16)entity->hp <= 0x4AF)
            parent->ext.death.flags |= 0x40;
        off = 0x200000;
        {
            s32 px = gEwramData->playerState.player->posX.whole;
            if (entity->posX.whole < px)
                off = -off;
        }
        tx = parent->posX.whole + off;
        ty = parent->posY.whole - 0x200000;
        MOVE_TO_A(off, y0, tx, ty, 4, ANGLE3);
        entity->ext.deathWeapon.angle += (entity->velX.whole + entity->velY.whole) / 256;
        if (parent->ext.death.flags & 4)
        {
            entity->state = 5;
            entity->ext.deathFloatingWeapon.timer = 0x40;
        }
        break;
    }
    case 5:
        entity->ext.deathWeapon.angle += 0x789;
        if (!(entity->timer & 0x1F))
            PlaySong(0x135);
        if (entity->ext.deathFloatingWeapon.timer != 0)
        {
            entity->ext.deathFloatingWeapon.timer--;
            break;
        }
        entity->ext.deathFloatingWeapon.timer = 0x80;
        entity->state = 6;
        break;
    case 6:
    {
        struct EwramData_EntityData *player = gEwramData->playerState.player;
        s32 tx = player->posX.whole;
        s32 ty = player->posY.whole;
        s32 x0, y0;
        MOVE_TO(x0, y0, tx, ty, 7);
        entity->ext.deathWeapon.angle += 0x789;
        if (!(entity->timer & 0x1F))
            PlaySong(0x135);
        if (entity->ext.deathFloatingWeapon.timer != 0)
        {
            entity->ext.deathFloatingWeapon.timer--;
            break;
        }
        entity->state = 4;
        break;
    }
    case 7:
    {
        u8 t = entity->timer;
        if (t & 1)
        {
            u16 a = entity->ext.deathWeapon.angle - 0x4000;
            if (({ s32 m_ = 2; m_ &= t; m_; }))
                a = (u32)((a << 16) + 0x80000000) >> 16;
            sx = (entity->timer >> 1) * Sin((s16)a + 0x4000);
            sy = (entity->timer >> 1) * Sin((s16)a);
            Effect_SpawnRandom(entity->posX.whole + sx, entity->posY.whole + sy, 0x40000, 0x40000, entity->spriteLayer);
        }
        if (entity->timer <= 0x2F)
        {
            entity->timer++;
            break;
        }
    }
    case 8:
        ObjAffine_Free(entity->affineIndex);
    delete:
        EntityDelete(entity);
        return;
    }

    if ((u8)(entity->state - 2) <= 4)
    {
        if (++entity->subState == 3)
        {
            struct EwramData_EntityData *child;
            entity->subState = 0;
            child = Entity_CreateObject(DeathWeaponTrail_Update);
            if (child != NULL)
            {
                child->ext.deathHelper.ownerIndex = ENTITY_INDEX(entity);
                child->scrollWithBg = 1;
                { register struct EwramData_EntityData *c asm("r1") = child; /* FAKEMATCH: copy of the new child in r1 */
                c->ext.deathWeapon.angle = entity->ext.deathWeapon.angle;
                c->posX.whole = entity->posX.whole;
                c->posY.whole = entity->posY.whole; }
            }
        }
    }
    ObjAffine_Set(entity->ext.deathWeapon.angle, 0x10000, 0x10000, entity->flipX, 0, entity->affineIndex);
    if (entity->state > 6)
        return;
    {
        s32 a = (s16)(entity->ext.deathWeapon.angle - 0x4000);
        sx = ((entity->timer & 3) * 16 - 0x20) * Sin(a + 0x4000);
        sy = ((entity->timer & 3) * 16 - 0x20) * Sin(a);
        HitBox_SetBodyRaw(entity, BOX(sx, sy));
    }
    {
        s32 a = (s16)(entity->ext.deathWeapon.angle - 0x4000);
        sx = Sin(a + 0x4000) * 24;
        sy = Sin(a) * 24;
        b = entity->ext.deathWeapon.angle;
        sx += (entity->timer & 3) * (Sin(b + 0x4000) << 3);
        sy += (entity->timer & 3) * (Sin(b) << 3);
        HitBox_SetAttackRaw(entity, BOX(sx, sy));
    }
    entity->timer++;
    if (!(entity->enemyFlags & 2))
    {
        Enemy_UpdateHitFlash(entity);
        Hit_CheckWithPlayer(&res, entity);
        return;
    }
    PlaySong(0x6E);
    parent->ext.death.flags |= 1;
    entity->state = 7;
    entity->timer = 0;
}

/* DeathBlade_Update, decompiled from the original assembly. The player position
 * is read through s32 statement expressions so it stays ldrsh, and dx is
 * narrowed to s16 before dy is computed. */

/* Spinning blade appearing at a random spot in the upper screen and flying at
 * the player, accelerating, until it is hit or its parent finishes. */
void DeathBlade_Update(struct EwramData_EntityData *entity)
{
    struct EwramData **ew = &gEwramData;
    struct EwramData_EntityData *parent = &(*ew)->entityData[(u8)entity->ext.deathHelper.ownerIndex];
    struct HitResult res;
    u16 slot;
    s16 angle;

    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, (void *)sDeathGfx, (void *)sDeathPalette, 0, (void *)sDeathSpriteSheet))
            goto delete;
        slot = ObjAffine_Alloc(1);
        if ((s16)slot < 0)
            goto delete;
        entity->attack = parent->attack * 7 / 10;
        entity->hp = 1;
        Hit_Enable(entity, 10, Enemy_OnContactMask1, DeathBlade_OnHit);
        HitBox_SetBodyRaw(entity, 0x1010F8F8);
        HitBox_SetAttackRaw(entity, 0x1010F8F8);
        entity->hit.flags |= 2;
        entity->affineIndex = slot;
        entity->affine = 1;
        entity->spriteLayer = 0x11;
        Anim_StartIndexed(entity, 0x10, 2, 0);
    {
        struct EwramData_EntityData *player;
        s32 y;
        s32 mask;
        { u32 r = RandomNumberGenerator(); mask = 0x7F; entity->posX.part16.integer = (r & mask) + 0x40; }
        y = (RandomNumberGenerator() & mask) + 0x10;
        entity->posY.part16.integer = y;
        player = gEwramData->playerState.player;
        {
            s16 dx = ({ s32 px = (s16)player->posX.part16.integer; px; }) - entity->posX.part16.integer;
            entity->ext.deathBlade.heading = ArcTan2(dx, ({ s32 py = (s16)player->posY.part16.integer; py; }) - y - 0x10);
        }
    }
        entity->flipX = RandomNumberGenerator() & 1;
        entity->state = 1;
    case 1:
        if (entity->animFlags & 4)
        {
            entity->state = 2;
            entity->hit.flags &= ~2;
        }
        Hit_CheckWithPlayer(&res, entity);
        if ((s16)entity->hp != 0)
            break;
        entity->state = 3;
        break;
    case 2:
        if (entity->ext.deathBlade.speed <= 0x17FFF)
            entity->ext.deathBlade.speed += 0x800;
        {
            s32 s = Sin(entity->ext.deathBlade.heading + 0x4000);
            s32 r = entity->ext.deathBlade.speed;
            entity->velX.whole = (s >> 8) * (r >> 8);
        }
        {
            s32 s = Sin(entity->ext.deathBlade.heading);
            s32 r = entity->ext.deathBlade.speed;
            entity->velY.whole = (s >> 8) * (r >> 8);
        }
        EntityAccelerateAndMove(entity, 0x80000);
        Hit_CheckWithPlayer(&res, entity);
        if ((s16)entity->hp == 0)
            entity->state = 3;
        if (parent->ext.death.flags & 1)
            entity->state = 3;
        break;
    case 3:
        Effect_SpawnAt(entity->posX.whole, entity->posY.whole, entity->spriteLayer);
    case 4:
        ObjAffine_Free(entity->affineIndex);
    delete:
        EntityDelete(entity);
        return;
    }
    if (EntityIsOutsideScreen(entity, 3))
        entity->state = 4;
    entity->ext.deathBlade.spin += 0x7A0;
    angle = entity->ext.deathBlade.spin;
    if (entity->flipX)
        angle = -angle;
    ObjAffine_Set((s16)angle, 0x10000, 0x10000, entity->flipX, 0, entity->affineIndex);
    Anim_Update(entity);
}

/* Pillar rising under the player (x of the player, y 0x90): grows, then
 * shrinks along a sine while throwing debris; hits during frames 15..17. */
void DeathPillar_Update(struct EwramData_EntityData *entity)
{
    struct EwramData **ew = &gEwramData;
    struct EwramData_EntityData *parent = &(*ew)->entityData[(u8)entity->ext.deathHelper.ownerIndex];
    struct HitResult res;
    u16 slot;

    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, (void *)sDeathGfx, (void *)sDeathPalette, 2, (void *)sDeathSpriteSheet))
            goto delete;
        slot = ObjAffine_Alloc(1);
        if ((s16)slot < 0)
            goto delete;
        entity->posX.whole = (*ew)->playerState.player->posX.whole;
        entity->posY.part16.integer = 0x90;
        entity->attack = parent->attack + (parent->attack >> 2);
        entity->hp = 1;
        entity->affineIndex = slot;
        entity->affine = 1;
        entity->spriteLayer = 0x11;
        Anim_StartIndexed(entity, 0xD, 2, 0);
        Anim_Update(entity);
        entity->ext.deathPillar.scaleX = 0x1000;
        Hit_Enable(entity, 8, DeathPillar_DamagePlayerOnContact, 0);
        HitBox_SetAttackRaw(entity, 0x800880FC);
        entity->hit.flags |= 4;
        PlaySong(0x11F);
        entity->state = 1;
    case 1:
        ObjAffine_Set(0, entity->ext.deathPillar.scaleX, entity->ext.deathPillar.scaleY, entity->flipX, 0, entity->affineIndex);
        if (entity->ext.deathPillar.scaleY <= 0xFFFF)
        {
            entity->ext.deathPillar.scaleY += 0x1000;
            break;
        }
        entity->timer = 0x20;
        entity->state = 2;
        break;
    case 2:
        entity->ext.deathPillar.scaleX = Sin(entity->timer << 10);
        ObjAffine_Set(0, entity->ext.deathPillar.scaleX, 0x10000, entity->flipX, 0, entity->affineIndex);
        if (!(entity->timer & 7))
        {
            s32 x = entity->posX.whole + RandomSigned(0x100000);
            s32 vx = -0x800 - (RandomNumberGenerator() & 0xFFF);
            struct EwramData_EntityData *debris = Effect_CreateFromTable(x, entity->posY.whole - 0x40000, 0, vx, 4, 6);
            if (debris != NULL)
                debris->velY.whole = -0x2000 - ((RandomNumberGenerator() & 0x1FF) << 8);
        }
        if (entity->timer == 0)
            goto destroy;
        if ((u8)(entity->timer - 0xF) <= 2)
            Hit_CheckWithPlayer(&res, entity);
        entity->timer--;
        break;
    case 3:
    destroy:
        ObjAffine_Free(entity->affineIndex);
    delete:
        EntityDelete(entity);
        return;
    }
    if (EntityIsOutsideScreen(entity, 3))
        entity->state = 3;
}

static inline s32 FixedToInt(s32 v)
{
    return (v < 0) ? -(-v >> 16) : (v >> 16);
}

/* Growing sparkle at the screen centre: grows, waits, drops an item
 * (BossRecoveryPickup_Spawn) and shrinks away, scattering debris every other frame. */
/**
 * @brief Grow and rotate an affine reward effect at room point (256,128).
 * After the countdown spawn the enemy-definition soul pickup and call
 * BossRecoveryPickup_Spawn, then shrink and release the affine slot; emit debris.
 */
void DeathRewardEffect_Update(struct EwramData_EntityData *entity)
{
    struct EwramData **ew = &gEwramData; /* address loaded before the switch */
    u16 slot;

    switch (entity->state)
    {
    case 0:
        SharedObjPalette_Release((void *)sDeathPalette, 2);
        if (!Entity_LoadGraphics(entity, (void *)sDeathGfx, (void *)sDeathPalette, 3, (void *)sDeathSpriteSheet))
            goto delete;
        slot = ObjAffine_Alloc(1);
        if ((s16)slot < 0)
            goto delete;
    {
        struct BgLayerState *camera = &(*ew)->bgInfo[1];
        entity->posX.part16.integer = 0x100 - camera->xPos.part16.integer;
        entity->posY.part16.integer = 0x80 - camera->yPos.part16.integer;
    }
        entity->affineIndex = slot;
        entity->affine = 1;
        entity->spriteLayer = 0x14;
        Anim_StartIndexed(entity, 0xF, 2, 0);
        Anim_Update(entity);
        PlaySong(0x120);
        entity->state = 1;
    case 1:
        if (entity->ext.deathReward.scale <= 0xFFFF)
        {
            entity->ext.deathReward.scale += 0x400;
            break;
        }
        entity->timer = 0x60;
        entity->state = 2;
    case 2:
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        SoulPickup_SpawnFromEnemyDefinition((s16)entity->posX.part16.integer, (s16)entity->posY.part16.integer, 0x6B, 1);
        BossRecoveryPickup_SpawnWide((s16)entity->posX.part16.integer, 0x20, 1, 0x60, 1);
        entity->state = 3;
        break;
    case 3:
        if (entity->ext.deathReward.scale > 0)
        {
            entity->ext.deathReward.scale -= 0x400;
            break;
        }
        ObjAffine_Free(entity->affineIndex);
    delete:
        EntityDelete(entity);
        return;
    }
    if (++entity->ext.deathReward.counter & 1)
    {
        u16 angle = RandomNumberGenerator();
        s32 s, dx, dy;
        struct EwramData_EntityData *debris;
        s = Sin((s16)angle + 0x4000);
        dx = FixedToInt(entity->ext.deathReward.scale << 5) * s;
        s = Sin((s16)angle);
        if ((entity->ext.deathReward.scale << 5) < 0)
            dy = -(-(entity->ext.deathReward.scale << 5) >> 16) * s;
        else
            dy = ((entity->ext.deathReward.scale << 5) >> 16) * s;
        debris = Effect_SpawnShared(entity->posX.whole + dx, entity->posY.whole + dy, 0x10, 6);
        if (debris != NULL)
        {
            s32 r = RandomNumberGenerator();
            s32 one = 1;
            debris->flipX = r & one;
            debris->flipY = RandomNumberGenerator() & one;
            debris->accelX.whole = -dx / 256;
            debris->accelY.whole = -dy / 256;
        }
    }
    entity->ext.deathReward.rotation += 0x1A0;
    ObjAffine_Set(entity->ext.deathReward.rotation, entity->ext.deathReward.scale, entity->ext.deathReward.scale,
                 entity->flipX, 0, entity->affineIndex);
}

/*
 * Death debris (spawned when Death is defeated): it spirals into the room
 * point (0x100, 0x80) while shrinking: sprite rotation and orbit angle (both
 * advanced by the growing spin speed), orbit radius and its change per frame,
 * affine scale and its change per frame. Token-identical expansion.
 */
#define DEBRIS_RADIUS_STEP(e) ENTITY_WORD_510(e)

/* Spinning projectile: flies outwards from the screen point (0x100, 0x80)
 * along a spiral, shrinking until its radius reaches zero. */
/**
 * @brief Animate a delayed affine debris part spiraling inward toward room
 * point (256,128) as scale and radius decrease. Release its slot and
 * remove when the radius reaches zero. This entity has no hit callback.
 */
void DeathDebris_Update(struct EwramData_EntityData *entity)
{
    u16 slot;

    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, (void *)sDeathGfx, (void *)sDeathPalette, 0, (void *)sDeathSpriteSheet))
            goto delete;
        slot = ObjAffine_Alloc(1);
        if ((s16)slot < 0)
            goto delete;
        entity->affineIndex = slot;
        entity->affine = 1;
        entity->spriteLayer = 0x13;
        Anim_StartIndexed(entity, entity->subState + 0x14, 2, 0);
        Anim_Update(entity);
        entity->ext.deathDebris.orbitAngle = ArcTan2(entity->posX.part16.integer - 0x100, entity->posY.part16.integer - 0x80);
    {
        struct BgLayerState *camera = &gEwramData->bgInfo[1];
        entity->posX.whole -= camera->xPos.whole;
        entity->posY.whole -= camera->yPos.whole;
    }
        entity->ext.deathDebris.radius = Entity_GetRoomPointDistance(entity, 0x1000000, 0x800000);
        entity->ext.deathDebris.scale = 0x10000;
        entity->ext.deathDebris.scaleStep = -0x155;
        DEBRIS_RADIUS_STEP(entity) = -(entity->ext.deathDebris.radius / 0xC0);
        entity->state = 1;
    case 1:
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        entity->state = 2;
    case 2:
        if (entity->ext.deathDebris.spin <= 0x5FF)
            entity->ext.deathDebris.spin += 0x10;
        entity->ext.deathDebris.rotation += entity->ext.deathDebris.spin;
        entity->ext.deathDebris.orbitAngle += entity->ext.deathDebris.spin;
        entity->ext.deathDebris.scale += entity->ext.deathDebris.scaleStep;
        if (entity->ext.deathDebris.scale < 0)
            entity->ext.deathDebris.scale = 0;
        if (entity->ext.deathDebris.radius > 0)
        {
            entity->ext.deathDebris.radius += DEBRIS_RADIUS_STEP(entity);
            break;
        }
    case 3:
        ObjAffine_Free(entity->affineIndex);
    delete:
        EntityDelete(entity);
        return;
    }
    {
        s32 s = Sin(entity->ext.deathDebris.orbitAngle + 0x4000);
        s32 r = entity->ext.deathDebris.radius;
        entity->velX.whole = (s >> 8) * (r >> 8);
    }
    {
        s32 s = Sin(entity->ext.deathDebris.orbitAngle);
        s32 r = entity->ext.deathDebris.radius;
        entity->velY.whole = (s >> 8) * (r >> 8);
    }
    {
        struct BgLayerState *camera;
        u16 *velXInt = &entity->velX.part16.integer;
        camera = &gEwramData->bgInfo[1];
        entity->posX.part16.integer = *velXInt - (u16)(camera->xPos.part16.integer - 0x100);
        entity->posY.part16.integer = entity->velY.part16.integer - (s16)(camera->yPos.part16.integer - 0x80);
    }
    ObjAffine_Set(entity->ext.deathDebris.rotation, entity->ext.deathDebris.scale, entity->ext.deathDebris.scale,
                 entity->flipX, 0, entity->affineIndex);
}

/* Move towards (tx, ty) (16.16) at a speed proportional to the distance;
 * velX/velY get the step taken. Returns the speed. */
static inline s32 MoveToward(struct EwramData_EntityData *entity, s32 tx, s32 ty)
{
    s32 x = entity->posX.whole;
    s32 y = entity->posY.whole;
    u32 dist = Sqrt(((x - tx) >> 16) * ((x - tx) >> 16) + ((y - ty) >> 16) * ((y - ty) >> 16)) << 16;
    u16 angle = ArcTan2(((tx < 0) ? -(-tx >> 16) : (tx >> 16)) - (s16)entity->posX.part16.integer,
                        ((ty < 0) ? -(-ty >> 16) : (ty >> 16)) - entity->posY.part16.integer);
    dist >>= 4;
    Entity_AccelerateAlongAngle(entity, angle, dist, 0, 0, 0);
    entity->velX.whole = entity->posX.whole - x;
    entity->velY.whole = entity->posY.whole - y;
    return dist;
}

/* Cutscene step: wait until the player walks left of x 0xC0 on screen, then
 * spawn two effects and pull the entity towards a point above the player. */
/**
 * @brief Intro state (0): gate menus and supply player input, wait for
 * the room-position threshold, signal introductory children, then move
 * above the player and enter the first phase.
 */
void Death_UpdateIntro(struct EwramData_EntityData *entity)
{
    struct EwramData *ew;
    switch (entity->subState)
    {
    case 0:
        if (gEwramData->gameState.stateFlags & 1)
            break;
        entity->subState = 1;
    case 1:
        gEwramData->gameState.stateFlags |= 1;
        Game_SetMenusEnabled(0);
        if ((s16)(ew = gEwramData)->playerState.player->posX.part16.integer + (s16)((struct BgLayerState *)((u8 *)ew + 0xA094))->xPos.part16.integer > 0xC0)
            break;
        entity->ext.death.flags |= 0x20;
        entity->ext.deathFloatingWeapon.timer = 0x60;
        entity->subState = 2;
        break;
    case 2:
        if (entity->ext.deathFloatingWeapon.timer != 0)
        {
            entity->ext.deathFloatingWeapon.timer--;
            break;
        }
        entity->ext.deathFloatingWeapon.timer = 0x30;
        entity->subState = 3;
        Afterimage_Create(entity, 0, 0x10, 0x31);
        Afterimage_Create(entity, 0, 8, 0x30);
        PlaySong(0x121);
        Music_SetCurrent(0x10);
        gEwramData->gameState.stateFlags &= ~1;
        Game_SetMenusEnabled(1);
        break;
    case 3:
    {
        MoveToward(entity, entity->posX.whole, gEwramData->playerState.player->posY.whole - 0x280000);
        if (entity->ext.deathFloatingWeapon.timer != 0)
        {
            entity->ext.deathFloatingWeapon.timer--;
            break;
        }
        entity->state = 1;
        entity->stage = 0;
        entity->subState = 0;
        break;
    }
    }
    if ((u8)(entity->subState - 1) <= 1)
    {
        s32 input = 0;
        if ((s16)(ew = gEwramData)->playerState.player->posX.part16.integer + (s16)((struct BgLayerState *)((u8 *)ew + 0xA094))->xPos.part16.integer > 0x7F)
            input = 0x20;
        SetPlayerInput(2, input);
    }
}

/* Death_UpdateFirstPhase, decompiled from the original assembly. The second player
 * read goes through the inline Player() (base kept, offset constant reloaded
 * as in the target); case 3 uses its own block variable for the radius. */

static inline void SpawnChild(struct EwramData_EntityData *e, void *func)
{
    struct EwramData_EntityData *child = Entity_CreateObject(func);
    if (child != NULL)
    {
        child->ext.deathHelper.ownerIndex = ENTITY_INDEX(e);
        child->scrollWithBg = 1;
    }
}

static inline struct EwramData_EntityData *Player(void) { return gEwramData->playerState.player; }

/**
 * @brief First combat phase (1): hover near the player, command the
 * floating weapon, summon blades and optionally pillars. Enter the
 * weapon transition when flag 1 reports the floating weapon defeated.
 */
void Death_UpdateFirstPhase(struct EwramData_EntityData *e)
{
    s32 r, x, vx, vy;

    switch (e->subState)
    {
    case 0:
        e->subState = 1;
    case 1:
        vy = gEwramData->playerState.player->posY.whole - 0x300000;
        if (vy <= 0x3FFFFF)
            vy = 0x400000;
        e->posY.whole += Sin(e->timer << 7);
        {
            s32 px = Player()->posX.whole;
            r = 0x400;
            if (px < e->posX.whole - 0x400000)
                e->accelX.whole = -0x400;
            else if (px > e->posX.whole + 0x400000)
                e->accelX.whole = r;
            else
            {
                e->accelX.whole = 0;
                EntityDampVelocityX(e, 0x28);
            }
            if (vy < e->posY.whole - 0x100000)
                e->accelY.whole = -0x400;
            else if (vy > e->posY.whole + 0x100000)
                e->accelY.whole = r;
            else
            {
                e->accelY.whole = 0;
                EntityDampVelocityY(e, 0x28);
            }
        }
        EntityAccelerateAndMove(e, 0x10000);
        if ((++e->ext.deathFloatingWeapon.timer & 0xFF) != 0)
            break;
        if ((e->ext.death.flags & 0x40) && (RandomNumberGenerator() & 7))
        {
            e->subState = 2;
            Anim_StartIndexed(e, 3, 2, 0);
            break;
        }
        e->subState = 7;
        e->stage = 0;
        Anim_StartIndexed(e, sDeathFirstPhaseAnims[0], 2, 0);
        e->ext.death.flags |= 4;
        break;
    case 2:
        e->accelX.whole = 0;
        e->accelY.whole = 0;
        EntityDampVelocityX(e, 0x28);
        EntityDampVelocityY(e, 0x28);
        EntityAccelerateAndMove(e, 0x10000);
        if (!(e->animFlags & 4))
            break;
        e->ext.deathFloatingWeapon.timer = 0x40;
        PlaySong(0x111);
        e->subState = 3;
        Anim_StartIndexed(e, 4, 2, 1);
        break;
    case 3:
    {
        s16 angle = RandomNumberGenerator();
        s32 rr = RandomNumberGenerator();
        do { rr &= 0xF; } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
        do { rr += 0x20; } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
        vx = rr * Sin(angle + 0x4000);
        vy = rr * Sin(angle);
        x = (((s16)e->posX.part16.integer - 0xC) << 16) + vx;
        { s32 y = (((s16)e->posY.part16.integer - 0x30) << 16) + vy;
        Effect_CreateFromTable(x, y, -vx / 0xC0, -vy / 0xC0, 4, 6); }
        if (e->ext.deathFloatingWeapon.timer == 0)
        {
            e->subState = 4;
            Anim_StartIndexed(e, 5, 2, 0);
            break;
        }
        e->ext.deathFloatingWeapon.timer--;
        break;
    }
    case 4:
        if (!(e->animFlags & 4))
            break;
        Anim_StartIndexed(e, 0x12, 2, 1);
        e->ext.deathFloatingWeapon.timer = 0x40;
        e->subState = 5;
        break;
    case 5:
        if (!(e->ext.deathFloatingWeapon.timer & 0xF))
            SpawnChild(e, DeathPillar_Update);
        if (e->ext.deathFloatingWeapon.timer != 0)
        {
            e->ext.deathFloatingWeapon.timer--;
            break;
        }
        e->subState = 6;
        Anim_StartIndexed(e, 0x13, 2, 0);
        break;
    case 6:
        if (!(e->animFlags & 4))
            break;
        e->subState = 1;
        Anim_StartIndexed(e, 1, 2, 1);
        break;
    case 7:
        e->accelX.whole = 0;
        e->accelY.whole = 0;
        EntityDampVelocityX(e, 0x28);
        EntityDampVelocityY(e, 0x28);
        EntityAccelerateAndMove(e, 0x10000);
        if (!(e->animFlags & 4))
            break;
        e->stage++;
        if (sDeathFirstPhaseAnims[e->stage] == 0xFF)
        {
            e->subState = 1;
            Anim_StartIndexed(e, 1, 2, 1);
            break;
        }
        Anim_StartIndexed(e, sDeathFirstPhaseAnims[e->stage], 2, 0);
        e->ext.death.flags &= ~4;
        break;
    }
    {
        s32 t = e->timer + 1;
        e->timer = t;
        if (!(t & (gEwramData->gameState.difficultyMode == 0 ? 0x7F : 0x3F)))
            SpawnChild(e, DeathBlade_Update);
    }
    if (e->ext.death.flags & 1)
    {
        e->state = 2;
        e->stage = 0;
        e->subState = 0;
    }
}

/* Death_UpdateWeaponTransition: scythe boss phase - glide to the screen centre, then play the swing animation and spawn two scythes. */

static inline struct EwramData_EntityData *SpawnScythe(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child = Entity_CreateObject(DeathWeapon_Update);
    if (child == NULL)
        return NULL;
    child->ext.deathHelper.ownerIndex = ENTITY_INDEX(entity);
    child->scrollWithBg = 1;
    return child;
}

/**
 * @brief Transition state (2): move toward the center position, animate
 * and create the primary and mirrored weapon instances at animation
 * step 0x30. Enter hover state 3 when the animation finishes.
 * Preserve the existing allocation assumptions in the paired spawn.
 */
void Death_UpdateWeaponTransition(struct EwramData_EntityData *entity)
{
    s32 targetX;
    s32 oldX, oldY;
    u32 dist;
    u16 angle;
    s32 ax;
    s32 ay;
    struct BgLayerState *bg;
    struct EwramData_EntityData *a, *b;

    switch (entity->subState)
    {
        case 0:
            entity->timer = 0x40;
            entity->subState = 1;
        case 1:
            bg = &gEwramData->bgInfo[1];
            targetX = (0xA0 - (s16)bg->xPos.part16.integer) << 16;
            oldX = entity->posX.whole;
            oldY = entity->posY.whole;
            dist = Sqrt(((oldX - targetX) >> 16) * ((oldX - targetX) >> 16)
                        + ((oldY - 0x600000) >> 16) * ((oldY - 0x600000) >> 16)) << 16;
            ax = (s16)((targetX < 0 ? -(-targetX >> 16) : targetX >> 16) - (s16)entity->posX.part16.integer);
            ay = 0x60 - (s16)entity->posY.part16.integer;
            ay <<= 16;
            ay >>= 16;
            angle = ((u16 (*)(s32, s32))ArcTan2)(ax, ay);
            dist >>= 6;
            Entity_AccelerateAlongAngle(entity, angle, dist, 0, 0, 0);
            entity->velX.whole = entity->posX.whole - oldX;
            entity->velY.whole = entity->posY.whole - oldY;
            if (entity->timer != 0)
            {
                entity->timer--;
                break;
            }
            entity->objMode = 0;
            Anim_StartIndexed(entity, 6, 2, 0);
            PlaySong(0x121);
            entity->subState = 2;
            break;
        case 2:
            {
            s16 mask = -1;
            if ((u16)(*(u16 *)&entity->animStep & mask) == 0x25)
                PlaySong(0x124);
            if ((u16)(*(u16 *)&entity->animStep & mask) == 0x2B)
                PlaySong(0x124);
            if ((u16)(*(u16 *)&entity->animStep & mask) == 0x30)
            {
                PlaySong(0x124);
                a = SpawnScythe(entity);
                b = SpawnScythe(entity);
                b->ext.deathWeapon.primaryIndex = ENTITY_INDEX(a);
                b->ext.deathWeapon.mirror = 1;
            }
            }
            if (entity->animFlags & 4)
            {
                Anim_StartIndexed(entity, 7, 2, 1);
                entity->state = 3;
                entity->stage = 0;
                entity->subState = 0;
                entity->hit.flags &= ~4;
            }
            break;
    }
}

/* Death second-phase hover and attack selection. State numbers are
 * observed dispatcher values, not evidence of original source names. */

#define PLAYER (gEwramData->playerState.player)

/* Keep these inline boundaries: they separate the address and random-mask
 * lifetimes. A byte match does not prove the original helper organization. */
static inline s32 PlayerX(void)
{
    return PLAYER->posX.whole;
}

static inline u8 RandomChoice(void)
{
    return RandomNumberGenerator() & 0x3F;
}

/**
 * @brief Hover state (3): track player X and oscillating height, then
 * choose dash (4), throw (5) or wall attack (6). Retain the empty substate
 * case and reuse of the zero-valued target temporary required for matching.
 */
void Death_UpdateHover(struct EwramData_EntityData *entity)
{
    s32 ty, s, acc, q;
    u32 r;

    switch (entity->subState)
    {
        case 0:
            entity->timer = 0;
            entity->subState = 1;
            /* Fall through. */
        case 1:
            ty = PLAYER->posY.whole;
            s = Sin(entity->timer << 8);
            if (ty < 0)
                ty += 7;
            q = ty >> 3;
            s = (s << 4) + 0x500000;
            ty = q + s;
            if (ty <= 0x3FFFFF)
                ty = 0x400000;
            entity->timer++;
            {
                s32 px = PlayerX();
                acc = 0x400;
                {
                    s32 x = entity->posX.whole;
                    if (px < x - 0x400000)
                        entity->accelX.whole = -0x400;
                    else if (px > x + 0x400000)
                        entity->accelX.whole = acc;
                    else
                    {
                        entity->accelX.whole = 0;
                        EntityDampVelocityX(entity, 0x28);
                    }
                }
            }
            {
                s32 y = entity->posY.whole;
                if (ty < y - 0x100000)
                    entity->accelY.whole = -0x400;
                else if (ty > y + 0x100000)
                    entity->accelY.whole = acc;
                else
                {
                    entity->accelY.whole = 0;
                    EntityDampVelocityY(entity, 0x28);
                }
            }
            EntityAccelerateAndMove(entity, 0x14000);
            /* Reuse the target temporary for the zero passed into states 4/6;
             * separate zero locals change allocation and tail sharing. */
            do { ty = entity->timer & 0x1F; } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
            if (ty == 0 && (RandomNumberGenerator() & 0x3F) <= 0x2F)
            {
                r = RandomChoice();
                entity->ext.death.unusedChoice = r;
                if (r <= 0x14)
                {
                    entity->state = 4;
                    entity->stage = ty;
                    entity->subState = ty;
                    break;
                }
                else if (r <= 0x29 && (s16)entity->posY.part16.integer > 0x60)
                {
                    entity->state = 6;
                    entity->stage = ty;
                    entity->subState = ty;
                    break;
                }
                else
                {
                    entity->state = 5;
                    entity->stage = 0;
                    entity->subState = 0;
                    break;
                }
            }
            break;
        /* Retain this empty case for the original switch decision tree. */
        case 2:
            break;
    }
}

/* Death_UpdateDashAttack, decompiled from the original assembly. The first player
 * read goes through the inline Player() (offset constant reloaded as in the
 * target). */

/* Move towards (tx, ty) (16.16) at a speed proportional to the distance;
 * velX/velY get the step taken. Returns the speed. */
static inline s32 MoveToward2(struct EwramData_EntityData *entity, s32 tx, s32 ty)
{
    s32 x = entity->posX.whole;
    s32 y = entity->posY.whole;
    u32 dist = Sqrt(((x - tx) >> 16) * ((x - tx) >> 16) + ((y - ty) >> 16) * ((y - ty) >> 16)) << 16;
    u16 angle = ArcTan2(((tx < 0) ? -(-tx >> 16) : (tx >> 16)) - (s16)entity->posX.part16.integer,
                        ((ty < 0) ? -(-ty >> 16) : (ty >> 16)) - entity->posY.part16.integer);
    u32 speed = dist >> 4;
    Entity_AccelerateAlongAngle(entity, angle, speed, 0, 0, 0);
    entity->velX.whole = entity->posX.whole - x;
    entity->velY.whole = entity->posY.whole - y;
    return speed;
}

/* Cutscene step: fly to 0x78 px beside the player, then dash sideways,
 * spawn two effects and play the landing animation. */

/**
 * @brief Dash attack state (4): approach a stored point beside the player,
 * dash with afterimages and update the weapon pose through animation
 * steps; return to hover when the recovery animation finishes.
 */
void Death_UpdateDashAttack(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
    {
        s32 offset;
        {
            struct EwramData_EntityData *player = Player();
            s32 px = player->posX.whole;
            entity->flipX = (entity->posX.whole < px) ? 1 : 0;
        }
        entity->ext.death.dashStep = 2;
        Anim_StartIndexed(entity, 9, 2, 1);
        offset = 0x78;
        {
            struct EwramData *ewram = gEwramData;
            struct EwramData_EntityData **player = &ewram->playerState.player;
            struct BgLayerState *camera;
            {
                s32 px = (*player)->posX.whole;
                if (entity->posX.whole < px)
                    offset = -offset;
            }
            {
                s32 px = (s16)(*player)->posX.part16.integer;
                camera = &ewram->bgInfo[1];
                entity->ext.death.targetX = px + camera->xPos.part16.integer + offset;
            }
            {
                s32 py = (s16)(*player)->posY.part16.integer;
                entity->ext.death.targetY = py + camera->yPos.part16.integer;
            }
        }
        entity->subState = 1;
    }
    case 1:
    {
        struct BgLayerState *camera;
        if (MoveToward2(entity, (entity->ext.death.targetX - (s16)(camera = &gEwramData->bgInfo[1])->xPos.part16.integer) << 16,
                       (entity->ext.death.targetY - (s16)camera->yPos.part16.integer) << 16) > 0x3FFF)
            break;
        Anim_StartIndexed(entity, 0xA, 2, 1);
        entity->subState = 2;
        {
            s32 vx = -0x50000;
            s32 ax;
            entity->velX.whole = vx;
            ax = 0xF00;
            entity->accelX.whole = ax;
            if (entity->flipX)
            {
                entity->velX.whole = -vx;
                entity->accelX.whole = -ax;
            }
        }
        entity->velY.whole = 0;
        entity->accelY.whole = 0;
        entity->timer = 0x10;
        entity->ext.death.dashStep = 3;
        Afterimage_Create(entity, 0, 0x10, 0x31);
        Afterimage_Create(entity, 0, 8, 0x30);
        PlaySong(0x87);
        break;
    }
    case 2:
        EntityAccelerateAndMove(entity, 0x50000);
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        Anim_StartIndexed(entity, 0xB, 2, 0);
        entity->subState = 3;
        break;
    case 3:
        EntityAccelerateAndMove(entity, 0x50000);
        switch (entity->animStep)
        {
        case 1:
        case 5:
            entity->ext.death.dashStep = 1;
            break;
        case 3:
            if (entity->animTimer == 0)
                PlaySong(0x86);
            entity->ext.death.dashStep = 4;
            break;
        case 6:
            if (entity->animTimer == 0)
                PlaySong(0x85);
            entity->ext.death.dashStep = 5;
            break;
        }
        if (entity->animFlags & 4)
        {
            entity->flipX = 0;
            entity->ext.death.dashStep = 0;
            Anim_StartIndexed(entity, 7, 2, 1);
            entity->state = 3;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    }
}

/* Death_UpdateThrowAttack. */
/* Move towards (tx, ty) (16.16) at a speed proportional to the distance;
 * velX/velY get the step taken. Returns the speed. */
static inline s32 MoveToward3(struct EwramData_EntityData *entity, s32 tx, s32 ty)
{
    s32 x = entity->posX.whole;
    s32 y = entity->posY.whole;
    u32 dist = Sqrt(((x - tx) >> 16) * ((x - tx) >> 16) + ((y - ty) >> 16) * ((y - ty) >> 16)) << 16;
    u16 angle = ArcTan2(((tx < 0) ? -(-tx >> 16) : (tx >> 16)) - (s16)entity->posX.part16.integer,
                        ((ty < 0) ? -(-ty >> 16) : (ty >> 16)) - entity->posY.part16.integer);
    u32 speed = dist >> 4;
    Entity_AccelerateAlongAngle(entity, angle, speed, 0, 0, 0);
    entity->velX.whole = entity->posX.whole - x;
    entity->velY.whole = entity->posY.whole - y;
    return speed;
}

/* Cutscene step: fly to a point 0x50 px beside the player (screen y 0x78),
 * then play the landing animations and hand back to state 3. */
/**
 * @brief Throw attack state (5): move beside the player, signal weapon
 * throw and return with owner flags, then restore hover state once the
 * weapon clears the return flag and the recovery animation finishes.
 */
void Death_UpdateThrowAttack(struct EwramData_EntityData *entity)
{
    s32 tx, ty;
    switch (entity->subState)
    {
    case 0:
    {
        struct EwramData *ewram = gEwramData;
        struct EwramData_EntityData **player = &ewram->playerState.player;
        struct BgLayerState *camera;
        {
            s32 px = (*player)->posX.whole;
            u8 right = 0;
            if (entity->posX.whole < px)
                right = 1;
            entity->flipX = right;
        }
        tx = -0x50;
        { s32 px = (*player)->posX.whole; if (entity->posX.whole < px) tx = 0x50; }
        {
            s32 px = (s16)(*player)->posX.part16.integer;
            camera = &ewram->bgInfo[1];
            entity->ext.death.targetX = px + camera->xPos.part16.integer - tx;
        }
        entity->ext.death.targetY = camera->yPos.part16.integer + 0x78;
        entity->subState = 1;
        break;
    }
    case 1:
    {
        struct BgLayerState *camera;
        tx = (entity->ext.death.targetX - (s16)(camera = &gEwramData->bgInfo[1])->xPos.part16.integer) << 16;
        ty = (entity->ext.death.targetY - (s16)camera->yPos.part16.integer) << 16;
        if (MoveToward3(entity, tx, ty) <= 0x3FFF)
        {
            Anim_StartIndexed(entity, 0x19, 2, 0);
            entity->ext.death.flags |= 8;
            entity->subState = 2;
        }
        break;
    }
    case 2:
        if (entity->animFlags & 4)
        {
            Anim_StartIndexed(entity, 0x1A, 2, 0);
            entity->ext.death.flags |= 0x84;
            entity->subState = 3;
        }
        break;
    case 3:
        if (!(entity->ext.death.flags & 4))
        {
            entity->ext.death.flags &= 0xFF7F;
            entity->flipX = 0;
            Anim_StartIndexed(entity, 0x19, 2, 0);
            entity->subState = 4;
        }
        break;
    case 4:
        if (entity->animFlags & 4)
        {
            Anim_StartIndexed(entity, 7, 2, 1);
            entity->state = 3;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    }
}

/* Short animation phase. */
/**
 * @brief Wall attack state (6): play animation 0x19 and set weapon flag
 * 0x10. Wait for the animation and for the weapon wall-traversal helper
 * to clear that flag, then return to hover state 3.
 */
void Death_UpdateWallAttack(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        Anim_StartIndexed(entity, 0x19, 2, 0);
        entity->ext.death.flags |= 0x10;
        entity->subState = 1;
        break;
    case 1:
        if (entity->animFlags & 4)
            entity->subState = 2;
        break;
    case 2:
        if (!(entity->ext.death.flags & 0x10))
        {
            Anim_StartIndexed(entity, 7, 2, 1);
            entity->state = 3;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    }
}

/* Death_UpdateDeath: boss death sequence (flash, debris parts); matched. */
static inline struct EwramData_EntityData *SpawnPart(struct EwramData_EntityData *owner, EntityFunc func)
{
    struct EwramData_EntityData *c = Entity_CreateObject(func);
    if (c == NULL)
        return NULL;
    c->ext.deathHelper.ownerIndex = ENTITY_INDEX(owner);
    c->scrollWithBg = 1;
    return c;
}

struct Flags58 { u8 lo:5; u8 bit5:1; u8 hi:2; };

/**
 * @brief Death state (7): wait, start palette effects when enabled, create
 * four inward-spiraling debris parts and the reward effect, then end
 * the fades and return nonzero for boss removal.
 */
s32 Death_UpdateDeath(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child;
    s32 i;
    u16 *px;
    struct Flags58 *flags;

    switch (entity->subState)
    {
    case 0:
        Anim_StartIndexed(entity, 0xE, 2, 1);
        entity->timer = 0x40;
        entity->subState = 1;
    case 1:
        if (entity->timer != 0)
            goto count;
        if (!(gEwramData->gameState.stateFlags & 0x10))
        {
            BgPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0, 0x10000, 0x10);
            ObjPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0x4000, 0x10000, 0x10);
        }
        i = 3;
        flags = (struct Flags58 *)(ENTITY_RENDER_FLAGS_ADDRESS(entity));
        px = &entity->posX.part16.integer;
        for (; i >= 0; i--)
        {
            child = SpawnPart(entity, DeathDebris_Update);
            if (child != NULL)
            {
                child->scrollWithBg = 0;
                child->subState = i;
                {
                    struct BgLayerState *bg = &gEwramData->bgInfo[1];
                    child->posX.part16.integer = *px + bg->xPos.part16.integer;
                    child->posY.part16.integer = entity->posY.part16.integer + bg->yPos.part16.integer + (s16)((i << 4) - 0x30);
                }
                child->timer = i << 2;
            }
        }
        entity->subState = 2;
        SpawnPart(entity, DeathRewardEffect_Update);
        flags->bit5 = 1;
        entity->timer = 0x10;
        break;
    case 2:
        if (entity->timer != 0)
        {
        count:
            entity->timer--;
            break;
        }
        entity->subState = 3;
        if (!(gEwramData->gameState.stateFlags & 0x10))
        {
            BgPalette_StopColorFade();
            ObjPalette_StopColorFade();
        }
        break;
    case 3:
        return 1;
    }
    return 0;
}

/**
 * @brief BF2E8 | Death create: the boss version (spawnParam0) is deleted once
 * beaten (bossEventFlags bit 1) unless in the boss rush mode; sprite layer 0x13,
 * animation 1, child DeathOverlay_Update; bestiary preview gives state 8; otherwise
 * box slot 8 (Hit_EnableContact) with box 0x3230CEE8, boxes off; the boss
 * version starts in state 0 at room position (128, 32) on screen with song
 * 0x1010 and child DeathIntroEffect_Update, else state 1 with song 0x10; HP 0x98C,
 * child DeathFloatingWeapon_Update and the boss HP bar.
 */
void EnemyDeathCreate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child;
    struct EwramData_EntityData *first;
    struct BgLayerState *bg;
    u8 frozen;

    if (entity->spawnParam0 != 0 && !gEwramData->gameState.bossRushMode
        && (gEwramData->gameState.bossEventFlags & 2))
        goto delete;
    if (!Entity_LoadGraphics(entity, sDeathGfx, sDeathPalette, 0, sDeathSpriteSheet))
    {
    delete:
        EntityDelete(entity);
        return;
    }
    entity->flipX = 0;
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 0x13;
    Anim_StartIndexed(entity, 1, 2, 1);
    Anim_Update(entity);
    first = Entity_CreateObject(DeathOverlay_Update);
    if (first != NULL)
    {
        first->ext.deathHelper.ownerIndex = ENTITY_INDEX(entity);
        first->scrollWithBg = 1;
    }
    frozen = *((u8 *)gEwramData + 0x8E) & 0x40;
    if (frozen)
    {
        entity->state = 8;
        return;
    }
    Hit_EnableContact(entity, 8, Enemy_OnContact, Death_OnHit);
    HitBox_SetBodyRaw(entity, 0x3230CEE8);
    HitBox_SetAttackRaw(entity, 0x3230CEE8);
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
    if (entity->spawnParam0 != 0)
    {
        entity->state = 0;
        bg = &gEwramData->bgInfo[1];
        entity->posX.part16.integer = 0x80 - bg->xPos.part16.integer;
        entity->posY.part16.integer = 0x20 - bg->yPos.part16.integer;
        PlaySong(0x1010);
        child = Entity_CreateObject(DeathIntroEffect_Update);
        if (child != NULL)
        {
            child->ext.deathHelper.ownerIndex = ENTITY_INDEX(entity);
            child->scrollWithBg = 1;
        }
    }
    else
    {
        entity->state = 1;
        Music_SetCurrent(0x10);
    }
    entity->objMode = 1;
    entity->hp = 0x98C;
    child = Entity_CreateObject(DeathFloatingWeapon_Update);
    if (child != NULL)
    {
        child->ext.deathHelper.ownerIndex = ENTITY_INDEX(entity);
        child->scrollWithBg = 1;
    }
    Player_SetSelectedTarget(entity);
    gEwramData->gameState.stateFlags |= 2;
}

/* Death_OnHit (Death). */

/**
 * @brief Mark the boss hit flag, then apply enemy damage. On defeat drop
 * rewards, spawn the defeat effect above the boss, restore its palette
 * and enter death state 7 with substate counters reset.
 */
void Death_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    entity->ext.death.flags |= 2;
    if (Enemy_TakeDamage(entity, attacker, hit))
    {
        Enemy_DropRewards(entity);
        BossDefeatEffect_Spawn(entity->posX.whole, entity->posY.whole - 0x200000);
        entity->palette = entity->basePalette;
        entity->state = 7;
        entity->stage = 0;
        entity->subState = 0;
    }
}

/* Death helpers 0x080BF504..0x080BF5A7 (Death_OnHit is in this file,
 * DeathFloatingWeaponHitBox_OnHit). */

/**
 * @brief Apply enemy damage to the floating weapon; on defeat mark flag 2
 * and restore the palette so its update starts the breakup sequence.
 */
void DeathFloatingWeapon_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    if (Enemy_TakeDamage(entity, attacker, hit))
    {
        entity->enemyFlags |= 2;
        entity->palette = entity->basePalette;
    }
}

/**
 * @brief Clear blade HP so its update starts the destruction sequence.
 */
void DeathBlade_OnHit(struct EwramData_EntityData *entity)
{
    entity->hp = 0;
}

/**
 * @brief BF548 | Player_ApplyContactDamage with (attack, 0, 0, 1) at the entity position.
 */
void DeathPillar_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attributes = 0;
    contactDamage.unused = 0;
    contactDamage.attack = entity->attack;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief BF578 | Player_ApplyContactDamage with (attack, 1, 0, 1) at the entity position
 * (same as Buer_DamagePlayerOnContact).
 */
void DeathWeapon_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attributes = 1;
    contactDamage.unused = 0;
    contactDamage.attack = entity->attack;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief Forward hit damage to the floating weapon in the owner slot;
 * on defeat mark its flag 2 and restore its palette.
 */
void DeathFloatingWeaponHitBox_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    struct EwramData_EntityData *weapon = &gEwramData->entityData[(u8)entity->ext.deathHelper.ownerIndex];

    if (Enemy_TakeDamage(weapon, attacker, hit))
    {
        weapon->enemyFlags |= 2;
        weapon->palette = weapon->basePalette;
    }
}

void EnemyDeathUpdate(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    switch (entity->state)
    {
    case 0: Death_UpdateIntro(entity); break;
    case 1: Death_UpdateFirstPhase(entity); break;
    case 2: Death_UpdateWeaponTransition(entity); break;
    case 3: Death_UpdateHover(entity); break;
    case 4: Death_UpdateDashAttack(entity); break;
    case 5: Death_UpdateThrowAttack(entity); break;
    case 6: Death_UpdateWallAttack(entity); break;
    case 7:
        if (Death_UpdateDeath(entity))
        {
            EntityDelete(entity);
            return;
        }
        break;
    case 8: Anim_Update(entity); return;
    }
    if (!(entity->enemyFlags & 2))
    {
        Enemy_UpdateHitFlash(entity);
        Hit_CheckWithPlayer(&result, entity);
    }
    Anim_Update(entity);
}

/**
 * @brief Display frame 9 using the selected palette, enable drawSkip at
 * countdown 4 and request deletion when the countdown expires.
 */
void EvilButcherDeathParticle_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sEvilButcherGfx, sEvilButcherPalette,
                         entity->spawnParam1, sEvilButcherSpriteSheet))
        {
            EntityDelete(entity);
            return;
        }
        entity->animFrame = 9;
        entity->spriteLayer = 0x13;
        entity->state++;
    case 1:
        if (entity->timer != 0)
        {
            if (entity->timer == 4) entity->drawSkip = 1;
            entity->timer--;
        }
        else entity->deleteRequested = 1;
        break;
    }
}
