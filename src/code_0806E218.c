/* Helpers 0x0806E218..0x0806E34F. */
#include "structs/ewram.h"

extern u16 Player_CalculateAttackPower(u16 attackCategory);
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *entity, s32 other, u16 *hit, s32 inputDamage);
extern void Entity_ShowDamage(struct EwramData_EntityData *owner, s16 value, s32 kind);
extern s32 Entity_IsInRect(struct EwramData_EntityData *entity, s16 left, s16 top, u16 width, u16 height);
extern void Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);

/**
 * @brief 6E218 | Common hit: flash (hitFlashTimer = 8), subtract the damage from
 * the HP and show it; 1 when the HP dropped to 0 or below.
 */
s32 Enemy_TakeDamage(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
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
        return 1;
    return 0;
}

/**
 * @brief 6E268 | Index (6..1) of the highest set bit among bits 5..0 of
 * attributes, 0 when none is set.
 */
u16 Attribute_GetHighestIndex(u16 attributes)
{
    s32 mask = 0x20;
    u16 index = 0;
    s32 i;

    for (i = 6; i > 0; i--)
    {
        if (attributes & mask)
        {
            index = i;
            break;
        }
        mask >>= 1;
    }
    return index;
}

/**
 * @brief 6E29C | Entity_IsInRect of the target (enemyTarget, or linkTargetProxy when
 * linkPlay is set) with a width x height box centred on entity; 0
 * without a target.
 */
s32 Target_IsNearEntity(struct EwramData_EntityData *entity, u16 width, u16 height)
{
    struct EwramData_EntityData *target = gEwramData->playerState.enemyTarget;
    u16 left = entity->posX.part16.integer - (width >> 1);
    u16 top = entity->posY.part16.integer - (height >> 1);

    if (gEwramData->gameState.linkPlay)
        target = gEwramData->playerState.linkTargetProxy;
    if (target == NULL)
        return 0;
    return Entity_IsInRect(target, left, top, width, height);
}

/**
 * @brief 6E314 | Unless enemyFlags bit 1 is set: Hit_CheckWithPlayer when the box
 * (-16, 0, 0x110, 0xB0) test of Entity_IsInRect succeeds.
 */
void Enemy_CheckHitPlayer(struct EwramData_EntityData *entity)
{
    u8 hitInfo[0x10];

    if (!(entity->enemyFlags & 2) && Entity_IsInRect(entity, -0x10, 0, 0x110, 0xB0))
        Hit_CheckWithPlayer(hitInfo, entity);
}
