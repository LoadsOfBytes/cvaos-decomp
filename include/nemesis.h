#ifndef NEMESIS_H
#define NEMESIS_H

/* Functions of enemy/nemesis.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void Nemesis_UpdatePositioning(struct EwramData_EntityData *entity);
extern void Nemesis_UpdateAttackSelection(struct EwramData_EntityData *entity);
extern void Nemesis_UpdateTurningAttack(struct EwramData_EntityData *entity);
extern void Nemesis_UpdateScriptedAttack(struct EwramData_EntityData *entity);
extern void Nemesis_UpdateSpinLunge(struct EwramData_EntityData *entity);
extern void Nemesis_UpdatePartnerFlight(struct EwramData_EntityData *entity);
extern void Nemesis_UpdateDeath(struct EwramData_EntityData *entity);
extern s32 Nemesis_CreatePartner(struct EwramData_EntityData *parent);
extern void Nemesis_SetAfterimagesHidden(struct EwramData_EntityData *entity, s32 hidden);
extern void Nemesis_UpdateState7(struct EwramData_EntityData *entity);
extern void Nemesis_OnContact(struct EwramData_EntityData *entity);
extern void Nemesis_CreateAfterimages(struct EwramData_EntityData *entity);
extern void Nemesis_OnHit(struct EwramData_EntityData *entity, s32 hit, u16 *data);

#endif /* NEMESIS_H */
