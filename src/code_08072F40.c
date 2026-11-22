#include "code_08009A0.h"
#include "code_08033CAC.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void Enemy_DropRewards(struct EwramData_EntityData *);
extern void BossDefeatEffect_Spawn(s32, s32);
extern void BossRecoveryPickup_Spawn(s32, s32, s32, s32, s32);

/**
 * @brief 72F40 | Enemy damage callback: subtract the damage from HP
 * (unk_518), flash (hitFlashTimer = 8); at 0 HP: Enemy_DropRewards, song 0x72, death
 * animation 7 with palette unk_510, state 2 for 40 frames; with a drop id
 * (unk_514) spawn BossDefeatEffect_Spawn and the item BossRecoveryPickup_Spawn (id 10) at its
 * x / 0x20, or at room (0x100, 0x350) when bossRushMode is 0, then
 * Player_SetSelectedTarget(NULL).
 */
void Manticore_OnDamaged(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit, s32 damage)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];

    entity->unk_510.part8.hitFlashTimer = 8;
    entity->unk_518.part16.hp -= damage;
    if ((s16)entity->unk_518.part16.hp > 0)
        return;
    Enemy_DropRewards(entity);
    PlaySong(0x72);
    Anim_StartIndexed(entity, 7, 0, 0);
    entity->palette = entity->unk_510.part8.basePalette;
    entity->timer = 0x28;
    entity->state = 2;
    entity->subState = 0;
    if (entity->unk_514.part16.spawnParam0 == 0)
        return;
    BossDefeatEffect_Spawn(entity->posX.whole, entity->posY.whole - 0x200000);
    if (gEwramData->gameState.bossRushMode)
        BossRecoveryPickup_Spawn((s16)entity->posX.part16.integer, 0x20, 0xA, 0x60, 0);
    else
        BossRecoveryPickup_Spawn((s16)(0x100 - bg->xPos.part16.integer), (s16)(0x350 - bg->yPos.part16.integer), 0xA, 0x60, 0);
    Player_SetSelectedTarget(NULL);
}
