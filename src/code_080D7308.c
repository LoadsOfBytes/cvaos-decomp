/* SoulPickup_SpawnFromEnemyDefinition / BossRecoveryPickup_Draw. */
#include "code_0800B700.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

/* Entity definitions (0x24 bytes each); only the bytes used here named. */
struct EnemySoulDropView {
    u8 pad_0[0x17];
    u8 soulType;
    u8 oneBasedSoulId;
    u8 pad_19[0x24 - 0x19];
};

extern struct EnemySoulDropView sEnemyData[];
extern struct EwramData_EntityData *SoulPickup_Create(s16 x, s16 y, s32 palette, s32 soulId, s32 stationary);

/**
 * @brief For Soma under the existing mode guard, create the enemy-definition soul pickup and store its persistence flag and substate two.
 */
void SoulPickup_SpawnFromEnemyDefinition(u16 x, u16 y, s32 enemyId, s32 spawnParam)
{
    struct EnemySoulDropView *enemyDef = &sEnemyData[enemyId];
    u8 palette = enemyDef->soulType;
    u8 soulId = enemyDef->oneBasedSoulId;

    if (gEwramData->gameState.bossRushMode == 0 && gEwramData->playerStatus.currentCharacter == 0)
    {
        struct EwramData_EntityData *pickup = SoulPickup_Create(x, y, palette, soulId - 1, 0);

        if (pickup != NULL)
        {
            pickup->spawnParam0 = spawnParam;
            pickup->subState = 2;
        }
    }
}

/**
 * @brief Draw the recovery pickup. While its substate is nonzero,
 * count stage down and then enable menus and clear pause. Preserve
 * this draw-time release relative to HP/MP display recovery.
 */
void BossRecoveryPickup_Draw(struct EwramData_EntityData *entity)
{
    Sprite_Draw(entity);
    if (entity->subState != 0)
    {
        if (entity->stage != 0)
            entity->stage--;
        else
        {
            Game_SetMenusEnabled(1);
            gEwramData->paused = 0;
        }
    }
}
