#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
extern s16 Hit_ResolveDamage(struct EwramData_EntityData *, s32, u16 *, s32);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s16, s32);
extern struct AnimScript sUnk_08528340;

void LegionDestructibleFragment_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    s32 damage = Player_CalculateAttackPower(hit[2]);
    damage = Hit_ResolveDamage(entity, (s32)attacker, hit, (s16)damage);
    entity->hitFlashTimer = 8;
    damage = (s16)damage;
    Entity_ShowDamage(entity, damage, 0);
    entity->hp -= damage;
    if ((s16)entity->hp <= 0)
    {
        entity->palette = entity->basePalette;
        entity->timer = 0x28;
        entity->hitFlashTimer = 0;
        entity->state = 10;
        entity->subState = 0;
        Anim_Start(entity, &sUnk_08528340, 2, 1);
    }
}
