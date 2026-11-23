#ifndef OBJECTS_19_1C_H
#define OBJECTS_19_1C_H

/* Functions of object/objects_19_1c.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void Object19Update(struct EwramData_EntityData *entity);
extern void Object19_OnHit(struct EwramData_EntityData *entity);
extern void Object19_Draw(struct EwramData_EntityData *entity);
extern void Object1BCreate(struct EwramData_EntityData *entity);
extern void Object1BUpdate(struct EwramData_EntityData *entity);
extern void Object1ACreate(struct EwramData_EntityData *entity);
extern void Object1AUpdate(struct EwramData_EntityData *entity);
extern void Object1CCreate(struct EwramData_EntityData *entity);
extern void Object1CUpdate(struct EwramData_EntityData *entity);

#endif /* OBJECTS_19_1C_H */
