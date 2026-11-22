/* Forward this enemy variant to its shared implementation. */
#include "structs/ewram.h"
extern void EnemyCurlyUpdate(struct EwramData_EntityData *);
void EnemyDurgaUpdate(struct EwramData_EntityData *entity)
{
    EnemyCurlyUpdate(entity);
}
