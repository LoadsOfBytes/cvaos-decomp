/* Skull Millione / Cagnazzo / Lubicant helpers 0x080B8004..0x080B80BB. */
#include "code_08009A0.h"
#include "structs/ewram.h"

extern s32 Enemy_TakeDamage(struct EwramData_EntityData *param_0);
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);
extern s32 SkullMillioneFamily_Init(struct EwramData_EntityData *entity, s32 variant);
extern void SkullMillioneFamily_Update(struct EwramData_EntityData *entity);

/**
 * @brief Apply enemy damage; on defeat drop rewards, restore the palette
 * and enter death state 3 with the substate counters reset.
 */
void SkullMillioneFamily_OnHit(struct EwramData_EntityData *entity)
{
    if (Enemy_TakeDamage(entity))
    {
        Enemy_DropRewards(entity);
        entity->palette = entity->unk_510.part8.basePalette;
        entity->state = 3;
        entity->stage = 0;
        entity->subState = 0;
    }
}

/**
 * @brief B8044 | Skull Millione create: SkullMillioneFamily_Init variant 0, delete the
 * entity when that fails.
 */
void EnemySkullMillioneCreate(struct EwramData_EntityData *entity)
{
    if (SkullMillioneFamily_Init(entity, 0))
        EntityDelete(entity);
}

/**
 * @brief B8060 | Skull Millione update.
 */
void EnemySkullMillioneUpdate(struct EwramData_EntityData *entity)
{
    SkullMillioneFamily_Update(entity);
}

/**
 * @brief B806C | Cagnazzo create: variant 1.
 */
void EnemyCagnazzoCreate(struct EwramData_EntityData *entity)
{
    if (SkullMillioneFamily_Init(entity, 1))
        EntityDelete(entity);
}

/**
 * @brief B8088 | Cagnazzo update: same as Skull Millione.
 */
void EnemyCagnazzoUpdate(struct EwramData_EntityData *entity)
{
    SkullMillioneFamily_Update(entity);
}

/**
 * @brief B8094 | Lubicant create: variant 2.
 */
void EnemyLubicantCreate(struct EwramData_EntityData *entity)
{
    if (SkullMillioneFamily_Init(entity, 2))
        EntityDelete(entity);
}

/**
 * @brief B80B0 | Lubicant update: same as Skull Millione.
 */
void EnemyLubicantUpdate(struct EwramData_EntityData *entity)
{
    SkullMillioneFamily_Update(entity);
}
