/* Helpers 0x080A1B80..0x080A1BEB. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern void EntityDampVelocityX(struct EwramData_EntityData *entity, s32 factor);
extern s32 Entity_GetVisibilityPattern(s32 step);

/**
 * @brief A1B80 | Hit callback: Player_CalculateAttackPower(hit[2]) and put the
 * parent (unk_4F8_entity) into state 1.
 */
void GladiatorShotTip_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    Player_CalculateAttackPower(hit[2]);
    entity->unk_4F8.unk_4F8_entity->state = 1;
}

/**
 * @brief A1B98 | Slow down and fall; blink (bit 5 of 0x53C) during the
 * last 0x28 frames of the timer timer and delete when it runs out.
 */
void GladiatorFragment_Update(struct EwramData_EntityData *entity)
{
    EntityDampVelocityX(entity, 0x20);
    EntityAccelerateAndMove(entity, 0x80000);
    if (entity->timer <= 0x28)
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);

    Anim_Update(entity);
    if ((s8)--entity->timer <= 0)
        EntityDelete(entity);
}
