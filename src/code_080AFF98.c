#include "code_08009A0.h"
#include "code_08033CAC.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
extern s16 Hit_ResolveDamage(struct EwramData_EntityData *, s32, u16 *, s32);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s16, s32);
extern void Enemy_DropRewards(struct EwramData_EntityData *);

void Minotaur_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
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
    if ((s16)entity->unk_518.part16.hp < 100 && !(entity->unk_4FC.whole & 1))
        entity->unk_4FC.whole &= ~2;
    if ((s16)entity->unk_518.part16.hp <= 0)
    {
        Enemy_DropRewards(entity);
        entity->palette = entity->unk_510.part8.basePalette;
        entity->timer = 0x28;
        entity->state = 3;
        Anim_StartIndexed(entity, 7, 0, 0);
        entity->unk_4FC.whole &= ~1;
        if (entity->unk_514.part16.spawnParam0 != 0)
        {
            Player_SetSelectedTarget(NULL);
            gEwramData->gameState.bossEventFlags |= 0x100;
        }
    }
}
