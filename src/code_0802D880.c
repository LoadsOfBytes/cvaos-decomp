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

/* Move a red-soul effect, deleting it when its soul is inactive or
 * when it leaves the extended screen bounds. */
/**
 * @brief Move a thrown bone fragment with acceleration; delete it outside the
 * screen area or when no red soul is active.
 */
void SoulBoneFragment_Update(struct EwramData_EntityData *entity)
{
    if (NoRedSoulActive())
    {
        EntityDelete(entity);
        return;
    }
    entity->posX.whole += entity->velX.whole;
    entity->velX.whole += entity->accelX.whole;
    entity->posY.whole += entity->velY.whole;
    entity->velY.whole += entity->accelY.whole;
    if (!Entity_IsInRect(entity, -16, 0, 0x110, 0xB0))
        EntityDelete(entity);
}
