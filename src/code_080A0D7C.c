/* Gladiator_OnDamaged. */
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern void Link_MarkEnemyDamaged(void);
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);

/**
 * @brief A0D7C | Hit callback: flash (hitFlashTimer = 8), subtract damage from
 * the HP in unk_518 and Link_MarkEnemyDamaged; at 0 HP play song 0x71, release the
 * hit box (Enemy_DropRewards) and enter the death state 1 (timer 0x3C, palette
 * from unk_510, gravity 0x1000).
 */
void Gladiator_OnDamaged(struct EwramData_EntityData *entity, s32 unused1, s32 unused2, s32 damage)
{
    entity->unk_510.part8.hitFlashTimer = 8;
    entity->unk_518.part16.hp -= damage;
    Link_MarkEnemyDamaged();
    if ((s16)entity->unk_518.part16.hp <= 0)
    {
        PlaySong(0x71);
        Enemy_DropRewards(entity);
        entity->unk_4F8.part8.unk_4F8 = 1;
        entity->palette = entity->unk_510.part8.basePalette;
        entity->timer = 0x3C;
        entity->state = 1;
        entity->subState = 0;
        entity->stage = 0;
        entity->accelX.whole = 0;
        entity->accelY.whole = 0x1000;
        entity->velY.whole = 0;
    }
}
