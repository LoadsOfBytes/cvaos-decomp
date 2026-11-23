/* Lethal hits shed four effects; selected attack kinds force a recovery state. */
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *,struct EwramData_EntityData *,u16 *,s32);
extern void Entity_ShowDamage(struct EwramData_EntityData *,s32,s32);
extern void Enemy_DropRewards(struct EwramData_EntityData *);
extern void Harpy_SpawnDeathFeather(struct EwramData_EntityData *,s32,s32);
void Harpy_OnHit(struct EwramData_EntityData *entity,struct EwramData_EntityData *attacker,u16 *info)
{
    struct EntityHit *attack=&attacker->hit;
    s32 damage=Hit_ResolveDamage(entity,attacker,info,(s16)Player_CalculateAttackPower(info[2]));
    s32 zero;
    entity->hitFlashTimer=8;
    {
        u16 hp=entity->hp;
        damage=(u32)damage<<16;
        damage>>=16;
        hp-=damage;
        zero=0;
        entity->hp=hp;
    }
    Entity_ShowDamage(entity,damage,0);
    if((s16)entity->hp<=0)
    {
        Enemy_DropRewards(entity);
        PlaySong(0x1C3);
        Anim_StartIndexed(entity,3,0,1);
        entity->palette=entity->basePalette;
        entity->timer=40;
        entity->state=2;
        entity->subState=zero;
        Harpy_SpawnDeathFeather(entity,entity->posX.whole,entity->posY.whole);
        Harpy_SpawnDeathFeather(entity,entity->posX.whole,entity->posY.whole);
        Harpy_SpawnDeathFeather(entity,entity->posX.whole,entity->posY.whole);
        Harpy_SpawnDeathFeather(entity,entity->posX.whole,entity->posY.whole);
    }
    else if((u8)(attack->type-3)<=1)
    {
        Anim_StartIndexed(entity,3,0,1);
        Anim_Update(entity);
        Entity_UpdateBoxes(entity);
        entity->state=3;
        entity->subState=zero;
    }
}
