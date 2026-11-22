#include "code_08009A0.h"
#include "mimic_entity.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern s32 Entity_MoveWithCollisionAlt(struct EwramData_EntityData *entity, s32 landingVelocity, s32 ceilingOffset, s32 speedLimit);
extern s32 Target_IsInRect(s16 x, s16 y, s32 w, s32 h);

/**
 * @brief 990D8 | Enemy sub-state machine (subState) with flags in unk_4FC:
 * 0 hidden; 1-2 falls (Entity_MoveWithCollisionAlt, accelY 0x2000) until it lands (bit
 * 2), then the 0x0E10F2F8 box with contact on; 3 waits for the player
 * within a 32x20 area above it (Target_IsInRect), then animation 0; 4
 * animation 1 after the current one ends; 5 hidden off screen.
 */
void Mimic_UpdateWait(struct EwramData_EntityData *entity)
{
    u32 box = 0x0E10F2F8;

    switch (entity->subState)
    {
    case 0:
        MIMIC_FLAGS(entity) |= 1;
        entity->hidden = 1;
        break;
    case 1:
        entity->subState++;
        entity->hidden = 0;
        entity->accelY.whole = 0x2000;
    case 2:
        MIMIC_FLAGS(entity) |= 3;
        MIMIC_HELD_FRAME(entity) = 0;
        entity->animFrame = 0;
        if (Entity_MoveWithCollisionAlt(entity, 0x10000, -0x100000, 0x40000) & 4)
        {
            entity->subState++;
            HitBox_SetBody(entity, &box);
            HitBox_SetAttack(entity, &box);
            entity->hit.flags &= 0xFB;
            entity->hit.flags |= 2;
        }
        break;
    case 3:
        MIMIC_FLAGS(entity) |= 2;
        MIMIC_HELD_FRAME(entity) = 0;
        entity->animFrame = 0;
        if (Target_IsInRect(entity->posX.part16.integer - 0x10, entity->posY.part16.integer - 0x14, 0x20, 0x14))
        {
            MIMIC_FLAGS(entity) = 0;
            Anim_StartIndexed(entity, 0, 0, 0);
            entity->subState++;
        }
        break;
    case 4:
        if (entity->animDone)
        {
            Anim_StartIndexed(entity, 1, 0, 1);
            entity->subState++;
        }
        break;
    case 5:
        if (EntityIsOutsideScreen(entity, 4))
            entity->hidden = 1;
        else
            entity->hidden = 0;
        break;
    }
}
