#ifndef OBJECTS_35_37_H
#define OBJECTS_35_37_H

/* Functions of object/objects_35_37.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void Object35Create(struct EwramData_EntityData *entity);
extern void Object35Update(struct EwramData_EntityData *entity);
extern void Object36Update(struct EwramData_EntityData *entity);
extern void Object36Create(struct EwramData_EntityData *entity);
extern void Object37Create(struct EwramData_EntityData *entity);
extern void Object37Update(struct EwramData_EntityData *entity);
extern void Shop_SetupBackgrounds(void);
extern void Shop_Open(void);

#endif /* OBJECTS_35_37_H */
