#ifndef VALKYRIE_H
#define VALKYRIE_H

/* Functions of enemy/valkyrie.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void EnemyValkyrieCreate(struct EwramData_EntityData *entity);
extern void EnemyValkyrieUpdate(struct EwramData_EntityData *entity);

#endif /* VALKYRIE_H */
