/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   HeadhunterAttackSegment_OnHit = ManticoreTail_OnHit */
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void Headhunter_ApplyResolvedDamage(struct EwramData_EntityData *entity, s32 unused1, u16 *unused2, s32 damage);
extern u16 Player_CalculateAttackPower(u16 attackCategory);
extern s16 Hit_ResolveDamage(struct EwramData_EntityData *entity, s32 other, u16 *hit, s32 inputDamage);
extern void Entity_ShowDamage(struct EwramData_EntityData *owner, s16 value, s32 kind);

/**
 * @brief Temporarily resolve boss damage at the struck segment position,
 * restore boss coordinates, display the number on that segment and
 * forward resolved damage to the boss phase handler.
 */
void HeadhunterAttackSegment_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    struct EwramData_EntityData *headhunter = ENTITY_OWNER(entity);
    s32 oldX = headhunter->posX.whole;
    s32 oldY = headhunter->posY.whole;
    s16 damage;
    s16 attackPower;

    headhunter->posX.whole = entity->posX.whole;
    headhunter->posY.whole = entity->posY.whole;
    attackPower = Player_CalculateAttackPower(hit[2]);
    damage = Hit_ResolveDamage(ENTITY_OWNER(entity), attacker, hit, attackPower);
    headhunter->posX.whole = oldX;
    headhunter->posY.whole = oldY;
    Entity_ShowDamage(entity, damage, 0);
    Headhunter_ApplyResolvedDamage(ENTITY_OWNER(entity), attacker, hit, damage);
}
