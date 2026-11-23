/* Hit callback: compute damage and perform the boss's death transition. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "structs/ewram.h"
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *, struct EwramData_EntityData *, u16 *, s16);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s32, s32);
extern void Enemy_DropRewards(struct EwramData_EntityData *);
extern void BossDefeatEffect_Spawn(s32, s32);
static inline s32 SubtractDamage(u16 health, s16 damage) { return health - damage; }
void Chaos2_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    u16 *savedHit = (u16 *)hit;
    s32 timer = (s8)entity->ext.chaos2.partCount * 20 + (u8)entity->ext.chaos2.baseDefense;
    s32 damage;
    if (timer <= 255) entity->defense = timer;
    else entity->defense = 255;
    damage = Hit_ResolveDamage(entity, attacker, savedHit, (s16)Player_CalculateAttackPower(hit[2]));
    entity->hitFlashTimer = 8;
    entity->hp = SubtractDamage(entity->hp, damage);
    Entity_ShowDamage(entity, (s16)damage, 0);
    if ((s16)entity->hp <= 0)
    {
        Enemy_DropRewards(entity);
        entity->palette = entity->basePalette;
        entity->timer = 120;
        entity->ext.chaos2.defeatReady = 0;
        entity->state = 3;
        entity->subState = 0;
        BossDefeatEffect_Spawn(entity->posX.whole, entity->posY.whole);
        gEwramData->gameState.stateFlags &= ~1;
        PlaySong(0x1010);
        gEwramData->startPauseEnabled = 0;
    }
}
