/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Quezlcoatl_OnHit = WingedSkeleton_OnHit */
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern u16 Player_CalculateAttackPower(u16 attackCategory);
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *entity, s32 other, u16 *hit, s32 inputDamage);
extern void Entity_ShowDamage(struct EwramData_EntityData *owner, s16 value, s32 kind);
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);

/**
 * @brief 7829C | Same as WingedSkeleton_OnHit.
 */
void Quezlcoatl_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
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
        PlaySong(0x6B);
        entity->palette = entity->unk_510.part8.basePalette;
        entity->timer = 0x28;
        entity->state = 2;
        entity->subState = 0;
    }
}
