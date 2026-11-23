#include "code_08009A0.h"
#include "soul_skill_entity.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code_080D73B8.h"

#define FIXED_TO_INT(v) ((v) < 0 ? -(-(v) >> 16) : (v) >> 16)
#define ABS(x) ((x) < 0 ? -(x) : (x))

/**
 * @brief Child of SkillRedMinotaurUse: orbiting hit box (slot 5, box
 * 0x4040E0E0, timer override 60) placed 80 units along the owner angle
 * (unk_510); enabled between owner states 2 and 3, removed with its red
 * soul slot at owner state 4.
 */
void SkillRedMinotaurHitBoxUpdate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.redSoul.owner;
    s32 angle, angle2;
    s32 dx, dy;

    entity->roomActive = 1;
    if (parent->state == 4)
    {
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
        Hit_SetTimerOverride(1, 60);
        Hit_Enable(entity, 5, 0, 0);
        HitBox_SetAttackRaw(entity, 0x4040E0E0);
        entity->hit.flags |= 2;
        entity->hit.flags |= 4;
        entity->state = 1;
        break;
    case 1:
        if (parent->state == 2)
        {
            entity->hit.flags &= ~2;
            entity->state++;
        }
        break;
    case 2:
        if (parent->state == 3)
            entity->hit.flags |= 2;
        break;
    }
    angle = SKILL_RED_MINOTAUR_ANGLE(parent);
    angle2 = angle - 0x4000;
    dx = Sin(angle) * 80;
    dy = Sin(angle2) * 80;
    entity->posX.whole = parent->posX.whole + dx;
    entity->posY.whole = parent->posY.whole + dy;
    dy = ABS(FIXED_TO_INT(dy));
    if (parent->ext.skillRedMinotaur.scale > 0xFFFF)
    {
        if (entity->posY.part16.integer < -dy || entity->posY.part16.integer > dy + 0xA0)
            parent->hidden = 1;
        else
            parent->hidden = 0;
    }
}
