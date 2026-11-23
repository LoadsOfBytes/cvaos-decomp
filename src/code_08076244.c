/* HarpyFeather_EnableHitBoxes (hit boxes). */
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern void HarpyFeather_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void HarpyFeather_OnHit(struct EwramData_EntityData *entity);

/**
 * @brief 76244 | Activate box slot 8 with HarpyFeather_DamagePlayerOnContact / HarpyFeather_OnHit and
 * set both boxes to the bytes of 0x0606FDFD (x -3, y -3, w 6, h 6 - not a
 * ROM address); clear flags bits 2 and 1.
 */
void HarpyFeather_EnableHitBoxes(struct EwramData_EntityData *entity)
{
    u32 box = 0x0606FDFD;

    Hit_Enable(entity, 8, HarpyFeather_DamagePlayerOnContact, (s32)HarpyFeather_OnHit);
    HitBox_SetBody(entity, &box);
    HitBox_SetAttack(entity, &box);
    entity->hit.flags &= ~4;
    entity->hit.flags &= ~2;
}
