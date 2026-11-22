/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   BeamSkeleton_UpdateBestiaryPreview = Imp_UpdateState4Animation */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

/**
 * @brief Bestiary state (4): start animation 0 once, then update it.
 */
void BeamSkeleton_UpdateBestiaryPreview(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->subState++;
        case 1:
            Anim_Update(entity);
            break;
    }
}
