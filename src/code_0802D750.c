#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
static inline s32 NoRedSoulActive(void)
{
    struct EwramData *ewram = gEwramData;
    u8 count = ewram->playerState.nbrActiveRedSoulEntities;
    s32 none = 0;
    if (count == 0 && ewram->playerState.activeRedSoulEntities[0] == NULL)
        none = 1;
    return none;
}

/* Animate a red-soul effect with velocity and acceleration; discard it
 * when its soul is inactive or its animation finishes. */
/**
 * @brief Move with velocity and acceleration and delete at the end of the
 * animation or when no red soul is active.
 */
void RedSoulEffect_UpdateAccelerated(struct EwramData_EntityData *entity)
{
    if (NoRedSoulActive())
    {
        EntityDelete(entity);
        return;
    }
    if (entity->state == 0)
    {
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->state = 1;
    }
    entity->posX.whole += entity->velX.whole;
    entity->velX.whole += entity->accelX.whole;
    entity->posY.whole += entity->velY.whole;
    entity->velY.whole += entity->accelY.whole;
    if (Anim_Update(entity) == 3)
        EntityDelete(entity);
}

