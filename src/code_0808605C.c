/* Headhunter effect: follow the owner (unk_4F8) until the timer runs out. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
/* bit 0 of the owner's unk_4FE ends its effects (TODO: meaning) */
#define OWNER_DONE(o) ((o)->unk_4FC.part8.unk_4FE & 1)

/**
 * @brief Follow and animate at the owner position until the signed
 * countdown expires or the owner sets effect-cancel bit 0.
 */
void HeadhunterAttackFollower_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = entity->unk_4F8.unk_4F8_entity;
    entity->posX.whole = owner->posX.whole;
    entity->posY.whole = owner->posY.whole;
    Anim_Update(entity);
    if ((s8)entity->timer-- <= 0)
        EntityDelete(entity);
    else if (OWNER_DONE(owner))
        EntityDelete(entity);
}
