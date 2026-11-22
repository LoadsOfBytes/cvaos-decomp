/* Slime helpers 0x080C13DC..0x080C148F. */
#include "code/code_080211F0.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern s32 Enemy_TakeDamage(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);

/**
 * @brief C13DC | Player_ApplyContactDamage with (attack, 0x40, 0, 0) at the entity
 * position.
 */
void Slime_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attributes = 0x40;
    contactDamage.unk_4 = 0;
    contactDamage.attack = entity->attack;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief Mark the hit-reaction flag and apply enemy damage; on defeat drop
 * rewards, restore the palette and enter death state 2.
 */
void Slime_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    entity->unk_4F4.part8.unk_4F5 |= 1;
    if (Enemy_TakeDamage(entity, attacker, hit))
    {
        Enemy_DropRewards(entity);
        entity->palette = entity->unk_510.part8.basePalette;
        entity->state = 2;
        entity->subState = 0;
    }
}

/**
 * @brief Accelerate and move the particle with speed cap 0x40000; request
 * deletion outside the expanded screen rectangle.
 */
void SlimeParticle_Update(struct EwramData_EntityData *entity)
{
    EntityAccelerateAndMove(entity, 0x40000);
    if (!Entity_IsInRect(entity, -0x20, -0x20, 0x130, 0xE0))
        entity->deleteRequested = 1;
}
