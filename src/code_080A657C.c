/* Chaos2SnakeSegment_EnableHitBoxes (hit boxes). */
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern void Chaos2SnakeSegment_DamagePlayerOnContact(struct EwramData_EntityData *entity);

/**
 * @brief A657C | Activate box slot 8 with Chaos2SnakeSegment_DamagePlayerOnContact and set both boxes to
 * the bytes of 0x0808FCFC (x -4, y -4, w 8, h 8 - not a ROM address);
 * clear flags bit 1, set bit 2.
 */
void Chaos2SnakeSegment_EnableHitBoxes(struct EwramData_EntityData *entity)
{
    u32 box = 0x0808FCFC;

    Hit_Enable(entity, 8, Chaos2SnakeSegment_DamagePlayerOnContact, 0);
    HitBox_SetBody(entity, &box);
    HitBox_SetAttack(entity, &box);
    entity->hit.flags &= ~2;
    entity->hit.flags |= 4;
}
