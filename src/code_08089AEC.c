/* Forward this enemy variant to its shared implementation. */
#include "structs/ewram.h"
extern void EnemyGorgonCreate(struct EwramData_EntityData *);
void EnemyCatoblepasCreate(struct EwramData_EntityData *entity)
{
    EnemyGorgonCreate(entity);
}
