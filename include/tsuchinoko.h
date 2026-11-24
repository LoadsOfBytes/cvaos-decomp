#ifndef TSUCHINOKO_H
#define TSUCHINOKO_H

/* Functions of enemy/tsuchinoko.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void Tsuchinoko_SpawnDroplet(struct EwramData_EntityData *parent);
extern u8 Tsuchinoko_LoadGraphics(struct EwramData_EntityData *entity);
extern void Tsuchinoko_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void Tsuchinoko_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit);

#endif /* TSUCHINOKO_H */
