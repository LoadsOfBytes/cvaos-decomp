/*
 * Witch: cast, charge and death states, entry points and hit callbacks (also of Student Witch and the witch ground projectile). Kept apart from witch.c: it calls Witch_SpawnOrbitParticles with an s16 phase (u16 in the definition).
 */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
#include "code/code_08040A38.h"
#include "code/code_080211F0.h"

/* Witch_UpdateCast. */
extern struct EwramData_EntityData *Entity_CreateObject(u32 *update);
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Target_GetX(struct EwramData_EntityData *);
extern s32 Target_GetY(struct EwramData_EntityData *);
extern s32 Target_GetPixelX(struct EwramData_EntityData *entity);
extern s32 Target_IsNearEntity(struct EwramData_EntityData *, s32, s32);
extern void EntityDampVelocityX(struct EwramData_EntityData *entity, s32 factor);
extern void EntityDampVelocityY(struct EwramData_EntityData *entity, s32 factor);
extern void EntityMoveX(struct EwramData_EntityData *entity);
extern void EntityMoveY(struct EwramData_EntityData *entity);
extern void Witch_SpawnOrbitParticles(struct EwramData_EntityData *entity, s16 phase);
extern void WitchCastEffect_Update(struct EwramData_EntityData *entity);
extern void WitchSpell_Update(struct EwramData_EntityData *entity);
extern void Witch_SpawnMovementParticle(struct EwramData_EntityData *entity);

static inline s32 ABS(s32 x)
{
    if (x < 0)
        x = -x;
    return x;
}
static inline u8 Near(s32 dx, s32 dy)
{
    u8 r;
    if (ABS(dx) <= 0x3FFFF && ABS(dy) <= 0x3FFFF)
        r = 1;
    else
        r = 0;
    return r;
}
#define ABSM(x) ((x) < 0 ? -(x) : (x))
#define TIMER entity->ext.witchCast.timer
#define STEER(f, d) \
    if ((d) > 0x40000) \
        f = 0x1000; \
    else if ((d) < -0x40000) \
        f = -0x1000; \
    else \
        f = 0;

static inline s32 Home(struct EwramData_EntityData *entity, s32 dx, s32 dy)
{
    STEER(entity->accelX.whole, dx)
    STEER(entity->accelY.whole, dy)
    EntityDampVelocityX(entity, 0x20);
    EntityDampVelocityY(entity, 0x20);
    EntityAccelerateAndMove(entity, 0x28000);
    if (({ register s32 t asm("r2") = dx; if (t < 0) t = -t; t; }) <= 0x3FFFF && ABSM(dy) <= 0x3FFFF) /* FAKEMATCH: pinned register */
        return 1;
    return 0;
}

static inline u8 PlayerIsRight(struct EwramData_EntityData *e)
{
    s16 *x = &e->posX.part16.integer;
    if (*x >= (s16)Target_GetPixelX(e))
        return 0;
    return 1;
}

#define SPAWN(func) \
    child = Entity_CreateObject((u32 *)func); \
    if (child != NULL) \
    { \
        child->ext.witchSpell.ownerIndex = ENTITY_INDEX(entity); \
        child->scrollWithBg = 1; \
        child->flipX = entity->flipX; \
        child->attack = entity->attack; \
    }

/**
 * @brief Cast state (1): approach above the target, turn, pause, then create
 * the attached effect, orbit particles and spell before returning to wait.
 * Keep the pinned register in the Home helper required by the matching source.
 */
void Witch_UpdateCast(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child;
    s32 dx, dy;

    switch (entity->subState)
    {
    case 0:
        TIMER = 0;
        entity->velX.whole = 0;
        entity->velY.whole = 0;
        entity->subState = 1;
    case 1:
        TIMER++;
        dx = (-(Entity_IsTargetRight(entity) << 6) + 0x20) << 16;
        dy = (Sin(TIMER << 9) << 3) - 0x300000;
        entity->ext.witch.moveX = Target_GetX(entity) + dx;
        entity->ext.witch.moveY = Target_GetY(entity) + dy;
        dx = entity->ext.witch.moveX - entity->posX.whole;
        dy = entity->ext.witch.moveY - entity->posY.whole;
        if (Home(entity, dx, dy) || Target_IsNearEntity(entity, 0x50, 0x36))
        {
            entity->timer = 0x20;
            entity->subState = 3;
            entity->stage = 0;
            break;
        }
        if (entity->flipX != Entity_IsTargetRight(entity) && ABSM((s16)Target_GetPixelX(entity) - (s16)entity->posX.part16.integer) > 8)
        {
            entity->flipX ^= 1;
            Anim_StartIndexed(entity, 1, 0, 0);
            entity->subState = 2;
            break;
        }
        if (TIMER & 1)
            Witch_SpawnMovementParticle(entity);
        break;
    case 2:
        dx = (-(Entity_IsTargetRight(entity) << 6) + 0x20) << 16;
        entity->ext.witch.moveX = Target_GetX(entity) + dx;
        entity->ext.witch.moveY = Target_GetY(entity) - 0x380000;
        dx = entity->ext.witch.moveX - entity->posX.whole;
        Home(entity, dx, entity->ext.witch.moveY - entity->posY.whole);
        if (entity->animFlags & 4)
        {
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->subState = 1;
        }
        break;
    case 3:
        EntityDampVelocityX(entity, 0x40);
        EntityDampVelocityY(entity, 0x40);
        EntityMoveX(entity);
        EntityMoveY(entity);
        if (entity->stage == 0 && entity->timer > 8)
        {
            if (entity->flipX != PlayerIsRight(entity))
            {
                entity->flipX ^= 1;
                Anim_StartIndexed(entity, 1, 0, 0);
                entity->stage = 1;
            }
        }
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        Anim_StartIndexed(entity, 2, 0, 0);
        entity->subState = 4;
        break;
    case 4:
        if (entity->animFlags & 4)
        {
            entity->timer = 0x40;
            entity->subState = 5;
            Anim_StartIndexed(entity, 3, 0, 1);
            *(u8 *)&entity->ext.witch.flags |= 2;
            SPAWN(WitchCastEffect_Update);
            PlaySong(0x81);
        }
        break;
    case 5:
        if (entity->timer != 0)
        {
        if (entity->timer > 0x30)
            Witch_SpawnOrbitParticles(entity, ((entity->timer << 10) + (RandomNumberGenerator() & 0xF)) << 4);
        if (entity->timer == 0x20)
        {
            SPAWN(WitchSpell_Update);
        }
        entity->timer--;
        break;
        }
        *(u8 *)&entity->ext.witch.flags &= 0xFD;
        Anim_StartIndexed(entity, 2, 0, 0);
        entity->subState = 6;
        break;
    case 6:
        if (entity->animFlags & 4)
        {
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->state = 0;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    }
}

/* Witch charge, guard and recovery sequence. */
extern struct EwramData_EntityData *Afterimage_Create(struct EwramData_EntityData *entity, s32 delay, s32 count, s32 flags);

/**
 * @brief Charge state (2): set guard/cast flags, spawn the attached effect,
 * slide and move diagonally with afterimages, then clear flags and recover.
 */
void Witch_UpdateCharge(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child;

    switch (entity->subState)
    {
        case 0:
            entity->timer = 0x40;
            entity->velX.whole = 0x18000;
            entity->accelX.whole = -0x600;
            if (entity->flipX)
            {
                entity->velX.whole = -entity->velX.whole;
                entity->accelX.whole = -entity->accelX.whole;
            }
            *(u8 *)&entity->ext.witch.flags |= 3;
            child = Entity_CreateObject((u32 *)WitchCastEffect_Update);
            if (child != NULL)
            {
                child->ext.witchSpell.ownerIndex = ENTITY_INDEX(entity);
                child->scrollWithBg = 1;
                child->flipX = entity->flipX;
                child->attack = entity->attack;
            }
            entity->subState = 1;
        case 1:
            EntityAccelerateAndMoveX(entity, 0x20000);
            if (entity->timer != 0)
            {
                entity->timer--;
                break;
            }
            entity->velX.whole = -0x38000;
            entity->accelX.whole = 0x600;
            entity->velY.whole = 0x1C000;
            entity->accelY.whole = -0xA00;
            if (entity->flipX)
            {
                entity->velX.whole = -entity->velX.whole;
                entity->accelX.whole = -entity->accelX.whole;
            }
            entity->timer = 0x40;
            entity->subState = 2;
            Anim_StartIndexed(entity, 0xC, 0, 1);
            Afterimage_Create(entity, 0x50, 0xC, 1);
            Afterimage_Create(entity, 0x50, 6, 0);
            break;
        case 2:
            EntityAccelerateAndMove(entity, 0x20000);
            if (entity->timer != 0)
            {
                if (entity->timer & 1)
                    Witch_SpawnMovementParticle(entity);
                entity->timer--;
                break;
            }
            *(u8 *)&entity->ext.witch.flags &= 0xFC;
            entity->timer = 0x20;
            entity->subState = 3;
            break;
        case 3:
            if (entity->stage == 0)
            {
                if (entity->flipX != PlayerIsRight(entity))
                {
                    entity->flipX ^= 1;
                    Anim_StartIndexed(entity, 1, 0, 0);
                    entity->stage = 1;
                }
            }
            EntityDampVelocityX(entity, 0x20);
            EntityDampVelocityY(entity, 0x20);
            EntityMoveX(entity);
            EntityMoveY(entity);
            if (entity->timer != 0)
            {
                entity->timer--;
                break;
            }
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->state = 0;
            entity->stage = 0;
            entity->subState = 0;
            break;
    }
}

extern void Witch_SpawnGroundProjectile(struct EwramData_EntityData *, s32, s32, s32);

/**
 * @brief Death state (3): emit rising particles, spawn the ground projectile
 * in mode 1 and return nonzero for removal.
 */
s32 Witch_UpdateDeath(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *particle;
    s32 y;
    switch (entity->subState)
    {
    case 0:
        entity->subState = 1;
        entity->drawSkip = 1;
        entity->timer = 0x20;
        /* fall through */
    case 1:
        if (!(entity->timer & 1))
        {
            u32 timer = entity->timer;
            y = (-(timer >> 1) + 4) << 16;
            particle = Effect_SpawnShared(entity->posX.whole + (Sin(timer << 13) << 3), entity->posY.whole + y, 5, 6);
            if (particle != NULL)
            {
                particle->velY.whole = -0x18000;
                particle->accelY.whole = 0x800;
                particle->spriteLayer = 3;
            }
        }
        if (entity->timer != 0) entity->timer--;
        else entity->subState = 2;
        break;
    case 2:
        Witch_SpawnGroundProjectile(entity, 0, -0x10, 1);
        return 1;
    }
    return 0;
}

/* Witch and Student Witch initialisation and state dispatchers. */
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern void Enemy_OnContact(struct EwramData_EntityData *);
extern void StudentWitch_OnHit(struct EwramData_EntityData *, s32, u16 *);
extern void Witch_OnHit(struct EwramData_EntityData *, s32, u16 *);
extern void Entity_EnableBoxes(struct EwramData_EntityData *, void *, void *);
extern s32 Hit_EnableContact(struct EwramData_EntityData *, s32, void *, s32);
extern s32 Enemy_IsStunned(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);
extern void StudentWitch_UpdateWalk(struct EwramData_EntityData *);
extern void StudentWitch_UpdateGroundAttack(struct EwramData_EntityData *);
extern void StudentWitch_UpdateAirAttack(struct EwramData_EntityData *);
extern s32 StudentWitch_UpdateDeath(struct EwramData_EntityData *);
extern void Witch_UpdateWait(struct EwramData_EntityData *);
extern u8 sWitchGfx[], sWitchPalette[], sWitchSpriteSheet[];

void EnemyWitchCreate(struct EwramData_EntityData *entity)
{
    if (!Entity_LoadGraphics(entity, sWitchGfx, sWitchPalette, 0, sWitchSpriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 4;
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    if (gEwramData->gameState.bestiaryPreview) entity->state = 4;
    else
    {
        Entity_EnableBoxes(entity, Enemy_OnContact, Witch_OnHit);
        Entity_UpdateBoxes(entity);
        entity->scrollWithBg = 1;
        entity->state = 0;
    }
}

void EnemyWitchUpdate(struct EwramData_EntityData *entity)
{
    u32 box[4];
    u32 outside = EntityIsOutsideScreen(entity, 4);
    outside = (entity->hidden = outside);
    if (!outside) Player_TrackEntity(entity);
    if (Enemy_IsStunned(entity))
    {
        if (entity->state != 3) Hit_CheckWithPlayer(box, entity);
        return;
    }
    switch (entity->state)
    {
    case 0: Witch_UpdateWait(entity); break;
    case 1: Witch_UpdateCast(entity); break;
    case 2: Witch_UpdateCharge(entity); break;
    case 3:
        if (Witch_UpdateDeath(entity)) { EntityDelete(entity); return; }
        break;
    case 4:
        Anim_Update(entity);
        return;
    }
    if (entity->state != 3) Hit_CheckWithPlayer(box, entity);
    Entity_AnimateWithBoxes(entity);
}

/* StudentWitch_OnHit: variant of an already decompiled function, generated with
 * a cloning script (constants adjusted where needed), checked byte for
 * byte.
 * Witch_OnHit and WitchGroundProjectile_OnHit decompiled locally. */

extern s32 Enemy_TakeDamage(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern void Entity_ShowDamage(struct EwramData_EntityData *owner, s16 value, s32 kind);
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);

/**
 * @brief Apply enemy damage; on defeat drop rewards, restore the palette
 * and enter death state 4 with substate counters reset.
 */
void StudentWitch_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    if (Enemy_TakeDamage(entity, attacker, hit))
    {
        Enemy_DropRewards(entity);
        entity->palette = entity->basePalette;
        entity->state = 4;
        entity->stage = 0;
        entity->subState = 0;
    }
}

/**
 * @brief While guard flag 1 is set, play the clank and show zero damage.
 * Otherwise apply enemy damage and enter death state 3 on defeat.
 */
void Witch_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s32 invulnerable = entity->ext.witch.flags & 1;

    if (invulnerable)
    {
        PlaySong(0x69);
        Entity_ShowDamage(entity, 0, 0);
    }
    else if (Enemy_TakeDamage(entity, attacker, hit))
    {
        Enemy_DropRewards(entity);
        entity->palette = entity->basePalette;
        entity->state = 3;
        entity->stage = invulnerable;
        entity->subState = invulnerable;
    }
}

/**
 * @brief Clear projectile HP so its update enters the burst state.
 */
void WitchGroundProjectile_OnHit(struct EwramData_EntityData *entity)
{
    entity->hp = 0;
}

/* WitchSpell_DamagePlayerOnContact. */
/**
 * @brief Apply contact damage equal to the stored damage plus one quarter,
 * with effect type 2 and zero effect flags at the spell position.
 */
void WitchSpell_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attributes = 2;
    contactDamage.unused = 0;
    contactDamage.attack = entity->attack + (entity->attack >> 2);
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}
