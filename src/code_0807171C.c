/* MermanShot_EnableHitBoxes (Merman hit boxes). */
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern void MermanShot_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void MermanShot_OnHit(struct EwramData_EntityData *entity);

/**
 * @brief 7171C | Activate box slot 0xA with MermanShot_DamagePlayerOnContact / MermanShot_OnHit and
 * set both boxes to the bytes of 0x0810FCF8 (x -8, y -4, w 16, h 8 - not a
 * ROM address); clear flags bits 1 and 2.
 */
void MermanShot_EnableHitBoxes(struct EwramData_EntityData *entity)
{
    u32 box = 0x0810FCF8;

    Hit_Enable(entity, 0xA, MermanShot_DamagePlayerOnContact, (s32)MermanShot_OnHit);
    HitBox_SetBody(entity, &box);
    HitBox_SetAttack(entity, &box);
    entity->hit.flags &= ~2;
    entity->hit.flags &= ~4;
}
