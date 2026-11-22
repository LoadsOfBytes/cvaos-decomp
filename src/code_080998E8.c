#include "code_08009A0.h"
#include "syscalls.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
extern s16 Hit_ResolveDamage(struct EwramData_EntityData *, s32, u16 *, s32);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s16, s32);
extern void Enemy_DropRewards(struct EwramData_EntityData *);

void Ectoplasm_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
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
    {
        s32 dx = entity->posX.whole - attacker->posX.whole;
        s32 dy = entity->posY.whole - attacker->posY.whole;
        u16 angle = ArcTan2(dx < 0 ? -((-dx) >> 16) : dx >> 16, dy < 0 ? -((-dy) >> 16) : dy >> 16);
        s32 speed;
        dx = Sin(angle + 0x4000);
        dy = Sin(angle);
        speed = 0x10000;
        entity->velX.whole = FixedMultiply(dx, speed);
        entity->velY.whole = FixedMultiply(dy, speed);
    }
    if ((s16)entity->unk_518.part16.hp <= 0)
    {
        Enemy_DropRewards(entity);
        PlaySong(0x6D);
        Anim_StartIndexed(entity, 1, 0, 1);
        entity->palette = entity->unk_510.part8.basePalette;
        entity->timer = 0x28;
        entity->state = 1;
        entity->subState = 0;
        entity->scrollWithBg = 1;
        entity->velX.whole = 0;
        entity->velY.whole = 0;
        entity->accelX.whole = 0;
        entity->accelY.whole = 0x1000;
    }
}
