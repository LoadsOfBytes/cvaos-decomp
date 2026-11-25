#ifndef SHOP_UPDATE_H
#define SHOP_UPDATE_H

/* Functions of object/shop_update.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern s32 Shop_Update(struct GameState *context);
extern void EventSceneControllerCreate(struct EwramData_EntityData *entity);
extern void EventSceneControllerUpdate(struct EwramData_EntityData *entity);
extern struct EwramData_EntityData *Entity_CreateFromObjectTable(s32 kind);
extern void Interaction_End(struct EwramData_EntityData *entity, s32 deleteEntity);
extern s32 Entity_SetupFromObjectTable(struct EwramData_EntityData *entity, s32 kind);
extern void Sound_Play(u16 song);
extern u32 GameFlag_IsSomaAwakened(void);
extern s32 Player_GetZoneAroundX(u16 centerX, u16 innerWidth, u16 outerWidth);
/* The same function for callers that were compiled without a prototype
 * (implicit int): result and arguments are not narrowed. */
extern s32 Player_GetZoneAroundXWide(s32 centerX, s32 innerWidth, s32 outerWidth) asm("Player_GetZoneAroundX");

#endif /* SHOP_UPDATE_H */
