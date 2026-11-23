/*
 * Ripper: body and projectiles.
 */
#include "core.h"
#include "graphics.h"
#include "structs/ewram.h"
#include "sound.h"
#include "gameplay.h"
#include "player_core.h"

extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, u8, void *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_EnableBoxes(struct EwramData_EntityData *, void *, void *);
extern void Enemy_OnContact(void);
void Ripper_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern u8 sFleamanRipperPalette[], sFleamanRipperSpriteSheet[], sFleamanRipperGfx[];

void EnemyRipperCreate(struct EwramData_EntityData *entity)
{
    void *palette = sFleamanRipperPalette;
    void *sprites = sFleamanRipperSpriteSheet;
    void *graphics = sFleamanRipperGfx;
    if (!Entity_LoadGraphics(entity, graphics, palette, 1, sprites))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 3;
        entity->stage = 0;
        entity->subState = 0;
        return;
    }
    entity->state = 1;
    entity->stage = 0;
    entity->subState = 0;
    if ((entity->spawnParam0 & entity->spawnParam1) == 0xFF)
        entity->ext.ripper.airborne = 1;
    else entity->ext.ripper.airborne = 0;
    Entity_EnableBoxes(entity, Enemy_OnContact, Ripper_OnHit);
    if ((u8)entity->ext.ripper.airborne != 1) Entity_SnapToGround(entity);
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   EnemyRipperUpdate = EnemyFleamanUpdate */

extern void Ripper_UpdateMovement(struct EwramData_EntityData *entity);
extern void Ripper_UpdateDeath(struct EwramData_EntityData *entity);
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *entity);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *entity);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *entity);
extern void Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);

/**
 * @brief C2558 | Same as EnemyFleamanUpdate.
 */
void EnemyRipperUpdate(struct EwramData_EntityData *entity)
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
            Ripper_UpdateMovement(entity);
            break;
        case 0xFF:
            Ripper_UpdateDeath(entity);
            break;
    }
    Entity_AnimateWithBoxes(entity);
    if (entity->state != 0xFF)
        Hit_CheckWithPlayer(collision, entity);
}

/* Hopping minion (sibling of Fleaman_UpdateMovement): jumps toward the player, flaps while airborne. */
extern s32 Collision_GetTileOnScreen(s32, s32);
extern s16 Collision_GetPushDown(s32, s32);
extern s16 Collision_GetPushLeft(s32, s32);
extern s16 Collision_GetPushRight(s32, s32);
extern s32 Entity_GetTargetDistanceX(struct EwramData_EntityData *);
extern struct EwramData_EntityData *Ripper_CreateProjectile(struct EwramData_EntityData *owner);
extern s16 GetEntityRoomXPositionInteger(struct EwramData_EntityData *entity);
extern s16 GetEntityRoomYPositionInteger(struct EwramData_EntityData *entity);

/**
 * @brief Alive state (1): handle carried release and random target-relative
 * hops. Near the jump apex, select a throwing animation and create
 * a projectile at its event. Preserve terrain tests and substate fallthrough.
 */
void Ripper_UpdateMovement(struct EwramData_EntityData *entity)
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
        entity->ext.ripper.anim = 0;
        {
            s32 v = 0x8000;
            entity->velX.whole = v;
            if (!entity->flipX)
                entity->velX.whole = -v;
        }
        st = entity->ext.ripper.airborne;
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
        if (entity->ext.ripper.anim != 3)
        {
            Anim_StartIndexed(entity, 3, 0, 1);
            entity->ext.ripper.anim = 3;
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
            entity->ext.ripper.anim = 0;
        }
        break;
    case 4:
        entity->posX.whole += entity->velX.whole;
        if ((s16)Entity_GetTargetDistanceX(entity) > 0)
            entity->flipX = 0;
        else
            entity->flipX = 1;
        switch (RandomNumberGenerator() % 6)
        {
        case 0: case 1: case 2: default:
            entity->velX.whole = 0xD000;
            entity->velY.whole = -0x30000;
            entity->accelY.whole = 0xA000;
            Anim_StartIndexed(entity, 0, 0, 1);
            st = 6;
            break;
        case 3:
            entity->velX.whole = 0x1A000;
            entity->velY.whole = -0x30000;
            entity->accelY.whole = 0x2800;
            Anim_StartIndexed(entity, 3, 0, 1);
            entity->ext.ripper.anim = 3;
            entity->animFlags |= 2;
            st = 5;
            break;
        case 4:
            entity->velX.whole = 0xD000;
            entity->velY.whole = -0x30000;
            entity->accelY.whole = 0x1400;
            Anim_StartIndexed(entity, 3, 0, 1);
            entity->ext.ripper.anim = 3;
            entity->animFlags |= 2;
            st = 5;
            break;
        }
        entity->subState = st;
        if (!entity->flipX)
            entity->velX.whole = -entity->velX.whole;
        {
            s32 dist = (s16)Entity_GetTargetDistanceX(entity);
            if (dist < 0)
                dist = -dist;
            if (dist <= 0x3F)
            {
                s16 p = entity->posX.part16.integer;
                if ((u32)(p - entity->velX.part16.integer - 1) <= 0xEE)
                    entity->velX.whole = -entity->velX.whole;
            }
        }
        break;
    case 5:
    case 6:
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        entity->velY.whole += entity->accelY.whole;
        if (entity->subState == 5)
        {
            if (entity->ext.ripper.anim == 3)
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
            if (entity->ext.ripper.anim == 1)
            {
                /* m doubles as the mask and the second reading (register allocation) */
                u16 *p = (u16 *)&entity->animStep;
                s32 m = 0xFFFF;
                s32 a = *p & m;
                if (a == 3)
                {
                    Anim_StartIndexed(entity, 3, 0, 1);
                    entity->ext.ripper.anim = a;
                }
                if ((m &= *p) == 2 && entity->ext.ripper.thrown != 0)
                {
                    Ripper_CreateProjectile(entity);
                    entity->ext.ripper.thrown = 0;
                }
            }
            if (entity->ext.ripper.anim == 3 && RandomNumberGenerator() % 3 == 0
                && (u32)(entity->velY.whole + 0x1FFF) <= 0x3FFE
                && entity->ext.ripper.thrown == 0)
            {
                entity->ext.ripper.thrown = 1;
                Anim_StartIndexed(entity, 1, 0, 0);
                entity->ext.ripper.anim = 1;
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

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Ripper_UpdateDeath = Fleaman_UpdateDeath */

extern void Enemy_DropRewards(struct EwramData_EntityData *entity);
extern void Enemy_UpdateFragmentDeathWithSpread(struct EwramData_EntityData *entity, s32 interval, s32 width, s32 height);

/**
 * @brief Death state (0xFF): drop rewards, restore the palette and update
 * the linked spawner count once, then run fragment death.
 */
void Ripper_UpdateDeath(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            PlaySong(0x72);
            Enemy_DropRewards(entity);
            Anim_StartIndexed(entity, 2, 0, 1);
            entity->ext.ripper.anim = 2;
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

/* Child projectile/hitbox creation. */
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, u8, void *);
extern void RipperProjectile_Update(struct EwramData_EntityData *);
void RipperProjectile_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void RipperProjectile_OnHit(struct EwramData_EntityData *entity);

/**
 * @brief Create and initialize the thrown projectile with owner pointer,
 * seven-eighths owner damage and hit callbacks. Return the child or NULL.
 * Preserve initialization after a failed graphics load and EntityDelete.
 */
struct EwramData_EntityData *Ripper_CreateProjectile(struct EwramData_EntityData *owner)
{
    /* the original loads the graphics pointers before the create call */
    u8 *gfx2 = sFleamanRipperPalette;
    u8 *gfx3 = sFleamanRipperSpriteSheet;
    u8 *gfx1 = sFleamanRipperGfx;
    struct EwramData_EntityData *e = EntityCreateInRange(0x1B, 0x48, (u32 *)RipperProjectile_Update);

    if (e != NULL)
    {
        if (!Entity_LoadGraphics(e, gfx1, gfx2, 1, gfx3))
            EntityDelete(e);
        e->ext.ripperProjectile.owner = owner;
        e->attack = owner->attack - (owner->attack >> 3);
        e->scrollWithBg = 1;
        e->animFrame = 0xB;
        Hit_Enable(e, 10, RipperProjectile_DamagePlayerOnContact, (s32)RipperProjectile_OnHit);
        HitBox_SetAttackRaw(e, 0x0808FCFC);
        HitBox_SetBodyRaw(e, 0x0808FCFC);
        e->hit.flags &= ~4;
        e->hit.flags &= ~2;
    }
    return e;
}

extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern s32 Entity_IsInRect(struct EwramData_EntityData *, s16, s16, u16, u16);

/**
 * @brief Place the projectile relative to its owner and accelerate it
 * horizontally. Hit state 5 falls; contact state 6 counts down to removal.
 * Keep collision checks during stun/time stop and the timer-underflow test.
 */
void RipperProjectile_Update(struct EwramData_EntityData *entity)
{
    u32 buf[4];
    struct EwramData_EntityData *owner = entity->ext.ripperProjectile.owner;

    Enemy_UpdateHitFlash(entity);
    if (Entity_IsTimeStopped(entity) == 0 || entity->state == 5 || entity->state == 6)
    {
        if (entity->hitFlashTimer == 0 || entity->state == 2 || entity->state == 5
            || entity->state == 6)
            goto run;
    }
    Hit_CheckWithPlayer(buf, entity);
    return;
run:
    switch (entity->state)
    {
    case 0:
        entity->state++;
    case 1:
        entity->velX.whole = 0x30000;
        entity->velY.whole = 0;
        entity->accelX.whole = 0x50;
        entity->accelY.whole = 0;
        entity->state++;
    case 2:
        entity->flipX = owner->flipX;
        if (entity->flipX)
            entity->posX.whole += 0xC0000 + owner->posX.whole;
        else
            entity->posX.whole += -0xC0000 + owner->posX.whole;
        entity->posY.whole += -0xC0000 + owner->posY.whole;
        entity->state++;
    case 3:
        if (!entity->flipX)
        {
            entity->velX.whole = -entity->velX.whole;
            entity->accelX.whole = -entity->accelX.whole;
        }
        PlaySong(0x89);
        entity->state++;
    case 4:
        entity->velX.whole += entity->accelX.whole;
        entity->posX.whole += entity->velX.whole;
        break;
    case 5:
        if (entity->subState == 0)
        {
            entity->accelX.whole = 0;
            entity->velX.whole = 0;
            entity->velY.whole = 0x8000;
            entity->accelY.whole = 0x1000;
            entity->subState++;
            entity->stage = 0;
        }
        entity->velY.whole += entity->accelY.whole;
        entity->posY.whole += entity->velY.whole;
        break;
    case 6:
        if (entity->subState == 0)
        {
            entity->accelY.whole = 0;
            entity->accelX.whole = 0;
            entity->velY.whole = 0;
            entity->timer = 5;
            entity->subState++;
            entity->stage = 0;
        }
        if (--entity->timer == 0xFF)
            entity->deleteRequested = 1;
        break;
    }
    Hit_CheckWithPlayer(buf, entity);
    if (!Entity_IsInRect(entity, -0x80, -0x40, 0x1F0, 0x120))
        entity->deleteRequested = 1;
}

/* RipperProjectile_DamagePlayerOnContact. */
/**
 * @brief Apply stored contact damage with zero effect type/flags, set hit
 * flash and hit flags 2/4, then enter timed removal state 6.
 */
void RipperProjectile_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0;
    contactDamage.unused = 0;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
    entity->hitFlashTimer = 8;
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
    entity->state = 6;
    entity->stage = 0;
    entity->subState = 0;
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Ripper_OnHit = Fleaman_OnHit */

extern u16 Player_CalculateAttackPower(u16 attackCategory);
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *entity, s32 other, u16 *hit, s32 inputDamage);
extern void Entity_ShowDamage(struct EwramData_EntityData *owner, s16 value, s32 kind);

/**
 * @brief Resolve and display damage with hit flash; subtract signed damage
 * from HP and begin the death helper immediately on defeat.
 */
void Ripper_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
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
        Ripper_UpdateDeath(entity);
    }
}

/* Ripper projectile hit response: enter falling state 5. */
/**
 * @brief Play the hit sound, set hit flash and flags 2/4, and enter falling
 * state 5 with substate counters reset. This callback belongs to the
 * Ripper projectile, not Ghost Dancer.
 */
void RipperProjectile_OnHit(struct EwramData_EntityData *entity)
{
    s32 zero;
    entity->hitFlashTimer=8;
    zero=0;
    PlaySong(0x13A);
    entity->hit.flags|=4;
    entity->hit.flags|=2;
    entity->state=5;
    entity->stage=zero;
    entity->subState=zero;
}
