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

extern void GateCreate(struct EwramData_EntityData *entity);
extern void GateUpdate(struct EwramData_EntityData *entity);
extern void BossDoorCreate(struct EwramData_EntityData *entity);
extern void BossDoorUpdate(struct EwramData_EntityData *entity);
extern void LinkPlayDoorCreate(struct EwramData_EntityData *entity);
extern void LinkPlayDoorUpdate(struct EwramData_EntityData *entity);
extern void OneWayBossDoorCreate(struct EwramData_EntityData *entity);
extern void OneWayBossDoorUpdate(struct EwramData_EntityData *entity);
extern void BarrierDoorCreate(struct EwramData_EntityData *entity);
extern void BarrierDoorUpdate(struct EwramData_EntityData *entity);
extern void ProximityDoorCreate(struct EwramData_EntityData *entity);
extern void ProximityDoorUpdate(struct EwramData_EntityData *entity);
extern void RoomEffectCreate(struct EwramData_EntityData *entity);
extern void RoomEffectUpdate(struct EwramData_EntityData *entity);
extern void Warp_ActivateCurrentRoom(void);
extern struct WarpRoomEntry *Warp_GetRoomEntry(u8 index);
extern s32 Warp_FindRoomIndex(u16 x, u16 y); // return s32 or s8?

#endif /* OBJECTS_00_07_H */
