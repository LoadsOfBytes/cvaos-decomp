/* Forward this enemy variant to its shared implementation. */
#include "structs/ewram.h"
extern void EnemyValkyrieCreate(struct EwramData_EntityData *);
void EnemyErinysCreate(struct EwramData_EntityData *entity)
{
    EnemyValkyrieCreate(entity);
}
