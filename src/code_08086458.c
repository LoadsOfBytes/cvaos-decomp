/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Headhunter_OnHit = Manticore_OnHit */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern void Headhunter_ApplyResolvedDamage(struct EwramData_EntityData *entity, s32 unused1, u16 *unused2, s32 damage);
extern u16 Player_CalculateAttackPower(u16 attackCategory);
extern s16 Hit_ResolveDamage(struct EwramData_EntityData *entity, s32 other, u16 *hit, s32 inputDamage);
extern void Entity_ShowDamage(struct EwramData_EntityData *owner, s16 value, s32 kind);

/**
 * @brief Resolve and display root body damage, then forward it to
 * the phase-aware resolved-damage helper.
 */
void Headhunter_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s16 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));

    Entity_ShowDamage(entity, damage, 0);
    Headhunter_ApplyResolvedDamage(entity, attacker, hit, damage);
}
