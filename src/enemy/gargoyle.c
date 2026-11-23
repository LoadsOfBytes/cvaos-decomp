/*
 * Gargoyle. The last part of the range also holds the entry points of Triton.
 */
#include "gargoyle.h"
#include "core.h"
#include "graphics.h"
#include "gameplay.h"
#include "structs/ewram.h"
#include "sound.h"
#include "player_core.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "math_anim.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "triton.h"

/* Decompiled locally from the original assembly (variant of an already
 * decompiled function). */

extern u8 Gargoyle_LoadGraphics(struct EwramData_EntityData *entity);
extern void Gargoyle_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void Gargoyle_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern s32 Hit_EnableContact(struct EwramData_EntityData *entity, s32 type, void *onHit, s32 onHurt);

/**
 * @brief 864A0 | Gargoyle create: like EnemyValkyrieCreate, but state 2 when
 * bestiaryPreview is set.
 */
void EnemyGargoyleCreate(struct EwramData_EntityData *entity)
{
    if (!Gargoyle_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 2;
        return;
    }
    entity->spriteLayer = 3;
    if (Anim_GetMaxBoxCount(entity) > 1)
        Hit_EnableContact(entity, 8, Gargoyle_DamagePlayerOnContact, (s32)Gargoyle_OnHit);
    else
        Hit_Enable(entity, 8, Gargoyle_DamagePlayerOnContact, (s32)Gargoyle_OnHit);
    Entity_UpdateBoxes(entity);
    entity->state = 0;
}

/* Gargoyle: AI, hit reaction and graphics. */
extern void EntityMoveY(struct EwramData_EntityData *);
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *entity, s32 other, u16 *hit, s32 inputDamage);
extern void Entity_ShowDamage(struct EwramData_EntityData *owner, s16 value, s32 kind);
extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);
extern u8 sGargoyleGfx[];
extern u8 sGargoylePalette[];
extern u8 sGargoyleSpriteSheet[];
#define ACCEL(e) ((e)->accelX.whole)
#define MAX_SPEED(e) ((e)->accelY.whole)
/* Gargoyle AI: fly towards a point 64 px above the target, swoop down,
 * glide along and climb back. */
void Gargoyle_UpdateFly(struct EwramData_EntityData *entity)
{
    s32 x, y;
    switch (entity->subState)
    {
    case 0:
        entity->flipX = Entity_IsTargetRight(entity);
        entity->velX.whole = entity->flipX ? 0x8000 : -0x8000;
        entity->velY.whole = 0;
        ACCEL(entity) = 0x1000;
        MAX_SPEED(entity) = 0x10000;
        entity->timer = 0;
        entity->subState++;
    case 1:
        x = Target_GetX(entity) - entity->posX.whole;
        {
            s32 target = Target_GetY(entity);
            s32 base = entity->posY.whole + 0x400000;
            y = target - base;
        }
        if (x < -0x80000)
        {
            entity->velX.whole -= ACCEL(entity);
            if (entity->velX.whole <= -MAX_SPEED(entity))
                entity->velX.whole = -MAX_SPEED(entity);
        }
        else if (x > 0x80000)
        {
            entity->velX.whole += ACCEL(entity);
            if (entity->velX.whole >= MAX_SPEED(entity))
                entity->velX.whole = MAX_SPEED(entity);
        }
        if (y < -0x80000)
        {
            entity->velY.whole -= ACCEL(entity);
            if (entity->velY.whole <= -MAX_SPEED(entity))
                entity->velY.whole = -MAX_SPEED(entity);
        }
        else if (y > 0x80000)
        {
            entity->velY.whole += ACCEL(entity);
            if (entity->velY.whole >= MAX_SPEED(entity))
                entity->velY.whole = MAX_SPEED(entity);
        }
        if (entity->velX.whole >= 0) entity->flipX = 1;
        else entity->flipX = 0;
        EntityMove(entity);
        if ((s8)--entity->timer <= 0)
        {
            entity->timer = 0;
            if (y > -0x80000 && y < 0x80000 && (x < 0 ? -x : x) < 0x600000)
            {
                entity->subState++;
                Anim_StartIndexed(entity, 3, 0, 0);
            }
        }
        break;
    case 2:
        entity->flipX = Entity_IsTargetRight(entity);
        EntityDampVelocityX(entity, 10);
        EntityDampVelocityY(entity, 10);
        EntityMove(entity);
        if (entity->timer++ > 19)
        {
            entity->subState++;
            entity->timer = 0;
            Anim_StartIndexed(entity, 1, 0, 0);
            ACCEL(entity) = entity->flipX ? 0xC00 : -0xC00;
            MAX_SPEED(entity) = 0x1800;
            PlaySong(0x87);
        }
        break;
    case 3:
        if (RandomNumberGenerator() % 256 == 0) Anim_StartIndexed(entity, 2, 0, 0);
        EntityAccelerateAndMove(entity, 0x20000);
        if (entity->timer++ > 23)
        {
            entity->subState++;
            entity->timer = 0;
            ACCEL(entity) *= 2;
        }
        break;
    case 4:
        if (RandomNumberGenerator() % 256 == 0) Anim_StartIndexed(entity, 2, 0, 0);
        EntityAccelerateAndMoveX(entity, 0x20000);
        EntityDampVelocityY(entity, 0x10);
        EntityMoveY(entity);
        if (entity->timer++ > 59)
        {
            entity->subState++;
            entity->timer = 0;
            ACCEL(entity) = 0x1000;
            MAX_SPEED(entity) = 0x18000;
            Anim_StartIndexed(entity, 0, 0, 1);
        }
        break;
    case 5:
        x = Target_GetX(entity) - entity->posX.whole;
        {
            s32 target = Target_GetY(entity);
            s32 base = entity->posY.whole + 0x400000;
            y = target - base;
        }
        if (x < -0x80000)
        {
            entity->velX.whole -= ACCEL(entity);
            if (entity->velX.whole <= -MAX_SPEED(entity))
                entity->velX.whole = -MAX_SPEED(entity);
        }
        else if (x > 0x80000)
        {
            entity->velX.whole += ACCEL(entity);
            if (entity->velX.whole >= MAX_SPEED(entity))
                entity->velX.whole = MAX_SPEED(entity);
        }
        if (y < 0)
        {
            entity->velY.whole -= ACCEL(entity);
            if (entity->velY.whole <= -MAX_SPEED(entity))
                entity->velY.whole = -MAX_SPEED(entity);
        }
        else
        {
            entity->subState = 1;
            entity->timer = 20;
            ACCEL(entity) = 0x1000;
            MAX_SPEED(entity) = 0x10000;
        }
        if (entity->velX.whole >= 0) entity->flipX = 1;
        else entity->flipX = 0;
        EntityMove(entity);
        break;
    }
}

/* Gargoyle hit: flash, subtract HP; at 0 HP start falling (state 1). */
void Gargoyle_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s32 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));

    entity->hitFlashTimer = 8;
    {
        u16 hp = entity->hp;
        damage <<= 16;
        damage >>= 16;
        entity->hp = hp - damage;
    }
    Entity_ShowDamage(entity, damage, 0);
    if ((s16)entity->hp <= 0)
    {
        Enemy_DropRewards(entity);
        PlaySong(0x6E);
        entity->palette = entity->basePalette;
        entity->timer = 0x28;
        entity->state = 1;
        entity->subState = 0;
        entity->accelX.whole = 0;
        entity->accelY.whole = 0x1000;
    }
}
/* Load the Gargoyle graphics; palette 0 for entity type 0x57, else 1. */
u8 Gargoyle_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 graphics = ObjGfx_LoadResource(sGargoyleGfx, 0);

    if (graphics < 0)
        return 0;
    if (entity->enemyId == 0x57)
        entity->basePalette = ObjPalette_FindOrLoadVariant(sGargoylePalette, 0);
    else
        entity->basePalette = ObjPalette_FindOrLoadVariant(sGargoylePalette, 1);
    Sprite_Attach(entity, sGargoyleSpriteSheet, graphics, entity->basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Gargoyle_DamagePlayerOnContact = Enemy_DamagePlayerOnContact
 * Gargoyle/Triton update and creation. */

extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern void Triton_OnHit(struct EwramData_EntityData *);

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
 * @brief 86964 | Same as Enemy_DamagePlayerOnContact.
 */
void Gargoyle_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/* Gargoyle: dispatch the AI state, a knocked-back state and the display state. */
void EnemyGargoyleUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    if (Enemy_PreUpdate(entity)) goto finished;
    switch (entity->state)
    {
    case 0: Gargoyle_UpdateFly(entity); break;
    case 1:
        EntityDampVelocityX(entity, 10);
        EntityAccelerateAndMove(entity, 0x30000);
        Enemy_UpdateFragmentDeath(entity);
        break;
    case 2:
        Anim_Update(entity);
        goto finished;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
finished:
    return;
}

/* Triton: dispatch movement, attack, death fade and display states. */
void EnemyTritonUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    if (entity->state != 2) Enemy_UpdateHitFlash(entity);
    if (Entity_IsTimeStopped(entity) && entity->state != 2)
    {
        Hit_CheckWithPlayer(collision, entity);
        goto finished;
    }
    switch (entity->state)
    {
    case 0: Triton_UpdateMove(entity); break;
    case 1: Triton_UpdateAttack(entity); break;
    case 2:
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if ((s8)--entity->timer <= 0)
        {
            entity->state = 3;
            entity->timer = 10;
            entity->hidden = 1;
        }
        break;
    case 3:
        entity->hidden = 1;
        if ((s8)--entity->timer <= 0) entity->deleteRequested = 1;
        break;
    case 4:
        Anim_Update(entity);
        goto finished;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
finished:
    return;
}

/* Create the Triton with its spear (Triton_CreateSpear) and contact callbacks. */
void EnemyTritonCreate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *spear;
    if (!Triton_LoadGraphics(entity)) goto fail;
    Anim_StartIndexed(entity, 1, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    spear = Triton_CreateSpear(entity);
    if (!spear)
    {
    fail:
        EntityDelete(entity);
        return;
    }
    entity->ext.triton.spear = spear;
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 4;
        return;
    }
    if (Anim_GetMaxBoxCount(entity) > 1)
        Hit_EnableContact(entity, 8, Triton_DamagePlayerOnContact, (s32)Triton_OnHit);
    else
        Hit_Enable(entity, 8, Triton_DamagePlayerOnContact, (s32)Triton_OnHit);
    Entity_UpdateBoxes(entity);
    entity->state = 0;
    Entity_SnapToGround(entity);
    entity->subState = 10;
}
