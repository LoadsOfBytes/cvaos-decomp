/* Apply a Merman hit and begin the falling death state when HP is exhausted. */
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code_080D73B8.h"
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *, struct EwramData_EntityData *, u16 *, s16);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s32, s32);
extern void Enemy_DropRewards(struct EwramData_EntityData *);
void Merman_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    s32 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));
    u8 *flashTimer = &entity->unk_510.part8.hitFlashTimer;
    s32 zero = 0;
    *flashTimer = 8;
    {
        s32 hp = entity->unk_518.part16.hp;
        damage = (s16)damage;
        entity->unk_518.part16.hp = hp - damage;
    }
    Entity_ShowDamage(entity, damage, 0);
    if ((s16)entity->unk_518.part16.hp <= 0)
    {
        Enemy_DropRewards(entity);
        PlaySong(0x6E);
        Anim_StartIndexed(entity, 1, 0, 1);
        entity->palette = entity->unk_510.part8.basePalette;
        entity->timer = 40;
        entity->state = 2;
        entity->velX.whole = zero;
        entity->accelX.whole = zero;
        entity->velY.whole = zero;
        entity->accelY.whole = 0x2000;
    }
}

