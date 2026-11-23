#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
extern s16 Hit_ResolveDamage(struct EwramData_EntityData *, s32, u16 *, s32);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s16, s32);
extern void Enemy_DropRewards(struct EwramData_EntityData *);

/**
 * @brief Hit on the head or a segment: resolve the damage and apply it to the
 * worm's root entity (flash, HP, death).
 */
void Worm_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    struct EwramData_EntityData *parent = ENTITY_OWNER(entity);
    s32 damage = Player_CalculateAttackPower(hit[2]);
    damage = Hit_ResolveDamage(entity, (s32)attacker, hit, (s16)damage);
    parent->hitFlashTimer = 8;
    {
        u16 hp = parent->hp;
        damage <<= 16;
        damage >>= 16;
        parent->hp = hp - damage;
    }
    Entity_ShowDamage(entity, damage, 0);
    if ((s16)parent->hp <= 0)
    {
        Enemy_DropRewards(parent);
        PlaySong(0x70);
        parent->palette = parent->basePalette;
        parent->state = 3;
        parent->timer = 0x40;
        parent->velX.whole = -0x8000;
        if (gEwramData->playerState.player->posX.whole > parent->posX.whole) parent->velX.whole = -parent->velX.whole;
        parent->velY.whole = -0x30000;
    }
}
