/* Forward this enemy variant to its shared implementation. */
#include "structs/ewram.h"
extern void EnemyGargoyleUpdate(struct EwramData_EntityData *);
void EnemyGremlinUpdate(struct EwramData_EntityData *entity)
{
    EnemyGargoyleUpdate(entity);
}
