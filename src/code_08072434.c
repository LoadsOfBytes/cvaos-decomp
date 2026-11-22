/* MudmanShot_EnableHitBoxes (hit boxes, same code as MermanShot_EnableHitBoxes). */
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern void MudmanShot_DamagePlayerOnContact(struct EwramData_EntityData *param_0);
extern void MudmanShot_OnHit(struct EwramData_EntityData *param_0);

/**
 * @brief 72434 | Activate box slot 0xA with MudmanShot_DamagePlayerOnContact / MudmanShot_OnHit and set both
 * boxes to the bytes of 0x0808FCFC (x -4, y -4, w 8, h 8 - not a ROM
 * address); clear flags bits 1 and 2.
 */
void MudmanShot_EnableHitBoxes(struct EwramData_EntityData *entity)
{
    u32 box = 0x0808FCFC;

    Hit_Enable(entity, 0xA, MudmanShot_DamagePlayerOnContact, (s32)MudmanShot_OnHit);
    HitBox_SetBody(entity, &box);
    HitBox_SetAttack(entity, &box);
    entity->hit.flags &= ~2;
    entity->hit.flags &= ~4;
}
