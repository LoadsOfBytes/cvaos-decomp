/* SkullArcher_OnHit (hit, knocked away from the attacker). */
#include "structs/ewram.h"

extern s32 Enemy_TakeDamage(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);

/**
 * @brief AFD3C | Hit by attacker: when killed (Enemy_TakeDamage), unk_504 is
 * the direction away from attacker (1 / -1), palette into 0x53E bits 3-6,
 * state 3.
 */
void SkullArcher_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    if (Enemy_TakeDamage(entity, (s32)attacker, hit))
    {
        Enemy_DropRewards(entity);
        {
            s32 speed = 1;

            entity->ext.skullArcher.hitSide = speed;
            if (entity->posX.whole < attacker->posX.whole)
                entity->ext.skullArcher.hitSide = -speed;
        }
        entity->palette = entity->basePalette;
        entity->state = 3;
        entity->stage = 0;
        entity->subState = 0;
    }
}
