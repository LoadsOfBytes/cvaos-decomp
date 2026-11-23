/* Student Witch ground movement, turning and attack choice. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"

extern s32 Target_IsNearEntity(struct EwramData_EntityData *, s32, s32);
extern u8 sStudentWitchWalkProfile[];

/**
 * @brief Walking state (0): vary speed by animation step, turn at obstacles
 * and select the ground or airborne attack based on proximity and randomness.
 * Preserve the existing result-dependent stage/substate assignments.
 */
void StudentWitch_UpdateWalk(struct EwramData_EntityData *entity)
{
    s32 flags;
    u8 *frame;
    s32 currentFrame;
    s32 result;
    switch (entity->subState)
    {
    case 0:
        entity->subState = 1;
        /* fall through */
    case 1:
        currentFrame = entity->animStep;
        frame = &entity->animStep;
        switch (currentFrame)
        {
        case 0: entity->ext.studentWitch.speed = 0xC000; break;
        case 1:
        case 2:
            entity->ext.studentWitch.speed -= 0x1200;
            if (entity->ext.studentWitch.speed < 0) entity->ext.studentWitch.speed = 0;
            break;
        case 3:
            if (entity->ext.studentWitch.speed <= 0xBFFF) entity->ext.studentWitch.speed += 0x4000;
            break;
        }
        flags = Entity_Walk(entity, entity->ext.studentWitch.speed, sStudentWitchWalkProfile, 8);
        if (flags & 3) entity->flipX ^= 1;
        result = flags & 8;
        if (result)
        {
            if (*(u16 *)frame != 0) break;
            result = Target_IsNearEntity(entity, 0xA0, 0x3C);
            if (result)
            {
                result = RandomNumberGenerator() & 4;
                if (result)
                {
                    entity->state = 3;
                    entity->stage = 0;
                    entity->subState = 0;
                }
                else
                    goto falling;
            }
            else goto waiting;
        }
        else goto falling;
        break;
falling:
        entity->state = 2;
        entity->stage = result;
        entity->subState = result;
        break;
waiting:
        if (++entity->stage > 4)
        {
            entity->stage = result;
            entity->flipX ^= 1;
        }
        break;
    }
}
