/* Belmont_UpdateBestiaryPreview: variant of an already decompiled function, generated with
 * a cloning script (constants adjusted where needed), checked byte for
 * byte. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

/**
 * @brief Bestiary state (12): select animation 5 once, then update it.
 */
void Belmont_UpdateBestiaryPreview(struct EwramData_EntityData *entity)
{
    if (entity->subState == 0)
    {
        if (entity->animScriptIndex != 5)
            Anim_StartIndexed(entity, 5, 1, 1);
        entity->subState++;
    }
    Anim_Update(entity);
}
