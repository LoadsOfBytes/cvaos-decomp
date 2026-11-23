/* Graham root callbacks and linked combat damage helpers.
 * EnemyGrahamUpdate is now matching C. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 Enemy_TakeDamage(struct EwramData_EntityData *entity);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *entity);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *entity);
extern u16 Player_CalculateAttackPower(u16 attackCategory);
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *entity, s32 other, u16 *hit, s32 inputDamage);
extern void Entity_ShowDamage(struct EwramData_EntityData *owner, s16 value, s32 kind);

void Graham_UpdateState2(struct EwramData_EntityData *entity);
void Graham_UpdateBestiaryPreview(struct EwramData_EntityData *entity);
s32 GrahamPart_TakeDamage(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);

/**
 * @brief Root state 2 callback; intentionally performs no actions.
 */
void Graham_UpdateState2(struct EwramData_EntityData *entity)
{
}

/**
 * @brief Root bestiary state 0xD0 callback; intentionally performs no
 * actions. Linked presentation entities update independently.
 */
void Graham_UpdateBestiaryPreview(struct EwramData_EntityData *entity)
{
}

/**
 * @brief Apply enemy damage; on defeat mark flag 2 and select state 0xE0
 * with substate counters reset.
 */
void GrahamCombatBody_OnHit(struct EwramData_EntityData *entity)
{
    if (Enemy_TakeDamage(entity))
    {
        entity->state = 0xE0;
        entity->stage = 0;
        entity->subState = 0;
        entity->enemyFlags |= 2;
    }
}

/**
 * @brief Apply the shared linked-part damage helper; on defeat select
 * state 0xF0 with substate counters reset.
 */
void GrahamAttacker_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    if (GrahamPart_TakeDamage(entity, attacker, hit))
    {
        entity->state = 0xF0;
        entity->stage = 0;
        entity->subState = 0;
    }
}

/**
 * @brief Select removal state 0xF0 and reset its counters.
 */
void GrahamSpreadProjectile_OnHit(struct EwramData_EntityData *entity)
{
    entity->state = 0xF0;
    entity->stage = 0;
    entity->subState = 0;
}

/**
 * @brief Request deletion when the attack animation is done; otherwise
 * animate with hit boxes and check player contact.
 */
void GrahamAttackEffect_Update(struct EwramData_EntityData *entity)
{
    if (entity->animDone)
    {
        entity->deleteRequested = 1;
    }
    else
    {
        Entity_AnimateWithBoxes(entity);
        Enemy_CheckHitPlayer(entity);
    }
}

/**
 * @brief Resolve damage, apply hit flash, subtract signed HP damage and
 * show its number 64 pixels higher while restoring full position values.
 * Return nonzero when HP is exhausted. Shared by linked combat entities.
 */
s32 GrahamPart_TakeDamage(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s32 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));
    s32 oldX;
    s32 oldY;

    entity->hitFlashTimer = 8;
    {
        u16 hp = entity->hp;
        damage = (damage << 16) >> 16;
        entity->hp = hp - damage;
    }
    oldX = entity->posX.whole;
    oldY = entity->posY.whole;
    entity->posY.whole = oldY - 0x400000;
    Entity_ShowDamage(entity, damage, 0);
    entity->posX.whole = oldX;
    entity->posY.whole = oldY;
    if ((s16)entity->hp <= 0)
        return 1;
    return 0;
}
