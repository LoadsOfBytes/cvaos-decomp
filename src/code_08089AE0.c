/* Forward this enemy variant to its shared implementation. */
#include "structs/ewram.h"
extern void EnemyGorgonUpdate(struct EwramData_EntityData *);
void EnemyCatoblepasUpdate(struct EwramData_EntityData *entity)
{
    EnemyGorgonUpdate(entity);
}
