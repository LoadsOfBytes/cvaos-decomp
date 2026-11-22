/* TsuchinokoShot_EnableHitBoxes (hit boxes, same code as MermanShot_EnableHitBoxes). */
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern void TsuchinokoShot_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void TsuchinokoShot_OnHit(struct EwramData_EntityData *param_0);

/**
 * @brief A785C | Activate box slot 0xA with TsuchinokoShot_DamagePlayerOnContact / TsuchinokoShot_OnHit and set both
 * boxes to the bytes of 0x0808FCFC (x -4, y -4, w 8, h 8 - not a ROM
 * address); clear flags bits 1 and 2.
 */
void TsuchinokoShot_EnableHitBoxes(struct EwramData_EntityData *entity)
{
    u32 box = 0x0808FCFC;

    Hit_Enable(entity, 0xA, TsuchinokoShot_DamagePlayerOnContact, (s32)TsuchinokoShot_OnHit);
    HitBox_SetBody(entity, &box);
    HitBox_SetAttack(entity, &box);
    entity->hit.flags &= ~2;
    entity->hit.flags &= ~4;
}
