/* Chaos-fight helper entities 0x080A6F58..0x080A712B. */
#include "code_08009A0.h"
#include "chaos_entity.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern s32 EntityIsOutsideScreen(struct EwramData_EntityData *entity, s32 margin);

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
 * @brief A6F58 | Reset fields 0x4FC..0x504 of the linked entity (unk_504)
 * and store sequence in its 0x508.
 */
void Chaos_StartPaletteSequence(struct EwramData_EntityData *entity, s32 sequence)
{
    struct EwramData_EntityData *controller = CHAOS_DATA_PTR(entity);

    controller->unk_4FC.whole = 0;
    controller->unk_500.whole = 0;
    controller->unk_504.whole = 0;
    controller->unk_508.whole = sequence;
}

/**
 * @brief A6F68 | Player_ApplyContactDamage with (attack, 0, 0, 0) at the entity position.
 */
void Chaos2Part_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/**
 * @brief A6F94 | Same as Chaos2Part_DamagePlayerOnContact.
 */
void Chaos2Shot_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/**
 * @brief A6FC0 | Switch to state 9 with a 60 frame timer, set 0x53C bit 5
 * and play sound 0x67.
 */
void Chaos2SummonedPart_OnContact(struct EwramData_EntityData *entity)
{
    entity->state = 9;
    entity->subState = 0;
    entity->timer = 0x3C;
    entity->hidden = 1;
    PlaySong(0x67);
}

/**
 * @brief A6FE4 | Follow enemyTarget at the offset 0x4F8/0x4FC, which itself
 * moves with velocity and acceleration; delete when the timer timer ends.
 */
void Chaos2Spark_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData **target = &gEwramData->playerState.enemyTarget;

    entity->posX.whole = (*target)->posX.whole + CHAOS_OFFSET_X(entity);
    entity->posY.whole = (*target)->posY.whole + CHAOS_OFFSET_Y(entity);
    entity->velX.whole += entity->accelX.whole;
    entity->velY.whole += entity->accelY.whole;
    CHAOS_OFFSET_X(entity) += entity->velX.whole;
    CHAOS_OFFSET_Y(entity) += entity->velY.whole;
    Anim_Update(entity);
    if ((s8)--entity->timer <= 0)
        EntityDelete(entity);
}

/**
 * @brief A7044 | Same as Chaos2Part_DamagePlayerOnContact.
 */
void Chaos2SnakeHead_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/**
 * @brief A7070 | Same as Chaos2Part_DamagePlayerOnContact.
 */
void Chaos2SnakeSegment_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/**
 * @brief A709C | Player_ApplyContactDamage with (attack, 0x10, 0, 1) at the entity position.
 */
void Chaos2SnakeShot_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0x10;
    contactDamage.unused = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief A70CC | Delete the entity when 0x53D bit 0 is set, else animate it.
 */
void Chaos2Effect_Update(struct EwramData_EntityData *entity)
{
    if (entity->animDone)
        EntityDelete(entity);
    else
        Anim_Update(entity);
}

/**
 * @brief A70F0 | Move by the velocity; once outside the screen (margin set 2)
 * release the affine slot in 0x53C bits 0-4 and delete the entity.
 */
void Chaos2Shard_Update(struct EwramData_EntityData *entity)
{
    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
    if (EntityIsOutsideScreen(entity, 2))
    {
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
    }
}
