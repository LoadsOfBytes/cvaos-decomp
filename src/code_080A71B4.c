/* Forward this enemy variant to its shared implementation. */
#include "structs/ewram.h"
extern void EnemyValkyrieUpdate(struct EwramData_EntityData *);
void EnemyErinysUpdate(struct EwramData_EntityData *entity)
{
    EnemyValkyrieUpdate(entity);
}
