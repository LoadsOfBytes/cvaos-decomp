/* Run Mudman's attack animation and timed recovery. */
#include "code/code_08039340.h"
#include "code_080D73B8.h"
extern void Mudman_ThrowShot(struct EwramData_EntityData *);
/**
 * @brief Attack state machine (state 1): animation 3, from its second step
 * Mudman_ThrowShot with sound 0x85; then back to state 0.
 */
void Mudman_UpdateAttack(struct EwramData_EntityData *entity)
{
    s32 substate = entity->subState;
    switch (substate)
    {
    case 0:
        Anim_StartIndexed(entity, 3, 0, 0);
        entity->subState++;
        break;
    case 1:
        if (entity->animStep > 1)
        {
            entity->subState = substate + 1;
            entity->timer = 0;
            Mudman_ThrowShot(entity);
            PlaySong(0x85);
        }
        break;
    case 2:
        if (entity->timer++ > 59)
        {
            entity->state = 0;
            Anim_StartIndexed(entity, 2, 0, 1);
            entity->animDone = 0;
            entity->timer = 100;
            entity->subState = substate;
            entity->velY.whole = 0x20000;
        }
        break;
    }
}

