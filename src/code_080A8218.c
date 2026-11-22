/* Forward this enemy variant to its shared implementation. */
#include "structs/ewram.h"
extern void EnemyGargoyleCreate(struct EwramData_EntityData *);
void EnemyGremlinCreate(struct EwramData_EntityData *entity)
{
    EnemyGargoyleCreate(entity);
}
