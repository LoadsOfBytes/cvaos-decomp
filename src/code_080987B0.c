/* CreakingSkullGuidedEffect_Update. */
#include "code_08009A0.h"
#include "structs/ewram.h"

/**
 * @brief Move toward the first stored room point while the timer exceeds
 * 7, then toward the second point using frame 0x35. Delete at zero;
 * retain the original countdown divisions and unreachable branch form.
 */
void CreakingSkullGuidedEffect_Update(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    s32 dx;
    s32 dy;
    s32 n;
    s32 timer;
    u8 t;

    timer = entity->timer - 1;
    if ((s8)(entity->timer = timer) <= 0)
    {
        EntityDelete(entity);
        return;
    }
    t = timer;
    if (t > 7)
    {
        dx = entity->posX.whole + bg->xPos.whole;
        dx = entity->unk_500.whole - dx;
        dy = entity->posY.whole + bg->yPos.whole;
        dy = entity->unk_504.whole - dy;
        if (t == 1)
            goto move;
        n = entity->timer;
        n--;
        n *= 2;
    }
    else
    {
        entity->animFrame = 0x35;
        dx = entity->posX.whole + bg->xPos.whole;
        dx = entity->unk_4F8.whole - dx;
        dy = entity->posY.whole + bg->yPos.whole;
        dy = entity->unk_4FC.whole - dy;
        if (entity->timer == 1)
            goto move;
        n = entity->timer;
        n--;
    }
    dx /= n;
    dy /= n;
move:
    entity->posX.whole += dx;
    entity->posY.whole += dy;
}
