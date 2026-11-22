/* Big Golem damage and contact callbacks. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code_08033CAC.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void Enemy_DropRewards(struct EwramData_EntityData *);
extern void BossDefeatEffect_Spawn(s32, s32);
extern void BossRecoveryPickup_Spawn(s32, s32, s32, s32, s32);
extern s16 Hit_ResolveDamage(struct EwramData_EntityData *, s32, u16 *, s16);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s32, s32);

extern void BigGolem_OnDamaged(struct EwramData_EntityData *, s32, u16 *, s32);
/* Route damage through the parent at this part's position, restoring its
 * coordinates afterwards. Parts 12/13 play a guarded-hit sound. */
void BigGolemPart_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    struct EwramData_EntityData *parent = entity->unk_500.unk_500_entity;
    s32 x, y, damage;
    if ((u8)(*(u8 *)&entity->unk_504 - 12) <= 1)
    {
        x = parent->posX.whole;
        y = parent->posY.whole;
        parent->posX.whole = entity->posX.whole;
        parent->posY.whole = entity->posY.whole;
        Player_CalculateAttackPower(hit[2]);
        Hit_ResolveDamage(parent, attacker, hit, 0);
        parent->posX.whole = x;
        parent->posY.whole = y;
        PlaySong(0x69);
    }
    else
    {
        x = parent->posX.whole;
        y = parent->posY.whole;
        parent->posX.whole = entity->posX.whole;
        parent->posY.whole = entity->posY.whole;
        damage = Player_CalculateAttackPower(hit[2]);
        damage = Hit_ResolveDamage(parent, attacker, hit, (s16)damage);
        parent->posX.whole = x;
        parent->posY.whole = y;
        damage = (s16)damage;
        Entity_ShowDamage(entity, damage, 0);
        BigGolem_OnDamaged(entity->unk_500.unk_500_entity, attacker, hit, damage);
    }
}
