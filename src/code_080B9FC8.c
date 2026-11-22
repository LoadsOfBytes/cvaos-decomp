/* Man Eater part hit: propagate flags, subtract HP and notify parent.
 * The local r6 binding preserves the hit-data pointer before address calculation.
 * Matching does not establish the original source spelling. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
extern s16 Hit_ResolveDamage(struct EwramData_EntityData *, s32, u16 *, s32);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s16, s32);

/**
 * @brief Resolve damage against the linked head, propagate hit flags and
 * subtract its HP. On defeat mark it dead and decrement the root owner
 * head count. Preserve the existing casts and hit-pointer allocation form.
 */
void ManEaterPart_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    struct EwramData_EntityData *part = &gEwramData->entityData[(u8)entity->unk_4F8.part8.unk_4F8];
    u16 strength = Player_CalculateAttackPower(hit[2]);
    s32 damage;
    if (!(part->enemyFlags & 2))
    {
        if (part->enemyFlags & 1) entity->enemyFlags |= 1;
        else part->enemyFlags |= 1;
        do { damage = Hit_ResolveDamage(entity, (s32)attacker, hit, (s16)strength); } while (0);
        part->unk_4F4.part8.unk_4F5 |= 2;
        {
            u16 hp = part->unk_518.part16.hp;
            damage <<= 16;
            damage >>= 16;
            part->unk_518.part16.hp = hp - damage;
        }
        Entity_ShowDamage(entity, damage, 0);
        if ((s16)part->unk_518.part16.hp <= 0)
        {
            struct EwramData_EntityData *parent = &gEwramData->entityData[(u8)part->unk_4F8.part8.unk_4F8];
            parent->unk_500.part8.unk_500--;
            part->enemyFlags |= 2;
            part->palette = part->unk_510.part8.basePalette;
        }
    }
}
