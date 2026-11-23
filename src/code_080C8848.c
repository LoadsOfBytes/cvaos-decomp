/* Killer Doll update and death waiting for the body parts to disappear.
 * Reconstructed from the original assembly. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void Entity_SnapToGround(struct EwramData_EntityData *entity);
extern u16 Player_CalculateAttackPower(u16 value);
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *entity, s32 hit, u16 *data, s32 kind);
extern void Entity_ShowDamage(struct EwramData_EntityData *entity, s16 damage, s32 flags);
extern void KillerDoll_SpawnDeathParts(struct EwramData_EntityData *entity);
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);
extern s32 Enemy_IsStunned(struct EwramData_EntityData *entity);
extern void Hit_CheckWithPlayer(void *buffer, struct EwramData_EntityData *entity);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *entity);
extern void KillerDoll_UpdateState2Animation(struct EwramData_EntityData *entity);
extern void KillerDoll_UpdateActive(struct EwramData_EntityData *entity);
extern void KillerDoll_UpdateDeath(struct EwramData_EntityData *entity);

void EnemyKillerDollUpdate(struct EwramData_EntityData *entity)
{
    u8 buffer[0x10];

    if (entity->state == 2)
    {
        KillerDoll_UpdateState2Animation(entity);
        return;
    }
    Player_TrackEntity(entity);
    if (Enemy_IsStunned(entity) && entity->state != 0xFF)
    {
        Hit_CheckWithPlayer(buffer, entity);
        return;
    }
    switch (entity->state)
    {
        case 1:
            KillerDoll_UpdateActive(entity);
            break;
        case 0xFF:
            KillerDoll_UpdateDeath(entity);
            break;
    }
    if (entity->state != 0xFF)
    {
        Entity_AnimateWithBoxes(entity);
        Hit_CheckWithPlayer(buffer, entity);
    }
}

/**
 * @brief Spawn body parts and drop rewards once, restore the palette and request deletion when the live-part count reaches zero.
 */
void KillerDoll_UpdateDeath(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            PlaySong(0x6E);
            KillerDoll_SpawnDeathParts(entity);
            Enemy_DropRewards(entity);
            entity->palette = entity->basePalette;
            entity->state = 0xFF;
            entity->subState++;
        case 1:
            if (entity->ext.doll.deathPartCount == 0)
                entity->deleteRequested = 1;
            break;
    }
}
