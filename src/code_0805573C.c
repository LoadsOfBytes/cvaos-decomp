/* Forward this enemy variant to its shared implementation. */
#include "structs/ewram.h"
extern void Anim_Update(struct EwramData_EntityData *);
void sub_0805573C(struct EwramData_EntityData *entity)
{
    Anim_Update(entity);
}
