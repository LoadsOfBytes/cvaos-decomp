/* StudentWitch_OnHit: variant of an already decompiled function, generated with
 * a cloning script (constants adjusted where needed), checked byte for
 * byte.
 * Witch_OnHit and WitchGroundProjectile_OnHit decompiled locally. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 Enemy_TakeDamage(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern void Entity_ShowDamage(struct EwramData_EntityData *owner, s16 value, s32 kind);
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);

/**
 * @brief Apply enemy damage; on defeat drop rewards, restore the palette
 * and enter death state 4 with substate counters reset.
 */
void StudentWitch_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    if (Enemy_TakeDamage(entity, attacker, hit))
    {
        Enemy_DropRewards(entity);
        entity->palette = entity->unk_510.part8.basePalette;
        entity->state = 4;
        entity->stage = 0;
        entity->subState = 0;
    }
}

/**
 * @brief While guard flag 1 is set, play the clank and show zero damage.
 * Otherwise apply enemy damage and enter death state 3 on defeat.
 */
void Witch_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s32 invulnerable = entity->ext.witch.flags & 1;

    if (invulnerable)
    {
        PlaySong(0x69);
        Entity_ShowDamage(entity, 0, 0);
    }
    else if (Enemy_TakeDamage(entity, attacker, hit))
    {
        Enemy_DropRewards(entity);
        entity->palette = entity->unk_510.part8.basePalette;
        entity->state = 3;
        entity->stage = invulnerable;
        entity->subState = invulnerable;
    }
}

/**
 * @brief Clear projectile HP so its update enters the burst state.
 */
void WitchGroundProjectile_OnHit(struct EwramData_EntityData *entity)
{
    entity->unk_518.part16.hp = 0;
}
