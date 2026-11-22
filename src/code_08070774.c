/* KillerFishSwarm_EnableHitBoxes (hit boxes). */
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern void KillerFishSwarm_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void KillerFishSwarm_OnHit(struct EwramData_EntityData *param_0);

/**
 * @brief 70774 | Activate box slot 8 with KillerFishSwarm_DamagePlayerOnContact / KillerFishSwarm_OnHit and
 * set both boxes to the bytes of 0x0808FCFC (x -4, y -4, w 8, h 8 - not a
 * ROM address); clear flags bits 2 and 1.
 */
void KillerFishSwarm_EnableHitBoxes(struct EwramData_EntityData *entity)
{
    u32 box = 0x0808FCFC;

    Hit_Enable(entity, 8, KillerFishSwarm_DamagePlayerOnContact, (s32)KillerFishSwarm_OnHit);
    HitBox_SetBody(entity, &box);
    HitBox_SetAttack(entity, &box);
    entity->hit.flags &= ~4;
    entity->hit.flags &= ~2;
}
