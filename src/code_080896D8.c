/* Apply damage and signal a hit to the body's current behavior. */
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code_080D73B8.h"
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *, struct EwramData_EntityData *, u16 *, s16);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s32, s32);
extern void Enemy_DropRewards(struct EwramData_EntityData *);
/**
 * @brief Hit: resolve the damage, flash (hitFlashTimer = 8), subtract it from the HP and
 * show it, mark the Gorgon as hit (unk_500 = 1); at HP <= 0 drop the
 * rewards and die (song 0x71, 60 frames, state 2).
 */
void Gorgon_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    s16 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));
    s32 zero;
    entity->unk_510.part8.hitFlashTimer = 8;
    {
        s32 hp = entity->unk_518.part16.hp;
        zero = 0;
        entity->unk_518.part16.hp = hp - damage;
    }
    Entity_ShowDamage(entity, damage, 0);
    entity->unk_500.part8.unk_500 = 1;
    if ((s16)entity->unk_518.part16.hp <= 0)
    {
        Enemy_DropRewards(entity);
        PlaySong(0x71);
        entity->palette = entity->unk_510.part8.basePalette;
        entity->timer = 60;
        entity->state = 2;
        entity->subState = zero;
        entity->stage = zero;
    }
}
