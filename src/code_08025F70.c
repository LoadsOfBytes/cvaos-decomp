#include "code_08009A0.h"
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
    struct EwramData_EntityData *owner = entity->unk_4F8.unk_4F8_entity;
    entity->roomActive = 1;
    if (owner->state == 3)
    {
        gEwramData->playerState.activeRedSoulEntities[entity->unk_4FC.part16.unk_4FE[0]] = NULL;
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
    entity->posX.whole = owner->posX.whole + owner->unk_508.whole;
    entity->posY.whole = owner->posY.whole + owner->unk_50C.whole;
}
