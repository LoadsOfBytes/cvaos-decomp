/*
 * Fleaman.
 */
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"

/* EnemyFleamanUpdate. */
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *entity);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *entity);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *entity);
extern void Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);
extern void Fleaman_UpdateMovement(struct EwramData_EntityData *entity);
extern void Fleaman_UpdateDeath(struct EwramData_EntityData *entity);

/**
 * @brief C1F1C | Fleaman update. State 3: start the animation once, then
 * only animate. Otherwise (Enemy_UpdateHitFlash unless state 0xFF) skip the state
 * and only run Hit_CheckWithPlayer while Entity_IsTimeStopped says so or hitFlashTimer is set
 * (not in states 2 / 0xFF); states 1 Fleaman_UpdateMovement, 0xFF Fleaman_UpdateDeath.
 */
void EnemyFleamanUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[0x10];

    if (entity->state == 3)
    {
        switch (entity->subState)
        {
            case 0:
                Anim_StartIndexed(entity, 0, 0, 1);
                entity->subState++;
            case 1:
                Anim_Update(entity);
                break;
        }
        return;
    }
    if (entity->state != 0xFF)
        Enemy_UpdateHitFlash(entity);
    if (Entity_IsTimeStopped(entity) && entity->state != 0xFF)
    {
        Hit_CheckWithPlayer(collision, entity);
        return;
    }
    if (entity->hitFlashTimer && entity->state != 2 && entity->state != 0xFF)
    {
        Hit_CheckWithPlayer(collision, entity);
        return;
    }
    switch (entity->state)
    {
        case 1:
            Fleaman_UpdateMovement(entity);
            break;
        case 0xFF:
            Fleaman_UpdateDeath(entity);
            break;
    }
    Entity_AnimateWithBoxes(entity);
    if (entity->state != 0xFF)
        Hit_CheckWithPlayer(collision, entity);
}

extern s32 Collision_GetTileOnScreen(s32, s32);
extern s16 Collision_GetPushDown(s32, s32);
extern s16 Collision_GetPushLeft(s32, s32);
extern s16 Collision_GetPushRight(s32, s32);
extern s32 Entity_GetTargetDistanceX(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern s16 GetEntityRoomXPositionInteger(struct EwramData_EntityData *entity);
extern s16 GetEntityRoomYPositionInteger(struct EwramData_EntityData *entity);

/**
 * @brief Alive state (1): handle carried release, landing and randomly
 * selected hops toward the target, updating animation steps around
 * the apex and resolving terrain collisions. Preserve all substate fallthrough.
 */
void Fleaman_UpdateMovement(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *link = &gEwramData->entityData[entity->ext.altairDrop.spawnerIndex];
    u8 st;
    u16 x;
    u16 y;
    s32 xs;
    s32 ys;
    s32 d;

    switch (entity->subState)
    {
    case 0:
        entity->spriteLayer = 0x14;
        Anim_StartIndexed(entity, 0, 0, 1);
        entity->ext.fleaman.anim = 0;
        entity->animFlags |= 2;
        {
            s32 v = 0x8000;
            entity->velX.whole = v;
            if (!entity->flipX)
                entity->velX.whole = -v;
        }
        st = entity->ext.fleaman.airborne;
        if (st != 1)
            st = 4;
        entity->subState = st;
    case 1:
        entity->velY.whole = 0x2000;
        entity->accelY.whole = 0x2000;
        if (link != NULL)
            break;
        entity->subState++;
        break;
    case 2:
        if (entity->ext.fleaman.anim != 3)
        {
            Anim_StartIndexed(entity, 3, 0, 1);
            entity->ext.fleaman.anim = 3;
            entity->animTimer = 0;
            entity->animStep = 3;
            entity->animFlags |= 2;
        }
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        entity->velY.whole += entity->accelY.whole;
        if (Collision_GetTileOnScreen((s16)entity->posX.part16.integer, (s16)entity->posY.part16.integer))
        {
            Entity_SnapToGround(entity);
            entity->subState++;
            entity->timer = 0;
        }
        break;
    case 3:
        entity->animTimer = 0;
        entity->animStep = 0;
        entity->animFlags |= 2;
        if (++entity->timer > 0xF)
        {
            entity->subState++;
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->ext.fleaman.anim = 0;
        }
        break;
    case 4:
        entity->posX.whole += entity->velX.whole;
        if ((s16)Entity_GetTargetDistanceX(entity) > 0)
            entity->flipX = 0;
        else
            entity->flipX = 1;
        switch (RandomNumberGenerator() % 30)
        {
        default:
            entity->velX.whole = 0;
            entity->velY.whole = -0x30000;
            entity->accelY.whole = 0x5000;
            Anim_StartIndexed(entity, 3, 0, 1);
            entity->animTimer = 0;
            entity->animStep = 1;
            entity->animFlags |= 2;
            st = 5;
            break;
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
            entity->velX.whole = 0xD000;
            entity->velY.whole = -0x30000;
            entity->accelY.whole = 0xA000;
            Anim_StartIndexed(entity, 0, 0, 1);
            st = 6;
            break;
        case 10:
            entity->velX.whole = 0x1A000;
            entity->velY.whole = -0x30000;
            entity->accelY.whole = 0x2800;
            goto jump;
        case 11:
        case 12:
        case 13:
            entity->velX.whole = 0xD000;
            entity->velY.whole = -0x50000;
            entity->accelY.whole = 0x3C00;
        jump:
            Anim_StartIndexed(entity, 3, 0, 1);
            entity->ext.fleaman.anim = 3;
            entity->animFlags |= 2;
            st = 5;
            break;
        }
        entity->subState = st;
        if (!entity->flipX)
            entity->velX.whole = -entity->velX.whole;
        break;
    case 5:
    case 6:
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        entity->velY.whole += entity->accelY.whole;
        if (entity->subState == 5)
        {
            if (entity->velY.whole >= -entity->accelY.whole && entity->velY.whole <= entity->accelY.whole)
            {
                entity->animTimer = 0;
                entity->animStep = 2;
                entity->animFlags |= 2;
            }
            else if (entity->animStep == 0)
            {
                entity->animTimer = 0;
                entity->animStep = 1;
                entity->animFlags |= 2;
            }
            else if (entity->animStep == 2)
            {
                entity->animTimer = 0;
                entity->animStep = 3;
                entity->animFlags |= 2;
            }
        }
        y = GetEntityRoomYPositionInteger(entity);
        x = GetEntityRoomXPositionInteger(entity);
        xs = (s16)x;
        ys = (s16)y;
        d = Collision_GetPushDown(xs, ys - 10);
        if (d)
            { u32 t = (u16)entity->posY.part16.integer; entity->posY.part16.integer = d + t; }
        if (entity->velX.whole > 0)
        {
            d = Collision_GetPushLeft(xs + 8, ys - 8);
            if (d)
                { u32 t = (u16)entity->posX.part16.integer; entity->posX.part16.integer = d + t; }
        }
        if (entity->velX.whole < 0)
        {
            d = Collision_GetPushRight((s16)x - 8, (s16)y - 8);
            if (d)
                { u32 t = (u16)entity->posX.part16.integer; entity->posX.part16.integer = d + t; }
        }
        if (Collision_GetTileOnScreen((s16)entity->posX.part16.integer, (s16)entity->posY.part16.integer))
        {
            if (entity->subState == 5)
            {
                entity->animTimer = 0;
                entity->animStep = 0;
            }
            entity->subState = 4;
        }
        break;
    }
}

/* Fleaman_UpdateDeath (Fleaman death). */
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);
extern void Enemy_UpdateFragmentDeathWithSpread(struct EwramData_EntityData *entity, s32 interval, s32 width, s32 height);

/**
 * @brief C2388 | Death: sub-state 0 plays song 0x72, animation 2, palette
 * into 0x53E bits 3-6, timer 50, state 0xFF, no velocity, accelY 0x2000;
 * sub-state 1 decrements unk_4FD of the entity unk_4F6 (counter of the
 * spawner) when unk_4FD is set. Then Enemy_UpdateFragmentDeathWithSpread (fall).
 */
void Fleaman_UpdateDeath(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            PlaySong(0x72);
            Enemy_DropRewards(entity);
            Anim_StartIndexed(entity, 2, 0, 1);
            entity->ext.fleaman.anim = 2;
            entity->palette = entity->basePalette;
            entity->timer = 0x32;
            entity->state = 0xFF;
            entity->velX.whole = 0;
            entity->accelX.whole = 0;
            entity->velY.whole = 0;
            entity->accelY.whole = 0x2000;
            entity->subState = 1;
        case 1:
            if (entity->ext.altairDrop.count)
            {
                struct EwramData_EntityData *spawner = &gEwramData->entityData[entity->ext.altairDrop.spawnerIndex];

                spawner->ext.altairDrop.count--;
            }
            entity->subState = 2;
        case 2:
            break;
    }
    Enemy_UpdateFragmentDeathWithSpread(entity, 0xF, 0x80000, 0x60000);
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Fleaman_DamagePlayerOnContact = Enemy_DamagePlayerOnContact */

static inline void Entity_DamagePlayerOnContact(struct EwramData_EntityData *entity, u16 attributes, u16 unused, u16 reaction)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = attributes;
    contactDamage.unused = unused;
    contactDamage.reaction = reaction;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief C242C | Same as Enemy_DamagePlayerOnContact.
 */
void Fleaman_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/* Fleaman damage callback, used by EnemyFleamanCreate. */
extern u16 Player_CalculateAttackPower(u16 attackCategory);
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *entity, s32 other, u16 *hit, s32 inputDamage);
extern void Entity_ShowDamage(struct EwramData_EntityData *owner, s16 value, s32 kind);

/**
 * @brief Resolve and display damage with hit flash; subtract signed damage
 * from HP and begin the death helper immediately on defeat.
 */
void Fleaman_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
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
    Entity_ShowDamage(entity, damage, 0);
    if ((s16)entity->hp <= 0)
    {
        entity->subState = 0;
        Fleaman_UpdateDeath(entity);
    }
}
