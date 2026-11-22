/* Forward this enemy variant to its shared implementation. */
#include "structs/ewram.h"
extern void EnemyCurlyCreate(struct EwramData_EntityData *);
void EnemyDurgaCreate(struct EwramData_EntityData *entity)
{
    EnemyCurlyCreate(entity);
}
