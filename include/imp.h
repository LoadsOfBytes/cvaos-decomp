#ifndef IMP_H
#define IMP_H

/* Functions of enemy/imp.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void Imp_OnContact(struct EwramData_EntityData *entity);
extern void Imp_ResetFlight(struct EwramData_EntityData *entity);
extern void Imp_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern s32 Imp_CreateGrabIndicator(struct EwramData_EntityData *entity);

#endif /* IMP_H */
