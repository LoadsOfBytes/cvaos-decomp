/*
 * Killer Mantle (hover, jump attack, death, create, hits, update), the Cockatrice family (parts, patrol, attack, death, create, hits, Cockatrice / Basilisk) and the Death overlay / weapon wall traversal.
 * Built without -f2003-patch (KillerMantle_UpdateHover and CockatriceFamily_UpdateAttack need it off; the rest compiles the same either way).
 */
#include "killer_mantle.h"
#include "core.h"
#include "graphics.h"
#include "structs/ewram.h"
#include "cockatrice_entity.h"
#include "player_core.h"
#include "bg_and_collision.h"
#include "music.h"
#include "sound.h"
#include "gameplay.h"
#include "syscalls.h"
#include "effects.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "entity_motion.h"
#include "hitbox.h"
#include "man_eater.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"
#include "entity_hit.h"

/* KillerMantle_UpdateHover (state 1 of the enemy created at 0x080BB70C); only matches without -f2003-patch
 * (NO_2003_PATCH_SRCS in ../Makefile). (Target_IsNearEntity test through an inline with two
 * returns, state 1 constant in a block variable). */

#define ABS(x) ((x) < 0 ? -(x) : (x))


// The offsets are passed sign-extended here (u16 in the definition).
void CockatriceFamilyAttack_Draw(struct EwramData_EntityData *entity);
void CockatriceFamilyAttack_DamagePlayerOnContact(struct EwramData_EntityData *entity);

void KillerMantle_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
void CockatriceFamily_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);

/**
 * @brief BB2D4 | Hover (state 1): sprite 1, then sprite 2 with a random
 * speed factor unk_4F8 (0x100..0x1FF). Sways back and forth (sine of the
 * timer unk_4FC, direction stage); when an input of Player_GetFocusedActionFlags (mask
 * 0x360) comes while the target is within 80 pixels in front, it dodges
 * (timer 0x30, timer). After 48 frames it attacks (state 2) when the
 * target is within 0xA0 x 0x46, otherwise turns and picks a new speed
 * (faster towards the target and after a dodge).
 */
static inline u32 InRange(struct EwramData_EntityData *e) { if (Target_IsNearEntity(e, 0xA0, 0x46)) return 1; return 0; }

void KillerMantle_UpdateHover(struct EwramData_EntityData *entity)
{
    s32 speed;
    u8 back;

    switch (entity->subState)
    {
        case 0:
            Anim_StartIndexed(entity, 1, 0, 0);
            {
                u8 one = 1;
                entity->velY.whole = 0;
                entity->accelY.whole = 0;
                entity->stage = 0;
                entity->ext.killerMantle.swayFrame = 0;
                entity->subState = one;
            }
        case 1:
            if (!(entity->animFlags & 4))
                break;
            Anim_StartIndexed(entity, 2, 0, 0);
            entity->subState = 2;
            entity->ext.killerMantle.speed = RandomNumberGenerator() % 256 + 0x100;
            break;
        case 2:
            entity->ext.killerMantle.swayFrame++;
            entity->accelY.whole = Sin(entity->ext.killerMantle.swayFrame << 11) / 48;
            speed = entity->ext.killerMantle.speed * (Sin(entity->ext.killerMantle.swayFrame * 3 << 8) / 256);
            if (entity->stage)
                speed = -speed;
            entity->timer = 0;
            if ((Player_GetFocusedActionFlags() & 0x360)
                && ABS((s16)Target_GetPixelX(entity) - (s16)entity->posX.part16.integer) <= 79)
            {
                back = entity->stage;
                if (entity->flipX)
                    back ^= 1;
                if (((!back && (s8)Entity_GetTargetDirection(entity) == -1) || (back && (s8)Entity_GetTargetDirection(entity) == 1))
                    && ((!Target_GetFlipX(entity) && (s8)Entity_GetTargetDirection(entity) == -1)
                        || (Target_GetFlipX(entity) && (s8)Entity_GetTargetDirection(entity) == 1)))
                {
                    entity->ext.killerMantle.swayFrame = 0x30;
                    entity->timer = 1;
                }
            }
            Entity_Walk(entity, speed, sKillerMantleWalkProfile, 0);
            if (entity->ext.killerMantle.swayFrame <= 47)
                break;
            if (InRange(entity) && entity->timer == 0)
            {
                entity->state = 2;
                entity->stage = 0;
                entity->subState = 0;
                break;
            }
            entity->stage ^= 1;
            if (entity->stage == 0)
                Anim_StartIndexed(entity, 2, 0, 0);
            else
                Anim_StartIndexed(entity, 7, 0, 0);
            back = entity->stage;
            if (entity->flipX)
                back ^= 1;
            entity->ext.killerMantle.speed = RandomNumberGenerator() % 256 + 0x100;
            if ((!back && (s8)Entity_GetTargetDirection(entity) == -1) || (back && (s8)Entity_GetTargetDirection(entity) == 1))
                entity->ext.killerMantle.speed += 0x100 + RandomNumberGenerator() % 64;
            if (entity->timer)
                entity->ext.killerMantle.speed += 0x100;
            entity->ext.killerMantle.swayFrame = 0;
            break;
    }
}

/* KillerMantle_UpdateJumpAttack (state 2 of the enemy created at 0x080BB70C). */

/**
 * @brief BB518 | Jump attack (state 2): crouch (sprite 3, unk_4FE bit 0)
 * until the animation ends, then jump (sprite 4, velY about -0x1A000,
 * gravity 0x1000, forward speed unk_4F8 0x48000..0x49F00, box slot bit 1
 * off, song 0x89) slowing by 0x2000 per frame; on landing spawn
 * KillerMantleLandingEffect_Update, box slot bit 1 on, sprite 5 and slow down by 0x1000 per
 * frame until the animation ends, then state 1.
 */
void KillerMantle_UpdateJumpAttack(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child;

    switch (entity->subState)
    {
        case 0:
            Entity_FaceTarget(entity);
            Anim_StartIndexed(entity, 3, 0, 0);
            entity->ext.killerMantle.flags |= 1;
            entity->subState = 1;
        case 1:
            if (!(entity->animFlags & 4))
                break;
            entity->subState = 2;
            Anim_StartIndexed(entity, 4, 0, 0);
            entity->velY.whole = RandomSigned(0x2000) - 0x1A000;
            entity->accelY.whole = 0x1000;
            entity->ext.killerMantle.speed = (RandomNumberGenerator() % 32 << 8) + 0x48000;
            entity->hit.flags &= ~2;
            PlaySong(0x89);
            break;
        case 2:
            if (entity->ext.killerMantle.speed > 0)
                entity->ext.killerMantle.speed -= 0x2000;
            Entity_Walk(entity, entity->ext.killerMantle.speed, sKillerMantleWalkProfile, 0);
            if (!(entity->animFlags & 4))
                break;
            child = Entity_CreateObject(KillerMantleLandingEffect_Update);
            if (child != NULL)
            {
                child->scrollWithBg = 1;
                ENTITY_OWNER(child) = entity;
            }
            entity->hit.flags |= 2;
            entity->ext.killerMantle.flags &= ~1;
            entity->subState = 3;
            Anim_StartIndexed(entity, 5, 0, 0);
            break;
        case 3:
            if (entity->ext.killerMantle.speed > 0)
                entity->ext.killerMantle.speed -= 0x1000;
            Entity_Walk(entity, entity->ext.killerMantle.speed, sKillerMantleWalkProfile, 0);
            if (entity->animFlags & 4)
            {
                entity->state = 1;
                entity->stage = 0;
                entity->subState = 0;
            }
            break;
    }
}

/* Death burst and create of the enemy at 0x080BB664..0x080BB7E3 (named
 * EnemyKillerMantleCreate upstream). */

/**
 * @brief BB664 | Death burst: sprite 6 and song 0x6D, then every fourth
 * frame a fragment (Effect_SpawnShared, layer 0x14, mirrored vertically) flying
 * in a random direction and slowing down by 1/24 of its speed. Returns 1
 * once the animation ended.
 */
s32 KillerMantle_UpdateDeath(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *effect;
    s16 angle;
    s32 timer;

    switch (entity->subState)
    {
        case 0:
            Anim_StartIndexed(entity, 6, 0, 0);
            entity->subState = 1;
            PlaySong(0x6D);
        case 1:
            timer = entity->timer + 1;
            entity->timer = timer;
            if (!(timer & 3))
            {
                effect = Effect_SpawnShared(entity->posX.whole, entity->posY.whole, 22, 8);
                if (effect != NULL)
                {
                    angle = RandomNumberGenerator();
                    effect->flipY = 1;
                    effect->velX.whole = Sin(angle + 0x4000);
                    effect->velY.whole = Sin(angle);
                    effect->accelX.whole = effect->velX.whole / 24;
                    effect->accelY.whole = effect->velY.whole / 24;
                    effect->spriteLayer = 0x14;
                }
            }
            if (entity->animFlags & 4)
                return 1;
            break;
    }
    return 0;
}

/**
 * @brief BB70C | Create: load the graphics (delete the enemy when that
 * fails), hit boxes, layer 0x13, sprite 0; state 4 when bestiaryPreview is set,
 * otherwise box callbacks (Enemy_OnContactMask1 / KillerMantle_OnHit), boxes, its part
 * KillerMantlePart_Update and state 0.
 */
void EnemyKillerMantleCreate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child;

    if (!Entity_LoadGraphics(entity, sKillerMantleGfx, sKillerMantlePalette, 0, sKillerMantleSpriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 0x13;
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 4;
        return;
    }
    Hit_EnableContact(entity, 8, Enemy_OnContactMask1, KillerMantle_OnHit);
    HitBox_SetBodyRaw(entity, 0x1010F8F8);
    HitBox_SetAttackRaw(entity, 0x1414F6EC);
    entity->hit.flags |= 2;
    child = Entity_CreateObject(KillerMantlePart_Update);
    if (child != NULL)
    {
        child->scrollWithBg = 1;
        ENTITY_OWNER(child) = entity;
    }
    entity->scrollWithBg = 1;
    entity->state = 0;
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   KillerMantle_OnHit = SkullMillioneFamily_OnHit */

/**
 * @brief Apply enemy damage; on defeat drop rewards, restore the palette
 * and enter death state 3 with the substate counters reset.
 */
void KillerMantle_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    if (Enemy_TakeDamage(entity, attacker, hit))
    {
        Enemy_DropRewards(entity);
        entity->palette = entity->basePalette;
        entity->state = 3;
        entity->stage = 0;
        entity->subState = 0;
    }
}

/* Killer Mantle update and part placement 0x080BB824..0x080BB923. */

/**
 * @brief BB824 | Cockatrice/Basilisk update: unless Enemy_PreUpdate handles the
 * frame, state 0 waits for the target within 0xDC x 0xA0 (then state 1),
 * states 1/2 run KillerMantle_UpdateHover / KillerMantle_UpdateJumpAttack, state 3 deletes the enemy
 * when KillerMantle_UpdateDeath says so, state 4 only animates; then Enemy_CheckHitPlayer and
 * animate.
 */
void EnemyKillerMantleUpdate(struct EwramData_EntityData *entity)
{
    if (Enemy_PreUpdate(entity))
        return;
    switch (entity->state)
    {
        case 0:
            if (Target_IsNearEntity(entity, 0xDC, 0xA0))
            {
                entity->state = 1;
                entity->stage = 0;
                entity->subState = 0;
            }
            break;
        case 1:
            KillerMantle_UpdateHover(entity);
            break;
        case 2:
            KillerMantle_UpdateJumpAttack(entity);
            break;
        case 3:
            if (KillerMantle_UpdateDeath(entity))
            {
                EntityDelete(entity);
                return;
            }
            break;
        case 4:
            Anim_Update(entity);
            return;
    }
    Enemy_CheckHitPlayer(entity);
    Anim_Update(entity);
}

/**
 * @brief Place the shared Cockatrice/Basilisk part at owner sprite point 4
 * plus a facing-dependent horizontal offset.
 */
void CockatriceFamilyPart_FollowOwner(struct EwramData_EntityData *entity, struct EwramData_EntityData *owner)
{
    struct { s16 x; s16 y; } point;
    s16 offset = sUnk_08119490[0];

    entity->flipX = owner->flipX;
    if (entity->flipX)
        offset = -offset;
    Sprite_GetPartCornerOffset(owner, 4, &point.x);
    entity->posX.part16.integer = offset + (owner->posX.part16.integer + point.x);
    entity->posY.part16.integer = point.y + owner->posY.part16.integer;
}

/**
 * @brief Load an affine part and follow the owner with angle and scale
 * wobble; increase amplitude while it attacks. Free the affine slot
 * after the owner termination flag. Exact visual identity is unresolved.
 */
void CockatriceFamilyPart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = ENTITY_OWNER(entity);
    u16 slot;

    switch (entity->state)
    {
        case 0:
            if (!Entity_LoadGraphics(entity, sCockatriceFamilyGfx, sCockatriceFamilyPalette, parent->ext.cockatrice.paletteSlot, sCockatriceFamilySpriteSheet))
                goto delete;
            slot = ObjAffine_Alloc(1);
            if ((s16)slot < 0)
                goto delete;
            entity->affineIndex = slot;
            entity->affine = 1;
            entity->animFrame = 2;
            entity->spriteLayer = parent->spriteLayer;
            entity->ext.cockatricePart.scaleX = 0x10000;
            entity->ext.cockatricePart.amplitude = 8;
            entity->state = 1;
        case 1:
            CockatriceFamilyPart_FollowOwner(entity, parent);
            if (parent->state == 2)
                entity->state = 2;
            if (gEwramData->gameState.timeStopTimer == 0)
                entity->ext.cockatricePart.phase -= 0x2B0;
            break;
        case 2:
            CockatriceFamilyPart_FollowOwner(entity, parent);
            entity->ext.cockatricePart.phase -= 0x17B0;
            entity->ext.cockatricePart.amplitude = 16;
            break;
        case 3:
            ObjAffine_Free(entity->affineIndex);
        delete:
            EntityDelete(entity);
            return;
    }
    if (!entity->flipX)
        entity->ext.cockatricePart.angle = Sin(entity->ext.cockatricePart.phase)
                                              * entity->ext.cockatricePart.amplitude / 256;
    else
        entity->ext.cockatricePart.angle = Sin(entity->ext.cockatricePart.phase)
                                              * -entity->ext.cockatricePart.amplitude / 256;
    entity->ext.cockatricePart.scaleX = (Sin(entity->ext.cockatricePart.phase + 0x2000) << 13 >> 16) + 0xE000;
    entity->palette = parent->palette;
    if (parent->ext.cockatrice.deathFlags & 1)
        entity->state = 3;
    ObjAffine_Set(entity->ext.cockatricePart.angle, entity->ext.cockatricePart.scaleX, 0x10000, entity->flipX, 0,
                 entity->affineIndex);
}

/**
 * @brief Create the shared affine attack effect, play its tracked sound,
 * rotate and scale it for 32 updates and check player contact at update 16.
 * Stop the sound and free the affine slot on removal.
 */
void CockatriceFamilyAttack_Update(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    struct EwramData_EntityData *parent = ENTITY_OWNER(entity);
    u16 slot;
    s32 scale;
    s32 growth;

    switch (entity->state)
    {
        case 0:
            if (!Entity_LoadGraphics(entity, sCockatriceFamilyGfx, sCockatriceFamilyPalette, parent->ext.cockatrice.paletteSlot, sCockatriceFamilySpriteSheet))
                goto delete;
            entity->animFrame = 1;
            entity->spriteLayer = parent->spriteLayer + 1;
            entity->drawFunc = CockatriceFamilyAttack_Draw;
            slot = ObjAffine_Alloc(1);
            if ((s16)slot < 0)
                goto delete;
            entity->affineIndex = slot;
            entity->affine = 1;
            entity->ext.cockatriceBeam.length = 0x1000000;
            Entity_PlaceRelativeToCopy(entity, parent, -24, -33);
            entity->ext.cockatriceBeam.angle = 0x6C00;
            if (entity->flipX)
            {
                s32 angle = -0xEC00; // 0x8000 - 0x6C00 in 16 bits

                entity->ext.cockatriceBeam.angle = angle;
            }
            Hit_Enable(entity, 8, CockatriceFamilyAttack_DamagePlayerOnContact, 0);
            HitBox_SetAttackRaw(entity, 0x10440CBC);
            entity->hit.flags |= 4;
            entity->objMode = 1;
            entity->state = 1;
            Sound_PlayTracked(0x65);
        case 1:
            entity->ext.cockatriceBeam.frame++;
            entity->ext.cockatriceBeam.scaleY = Sin(entity->ext.cockatriceBeam.frame << 10);
            if (entity->ext.cockatriceBeam.frame <= 15)
                entity->posY.whole -= 0x2000;
            if (!entity->flipX)
                entity->ext.cockatriceBeam.angle += 8;
            else
                entity->ext.cockatriceBeam.angle -= 8;
            if (entity->ext.cockatriceBeam.frame == 16)
                Hit_CheckWithPlayer(&result, entity);
            if (entity->ext.cockatriceBeam.frame <= 31)
                break;
        case 2:
            Sound_StopTracked(0x65);
            ObjAffine_Free(entity->affineIndex);
        delete:
            EntityDelete(entity);
            return;
    }
    growth = entity->ext.cockatriceBeam.length;
    if (growth > 0x1000000)
        growth = 0x1000000;
    growth /= 16;
    scale = growth / 16;
    entity->ext.cockatriceBeam.stepX = Sin(entity->ext.cockatriceBeam.angle + 0x4000) << 4;
    COCKATRICE_BEAM_STEP_Y(entity) = Sin(entity->ext.cockatriceBeam.angle) << 4;
    entity->ext.cockatriceBeam.stepX = entity->ext.cockatriceBeam.stepX / 0x10000 * scale;
    COCKATRICE_BEAM_STEP_Y(entity) = COCKATRICE_BEAM_STEP_Y(entity) / 0x10000 * scale;
    ObjAffine_Set(entity->ext.cockatriceBeam.angle, 0x10000, entity->ext.cockatriceBeam.scaleY, 0, 0, entity->affineIndex);
}

static inline s32 FrameOffset(s32 frame)
{
    return (frame << 14) + 0x1000;
}

/* Patrol: turn at walls or after animation cycles, then check for attack. */
/**
 * @brief Patrol state (0): walk at animation-dependent speed, turn at walls
 * or after animation cycles, and activate attack state 1 near the target
 * or after damage.
 */
void CockatriceFamily_UpdatePatrol(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        entity->subState = 1;
    case 1:
    {
        u8 frame = (entity->animStep + 1) % 8;
        s32 offset = FrameOffset(frame);
        if (frame > 4) offset = 0;
        if ((Entity_Walk(entity, offset, sCockatriceFamilyWalkProfile, 20) & 0x1B) != 8)
        {
            if (entity->velX.part16.integer != 0)
                entity->flipX ^= 1;
        }
        else if (*(u16 *)&entity->animStep == 0)
        {
            if (entity->stage++ > 2)
            {
                entity->flipX ^= 1;
                entity->stage = 0;
            }
        }
        if (Target_IsNearEntity(entity, 240, 100) || Enemy_IsDamaged(entity))
        {
            entity->state = 1;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    }
    }
}

/* Cockatrice/Basilisk walk 0x080BBD90. This unit omits -f2003-patch to preserve the signed -1 test.
 * Local register bindings preserve the frame-speed and timer copies.
 * Matching does not establish original source spelling. */

/**
 * @brief Attack state (1): walk and face the target, start animation 1
 * when the direction, distance and player flag tests allow it, then spawn
 * the attack effect at animation step 10. Recover for 32 updates.
 */
void CockatriceFamily_UpdateAttack(struct EwramData_EntityData *entity)
{
    u8 frame;
    s32 speed;
    s32 attack;
    s32 distance;
    struct EwramData_EntityData *child;

    switch (entity->subState)
    {
        case 0:
            entity->subState = 1;
        case 1:
            frame = (entity->animStep + 1) % 8;
            {
                register s32 scaled asm("r1") = frame * 0x4000; /* FAKEMATCH: pinned register */
                speed = scaled + 0x1000;
            }
            if (frame > 4)
                speed = 0;
            Entity_Walk(entity, speed, sCockatriceFamilyWalkProfile, 20);
            // animStep and animTimer read together
            if (*(u16 *)&entity->animStep == 0)
            {
                if ((entity->flipX && (s8)Entity_GetTargetDirection(entity) == 1)
                    || (!entity->flipX && (s8)Entity_GetTargetDirection(entity) == -1))
                {
                    distance = Entity_GetTargetDistance(entity);
                    attack = 1;
                    if (distance <= 119)
                        goto test;
                }
                attack = 0;
            test:
                if (attack && !(gEwramData->playerStatus.statusEffects & 4))
                {
                    entity->subState = 2;
                    Anim_StartIndexed(entity, 1, 0, 0);
                }
                Entity_FaceTarget(entity);
            }
            break;
        case 2:
            if (*(u16 *)&entity->animStep == 10 && (child = Entity_CreateObject(CockatriceFamilyAttack_Update)) != NULL)
            {
                ENTITY_OWNER(child) = entity;
                child->scrollWithBg = 1;
                child->attack = entity->attack - (entity->attack >> 3);
            }
            if (entity->animFlags & 4)
            {
                entity->timer = 0x20;
                entity->subState = 3;
            }
            break;
        case 3:
        {
            u8 raw = entity->timer;
            s8 timer = raw;
            if (timer != 0)
            {
                entity->timer = raw - 1;
                break;
            }
            entity->state = timer;
            entity->stage = timer;
            entity->subState = timer;
            Anim_StartIndexed(entity, 0, 0, 1);
            break;
        }
    }
}

/* Shared Cockatrice/Basilisk death and creation.
 * Decompiled locally; patrol and attack helpers are now matching C. */

/* Graphics of the Cockatrice/Basilisk (see this file). */

/**
 * @brief Death state (2): stop tracked sound 0x65, animate with sparks and
 * palette flashes, then set the attached part termination flag. Return
 * nonzero on the following substate; preserve the stored-position arithmetic.
 */
s32 CockatriceFamily_UpdateDeath(struct EwramData_EntityData *entity)
{
    s32 wave;
    s32 jitter;
    struct EwramData_EntityData *spark;

    switch (entity->subState)
    {
        case 0:
            Sound_StopTracked(0x65);
            Anim_StartIndexed(entity, 2, 0, 1);
            entity->timer = 0x40;
            entity->subState = 1;
        case 1:
            entity->velY.whole -= 0x6000;
            entity->velX.whole += 0x3A00;
            if (!(entity->timer & 7))
                PlaySong(0x6E);
            if (!(entity->timer & 1))
            {
                wave = Sin(entity->velX.whole + 0x4000);
                jitter = RandomSigned(0x40000);
                spark = Effect_SpawnShared(entity->posX.whole + wave * 24 + jitter, entity->posY.whole + entity->velY.whole, 1, 7);
                if (spark != NULL)
                {
                    spark->velY.whole = 0x8000;
                    spark->accelY.whole = -0x1000;
                }
            }
            if (Entity_GetVisibilityPattern(entity->timer))
                entity->palette = 5;
            else
                entity->palette = entity->basePalette;
            if (entity->timer != 0)
            {
                entity->timer--;
                break;
            }
            entity->subState = 2;
            entity->ext.cockatrice.deathFlags |= 1;
            break;
        case 2:
            return 1;
    }
    return 0;
}

/**
 * @brief Create Cockatrice (palette 0) or Basilisk (palette 1), including
 * the shared affine part. Bestiary preview uses state 3; gameplay sets
 * hit callbacks and snaps to the ground. Delete on graphics failure.
 */
void CockatriceFamily_Create(struct EwramData_EntityData *entity, s32 paletteSlot)
{
    struct EwramData_EntityData *part;

    if (!Entity_LoadGraphics(entity, sCockatriceFamilyGfx, sCockatriceFamilyPalette, paletteSlot, sCockatriceFamilySpriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    entity->ext.cockatrice.paletteSlot = paletteSlot;
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 0x11;
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    part = Entity_CreateObject(CockatriceFamilyPart_Update);
    if (part != NULL)
    {
        ENTITY_OWNER(part) = entity;
        part->scrollWithBg = 1;
    }
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 3;
        return;
    }
    Hit_Enable(entity, 8, Enemy_OnContact, CockatriceFamily_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    entity->posY.part16.integer++;
    entity->state = 0;
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   CockatriceFamily_OnHit = HeadStackSegment_OnHit */

/**
 * @brief Apply enemy damage; on defeat drop rewards, restore the palette
 * and enter death state 2 with substate counters reset.
 */
void CockatriceFamily_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    if (Enemy_TakeDamage(entity, attacker, hit))
    {
        Enemy_DropRewards(entity);
        entity->palette = entity->basePalette;
        entity->state = 2;
        entity->stage = 0;
        entity->subState = 0;
    }
}

/* Cockatrice/Basilisk hit callback, slash trail and Cockatrice create
 * 0x080BC0F4..0x080BC163. */

/**
 * @brief Apply the shared attack damage at the effect position with
 * effect type 0x100 and zero effect flags.
 */
void CockatriceFamilyAttack_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attributes = 0x100;
    contactDamage.unused = 0;
    contactDamage.attack = entity->attack;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief Draw 16 sprite copies stepped by the stored direction vector,
 * then restore both full position values.
 */
void CockatriceFamilyAttack_Draw(struct EwramData_EntityData *entity)
{
    s32 x = entity->posX.whole;
    s32 y = entity->posY.whole;
    s32 i;

    for (i = 16; i != 0; i--)
    {
        Sprite_Draw(entity);
        entity->posX.whole += entity->ext.cockatriceBeam.stepX;
        entity->posY.whole += COCKATRICE_BEAM_STEP_Y(entity);
    }
    entity->posX.whole = x;
    entity->posY.whole = y;
}

/**
 * @brief BC158 | Cockatrice create: Cockatrice/Basilisk with palette row 0.
 */
void EnemyCockatriceCreate(struct EwramData_EntityData *entity)
{
    CockatriceFamily_Create(entity, 0);
}

/* EnemyCockatriceUpdate. */

/**
 * @brief BC164 | Cockatrice update. Outside state 2: stop when outside the
 * screen and only run Hit_CheckWithPlayer while Enemy_IsStunned says so. States: 0
 * CockatriceFamily_UpdatePatrol, 1 CockatriceFamily_UpdateAttack, 2 CockatriceFamily_UpdateDeath (delete when nonzero), 3 only
 * animate; then Hit_CheckWithPlayer (not in state 2) and animate.
 */
void EnemyCockatriceUpdate(struct EwramData_EntityData *entity)
{
    struct HitResult result;

    Entity_HideAboveY(entity, -0x20);
    if (entity->state != 2)
    {
        if (EntityIsOutsideScreen(entity, 6))
            return;
        if (Enemy_IsStunned(entity))
        {
            Hit_CheckWithPlayer(&result, entity);
            return;
        }
    }
    switch (entity->state)
    {
        case 0:
            CockatriceFamily_UpdatePatrol(entity);
            break;
        case 1:
            CockatriceFamily_UpdateAttack(entity);
            break;
        case 2:
            if (CockatriceFamily_UpdateDeath(entity))
            {
                EntityDelete(entity);
                return;
            }
            break;
        case 3:
            Anim_Update(entity);
            return;
    }
    if (entity->state != 2)
        Hit_CheckWithPlayer(&result, entity);
    Anim_Update(entity);
}

/**
 * @brief BC1F8 | Basilisk create: CockatriceFamily_Create(entity, 1).
 */
void EnemyBasiliskCreate(struct EwramData_EntityData *entity)
{
    CockatriceFamily_Create(entity, 1);
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   EnemyBasiliskUpdate = EnemyCockatriceUpdate */

/**
 * @brief BC204 | Same as EnemyCockatriceUpdate.
 */
void EnemyBasiliskUpdate(struct EwramData_EntityData *entity)
{
    struct HitResult result;

    Entity_HideAboveY(entity, -0x20);
    if (entity->state != 2)
    {
        if (EntityIsOutsideScreen(entity, 6))
            return;
        if (Enemy_IsStunned(entity))
        {
            Hit_CheckWithPlayer(&result, entity);
            return;
        }
    }
    switch (entity->state)
    {
        case 0:
            CockatriceFamily_UpdatePatrol(entity);
            break;
        case 1:
            CockatriceFamily_UpdateAttack(entity);
            break;
        case 2:
            if (CockatriceFamily_UpdateDeath(entity))
            {
                EntityDelete(entity);
                return;
            }
            break;
        case 3:
            Anim_Update(entity);
            return;
    }
    if (entity->state != 2)
        Hit_CheckWithPlayer(&result, entity);
    Anim_Update(entity);
}

/* Death boss overlay part (DeathOverlay_Update): follows its owner, hidden while
 * the owner is in state 4 or flagged; deleted when the owner reaches state 7. */

void DeathOverlay_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = &gEwramData->entityData[(u8)entity->ext.deathHelper.ownerIndex];

    if (owner->state == 7)
        goto delete;
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sDeathGfx, sDeathPalette, 0, sDeathSpriteSheet))
        {
        delete:
            EntityDelete(entity);
            return;
        }
        entity->spriteLayer = owner->spriteLayer + 1;
        Anim_StartIndexed(entity, 0, 2, 1);
        entity->scrollWithBg = 1;
        entity->state = 1;
    case 1:
        if (owner->state == 4 || (owner->ext.death.flags & 0x80))
            entity->hidden = 1;
        else
            entity->hidden = 0;
        entity->posX.whole = owner->posX.whole;
        entity->posY.whole = owner->posY.whole;
        break;
    }
    entity->objMode = owner->objMode;
    { u8 flags = owner->palette; entity->palette = flags; }
    Anim_Update(entity);
}

/* Wall-crawling wheel: rolls around a block shedding debris, then homes in on its target while spinning. */

#define SPIN entity->ext.deathWeapon.angle
#define SPIN_SPEED entity->ext.deathWeapon.spin
#define SPEED entity->ext.deathWeapon.speed

static inline void Debris(struct EwramData_EntityData *entity, s16 dx, s16 dy, s32 vy, s32 vx)
{
    struct EwramData_EntityData *p;

    if (!(entity->timer & 1))
    {
        p = Effect_CreateSharedFrame(entity->posX.whole + (dx << 16), entity->posY.whole + (dy << 16), (RandomNumberGenerator() & 1) + 0x40, 6);
        if (p != NULL)
        {
            p->velY.whole = RandomSigned(0x8000) + vy;
            p->accelY.whole = 0x2000;
            p->velX.whole = RandomSigned(0x20000) + vx;
        }
    }
}

/**
 * @brief Move the weapon around the room collision surfaces with debris,
 * then home toward the supplied owner and decelerate its rotation.
 * Return nonzero when the existing rotation-angle completion test passes.
 */
s32 DeathWeapon_UpdateWallTraversal(struct EwramData_EntityData *entity, struct EwramData_EntityData *target)
{
    u16 *px = &entity->posX.part16.integer;
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    u16 x = *px + bg->xPos.part16.integer;
    u16 *py = &entity->posY.part16.integer;
    u16 y = *py + bg->yPos.part16.integer;

    switch (entity->stage)
    {
    case 0:
        SPEED = 0;
        SPIN_SPEED = 0;
        entity->stage = 1;
        entity->velY.whole = -0x18000;
        entity->velX.whole = 0;
        entity->accelX.whole = 0;
        break;
    case 1:
        {
        s16 c = Collision_GetPushDown((s16)x, (s16)y - 0x18);
        if (c != 0)
        {
            Sound_PlayTracked(0x105);
            entity->posY.part16.integer = c + entity->posY.part16.integer;
            entity->velY.whole = 0;
            entity->stage = 2;
        }
        }
        break;
    case 2:
        Debris(entity, 0, -0x18, -0x8000, 0);
        entity->velX.whole = SPEED;
        {
        s32 c = Collision_GetPushLeft((s16)x + 0x18, (s16)y);
        if (c != 0)
        {
            {
                u32 t = (u16)entity->posX.part16.integer;
                entity->posX.part16.integer = c + t;
            }
            entity->velX.whole = 0;
            entity->stage = 3;
        }
        }
        break;
    case 3:
        Debris(entity, 0x18, 0, -0x8000, -0x20000);
        entity->velY.whole = SPEED;
        {
            s16 sx = x;
            s16 sy = y;
            s32 c2, c = Collision_GetPushLeft(sx + 8, sy + 0x28);
            if (c != 0)
                {
                    u32 t = (u16)entity->posX.part16.integer;
                    entity->posX.part16.integer = c + t;
                }
            c2 = Collision_GetPushUp(sx, sy + 0x18);
            if (c2 != 0)
            {
                {
                    u32 t = (u16)entity->posY.part16.integer;
                    entity->posY.part16.integer = c2 + t;
                }
                entity->velY.whole = 0;
                entity->stage = 4;
            }
        }
        break;
    case 4:
        Debris(entity, 0, 0x18, -0x28000, 0);
        entity->velX.whole = -SPEED;
        {
        s32 c = Collision_GetPushRight((s16)x - 0x20, (s16)y);
        if (c != 0)
        {
            {
                u32 t = (u16)entity->posX.part16.integer;
                entity->posX.part16.integer = c + t;
            }
            entity->velX.whole = 0;
            entity->stage = 5;
        }
        }
        break;
    case 5:
        Debris(entity, -0x18, 0, -0x8000, 0x20000);
        entity->velY.whole = -SPEED;
        {
        s16 c = Collision_GetPushUp((s16)x, (s16)y - 0x20);
        if (c != 0)
        {
            Sound_StopTracked(0x105);
            entity->posY.part16.integer = c + entity->posY.part16.integer;
            entity->stage = 6;
            SPIN_SPEED = 0xF877;
        }
        }
        break;
    case 6:
    {
        s32 tx, ty, ex, ey;
        u32 dist;
        s32 speed;

        SPIN += SPIN_SPEED;
        if (!(entity->timer & 0xF))
            PlaySong(0x135);
        tx = target->posX.whole;
        ty = target->posY.whole - 0x260000;
        ex = entity->posX.whole;
        ey = entity->posY.whole;
        dist = Sqrt(((ex - tx) >> 16) * ((ex - tx) >> 16) + ((ey - ty) >> 16) * ((ey - ty) >> 16)) << 16;
        Entity_AccelerateAlongAngle(entity,
                     ArcTan2((tx < 0 ? -(-tx >> 16) : tx >> 16) - (s16)entity->posX.part16.integer,
                             (ty < 0 ? -(-ty >> 16) : ty >> 16) - (s16)entity->posY.part16.integer),
                     speed = dist >> 4, 0, 0, 0);
        entity->velX.whole = entity->posX.whole - ex;
        entity->velY.whole = entity->posY.whole - ey;
        if (speed <= 0x7FFF)
            entity->stage = 7;
        break;
    }
    case 7:
        SPIN += SPIN_SPEED;
        if (!(entity->timer & 0x1F))
            PlaySong(0x135);
        if (SPIN_SPEED < -0x200)
        {
            SPIN_SPEED += 0x20;
            break;
        }
        if ((u16)((SPIN & 0x7FFF) - 0x3001) <= 0x7FE)
            return 1;
        break;
    }
    if (entity->stage <= 5)
    {
        EntityAccelerateAndMove(entity, 0x70000);
        SPIN += SPIN_SPEED;
    }
    if (entity->stage > 1)
    {
        entity->timer++;
        if (entity->stage <= 5 && SPEED <= 0x6FFFF)
        {
            SPEED += 0x4000;
            SPIN_SPEED = -0x100 - SPEED / 0x60;
        }
    }
    return 0;
}
