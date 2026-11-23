/*
 * Dead Warrior. The last part of the range also holds the contact damage of the Skull Millione family.
 */
#include "core.h"
#include "sound.h"
#include "player_core.h"
#include "graphics.h"
#include "gameplay.h"
#include "entity_util.h"
#include "structs/ewram.h"
#include "structs/enemy_variant_b7e80.h"

/* Dead Warrior charge, braking and death. */
extern s32 Target_IsNearEntity(struct EwramData_EntityData *, s32, s32);
extern s32 Entity_GetVisibilityPattern(s32);
extern u8 sDeadWarriorWalkProfile[];

/**
 * @brief Alive state (0): walk using the stored speed, accelerate, then
 * brake with particles and reverse direction when animation 2 finishes.
 * Retain the fallthrough from substate 1 into the acceleration logic.
 */
void DeadWarrior_UpdateCharge(struct EwramData_EntityData *entity)
{
    s32 timer;
    s32 x;
    s32 y;
    struct EwramData_EntityData *particle;
    Entity_Walk(entity, entity->ext.deadWarrior.speed, sDeadWarriorWalkProfile, 4);
    switch (entity->subState)
    {
    case 0:
        entity->ext.deadWarrior.speed = 0;
        entity->stage = 0;
        entity->subState = 1;
        /* fall through */
    case 1:
        if (Target_IsNearEntity(entity, 0x64, 0x3C) && *(u16 *)&entity->animStep == 0x100)
        {
            Anim_StartIndexed(entity, 1, 0, 1);
            entity->subState = 2;
        }
        /* fall through */
    case 2:
        if ((u8)((u32)entity->timer % 25) == 0
            && Entity_IsInRect(entity, -0x40, -0x40, 0x170, 0x120)) PlaySong(0x7D);
        entity->timer++;
        if ((s32)entity->ext.deadWarrior.speed <= 0x1FFFF)
            entity->ext.deadWarrior.speed += 0x4000;
        entity->ext.deadWarrior.chargeFrame++;
        if (*(u16 *)&entity->animStep == 0x101) entity->stage++;
        if (entity->ext.deadWarrior.chargeFrame > 0x64)
        {
            PlaySong(0x127);
            Anim_StartIndexed(entity, 2, 0, 0);
            entity->subState = 3;
            entity->timer = 0;
        }
        break;
    case 3:
        if ((s32)entity->ext.deadWarrior.speed > 0) entity->ext.deadWarrior.speed -= 0x1000;
        timer = entity->timer + 1;
        entity->timer = timer;
        if (!(timer & 3))
        {
            x = entity->posX.whole;
            y = entity->posY.whole;
            particle = Effect_SpawnShared(x, y, (RandomNumberGenerator() & 1) + 4, 6);
            if (particle != NULL)
            {
                particle->drawSkip = 1;
                particle->velX.whole = 0x8000;
                if (entity->flipX) particle->velX.whole = -particle->velX.whole;
            }
        }
        if (entity->animFlags & 4)
        {
            entity->ext.deadWarrior.chargeFrame = 0;
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->subState = 0;
            entity->flipX ^= 1;
        }
        break;
    }
}

/**
 * @brief Death state (1): launch backward and upward, flash and emit
 * particles. Return nonzero when the existing bounds or animation/timer
 * condition allows removal; preserve the timer comparison as written.
 */
s32 DeadWarrior_UpdateDeath(struct EwramData_EntityData *entity)
{
    s32 x;
    s32 timer;
    switch (entity->subState)
    {
    case 0:
        Anim_StartIndexed(entity, 3, 0, 0);
        entity->subState = 1;
        entity->velX.whole = -0x28000;
        if (entity->flipX) entity->velX.whole = -entity->velX.whole;
        entity->velY.whole = -0x28000;
        entity->accelY.whole = 0x2000;
        entity->timer = 0;
        /* fall through */
    case 1:
        if (entity->timer <= 0x27)
        {
            if (Entity_GetVisibilityPattern(0x28 - entity->timer)) entity->palette = 5;
            else entity->palette = entity->basePalette;
        }
        timer = entity->timer + 1;
        entity->timer = timer;
        if (!(timer & 7)) PlaySong(0x6E);
        if (!(entity->timer & 3))
        {
            x = entity->posX.whole + (RandomSigned(0x10) << 16);
            Effect_SpawnShared(x, entity->posY.whole + ((RandomSigned(0x10) - 0x20) << 16), 1, 7);
        }
        EntityAccelerateAndMove(entity, 0x80000);
        if (!Entity_IsInRect(entity, -0x80, -0x40, 0x1F0, 0x120)
            || ((entity->animFlags & 4) && entity->timer <= 0x80)) return 1;
        break;
    }
    return 0;
}

extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Enemy_OnContact(struct EwramData_EntityData *);
extern void DeadWarrior_OnHit(struct EwramData_EntityData *);
extern void Entity_EnableBoxes(struct EwramData_EntityData *, void *, void *);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern u8 sDeadWarriorGfx[], sDeadWarriorPalette[], sDeadWarriorSpriteSheet[];

void EnemyDeadWarriorCreate(struct EwramData_EntityData *entity)
{
    if (!Entity_LoadGraphics(entity, sDeadWarriorGfx, sDeadWarriorPalette, 0, sDeadWarriorSpriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 0x13;
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    if (gEwramData->gameState.bestiaryPreview)
        entity->state = 2;
    else
    {
        Entity_EnableBoxes(entity, Enemy_OnContact, DeadWarrior_OnHit);
        Entity_UpdateBoxes(entity);
        Entity_SnapToGround(entity);
        entity->posY.part16.integer++;
        entity->scrollWithBg = 1;
        entity->state = 0;
        entity->ext.deadWarrior.chargeFrame = 0x32;
    }
}

/* DeadWarrior_OnHit: variant of an already decompiled function, generated with
 * a cloning script (constants adjusted where needed), checked byte for
 * byte. */

extern s32 Enemy_TakeDamage(struct EwramData_EntityData *entity);
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);

/**
 * @brief Apply enemy damage; on defeat drop rewards, restore the palette
 * and enter death state 1 with the substate counters reset.
 */
void DeadWarrior_OnHit(struct EwramData_EntityData *entity)
{
    if (Enemy_TakeDamage(entity))
    {
        Enemy_DropRewards(entity);
        entity->palette = entity->basePalette;
        entity->state = 1;
        entity->stage = 0;
        entity->subState = 0;
    }
}

/* Dead Warrior dispatcher and Skull Millione family effect records. */
extern s32 Enemy_PreUpdate(struct EwramData_EntityData *);
extern void Entity_HideAboveY(struct EwramData_EntityData *, s32);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);

void EnemyDeadWarriorUpdate(struct EwramData_EntityData *entity)
{
    if (Enemy_PreUpdate(entity)) return;
    Entity_HideAboveY(entity, -0x10);
    switch (entity->state)
    {
    case 0: DeadWarrior_UpdateCharge(entity); break;
    case 1:
        if (DeadWarrior_UpdateDeath(entity))
        {
            EntityDelete(entity);
            return;
        }
        break;
    case 2:
        Anim_Update(entity);
        return;
    }
    Enemy_CheckHitPlayer(entity);
    Entity_AnimateWithBoxes(entity);
}

/**
 * @brief Apply contact damage using the selected variant record; enable its
 * effect type only in attack state 2. Variant 2 adds the existing
 * player-stat-derived amount to the entity damage value.
 */
void SkullMillioneFamily_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    const struct SkullMillioneFamilyVariant *attack = &sSkullMillioneFamilyVariants[(u8)entity->ext.skullMillione.variant];
    struct PlayerContactDamage effect;
    u32 value;
    s32 sum;
    if (entity->state == 2) effect.attributes = attack->effectType;
    else effect.attributes = 0;
    effect.unused = 0;
    if ((u8)entity->ext.skullMillione.variant != 2)
        effect.attack = entity->attack;
    else
    {
        value = (u16)gEwramData->playerState.equipmentDefenseBonus;
        value <<= 16;
        sum = ((s32)value >> 17) + ((s32)value >> 18);
        sum += gEwramData->playerStatus.maxHP >> 4;
        sum <<= 16; sum = (u32)sum >> 16;
        effect.attack = entity->attack + (u32)sum / 3;
    }
    effect.reaction = attack->effectFlags;
    effect.sourceX = entity->posX.whole;
    effect.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&effect);
}
