#ifndef HITBOX_H
#define HITBOX_H

/* Functions of engine/hitbox.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

struct HitResult;

extern u32 HitBox_Overlap(struct EwramData_EntityData *e1, struct SpriteBox *hb1, struct EwramData_EntityData *e2, struct SpriteBox *hb2);
extern u32 Hit_Check(u8 *a, u8 *b);
extern u32 Hit_CheckPlayerAttack(u8 *e);
extern u32 Hit_CheckPlayerHurt(u8 *e);
extern s32 HitBox_GetBoxes(struct SpriteBox *attack, struct SpriteBox *body, struct EwramData_EntityData *entity); // param types?
extern u32 Hit_CheckPair(struct HitResult *res, u8 *a, u8 *b);
extern void HitBox_UpdatePlayerRects(void);
extern void Hit_UpdateTimers(struct EwramData_EntityData *entity);
extern void HitBox_ClearAll(void);
extern void Hit_Disable(struct EwramData_EntityData *entity);
extern s32 HitBox_SetAttack(struct EwramData_EntityData *entity, void *box);
extern s32 HitBox_SetBody(struct EwramData_EntityData *entity, void *box);
extern s32 HitBox_SetBodyRaw(struct EwramData_EntityData *entity, s32 box);
extern u32 Hit_CheckWithPlayer(struct HitResult *res, struct EwramData_EntityData *entity);
extern s32 HitBox_SetAttackRaw(struct EwramData_EntityData *entity, s32 box);
extern s32 Hit_EnableContact(struct EwramData_EntityData *entity, s32 type, void *onHit, void *onHurt);
extern s32 Hit_Enable(struct EwramData_EntityData *entity, s32 type, void *onHit, void *onHurt);

#endif /* HITBOX_H */
