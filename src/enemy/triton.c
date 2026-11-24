/*
 * Triton: body, spear, homing shots, wave particles and effects (entry points at the end of gargoyle.c).
 */
#include "triton.h"
#include "core.h"
#include "graphics.h"
#include "sound.h"
#include "structs/ewram.h"
#include "gameplay.h"
#include <string.h>
#include "syscalls.h"
#include "player_core.h"
#include "entity_common.h"
#include "entity_motion.h"
#include "hitbox.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "data/data_118B48.h"
#include "damage_numbers.h"
#include "entity_hit.h"

/* Triton movement (Triton_UpdateMove) and attack: fire the homing shot, or raise the spear and emit wave
 * particles until the spear comes back. */

void Triton_SpawnHomingShot(struct EwramData_EntityData *, s32, s32);
void Triton_SpawnWaveParticle(struct EwramData_EntityData *, s32, s32);
extern s32 Collision_FindWaterSurfaceOffset(s32, s32);
#define NEXT_STATE(e) (e->ext.triton.nextState)
#define ACCEL_X(e) ((e)->accelX.whole)
#define ACCEL_Y(e) ((e)->accelY.whole)

extern s32 Hit_ResolveDamage(struct EwramData_EntityData *entity, s32 other, u16 *hit, s32 inputDamage);

void TritonSpear_Update(struct EwramData_EntityData *);
void TritonSpear_DamagePlayerOnContact(struct EwramData_EntityData *);
void TritonSpear_SpawnEffect(struct EwramData_EntityData *, s32, s32);
void TritonHomingShot_DamagePlayerOnContact(struct EwramData_EntityData *);
void TritonHomingShot_OnHit(struct EwramData_EntityData *);
void TritonHomingShot_Update(struct EwramData_EntityData *entity);
void TritonWaveParticle_Update(struct EwramData_EntityData *entity);
void TritonEffect_Update(struct EwramData_EntityData *);

/* Sink slowly in water (tile type 3), faster elsewhere. */
/* Triton movement: wait for the target in range, walk/swim towards it,
 * turn around, and switch to the attack (state 1) when close enough. */
void Triton_UpdateMove(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    s32 flags;
    s32 range;
    switch (entity->subState)
    {
    case 0:
        entity->flipX = Entity_IsTargetRight(entity);
        NEXT_STATE(entity) = 0;
        flags = Entity_MoveAndCollide(entity, 0x1C0000, -0x1B0000, 0, 0x40000);
        if (!(flags & 0x80))
            entity->posY.part16.integer += Collision_FindWaterSurfaceOffset((s16)entity->posX.part16.integer, (s16)entity->posY.part16.integer - 27);
        range = 0x5A0000;
        goto check_range;
    case 1:
        entity->flipX = Entity_IsTargetRight(entity);
        if (entity->flipX) ACCEL_X(entity) = 0x400;
        else ACCEL_X(entity) = -0x400;
        flags = Entity_MoveAndCollide(entity, 0x1C0000, -0x1B0000, 0, 0x10000);
        if (!(flags & 0x80))
            entity->posY.part16.integer += Collision_FindWaterSurfaceOffset((s16)entity->posX.part16.integer, (s16)entity->posY.part16.integer - 27);
        break;
    case 10:
        NEXT_STATE(entity) = 10;
        if (entity->flipX != Entity_IsTargetRight(entity)) goto turn;
        range = 0x640000;
    check_range:
        if (Target_IsWithinDistance(entity, range) || Enemy_IsDamaged(entity)) entity->subState++;
        break;
    case 11:
        if (entity->flipX != Entity_IsTargetRight(entity))
        {
        turn:
            entity->subState = 12;
            break;
        }
        if (entity->flipX) ACCEL_X(entity) = 0x400;
        else ACCEL_X(entity) = -0x400;
        flags = Entity_MoveWithCollision(entity, 0x10000, 0, 0x8000, 32);
        if (entity->timer++ > 59)
        {
            entity->timer = 0;
            if (Target_IsWithinDistance(entity, 0x640000) && RandomNumberGenerator() % 128 < 96 && (flags & 4))
            {
                entity->state = 1;
                entity->subState = 0;
                break;
            }
        }
        /* sink slowly in water (tile type 3), faster elsewhere */
        if (Tile_Classify((s16)(camera->xPos.part16.integer + entity->posX.part16.integer),
                         (s16)(camera->yPos.part16.integer + entity->posY.part16.integer)) == 3)
            ACCEL_Y(entity) = 0x800;
        else
            ACCEL_Y(entity) = 0x2000;
        break;
    case 12:
        Anim_StartIndexed(entity, 5, 0, 0);
        entity->subState++;
        ACCEL_X(entity) = 0;
        ACCEL_Y(entity) = 0;
        EntityDampVelocityX(entity, 0x10);
        EntityDampVelocityY(entity, 0x10);
        Entity_MoveWithCollision(entity, 0x10000, 0, 0x8000, 32);
        /* sink slowly in water (tile type 3), faster elsewhere */
        if (Tile_Classify((s16)(camera->xPos.part16.integer + entity->posX.part16.integer),
                         (s16)(camera->yPos.part16.integer + entity->posY.part16.integer)) == 3)
            ACCEL_Y(entity) = 0x800;
        else
            ACCEL_Y(entity) = 0x2000;
        break;
    case 13:
        EntityDampVelocityX(entity, 0x10);
        EntityDampVelocityY(entity, 0x10);
        Entity_MoveWithCollision(entity, 0x10000, 0, 0x8000, 32);
        if (entity->animDone)
        {
            Anim_StartIndexed(entity, 1, 0, 1);
            entity->flipX = ~entity->flipX;
            entity->subState = 11;
        }
        /* sink slowly in water (tile type 3), faster elsewhere */
        if (Tile_Classify((s16)(camera->xPos.part16.integer + entity->posX.part16.integer),
                         (s16)(camera->yPos.part16.integer + entity->posY.part16.integer)) == 3)
            ACCEL_Y(entity) = 0x800;
        else
            ACCEL_Y(entity) = 0x2000;
        break;
    }
}

#define FRAME_PAIR(e) (*(u16 *)&(e)->animStep)
#define SPEAR_FLAG(e) (*(u8 *)&e->ext.triton.thrust)
/**
 * @brief Attack state machine (state 1): randomly thrust the spear, fire a homing
 * shot (Triton_SpawnHomingShot) or send wave particles
 * (Triton_SpawnWaveParticle); then back to state 0.
 */
void Triton_UpdateAttack(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *spear = entity->ext.triton.spear;
    switch (entity->subState)
    {
    case 0:
        Entity_MoveWithCollision(entity, 0x10000, 0, 0x8000, 32);
        if (RandomNumberGenerator() % 128 < 32)
        {
            entity->subState = 10;
            entity->accelX.whole = 0;
            entity->accelY.whole = 0;
        }
        else
            entity->subState++;
        break;
    case 1:
        Entity_MoveWithCollision(entity, 0x10000, 0, 0x8000, 32);
        if (FRAME_PAIR(entity) == 1)
        {
            if (entity->flipX) Triton_SpawnHomingShot(entity, -0xC0000, -0xA0000);
            else Triton_SpawnHomingShot(entity, 0xE0000, -0xA0000);
            entity->subState = 2;
        }
        break;
    case 2:
        Entity_MoveWithCollision(entity, 0x10000, 0, 0x8000, 32);
        goto finish;
    case 10:
        EntityDampVelocityX(entity, 0x20);
        EntityDampVelocityY(entity, 0x20);
        Entity_MoveWithCollision(entity, 0x10000, 0, 0x8000, 32);
        if (FRAME_PAIR(entity) == 0)
        {
            Anim_StartIndexed(entity, 3, 0, 0);
            entity->timer = 0;
            entity->subState++;
        }
        break;
    case 11:
        EntityDampVelocityX(entity, 0x20);
        EntityDampVelocityY(entity, 0x20);
        Entity_MoveWithCollision(entity, 0x10000, 0, 0x8000, 32);
        if (entity->animDone)
        {
            Anim_StartIndexed(entity, 2, 0, 1);
            entity->timer = 0;
            entity->subState++;
            PlaySong(0x8A);
        }
        break;
    case 12:
        EntityDampVelocityX(entity, 0x20);
        EntityDampVelocityY(entity, 0x20);
        Entity_MoveWithCollision(entity, 0x10000, 0, 0x8000, 32);
        if (entity->timer++ > 29)
        {
            entity->ext.triton.thrust = 1;
            spear->ext.triton.thrust = 0;
            entity->subState++;
        }
        break;
    case 13:
        EntityDampVelocityX(entity, 0x20);
        EntityDampVelocityY(entity, 0x20);
        Entity_MoveWithCollision(entity, 0x10000, 0, 0x8000, 32);
        entity->timer++;
        if ((entity->timer & 0x1F) == 0)
        {
            PlaySong(0x8A);
            if (entity->flipX) Triton_SpawnWaveParticle(entity, 0xE0000, -0x3A0000);
            else Triton_SpawnWaveParticle(entity, -0xC0000, -0x3A0000);
        }
        if (SPEAR_FLAG(spear))
        {
            SPEAR_FLAG(entity) = 0;
            Anim_StartIndexed(entity, 4, 0, 0);
            entity->subState++;
        }
        break;
    case 14:
        EntityDampVelocityX(entity, 0x20);
        EntityDampVelocityY(entity, 0x20);
        Entity_MoveWithCollision(entity, 0x10000, 0, 0x8000, 32);
        if (entity->animDone)
        {
            Anim_StartIndexed(entity, 1, 0, 1);
        finish:
            entity->state = 0;
            entity->subState = NEXT_STATE(entity);
            entity->timer = 0;
        }
        break;
    }
}

/* Triton_OnHit (hit with HP / death), generated with a template script from the
 * template of WingedSkeleton_OnHit and checked byte for byte. */

/**
 * @brief 87138 | Hit: flash (hitFlashTimer = 8), subtract the damage from the HP
 * and show it; at HP <= 0 die (Enemy_DropRewards, song 0x6d, animation 0, palette into
 * 0x53E bits 3-6, timer 0x28, state 2). Same template as WingedSkeleton_OnHit.
 */
void Triton_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s32 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));

    entity->hitFlashTimer = 8;
    {
        u16 hp = entity->hp;

        // (s16)damage, written so that agbcc sign-extends after the HP load
        damage <<= 16;
        damage >>= 16;
        entity->hp = hp - damage;
    }
    Entity_ShowDamage(entity, (s16)damage, 0);
    if ((s16)entity->hp <= 0)
    {
        Enemy_DropRewards(entity);
        PlaySong(0x6D);
        Anim_StartIndexed(entity, 0, 0, 0);
        entity->palette = entity->basePalette;
        entity->timer = 0x28;
        entity->state = 2;
        entity->subState = 0;
    }
}

/* Triton spear creation. */
#define PART_ANGLE(e) (e->ext.tritonSpear.angle)
/* Create the Triton's spear (TritonSpear_Update) with its own affine slot,
 * hit box -3, -15, 6x6 and contact callback TritonSpear_DamagePlayerOnContact. */
struct EwramData_EntityData *Triton_CreateSpear(struct EwramData_EntityData *owner)
{
    struct SpriteBox box;
    struct EwramData_EntityData *part = Entity_CreateObject(TritonSpear_Update);
    if (part != NULL)
    {
        s32 graphics;
        u16 slot;
        part->posX.whole = owner->posX.whole;
        part->posY.whole = owner->posY.whole;
        part->spriteLayer = owner->spriteLayer - 1;
        graphics = ObjGfx_Find(sTritonGfx);
        Sprite_Attach(part, sTritonSpriteSheet, graphics, owner->basePalette);
        Anim_StartIndexed(part, 10, 0, 0);
        Anim_Update(part);
        if ((s32)((slot = ObjAffine_Alloc(1)) << 16) < 0)
        {
            EntityDelete(part);
            return; /* the original leaves the return value undefined here */
        }
        part->affineIndex = slot;
        part->affine = 1;
        PART_ANGLE(part) = 0x1C000;
        ObjAffine_SetBiased(PART_ANGLE(part), 0x10000, 0x10000, part->flipX, part->flipY, part->affineIndex);
        part->attack = owner->attack;
        if (!gEwramData->gameState.bestiaryPreview)
        {
            s32 zero = 0;
            box.x = -3;
            box.y = -15;
            box.width = 6;
            box.height = 6;
            HitBox_SetAttack(part, &box);
            /* an OR with a zero variable survives in the original */
            part->hit.flags |= 4;
            part->hit.flags |= zero;
            part->hit.flags &= ~2;
            Hit_Enable(part, 8, TritonSpear_DamagePlayerOnContact, 0);
        }
        part->ext.tritonSpear.owner = owner;
        part->scrollWithBg = 0;
        part->drawFunc = Sprite_DrawEntity;
    }
    return part;
}

#define PART_TARGET_ANGLE(e) ((e)->accelX.whole)
#define PIXEL(v) ((v) < 0 ? -((-(v)) >> 16) : (v) >> 16)
/* Triton spear update. Angle targets are derived from a point 24 pixels below the spear. */
void TritonSpear_Update(struct EwramData_EntityData *spear)
{
    struct EwramData_EntityData *owner = spear->ext.tritonSpear.owner;
    struct HitResult result;
    s8 offsets[32];
    struct SpriteBox box;
    s32 x, y;
    s32 angle;

    memcpy(offsets, sTritonSpearOffsets, sizeof(offsets));
    if (!owner) return;
    if (!owner->updateFunc || owner->enemyId != 0x43) goto fail;
    if (spear->state == 0 && owner->state == 2)
    {
        spear->state = 1;
        spear->scrollWithBg = 1;
        spear->timer = 41;
    }
restart:
    if (spear->state == 0)
    {
        switch (spear->subState)
        {
        case 0:
        {
            s32 ownerX = owner->posX.whole;
            spear->posX.whole = ownerX;
            spear->posY.whole = owner->posY.whole + (offsets[owner->animFrame] << 16);
            if (owner->flipX)
            {
                if (owner->animFrame == 13) spear->posX.whole = ownerX + 0xC0000;
                else spear->posX.whole = ownerX + 0x140000;
            }
            else
            {
                if (owner->animFrame == 13) spear->posX.whole = ownerX - 0xB0000;
                else spear->posX.whole = ownerX - 0x130000;
            }
            if (*(u8 *)&owner->ext.triton.thrust)
            {
                spear->subState = 1;
                spear->timer = 0;
                spear->scrollWithBg = 1;
            }
            break;
        }
        case 1:
            if (!Entity_IsTimeStopped(owner))
            {
                spear->posY.whole -= 0x18000;
                if (spear->posY.whole <= 0x200000)
                {
                    spear->subState++;
                    spear->timer = 0;
                    PlaySong(0x86);
                }
            }
            break;
        case 2:
            if (!Entity_IsTimeStopped(owner))
            {
                if (owner->flipX)
                {
                    PART_ANGLE(spear) -= 0x1000;
                    if (PART_ANGLE(spear) <= 0xC000)
                    {
                        PlaySong(0x86);
                        PART_ANGLE(spear) += 0x10000;
                        if (++spear->timer > 2)
                        {
                            spear->subState++;
                            spear->timer = 0;
                            x = Target_GetX(spear) - spear->posX.whole;
                            {
                                s32 target = Target_GetY(spear);
                                s32 base = spear->posY.whole + 0x180000;
                                y = target - base;
                            }
                            angle = (u16)ArcTan2(x < 0 ? (s16)(-((-x) >> 16)) : x >> 16,
                                                 y < 0 ? (s16)(-((-y) >> 16)) : y >> 16);
                            PART_TARGET_ANGLE(spear) = angle + 0x10000;
                        }
                    }
                }
                else
                {
                    PART_ANGLE(spear) += 0x1000;
                    if (PART_ANGLE(spear) > 0x2BFFF)
                    {
                        PlaySong(0x86);
                        PART_ANGLE(spear) -= 0x10000;
                        if (++spear->timer > 2)
                        {
                            spear->subState++;
                            spear->timer = 0;
                            x = Target_GetX(spear) - spear->posX.whole;
                            {
                                s32 target = Target_GetY(spear);
                                s32 base = spear->posY.whole + 0x180000;
                                y = target - base;
                            }
                            angle = (u16)ArcTan2(x < 0 ? (s16)(-((-x) >> 16)) : x >> 16,
                                                 y < 0 ? (s16)(-((-y) >> 16)) : y >> 16);
                            PART_TARGET_ANGLE(spear) = angle + 0x20000;
                        }
                    }
                }
            }
            break;
        case 3:
            if (!Entity_IsTimeStopped(owner))
            {
                if (owner->flipX)
                {
                    PART_ANGLE(spear) -= 0x1000;
                    if (PART_ANGLE(spear) <= PART_TARGET_ANGLE(spear)) { PART_ANGLE(spear) = PART_TARGET_ANGLE(spear); spear->subState++; }
                }
                else
                {
                    PART_ANGLE(spear) += 0x1000;
                    if (PART_ANGLE(spear) >= PART_TARGET_ANGLE(spear))
                    {

                        PART_ANGLE(spear) = PART_TARGET_ANGLE(spear);
                        spear->subState++;
                    }
                }
            }
            break;
        case 4:
            if (!Entity_IsTimeStopped(owner))
            {
                if (++spear->timer == 11)
                {
                    angle = PART_ANGLE(spear);
                    x = Sin(angle + 0x4000);
                    y = Sin(angle);
                    angle = 0xB0000;
                    x = FixedMultiply(x, angle);
                    y = FixedMultiply(y, angle);
                    TritonSpear_SpawnEffect(spear, x, y);
                }
                if (spear->timer > 59)
                {
                    spear->subState++;
                    angle = PART_ANGLE(spear);
                    x = Sin(angle + 0x4000);
                    y = Sin(angle);
                    angle = 0x40000;
                    spear->velX.whole = FixedMultiply(x, angle);
                    spear->velY.whole = FixedMultiply(y, angle);
                    PlaySong(0x85);
                }
            }
            break;
        case 5:
            if (!Entity_IsTimeStopped(owner))
            {
                EntityMove(spear);
                if (EntityIsOutsideScreen(spear, 2))
                {
                    s32 ownerX;
                    spear->subState++;
                    ownerX = owner->posX.whole;
                    spear->posX.whole = ownerX;
                    if (owner->flipX) spear->posX.whole = ownerX + 0x140000;
                    else spear->posX.whole = ownerX - 0x130000;
                    spear->posY.whole = 0xF00000;
                    PART_ANGLE(spear) = 0x1C000;
                }
            }
            break;
        case 6:
            if (!Entity_IsTimeStopped(owner))
            {
                s32 ownerX = owner->posX.whole;
                spear->posX.whole = ownerX;
                spear->posX.whole = ownerX + (owner->flipX ? 0x140000 : -0x130000);
                spear->posY.whole -= 0x18000;
                if (owner->posY.whole - 0x300000 >= spear->posY.whole)
                {
                    /* Keep the shared zero to reproduce the original stores. */
                    s32 zero = 0;
                    *(u8 *)&spear->ext.triton.thrust = 1;
                    *(u8 *)&owner->ext.triton.thrust = zero;
                    spear->subState = zero;
                    spear->scrollWithBg = 0;
                    goto restart;
                }
            }
            break;
        }
    }
    angle = PART_ANGLE(spear);
    x = Sin(angle + 0x4000);
    y = Sin(angle);
    angle = 0xC0000;
    x = FixedMultiply(x, angle);
    y = FixedMultiply(y, angle);
    if (!gEwramData->gameState.bestiaryPreview)
    {
        box.x = PIXEL(x) - 3;
        box.y = PIXEL(y) - 3;
        box.width = 6;
        box.height = 6;
        HitBox_SetAttack(spear, &box);
    }
    /* Reuse angle here: a separate expression changes agbcc register allocation. */
    angle = PART_ANGLE(spear) - 0xC000;
    ObjAffine_SetBiased(angle, 0x10000, 0x10000, spear->flipX, spear->flipY, spear->affineIndex);
    if (spear->state == 1)
    {
        spear->hidden = Entity_GetVisibilityPattern(spear->timer--);
        if (owner->state == 3)
        {
            spear->state = 2;
            spear->timer = 4;
        }
    }
    if (spear->state == 2)
    {
        if ((s8)spear->timer-- == 0)
        {
            Effect_SpawnAt(spear->posX.whole, spear->posY.whole, spear->spriteLayer - 1);
            spear->deleteRequested = 1;
            ObjAffine_Free(spear->affineIndex);
        }
    }
    if (spear->state == 0 && !gEwramData->gameState.bestiaryPreview)
        Hit_CheckWithPlayer(&result, spear);
    return;
fail:
    EntityDelete(spear);
}

/* Triton attacks: homing shot and wave particles. */
#define SHOT_ACCEL(e) ((e)->accelX.whole)
#define SHOT_MAX_SPEED(e) ((e)->accelY.whole)
#define WAVE_ANGLE(e) (e->ext.tritonWave.angle)
#define WAVE_AGE(e) (e->ext.tritonWave.age)

/* Create the homing shot (TritonHomingShot_Update) at the owner + (x, y), starting
 * towards the side the target is on. */
void Triton_SpawnHomingShot(struct EwramData_EntityData *owner, s32 x, s32 y)
{
    struct EwramData_EntityData *shot = EntityCreateInRange(0x49, 0xCF, TritonHomingShot_Update);
    struct SpriteBox box = {-2, -2, 4, 4};
    if (shot != NULL)
    {
        s32 graphics;
        shot->posX.whole = owner->posX.whole + x;
        shot->posY.whole = owner->posY.whole + y;
        shot->flipX = Entity_IsTargetRight(shot);
        shot->velX.whole = shot->flipX ? 0x8000 : -0x8000;
        shot->velY.whole = 0;
        SHOT_ACCEL(shot) = 0x800;
        SHOT_MAX_SPEED(shot) = 0xC000;
        shot->spriteLayer = owner->spriteLayer - 1;
        graphics = ObjGfx_Find(sTritonGfx);
        shot->basePalette = 7;
        Sprite_Attach(shot, sTritonSpriteSheet, graphics, 7);
        Anim_StartIndexed(shot, 7, 0, 0);
        Anim_Update(shot);
        Hit_Enable(shot, 8, TritonHomingShot_DamagePlayerOnContact, TritonHomingShot_OnHit);
        HitBox_SetAttack(shot, &box);
        HitBox_SetBody(shot, &box);
        shot->hit.flags &= ~4;
        shot->hit.flags &= ~2;
        shot->attack = owner->attack * 3 / 4;
        shot->ext.tritonShot.owner = owner;
        shot->scrollWithBg = 1;
        shot->drawFunc = Sprite_DrawEntity;
    }
}
/* Homing shot: after 50 frames accelerate towards the target (at most
 * SHOT_MAX_SPEED horizontally and half of it vertically); despawn after 180
 * frames. */
void TritonHomingShot_Update(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    if (entity->animScriptIndex == 7 && entity->animDone)
        Anim_StartIndexed(entity, 8, 0, 1);
    if (entity->timer > 49)
    {
        s32 x = Target_GetX(entity) - entity->posX.whole;
        s32 y;
        {
            s32 target = Target_GetY(entity);
            s32 base = entity->posY.whole + 0x100000;
            y = target - base;
        }
        if (x < -0x10000)
        {
            entity->velX.whole -= SHOT_ACCEL(entity);
            if (entity->velX.whole <= -SHOT_MAX_SPEED(entity))
                entity->velX.whole = -SHOT_MAX_SPEED(entity);
        }
        else if (x > 0x10000)
        {
            entity->velX.whole += SHOT_ACCEL(entity);
            if (entity->velX.whole >= SHOT_MAX_SPEED(entity))
                entity->velX.whole = SHOT_MAX_SPEED(entity);
        }
        if (y < -0x10000)
        {
            entity->velY.whole -= SHOT_ACCEL(entity);
            if (entity->velY.whole <= -(SHOT_MAX_SPEED(entity) / 2))
                entity->velY.whole = -(SHOT_MAX_SPEED(entity) / 2);
        }
        else if (y > 0x10000)
        {
            entity->velY.whole += SHOT_ACCEL(entity);
            if (entity->velY.whole >= SHOT_MAX_SPEED(entity) / 2)
                entity->velY.whole = SHOT_MAX_SPEED(entity) / 2;
        }
    }
    EntityMove(entity);
    Anim_Update(entity);
    if (entity->timer++ > 179)
        EntityDelete(entity);
    else if (entity->timer & 1)
        Hit_CheckWithPlayer(&result, entity);
}

/* Create the wave particle (TritonWaveParticle_Update) at the owner + (x, y), its
 * starting angle depending on the owner's facing. */
void Triton_SpawnWaveParticle(struct EwramData_EntityData *owner, s32 x, s32 y)
{
    struct EwramData_EntityData *effect = EntityCreateInRange(0x49, 0xCF, TritonWaveParticle_Update);
    if (effect != NULL)
    {
        s32 graphics;
        effect->posX.whole = owner->posX.whole + x;
        effect->posY.whole = owner->posY.whole + y;
        effect->spriteLayer = owner->spriteLayer - 1;
        graphics = ObjGfx_Find(sTritonGfx);
        Sprite_Attach(effect, sTritonSpriteSheet, graphics, 6);
        Anim_StartIndexed(effect, 6, 0, 1);
        Anim_Update(effect);
        effect->flipX = 0;
        effect->scrollWithBg = 1;
        effect->drawFunc = Sprite_DrawEntity;
        if (owner->flipX)
        {
            WAVE_ANGLE(effect) = 0x1F000;
            effect->timer = 24;
        }
        else
        {
            WAVE_ANGLE(effect) = 0x19000;
            effect->timer = 8;
        }
    }
}
/* Wave particle: swing the angle back and forth while moving, blink out
 * and despawn after 60 frames or off screen. */
void TritonWaveParticle_Update(struct EwramData_EntityData *entity)
{
    s32 bounds[4];
    if (entity->state == 0)
    {
        u32 time = (u8)++entity->timer;
        if (time <= 15)
            WAVE_ANGLE(entity) += 0x400;
        else if (time <= 30)
        {
            WAVE_ANGLE(entity) -= 0x400;
            if (time > 29) entity->timer = 0;
        }
        EntitySetVelocityFromAngle(entity, WAVE_ANGLE(entity), 0x10000);
        EntityMove(entity);
        entity->hidden = Entity_GetVisibilityPattern(60 - WAVE_AGE(entity));
        if (WAVE_AGE(entity)++ > 59)
            EntityDelete(entity);
        else if (EntityIsOutsideScreen(entity, 1))
            EntityDelete(entity);
    }
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   TritonSpear_DamagePlayerOnContact = DevilPart_DamagePlayerOnContact */

/**
 * @brief 87C24 | Same as DevilPart_DamagePlayerOnContact.
 */
void TritonSpear_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 1;
    contactDamage.unused = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/* Homing shot contact: Player_ApplyContactDamage at the position and set
 * timer to 0xFA. */
void TritonHomingShot_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage effect;
    effect.attack = entity->attack;
    effect.attributes = 0;
    effect.unused = 0;
    effect.reaction = 0;
    effect.sourceX = entity->posX.whole;
    effect.sourceY = entity->posY.whole;
    entity->timer = 0xFA;
    Player_ApplyContactDamage(&effect);
}
/* Only set timer to 0xFA (callbacks registered with TritonHomingShot_DamagePlayerOnContact). */
void TritonHomingShot_OnHit(struct EwramData_EntityData *entity)
{
    entity->timer = 0xFA;
}
/* Create the effect entity TritonEffect_Update at the owner + (x, y), one layer
 * below, facing the same way. */
void TritonSpear_SpawnEffect(struct EwramData_EntityData *owner, s32 x, s32 y)
{
    struct EwramData_EntityData *effect = EntityCreateInRange(0x49, 0xCF, TritonEffect_Update);
    if (effect != NULL)
    {
        s32 graphics;
        effect->posX.whole = owner->posX.whole + x;
        effect->posY.whole = owner->posY.whole + y;
        effect->spriteLayer = owner->spriteLayer - 1;
        graphics = ObjGfx_Find(sTritonGfx);
        Sprite_Attach(effect, sTritonSpriteSheet, graphics, 6);
        Anim_StartIndexed(effect, 9, 0, 0);
        Anim_Update(effect);
        effect->flipX = owner->flipX;
        effect->scrollWithBg = 1;
        effect->drawFunc = Sprite_DrawEntity;
    }
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Triton_LoadGraphics = Object25_LoadGraphics
 *   Triton_DamagePlayerOnContact = Enemy_DamagePlayerOnContact
 *   TritonEffect_Update = GolemIdleEffect_Update */

/**
 * @brief 87D24 | Same as Object25_LoadGraphics.
 */
u8 Triton_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sTritonGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sTritonPalette, 0);
    Sprite_Attach(entity, sTritonSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 87D70 | Same as Enemy_DamagePlayerOnContact.
 */
void Triton_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/**
 * @brief 87D9C | Same as GolemIdleEffect_Update.
 */
void TritonEffect_Update(struct EwramData_EntityData *entity)
{
    Anim_Update(entity);
    if (entity->animDone)
        EntityDelete(entity);
}
