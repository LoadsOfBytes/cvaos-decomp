#ifndef OBJECTS_00_07_H
#define OBJECTS_00_07_H

/* Functions of object/objects_00_07.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

struct WarpRoomEntry {
    s16 mapX;
    s16 mapY;
    void *roomDefinition;
};

extern void Object00Create(struct EwramData_EntityData *entity);
extern void Object00Update(struct EwramData_EntityData *entity);
extern void Object02Create(struct EwramData_EntityData *entity);
extern void Object02Update(struct EwramData_EntityData *entity);
extern void Object03Create(struct EwramData_EntityData *entity);
extern void Object03Update(struct EwramData_EntityData *entity);
extern void Object04Create(struct EwramData_EntityData *entity);
extern void Object04Update(struct EwramData_EntityData *entity);
extern void Object06Create(struct EwramData_EntityData *entity);
extern void Object06Update(struct EwramData_EntityData *entity);
extern void Object05Create(struct EwramData_EntityData *entity);
extern void Object05Update(struct EwramData_EntityData *entity);
extern void Object07Create(struct EwramData_EntityData *entity);
extern void Object07Update(struct EwramData_EntityData *entity);
extern void Warp_ActivateCurrentRoom(void);
extern struct WarpRoomEntry *Warp_GetRoomEntry(u8 index);
extern s32 Warp_FindRoomIndex(u16 x, u16 y); // return s32 or s8?

#endif /* OBJECTS_00_07_H */
