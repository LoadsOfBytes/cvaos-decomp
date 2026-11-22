/* Golem helpers 0x08081A78..0x08081B0F (Iron, Wooden and Flesh Golem use
 * the Golem functions). */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void EnemyGolemUpdate(struct EwramData_EntityData *entity);
extern void EnemyGolemCreate(struct EwramData_EntityData *entity);

static inline void Entity_DamagePlayerOnContact(struct EwramData_EntityData *entity, u16 attributes, u16 param_2, u16 reaction)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = attributes;
    contactDamage.unk_4 = param_2;
    contactDamage.reaction = reaction;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief Advance the idle effect animation and remove it on completion.
 */
void GolemIdleEffect_Update(struct EwramData_EntityData *entity)
{
    Anim_Update(entity);
    if (entity->animDone)
        EntityDelete(entity);
}

/**
 * @brief Apply stored root contact damage with effect type 0 and flags 0
 * through the existing static inline helper.
 */
void Golem_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/**
 * @brief 81AC8 | Iron Golem update: same as Golem.
 */
void EnemyIronGolemUpdate(struct EwramData_EntityData *entity)
{
    EnemyGolemUpdate(entity);
}

/**
 * @brief 81AD4 | Iron Golem create: same as Golem.
 */
void EnemyIronGolemCreate(struct EwramData_EntityData *entity)
{
    EnemyGolemCreate(entity);
}

/**
 * @brief 81AE0 | Wooden Golem update: same as Golem.
 */
void EnemyWoodenGolemUpdate(struct EwramData_EntityData *entity)
{
    EnemyGolemUpdate(entity);
}

/**
 * @brief 81AEC | Wooden Golem create: same as Golem.
 */
void EnemyWoodenGolemCreate(struct EwramData_EntityData *entity)
{
    EnemyGolemCreate(entity);
}

/**
 * @brief 81AF8 | Flesh Golem update: same as Golem.
 */
void EnemyFleshGolemUpdate(struct EwramData_EntityData *entity)
{
    EnemyGolemUpdate(entity);
}

/**
 * @brief 81B04 | Flesh Golem create: same as Golem.
 */
void EnemyFleshGolemCreate(struct EwramData_EntityData *entity)
{
    EnemyGolemCreate(entity);
}
