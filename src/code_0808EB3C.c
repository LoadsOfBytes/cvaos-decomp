/* Great Armor helpers 0x0808EB3C..0x0808EC63 (Final Guard and Shadow Knight
 * use the Great Armor functions). */
#include "code_08009A0.h"
#include "joint_entity.h"
#include "great_armor_entity.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern void EnemyGreatArmorUpdate(struct EwramData_EntityData *entity);
extern void EnemyGreatArmorCreate(struct EwramData_EntityData *entity);
extern u16 Player_CalculateAttackPower(u16 attackCategory);
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *entity, s32 other, u16 *hit, s32 inputDamage);
extern void Entity_ShowDamage(struct EwramData_EntityData *owner, s16 value, s32 kind);

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
 * @brief 8EB3C | Follow the parent (unk_500) at the offset of its part 12 plus
 * velX/velY; delete when 0x53D bit 0 is set or the parent is in state 2.
 */
void GreatArmorSlashEffect_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *armor = GREAT_ARMOR_SLASH_OWNER(entity);
    struct GreatArmorPart *parts = armor->unk_500.greatArmorParts;
    s32 offsetX = parts[12].offsetX;
    s32 *offsetY = &parts[12].offsetY;

    entity->posX.whole = armor->posX.whole + offsetX + entity->velX.whole;
    entity->posY.whole = armor->posY.whole + *offsetY + entity->velY.whole;
    if (entity->animDone || armor->state == 2)
        EntityDelete(entity);
    else
        Anim_Update(entity);
}

/**
 * @brief 8EB90 | Player_ApplyContactDamage at the entity position with (attack, 1, 0, 1)
 * for attack 12 (unk_504), else (attack, 0, 0, 0).
 */
void GreatArmorPart_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0;
    contactDamage.unused = 0;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    if ((s8)JOINT_PART_INDEX(entity) == 12)
    {
        contactDamage.attributes = 1;
        contactDamage.reaction = 1;
    }
    else
    {
        contactDamage.attributes = 0;
    }
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief 8EBD8 | Hit without damage: hit effect, "0" damage and sound 0x69.
 */
void GreatArmorPart_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    Player_CalculateAttackPower(hit[2]);
    Hit_ResolveDamage(entity, attacker, hit, 0);
    Entity_ShowDamage(entity, 0, 0);
    PlaySong(0x69);
}

/**
 * @brief 8EC08 | Player_ApplyContactDamage with (attack, 0, 0, 0) at the entity position
 * (same as Chaos2Part_DamagePlayerOnContact).
 */
void GreatArmor_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/**
 * @brief 8EC34 | Final Guard update: same as Great Armor.
 */
void EnemyFinalGuardUpdate(struct EwramData_EntityData *entity)
{
    EnemyGreatArmorUpdate(entity);
}

/**
 * @brief 8EC40 | Final Guard create: same as Great Armor.
 */
void EnemyFinalGuardCreate(struct EwramData_EntityData *entity)
{
    EnemyGreatArmorCreate(entity);
}

/**
 * @brief 8EC4C | Shadow Knight update: same as Great Armor.
 */
void EnemyShadowKnightUpdate(struct EwramData_EntityData *entity)
{
    EnemyGreatArmorUpdate(entity);
}

/**
 * @brief 8EC58 | Shadow Knight create: same as Great Armor.
 */
void EnemyShadowKnightCreate(struct EwramData_EntityData *entity)
{
    EnemyGreatArmorCreate(entity);
}
