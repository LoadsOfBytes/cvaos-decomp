#ifndef BAEL_H
#define BAEL_H

/* Functions of enemy/bael.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void BuerHead_Update(struct EwramData_EntityData *entity);
extern void Buer_AllocateAffine(struct EwramData_EntityData *entity);
extern void Buer_UpdateEntry(struct EwramData_EntityData *entity);
extern void Buer_UpdateState4Rotation(struct EwramData_EntityData *entity);
extern void Buer_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void Buer_OnHit(struct EwramData_EntityData *entity);
extern s32 Buer_OffsetBySign(s32 value, s32 offset);
extern s32 Buer_CheckCleanup(struct EwramData_EntityData *entity);
extern void Buer_RestoreXAtProbe(struct EwramData_EntityData *entity, s32 savedX);

#endif /* BAEL_H */
