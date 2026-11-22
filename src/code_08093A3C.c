/* Oscillate vertical acceleration and select the rising/falling animation. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
extern void EntityMove(struct EwramData_EntityData *);
/**
 * @brief Flight state (state 0): weave up and down (velY +- accelY, direction
 * flipped every unk_506 frames) while moving horizontally.
 */
void MedusaHead_UpdateFly(struct EwramData_EntityData *entity)
{
    if (entity->subState != 0) return;
    if ((s8)entity->unk_504.part8.unk_504 != 0) entity->velY.whole -= entity->accelY.whole;
    else entity->velY.whole += entity->accelY.whole;
    if ((s8)++entity->unk_504.part8.unk_505 >= entity->unk_504.part8.unk_506)
    {
        entity->unk_504.part8.unk_505 = 0;
        entity->unk_504.part8.unk_504 ^= 1;
    }
    EntityMove(entity);
    if (entity->velY.whole >= 0)
    {
        if (entity->animScriptIndex == 2)
        {
            if (!(RandomNumberGenerator() & 1)) Anim_StartIndexed(entity, 1, 0, 1);
            else Anim_StartIndexed(entity, 0, 0, 1);
        }
    }
    else if (entity->animScriptIndex != 2) Anim_StartIndexed(entity, 2, 0, 1);
}
