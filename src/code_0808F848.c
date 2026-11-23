#include "code_08009A0.h"
#include "dead_crusader_entity.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
extern s16 Hit_ResolveDamage(struct EwramData_EntityData *, s32, u16 *, s32);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s16, s32);
extern void Enemy_DropRewards(struct EwramData_EntityData *);

/**
 * @brief Hit: while crouching behind the shield (DEAD_CRUSADER_GUARDING) take no damage (sound
 * 0x69), else resolve the damage; flash, subtract and show it; at HP <= 0
 * drop the rewards and die (song 0x72, 0x28 frames, state 2).
 */
void DeadCrusader_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    s32 signedDamage;
    u16 damage = Player_CalculateAttackPower(hit[2]);
    if ((s8)DEAD_CRUSADER_GUARDING(entity) != 0)
    {
        Hit_ResolveDamage(entity, (s32)attacker, hit, 0);
        damage = 0;
        PlaySong(0x69);
    }
    else damage = Hit_ResolveDamage(entity, (s32)attacker, hit, (s16)damage);
    entity->unk_510.part8.hitFlashTimer = 8;
    {
        u16 hp = entity->unk_518.part16.hp;
        signedDamage = damage;
        signedDamage <<= 16;
        signedDamage >>= 16;

        entity->unk_518.part16.hp = hp - signedDamage;
    }
    Entity_ShowDamage(entity, signedDamage, 0);
    if ((s16)entity->unk_518.part16.hp <= 0)
    {
        Enemy_DropRewards(entity);
        PlaySong(0x72);
        entity->palette = entity->unk_510.part8.basePalette;
        entity->timer = 0x28;
        entity->state = 2;
        entity->subState = 0;
    }
}
