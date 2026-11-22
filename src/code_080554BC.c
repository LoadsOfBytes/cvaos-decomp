/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   SaveSparkle_Draw = Effect_DrawAtOffsetPair */
#include "code/code_08039340.h"
#include "particle_effect.h"
#include "structs/ewram.h"

/**
 * @brief Same as Effect_DrawAtOffsetPair.
 */
void SaveSparkle_Draw(struct EwramData_EntityData *entity)
{
    Sprite_Draw(entity);
    entity->posX.whole += PARTICLE_PAIR_OFFSET_X(entity);
    entity->posY.whole += PARTICLE_PAIR_OFFSET_Y(entity);
    Sprite_Draw(entity);
    entity->posX.whole -= PARTICLE_PAIR_OFFSET_X(entity);
    entity->posY.whole -= PARTICLE_PAIR_OFFSET_Y(entity);
}
