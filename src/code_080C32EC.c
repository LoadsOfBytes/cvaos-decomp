/* Apply damage, flash the sprite, then enter the defeated animation. */
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "structs/ewram.h"
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *,s32,u16 *,s32);
extern void Entity_ShowDamage(struct EwramData_EntityData *,s32,s32);
extern void Enemy_DropRewards(struct EwramData_EntityData *);
extern void GhostDancer_UpdateDeath(struct EwramData_EntityData *);
/**
 * @brief Resolve and display damage with hit flash; on defeat drop rewards,
 * restore the palette, play the death sound and start death state 3.
 */
void GhostDancer_OnHit(struct EwramData_EntityData *entity,s32 hit,u16 *info)
{
    s32 damage=Hit_ResolveDamage(entity,hit,info,(s16)Player_CalculateAttackPower(info[2]));
    entity->hitFlashTimer=8;
    { u16 hp=entity->hp;
        damage=(u32)damage<<16;
        damage>>=16;
        entity->hp=hp-damage; }
    Entity_ShowDamage(entity,damage,0);
    if((s16)entity->hp<=0)
    {
        Enemy_DropRewards(entity);
        entity->palette=entity->basePalette;
        entity->state=3;
        entity->subState=0;
        PlaySong(0x6D);
        GhostDancer_UpdateDeath(entity);
    }
}
