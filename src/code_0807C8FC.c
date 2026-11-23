/* Apply damage only when the current animation permits it. */
#include "code/code_080211F0.h"
#include "alura_une_entity.h"
#include "code/code_08039340.h"
#include "code_080D73B8.h"
#include "code_08009A0.h"
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *, struct EwramData_EntityData *, u16 *, s16);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s32, s32);
extern void Enemy_DropRewards(struct EwramData_EntityData *);
extern void Sound_StopTracked(s32);
/**
 * @brief Hit: no damage while closed (animation 0, sound 0x6A), else subtract and
 * show it; mark the hit (unk_505 = 1, cuts the idle wait short); at
 * HP <= 0 die.
 */
void AluraUne_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    u16 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));
    entity->unk_510.part8.hitFlashTimer = 8;
    RandomNumberGenerator();
    if (entity->animScriptIndex == 0)
    {
        damage = 0;
        PlaySong(0x6A);
    }
    {
        s32 hp = entity->unk_518.part16.hp;
        s32 applied = (s16)damage;
        entity->unk_518.part16.hp = hp - applied;
        Entity_ShowDamage(entity, applied, 0);
    }
    ALURA_UNE_HIT(entity) = 1;
    if ((s16)entity->unk_518.part16.hp <= 0)
    {
        Sound_StopTracked(0x105);
        Enemy_DropRewards(entity);
        PlaySong(0x6E);
        entity->palette = entity->unk_510.part8.basePalette;
        entity->timer = 40;
        entity->state = 2;
        entity->subState = 0;
    }
}
