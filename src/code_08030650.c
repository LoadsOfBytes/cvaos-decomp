/* SkillCreakingSkullHitBoxUpdate. */
#include "code_08009A0.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

/**
 * @brief Child of SkillCreakingSkullUse: orbiting hit box (slot 6,
 * 0x2020F0F0) active while the parent flag is set, radius alternating
 * 0x30/0x10 around the parent at its angle, mirrored by flipX.
 */
void SkillCreakingSkullHitBoxUpdate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *target = &gEwramData->entityData[entity->unk_514.part8.unk_514];
    struct EwramData_EntityData *parent = entity->unk_4F8.unk_4F8_entity;
    u16 angle;
    s32 radius;
    s32 timer;

    entity->roomActive = 1;
    if (entity->state == 0)
    {
        Hit_Enable(entity, 6, 0, 0);
        HitBox_SetAttackRaw(entity, 0x2020F0F0);
        entity->hit.flags |= 4;
        entity->state++;
        entity->scrollWithBg = 1;
    }
    if (target->unk_514.part8.unk_515 == 0)
    {
        entity->hit.flags |= 2;
        return;
    }
    entity->hit.flags &= ~2;
    timer = entity->timer + 1;
    entity->timer = timer;
    if (timer & 1)
        entity->unk_514.part16.spawnParam1 = 0x30;
    else
        entity->unk_514.part16.spawnParam1 = 0x10;
    if (!parent->flipX)
        angle = -0x8000 + parent->unk_510.part16.unk_510;
    else
        angle = -parent->unk_510.part16.unk_510;
    radius = entity->unk_514.part16.spawnParam1;
    entity->posX.whole = parent->posX.whole + radius * Sin((s16)angle + 0x4000);
    entity->posY.whole = parent->posY.whole + radius * Sin((s16)angle);
}
