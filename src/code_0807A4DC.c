/* Apply damage and choose full destruction or a short reaction to selected attacks. */
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_080211F0.h"
#include "structs/ewram.h"
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *,struct EwramData_EntityData *,u16 *,s32);
extern void Entity_ShowDamage(struct EwramData_EntityData *,s32,s32);
extern void Sound_StopTracked(s32);
extern void Enemy_DropRewards(struct EwramData_EntityData *);
/**
 * @brief Hit: resolve, flash, subtract and show the damage; at HP <= 0 stop sound
 * 0x82, drop the rewards and die (song 0x6E, animation 3, 90 frames,
 * state 2); outside the attack a hit of type 3 / 4 starts the recoil
 * (state 3).
 */
void Arachne_OnHit(struct EwramData_EntityData *entity,struct EwramData_EntityData *attacker,u16 *info)
{
    s32 power=Player_CalculateAttackPower(info[2]);
    struct EntityHit *attack=&attacker->hit;
    s32 damage=Hit_ResolveDamage(entity,attacker,info,(s16)power);
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
        Sound_StopTracked(0x82);
        Enemy_DropRewards(entity);
        PlaySong(0x6E);
        Anim_StartIndexed(entity,3,0,0);
        entity->palette=entity->basePalette;
        entity->timer=90;
        entity->state=2;
        entity->subState=zero;
    }
    else if(entity->state!=1 && (u8)(attack->type-3)<=1)
    {
        Anim_StartIndexed(entity,3,0,0);
        entity->state=3;
        entity->subState=zero;
    }
}
