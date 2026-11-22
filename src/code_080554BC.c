/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   SaveSparkle_Draw = Effect_DrawAtOffsetPair */
#include "code/code_08039340.h"
#include "structs/ewram.h"

/**
 * @brief Same as Effect_DrawAtOffsetPair.
 */
void SaveSparkle_Draw(struct EwramData_EntityData *entity)
{
    Sprite_Draw(entity);
    entity->posX.whole += entity->unk_500.whole;
    entity->posY.whole += entity->unk_504.whole;
    Sprite_Draw(entity);
    entity->posX.whole -= entity->unk_500.whole;
    entity->posY.whole -= entity->unk_504.whole;
}
