#include "code_08009A0.h"
#include "soul_skill_entity.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
/**
 * @brief Child of SkillWingedSkeletonUse: attack box 0x1010F8F8 (slot 5,
 * timer override 20) kept at the owner position plus its unk_508/unk_50C
 * offset; frees its red soul slot when the owner reaches state 3.
 */
void SkillWingedSkeletonHitBoxUpdate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = entity->ext.redSoul.owner;
    entity->roomActive = 1;
    if (owner->state == 3)
    {
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    if (entity->state == 0)
    {
        Hit_SetTimerOverride(1, 20);
        Hit_Enable(entity, 5, NULL, 0);
        HitBox_SetAttackRaw(entity, 0x1010F8F8);
        entity->hit.flags |= 4;
        entity->state = 1;
    }
    entity->posX.whole = owner->posX.whole + owner->ext.skillWingedSkeleton.tipX;
    entity->posY.whole = owner->posY.whole + owner->ext.skillWingedSkeleton.tipY;
}
