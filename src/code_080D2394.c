/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   BelmontMovementEffect_Update = ManticoreEffect_Update */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

/**
 * @brief Update the effect animation or delete when it is done.
 */
void BelmontMovementEffect_Update(struct EwramData_EntityData *entity)
{
    if (entity->animDone)
        EntityDelete(entity);
    else
        Anim_Update(entity);
}
