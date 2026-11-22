/* Apply a Killer Fish hit and select its death state when HP reaches zero. */
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *, struct EwramData_EntityData *, u16 *, s16);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s32, s32);
extern void Enemy_DropRewards(struct EwramData_EntityData *);
void KillerFish_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    s32 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));
    entity->unk_510.part8.hitFlashTimer = 8;
    {
        s32 hp = entity->unk_518.part16.hp;
        damage = (s16)damage;
        entity->unk_518.part16.hp = hp - damage;
    }
    Entity_ShowDamage(entity, damage, 0);
    if ((s16)entity->unk_518.part16.hp <= 0)
    {
        Enemy_DropRewards(entity);
        Anim_StartIndexed(entity, 1, 0, 0);
        Anim_Update(entity);
        entity->palette = entity->unk_510.part8.basePalette;
        entity->timer = 40;
        if (entity->subState == 99) entity->state = 3;
        else entity->state = 2;
        entity->subState = 0;
        entity->unk_518.part8.spawnIndex = 255;
    }
}



