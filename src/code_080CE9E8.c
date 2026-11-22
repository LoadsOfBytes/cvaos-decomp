/* Alastor damage, stun counter and linked-part state. */
#include "code_080D73B8.h"
#include "alastor_entity.h"
#include "structs/ewram.h"
extern u16 Player_CalculateAttackPower(u16);
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *,s32,u16 *,s32);
extern void Entity_ShowDamage(struct EwramData_EntityData *,s32,s32);
extern void Enemy_DropRewards(struct EwramData_EntityData *);
#define PART(e) (&gEwramData->entityData[ALASTOR_PART_INDEX((e))])
/**
 * @brief Resolve/display damage and apply hit flash. Count surviving hits;
 * on the reaction threshold or defeat, coordinate linked-part state 3
 * and enter recoil state 7. Drop rewards on defeat and reveal the owner.
 */
void Alastor_OnHit(struct EwramData_EntityData *entity,s32 kind,u16 *hit)
{
    s32 damage=Hit_ResolveDamage(entity,kind,hit,(s16)Player_CalculateAttackPower(hit[2]));
    {
        u16 hp=entity->unk_518.part16.hp;
        damage<<=16;
        damage>>=16;
        entity->unk_518.part16.hp=hp-damage;
    }
    Entity_ShowDamage(entity,damage,0);
    entity->unk_510.part8.hitFlashTimer=8;
    if (ALASTOR_HITS_BEFORE_REACT(entity) && (s16)entity->unk_518.part16.hp>0)
        ALASTOR_HITS_BEFORE_REACT(entity)--;
    else
    {
        struct EwramData_EntityData *part;
        if ((s16)entity->unk_518.part16.hp<=0)
        {
            Enemy_DropRewards(entity);
            PlaySong(0x1BE);
        }
        PART(entity)->state=3;
        PART(entity)->subState=PART(entity)->stage=0;
        entity->state=7;
        entity->stage=0;
        entity->subState=0;
        ALASTOR_HITS_BEFORE_REACT(entity)=3;
    }
    entity->hidden=0;
}
