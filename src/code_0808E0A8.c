#include "code_08009A0.h"
#include "great_armor_entity.h"
#include "code_08033CAC.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
extern s16 Hit_ResolveDamage(struct EwramData_EntityData *, s32, u16 *, s32);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s16, s32);
extern void Enemy_DropRewards(struct EwramData_EntityData *);

extern void BossDefeatEffect_Spawn(s32, s32);
extern void BossRecoveryPickup_Spawn(s32, s32, s32, s32, s32);

/**
 * @brief Hit: resolve the damage, flash (hitFlashTimer = 8), subtract it from the HP and
 * show it, count the hit (unk_4FD, unk_4FE = 1); at HP <= 0 drop the
 * rewards and crumble (state 2, timer 0xFF), running the boss-defeat effects
 * BossDefeatEffect_Spawn / BossRecoveryPickup_Spawn when unk_514 is set.
 */
void GreatArmor_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    s32 damage = Player_CalculateAttackPower(hit[2]);
    damage = Hit_ResolveDamage(entity, (s32)attacker, hit, (s16)damage);
    entity->unk_510.part8.hitFlashTimer = 8;
    {
        u16 hp = entity->unk_518.part16.hp;
        damage <<= 16;
        damage >>= 16;
        entity->unk_518.part16.hp = hp - damage;
    }
    Entity_ShowDamage(entity, damage, 0);
    GREAT_ARMOR_HIT_COUNT(entity)++;
    GREAT_ARMOR_HIT(entity) = 1;
    if ((s16)entity->unk_518.part16.hp <= 0)
    {
        Enemy_DropRewards(entity);
        entity->palette = entity->unk_510.part8.basePalette;
        entity->timer = 0xFF;
        entity->state = 2;
        entity->subState = 0;
        if (entity->unk_514.part16.spawnParam0 != 0)
        {
            BossDefeatEffect_Spawn(entity->posX.whole, entity->posY.whole - 0x200000);
            BossRecoveryPickup_Spawn((s16)entity->posX.part16.integer, 0x20, entity->unk_514.part16.spawnParam0, 0x60, 0);
            Player_SetSelectedTarget(NULL);
        }
    }
}
