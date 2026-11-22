/* Killer Doll: stop movement, resolve floor placement and initialize three
 * randomized timers. */
#include "code_08009A0.h"
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
 * @brief Clear velocity and acceleration, snap to ground and enter state 1 with three randomized behavior countdowns.
 */
void KillerDoll_ResetActive(struct EwramData_EntityData *entity)
{
    s32 zero = 0;

    entity->accelX.whole = zero;
    entity->velX.whole = zero;
    entity->accelY.whole = zero;
    entity->velY.whole = zero;
    Entity_SnapToGround(entity);
    entity->state = 1;
    entity->stage = zero;
    entity->subState = zero;
    entity->unk_4F8.part16.unk_4F8 = (RandomNumberGenerator() % 0x800) + 0x80;
    entity->unk_4F8.part16.unk_4FA = (RandomNumberGenerator() % 0x100) + 0x20;
    entity->unk_4FC.part16.unk_4FC = (RandomNumberGenerator() % 0x200) + 0x100;
}
