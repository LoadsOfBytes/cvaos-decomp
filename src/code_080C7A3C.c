/* Killer Doll child visibility/deletion helpers, reconstructed from the
 * original assembly. */
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
 * @brief Set hidden on each nonzero slot in the three stored afterimage slots.
 */
void Nemesis_SetAfterimagesHidden(struct EwramData_EntityData *entity, s32 hidden)
{
    u8 children[3] = {
        entity->unk_504.part8.unk_505,
        entity->unk_504.part8.unk_506,
        entity->unk_504.part8.unk_507
    };
    s32 i;

    for (i = 0; i < 3; i++)
        if (children[i] != 0)
        {
            struct EwramData_EntityData *child = &gEwramData->entityData[children[i]];
            child->hidden = hidden;
        }
}

/**
 * @brief Request deletion of each nonzero slot in the three stored afterimage slots.
 */
void Nemesis_DeleteAfterimages(struct EwramData_EntityData *entity)
{
    u8 children[3] = {
        entity->unk_504.part8.unk_505,
        entity->unk_504.part8.unk_506,
        entity->unk_504.part8.unk_507
    };
    s32 i;

    for (i = 0; i < 3; i++)
        if (children[i] != 0)
        {
            struct EwramData_EntityData *child = &gEwramData->entityData[children[i]];
            child->deleteRequested = 1;
        }
}

/**
 * @brief Empty state-7 handler. Its gameplay purpose remains unresolved.
 */
void Nemesis_UpdateState7(struct EwramData_EntityData *entity)
{
}
