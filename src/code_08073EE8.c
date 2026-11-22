/* ManticoreTail_OnHit (Manticore body part hit). */
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u16 Player_CalculateAttackPower(u16 attackCategory);
extern s16 Hit_ResolveDamage(struct EwramData_EntityData *entity, s32 other, u16 *hit, s32 inputDamage);
extern void Entity_ShowDamage(struct EwramData_EntityData *owner, s16 value, s32 kind);
extern void Manticore_OnDamaged(struct EwramData_EntityData *entity, s32 attacker, u16 *hit, s32 damage);

/**
 * @brief 73EE8 | A part was hit: compute the damage for the parent
 * (unk_4F8) as if it stood at the part's position, show it on the part
 * (Entity_ShowDamage) and pass the hit on to the parent (Manticore_OnDamaged).
 */
void ManticoreTail_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    struct EwramData_EntityData *manticore = ENTITY_OWNER(entity);
    s32 oldX = manticore->posX.whole;
    s32 oldY = manticore->posY.whole;
    s16 damage;
    s16 attackPower;

    manticore->posX.whole = entity->posX.whole;
    manticore->posY.whole = entity->posY.whole;
    attackPower = Player_CalculateAttackPower(hit[2]);
    damage = Hit_ResolveDamage(ENTITY_OWNER(entity), attacker, hit, attackPower);
    manticore->posX.whole = oldX;
    manticore->posY.whole = oldY;
    Entity_ShowDamage(entity, damage, 0);
    Manticore_OnDamaged(ENTITY_OWNER(entity), attacker, hit, damage);
}
