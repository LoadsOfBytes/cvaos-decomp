/* ZombieSoldier_OnHit (hit with HP / death), generated with a template script from the
 * template of WingedSkeleton_OnHit and checked byte for byte. */
#include "code/code_08039340.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern u16 Player_CalculateAttackPower(u16 attackCategory);
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *entity, s32 other, u16 *hit, s32 inputDamage);
extern void Entity_ShowDamage(struct EwramData_EntityData *owner, s16 value, s32 kind);
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);

/**
 * @brief 92B38 | Hit: flash (hitFlashTimer = 8), subtract the damage from the HP
 * and show it; at HP <= 0 die (Enemy_DropRewards, song 0x70, animation 3, palette into
 * 0x53E bits 3-6, timer 0x28, state 2). Same template as WingedSkeleton_OnHit.
 */
void ZombieSoldier_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s32 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));

    entity->unk_510.part8.hitFlashTimer = 8;
    {
        u16 hp = entity->unk_518.part16.hp;

        // (s16)damage, written so that agbcc sign-extends after the HP load
        damage <<= 16;
        damage >>= 16;
        entity->unk_518.part16.hp = hp - damage;
    }
    Entity_ShowDamage(entity, damage, 0);
    if ((s16)entity->unk_518.part16.hp <= 0)
    {
        Enemy_DropRewards(entity);
        PlaySong(0x70);
        Anim_StartIndexed(entity, 3, 0, 0);
        entity->palette = entity->unk_510.part8.basePalette;
        entity->timer = 0x28;
        entity->state = 2;
        entity->subState = 0;
    }
}
