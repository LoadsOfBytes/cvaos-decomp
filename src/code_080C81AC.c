/* Killer Doll damage and transition to death, reconstructed from the
 * original assembly. */
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

/**
 * @brief Resolve damage and flash; on lethal damage spawn counted body parts, drop rewards and wait for the part count to reach zero.
 */
void KillerDoll_OnHit(struct EwramData_EntityData *entity, s32 hit, u16 *data)
{
    s32 damage = Hit_ResolveDamage(entity, hit, data, (s16)Player_CalculateAttackPower(data[2]));
    u16 hp = entity->unk_518.part16.hp;

    damage = (damage << 16) >> 16;
    entity->unk_518.part16.hp = hp - damage;
    Entity_ShowDamage(entity, damage, 0);
    entity->unk_510.part8.hitFlashTimer = 8;
    if ((s16)entity->unk_518.part16.hp <= 0)
    {
        entity->subState = 0;
        PlaySong(0x6E);
        KillerDoll_SpawnDeathParts(entity);
        Enemy_DropRewards(entity);
        entity->palette = entity->unk_510.part8.basePalette;
        entity->state = 0xFF;
        entity->subState++;
        if (entity->ext.doll.deathPartCount == 0)
            entity->deleteRequested = 1;
    }
}
