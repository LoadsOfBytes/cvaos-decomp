/* Bone Pillar / Fish Head helpers 0x080B3E70..0x080B3F4B. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 Enemy_TakeDamage(struct EwramData_EntityData *entity);
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);
extern void EntityDampVelocityX(struct EwramData_EntityData *entity, s32 factor);
extern void EntityAccelerateAndMove(struct EwramData_EntityData *entity, s32 maxSpeed);
extern s32 EntityIsOutsideScreen(struct EwramData_EntityData *entity, s32 margin);
extern void HeadStack_Create(struct EwramData_EntityData *entity, s32 type);
extern void HeadStack_Update(struct EwramData_EntityData *entity);

/**
 * @brief Apply damage; on death drop rewards, restore the palette and enter state 2.
 */
void HeadStackSegment_OnHit(struct EwramData_EntityData *entity)
{
    if (Enemy_TakeDamage(entity))
    {
        Enemy_DropRewards(entity);
        entity->palette = entity->unk_510.part8.basePalette;
        entity->state = 2;
        entity->stage = 0;
        entity->subState = 0;
    }
}

/**
 * @brief B3EB0 | Player_ApplyContactDamage with (attack, 2, 0, 1) at the entity position.
 */
void HeadStackProjectile_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attributes = 2;
    contactDamage.unused = 0;
    contactDamage.attack = entity->attack;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief Clear projectile health; used as a projectile hit callback.
 */
void HeadStackProjectile_ClearHealth(struct EwramData_EntityData *entity)
{
    entity->unk_518.part16.hp = 0;
}

/**
 * @brief Damp horizontal velocity, accelerate and move downward, and request deletion outside the screen.
 */
void HeadStackDebris_Update(struct EwramData_EntityData *entity)
{
    EntityDampVelocityX(entity, 0x28);
    EntityAccelerateAndMove(entity, 0x80000);
    if (EntityIsOutsideScreen(entity, 4))
        entity->deleteRequested = 1;
}

/**
 * @brief B3F1C | Bone Pillar create: HeadStack_Create variant 0.
 */
void EnemyBonePillarCreate(struct EwramData_EntityData *entity)
{
    HeadStack_Create(entity, 0);
}

/**
 * @brief B3F28 | Bone Pillar update.
 */
void EnemyBonePillarUpdate(struct EwramData_EntityData *entity)
{
    HeadStack_Update(entity);
}

/**
 * @brief B3F34 | Fish Head create: HeadStack_Create variant 1.
 */
void EnemyFishHeadCreate(struct EwramData_EntityData *entity)
{
    HeadStack_Create(entity, 1);
}

/**
 * @brief B3F40 | Fish Head update: same as Bone Pillar.
 */
void EnemyFishHeadUpdate(struct EwramData_EntityData *entity)
{
    HeadStack_Update(entity);
}
