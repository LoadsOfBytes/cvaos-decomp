/* Enemy helpers 0x080B6AE8..0x080B6B7F. */
#include "code_08033CAC.h"
#include "balore_entity.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "syscalls.h"
#include "structs/ewram.h"

extern s32 Enemy_TakeDamage(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);
extern s32 Target_GetX(struct EwramData_EntityData *entity);
extern s32 Target_GetY(struct EwramData_EntityData *entity);
extern void HitEffect_SpawnAtContact(s32 attacker, struct EwramData_EntityData *entity, s32 type);

/**
 * @brief B6AE8 | Hit: set bit 0 of unk_502; when killed (Enemy_TakeDamage)
 * also bit 1, Enemy_DropRewards, palette into 0x53E bits 3-6, state 4 and
 * Player_SetSelectedTarget(NULL).
 */
void Balore_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    BALORE_FLAGS(entity) |= 1;
    if (Enemy_TakeDamage(entity, attacker, hit))
    {
        BALORE_FLAGS(entity) |= 2;
        Enemy_DropRewards(entity);
        entity->palette = entity->unk_510.part8.basePalette;
        entity->state = 4;
        entity->subState = 0;
        Player_SetSelectedTarget(NULL);
    }
}

/**
 * @brief B6B3C | Player_ApplyContactDamage with (0x4B, 2, 0, 1) at the entity position.
 */
void BaloreSpark_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attributes = 2;
    contactDamage.unk_4 = 0;
    contactDamage.attack = 0x4B;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief B6B68 | HitEffect_SpawnAtContact(attacker, entity, 0) and play sound 0x6A.
 */
void BaloreHandHitBox_OnHit(struct EwramData_EntityData *entity, s32 attacker)
{
    HitEffect_SpawnAtContact(attacker, entity, 0);
    PlaySong(0x6A);
}

/* Integer part of a 16.16 value, rounded towards zero. */
#define FIXED_TO_INT_TRUNC(a) (((a) < 0) ? -(-(a) >> 16) : ((a) >> 16))

/**
 * @brief B6B80 | Angle from the entity (offset by 0x20, 0x10) to the
 * target position given by Target_GetX / Target_GetY.
 */
u16 Balore_GetAngleToTarget(struct EwramData_EntityData *entity)
{
    s16 pos = entity->posX.part16.integer;
    s32 dx = (s16)(FIXED_TO_INT_TRUNC(Target_GetX(entity)) - pos - 0x20);
    s32 dy;

    pos = entity->posY.part16.integer;
    dy = (s16)(FIXED_TO_INT_TRUNC(Target_GetY(entity)) - pos - 0x10);
    return ArcTan2(dx, dy);
}
