/* Unreferenced: advance the animation (next to the empty Object1E callbacks). */
#include "structs/ewram.h"
extern void Anim_Update(struct EwramData_EntityData *);
void Object1E_AnimUpdate(struct EwramData_EntityData *entity)
{
    Anim_Update(entity);
}
