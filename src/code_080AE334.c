/* Giant Worm / Poison Worm helpers 0x080AE334..0x080AE3FB. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void Worm_Init(struct EwramData_EntityData *root, s32 variant);
extern void Worm_Update(struct EwramData_EntityData *entity);

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
 * @brief AE334 | Player_ApplyContactDamage with (attack, 0, 0, 0) at the entity position
 * (same as Chaos2Part_DamagePlayerOnContact).
 */
void Worm_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/**
 * @brief AE360 | Giant Worm create: Worm_Init variant 0.
 */
void EnemyGiantWormCreate(struct EwramData_EntityData *entity)
{
    Worm_Init(entity, 0);
}

/**
 * @brief AE36C | Giant Worm update.
 */
void EnemyGiantWormUpdate(struct EwramData_EntityData *entity)
{
    Worm_Update(entity);
}

/**
 * @brief AE378 | Poison Worm create: Worm_Init variant 1.
 */
void EnemyPoisonWormCreate(struct EwramData_EntityData *entity)
{
    Worm_Init(entity, 1);
}

/**
 * @brief AE384 | Poison Worm update: same as Giant Worm.
 */
void EnemyPoisonWormUpdate(struct EwramData_EntityData *entity)
{
    Worm_Update(entity);
}

/**
 * @brief AE390 | Draw up to 5 body segments, following the chain of slots
 * in unk_504 until an empty entity.
 */
void Worm_Draw(struct EwramData_EntityData *entity)
{
    s32 i = 0;
    struct EwramData **ewramRef = &gEwramData;

    do
    {
        if (entity->updateFunc == NULL)
            break;
        Sprite_Draw(entity);
        entity = &(*ewramRef)->entityData[entity->unk_504.part8.unk_504];
    } while (++i <= 4);
}

/**
 * @brief AE3CC | Player_ApplyContactDamage with (attack, 0x40, 0, 0) at the entity
 * position.
 */
void PoisonWorm_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0x40;
    contactDamage.unk_4 = 0;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}
