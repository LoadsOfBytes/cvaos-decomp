/* SkillWitchGiantGhost_OnContact. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "syscalls.h"
#include "structs/ewram.h"

extern void Hit_GetContactPoint(struct EwramData_EntityData *a, struct EwramData_EntityData *b, s16 *outX, s16 *outY);
void SkillWitchGiantGhostProjectileUpdate(struct EwramData_EntityData *entity);

/**
 * @brief Contact callback of the Witch/Giant Ghost soul: spawn a
 * SkillWitchGiantGhostProjectileUpdate child at the contact point moving
 * away from the entity, then set palette 4.
 */
void SkillWitchGiantGhost_OnContact(struct EwramData_EntityData *entity, struct EwramData_EntityData *target)
{
    struct EwramData_EntityData *child = BlueSoul_CreateEntity(entity, SkillWitchGiantGhostProjectileUpdate);
    s16 x, y;
    s16 dx;
    u16 angle;

    if (child != NULL)
    {
        Hit_GetContactPoint(entity, target, &x, &y);
        dx = x - entity->posX.part16.integer;
        angle = ArcTan2(dx, y - (entity->posY.part16.integer - 0x10) + RandomSigned(4));
        child->posX.part16.integer = x;
        child->posY.part16.integer = y;
        child->velX.whole = Sin(angle + 0x4000) << 2;
        child->velY.whole = Sin(angle) << 2;
    }
    entity->palette = 4;
}
