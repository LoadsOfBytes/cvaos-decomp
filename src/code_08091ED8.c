/* Devil helpers 0x08091ED8..0x08092173 (also used by Flame Demon, Arc Demon
 * and Demon Lord). */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

#define DEVIL_PART_COUNT 8

extern void Devil_UpdatePart(struct EwramData_EntityData *entity, s32 index);
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *entity);
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);
extern u16 Player_CalculateAttackPower(u16 attackCategory);
extern s16 Hit_ResolveDamage(struct EwramData_EntityData *entity, s32 other, u16 *hit, s32 inputDamage);
extern void Entity_ShowDamage(struct EwramData_EntityData *owner, s16 value, s32 kind);
extern void EnemyDevilUpdate(struct EwramData_EntityData *entity);
extern void EnemyDevilCreate(struct EwramData_EntityData *entity);

void Devil_OnDamaged(struct EwramData_EntityData *entity, s32 attacker, u16 *hit, s32 damage);

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
 * @brief 91ED8 | Unless timer is 0, run Devil_UpdatePart on parts 0..7.
 */
void Devil_UpdateAllParts(struct EwramData_EntityData *entity)
{
    s32 i;

    if (entity->timer != 0)
    {
        for (i = 0; i < DEVIL_PART_COUNT; i++)
            Devil_UpdatePart(entity, i);
    }
}

/**
 * @brief 91EF8 | Unless timer is 0, run Devil_UpdatePart on the 8 parts
 * listed in order.
 */
void Devil_UpdateParts(struct EwramData_EntityData *entity, s8 *order)
{
    s32 i;

    if (entity->timer != 0)
    {
        for (i = 0; i < DEVIL_PART_COUNT; i++)
            Devil_UpdatePart(entity, order[i]);
    }
}

/**
 * @brief 91F20 | Face the direction given by Entity_IsTargetRight; when turning
 * around mirror all parts (x offset and unk_8).
 */
void Devil_FaceTarget(struct EwramData_EntityData *entity)
{
    u32 flipX = entity->flipX;
    struct DevilPart *parts;
    s32 i;

    if (flipX != Entity_IsTargetRight(entity))
    {
        entity->flipX ^= 1;
        parts = entity->unk_500.devilParts;
        for (i = 0; i < DEVIL_PART_COUNT; i++)
        {
            parts[i].offsetX = -parts[i].offsetX;
            parts[i].rotationAngle = 0x20000 - parts[i].rotationAngle;
        }
    }
}

/**
 * @brief 91F70 | Clear unk_10 and unk_14 of part partIndex.
 */
void Devil_ResetPartAngles(struct EwramData_EntityData *entity, s32 partIndex)
{
    struct DevilPart *parts = entity->unk_500.devilParts;

    parts[partIndex].targetSwingAngle = 0;
    parts[partIndex].swingAngle = 0;
}

/**
 * @brief 91F80 | Save the offsets of all parts.
 */
void Devil_SaveParts(struct EwramData_EntityData *entity)
{
    struct DevilPart *parts = entity->unk_500.devilParts;
    s32 i;

    for (i = 0; i < DEVIL_PART_COUNT; i++)
    {
        parts[i].savedOffsetX = parts[i].offsetX;
        parts[i].savedOffsetY = parts[i].offsetY;
    }
}

/**
 * @brief 91F9C | Restore the offsets saved by Devil_SaveParts.
 */
void Devil_RestoreParts(struct EwramData_EntityData *entity)
{
    struct DevilPart *parts = entity->unk_500.devilParts;
    s32 i;

    for (i = 0; i < DEVIL_PART_COUNT; i++)
    {
        parts[i].offsetX = parts[i].savedOffsetX;
        parts[i].offsetY = parts[i].savedOffsetY;
    }
}

/**
 * @brief 91FB8 | Player_ApplyContactDamage with (attack, 1, 0, 1) at the entity position.
 */
void DevilPart_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 1;
    contactDamage.unk_4 = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief 91FE8 | Player_ApplyContactDamage with (attack, 2, 0, 0) at the entity position.
 */
void DevilProjectile_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 2;
    contactDamage.unk_4 = 0;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief 92018 | Follow the entity unk_500 at offset (unk_4F8, unk_4FC);
 * delete when the timer timer ends.
 */
void DevilChargeEffect_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *devil = entity->unk_500.unk_500_entity;
    u32 var_1[4];

    entity->posX.whole = devil->posX.whole + entity->unk_4F8.whole;
    entity->posY.whole = devil->posY.whole + entity->unk_4FC.whole;
    if ((s8)--entity->timer <= 0)
        EntityDelete(entity);
}

/**
 * @brief Draw the Devil unless it is hidden above the screen top (unk_50B).
 */
void Devil_Draw(struct EwramData_EntityData *entity)
{
    if ((s8)entity->unk_508.part8.unk_50B == 0)
        Sprite_DrawEntity(entity);
}

/**
 * @brief 92064 | Player_ApplyContactDamage with (attack, 0, 0, 0) at the entity position
 * (same as Chaos2Part_DamagePlayerOnContact).
 */
void Devil_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/**
 * @brief 92090 | Hit: damage from hit, damage number, then Devil_OnDamaged.
 */
void Devil_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s16 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));

    Entity_ShowDamage(entity, damage, 0);
    Devil_OnDamaged(entity, attacker, hit, damage);
}

/**
 * @brief 920D8 | Subtract damage from the HP in unk_518; at 0 die (state 2,
 * sound 0x72, 0x28 frames).
 */
void Devil_OnDamaged(struct EwramData_EntityData *entity, s32 attacker, u16 *hit, s32 damage)
{
    entity->unk_510.part8.hitFlashTimer = 8;
    entity->unk_518.part16.hp -= damage;
    if ((s16)entity->unk_518.part16.hp <= 0)
    {
        Enemy_DropRewards(entity);
        PlaySong(0x72);
        entity->palette = entity->unk_510.part8.basePalette;
        entity->timer = 0x28;
        entity->state = 2;
        entity->subState = 0;
    }
}

/**
 * @brief 9212C | Flame Demon update: same as Devil.
 */
void EnemyFlameDemonUpdate(struct EwramData_EntityData *entity)
{
    EnemyDevilUpdate(entity);
}

/**
 * @brief 92138 | Flame Demon create: same as Devil.
 */
void EnemyFlameDemonCreate(struct EwramData_EntityData *entity)
{
    EnemyDevilCreate(entity);
}

/**
 * @brief 92144 | Arc Demon update: same as Devil.
 */
void EnemyArcDemonUpdate(struct EwramData_EntityData *entity)
{
    EnemyDevilUpdate(entity);
}

/**
 * @brief 92150 | Arc Demon create: same as Devil.
 */
void EnemyArcDemonCreate(struct EwramData_EntityData *entity)
{
    EnemyDevilCreate(entity);
}

/**
 * @brief 9215C | Demon Lord update: same as Devil.
 */
void EnemyDemonLordUpdate(struct EwramData_EntityData *entity)
{
    EnemyDevilUpdate(entity);
}

/**
 * @brief 92168 | Demon Lord create: same as Devil.
 */
void EnemyDemonLordCreate(struct EwramData_EntityData *entity)
{
    EnemyDevilCreate(entity);
}
