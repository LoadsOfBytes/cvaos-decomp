/* Dead Warrior dispatcher and Skull Millione family effect records. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/enemy_variant_b7e80.h"
extern s32 Enemy_PreUpdate(struct EwramData_EntityData *);
extern void Entity_HideAboveY(struct EwramData_EntityData *, s32);
extern void DeadWarrior_UpdateCharge(struct EwramData_EntityData *);
extern s32 DeadWarrior_UpdateDeath(struct EwramData_EntityData *);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);


void EnemyDeadWarriorUpdate(struct EwramData_EntityData *entity)
{
    if (Enemy_PreUpdate(entity)) return;
    Entity_HideAboveY(entity, -0x10);
    switch (entity->state)
    {
    case 0: DeadWarrior_UpdateCharge(entity); break;
    case 1:
        if (DeadWarrior_UpdateDeath(entity))
        {
            EntityDelete(entity);
            return;
        }
        break;
    case 2:
        Anim_Update(entity);
        return;
    }
    Enemy_CheckHitPlayer(entity);
    Entity_AnimateWithBoxes(entity);
}

/**
 * @brief Apply contact damage using the selected variant record; enable its
 * effect type only in attack state 2. Variant 2 adds the existing
 * player-stat-derived amount to the entity damage value.
 */
void SkullMillioneFamily_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    const struct SkullMillioneFamilyVariant *attack = &sSkullMillioneFamilyVariants[(u8)entity->unk_4F8.part8.unk_4F8];
    struct PlayerContactDamage effect;
    u32 value;
    s32 sum;
    if (entity->state == 2) effect.attributes = attack->effectType;
    else effect.attributes = 0;
    effect.unk_4 = 0;
    if ((u8)entity->unk_4F8.part8.unk_4F8 != 2)
        effect.attack = entity->attack;
    else
    {
        value = (u16)gEwramData->playerState.equipmentDefenseBonus;
        value <<= 16;
        sum = ((s32)value >> 17) + ((s32)value >> 18);
        sum += gEwramData->playerStatus.maxHP >> 4;
        sum <<= 16; sum = (u32)sum >> 16;
        effect.attack = entity->attack + (u32)sum / 3;
    }
    effect.reaction = attack->effectFlags;
    effect.sourceX = entity->posX.whole;
    effect.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&effect);
}
