/* Death helpers 0x080BF504..0x080BF5A7 (Death_OnHit is in code_080BF4AC.c,
 * DeathFloatingWeaponHitBox_OnHit). */
#include "code/code_080211F0.h"
#include "death_entity.h"
#include "structs/ewram.h"

extern s32 Enemy_TakeDamage(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);

/**
 * @brief Apply enemy damage to the floating weapon; on defeat mark flag 2
 * and restore the palette so its update starts the breakup sequence.
 */
void DeathFloatingWeapon_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    if (Enemy_TakeDamage(entity, attacker, hit))
    {
        entity->enemyFlags |= 2;
        entity->palette = entity->unk_510.part8.basePalette;
    }
}

/**
 * @brief Clear blade HP so its update starts the destruction sequence.
 */
void DeathBlade_OnHit(struct EwramData_EntityData *entity)
{
    entity->unk_518.part16.hp = 0;
}

/**
 * @brief BF548 | Player_ApplyContactDamage with (attack, 0, 0, 1) at the entity position.
 */
void DeathPillar_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attributes = 0;
    contactDamage.unk_4 = 0;
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
    contactDamage.unk_4 = 0;
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
    struct EwramData_EntityData *weapon = &gEwramData->entityData[(u8)DEATH_OWNER_INDEX(entity)];

    if (Enemy_TakeDamage(weapon, attacker, hit))
    {
        weapon->enemyFlags |= 2;
        weapon->palette = weapon->unk_510.part8.basePalette;
    }
}
