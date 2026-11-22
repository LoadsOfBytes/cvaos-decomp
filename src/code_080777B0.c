/* WhiteDragonBreath_EnableHitBoxes (hit boxes). */
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern void WhiteDragonBreath_DamagePlayerOnContact(struct EwramData_EntityData *entity);

/**
 * @brief 777B0 | Activate box slot 8 with WhiteDragonBreath_DamagePlayerOnContact and set both boxes to
 * the bytes of 0x0808FCFC (x -4, y -4, w 8, h 8 - not a ROM address);
 * clear flags bit 1, set bit 2.
 */
void WhiteDragonBreath_EnableHitBoxes(struct EwramData_EntityData *entity)
{
    u32 box = 0x0808FCFC;

    Hit_Enable(entity, 8, WhiteDragonBreath_DamagePlayerOnContact, 0);
    HitBox_SetBody(entity, &box);
    HitBox_SetAttack(entity, &box);
    entity->hit.flags &= ~2;
    entity->hit.flags |= 4;
}
