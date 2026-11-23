#ifndef SHOP_UPDATE_H
#define SHOP_UPDATE_H

/* Functions of object/shop_update.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern s32 Shop_Update(struct GameState *context);
extern void Object20Create(struct EwramData_EntityData *entity);
extern void Object20Update(struct EwramData_EntityData *entity);
extern struct EwramData_EntityData *Entity_CreateFromObjectTable(s32 kind);
extern void Interaction_End(struct EwramData_EntityData *entity, s32 deleteEntity);
extern s32 Entity_SetupFromObjectTable(struct EwramData_EntityData *entity, s32 kind);
extern void Sound_Play(u16 song);

#endif /* SHOP_UPDATE_H */
