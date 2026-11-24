#ifndef KYOMA_DEMON_B_H
#define KYOMA_DEMON_B_H

/* Functions of enemy/kyoma_demon_b.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void KyomaDemon_UpdateAttack(struct EwramData_EntityData *entity);
extern u8 KyomaDemon_LoadGraphics(struct EwramData_EntityData *entity);
extern void KyomaDemon_UpdateDeath(struct EwramData_EntityData *entity);
extern void KyomaDemon_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void KyomaDemon_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit);

#endif /* KYOMA_DEMON_B_H */
