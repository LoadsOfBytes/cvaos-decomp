/* Entity helpers 0x0809FE4C..0x0809FF0F. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "structs/ewram.h"

extern void EntityAccelerateAndMove(struct EwramData_EntityData *entity, s32 maxSpeed);
extern s32 Entity_GetVisibilityPattern(s32 step);

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
 * @brief 9FE4C | Player_ApplyContactDamage with (attack, 0, 0, 0) at the entity position
 * (same as Chaos2Part_DamagePlayerOnContact).
 */
void LegionDestructibleFragment_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/**
 * @brief 9FE78 | Follow the parent (unk_4F4) at offset (unk_4F8, unk_4FC)
 * with its visibility; delete when the parent is gone or changed type
 * (enemyId).
 */
void LegionCoreDetail_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *core = entity->ext.legionPart.owner;

    if (core != NULL && core->updateFunc != NULL
        && core->enemyId == entity->enemyId)
    {
        entity->hidden = core->hidden;
        entity->posX.whole = core->posX.whole + entity->ext.legionPart.offsetX;
        entity->posY.whole = core->posY.whole + entity->ext.legionPart.offsetY;
    }
    else
    {
        EntityDelete(entity);
    }
}

/**
 * @brief 9FED0 | Fall (max 0x40000), blink, delete after timer.
 */
void LegionFragment_Update(struct EwramData_EntityData *entity)
{
    EntityAccelerateAndMove(entity, 0x40000);
    entity->hidden = Entity_GetVisibilityPattern(entity->timer);
    if ((s8)--entity->timer <= 0)
        EntityDelete(entity);
}
