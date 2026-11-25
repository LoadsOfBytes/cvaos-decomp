#ifndef AREA_PROPS_H
#define AREA_PROPS_H

/* Functions of object/area_props.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void ArenaAnimatedPropUpdate(struct EwramData_EntityData *entity);
extern void ArenaAnimatedPropCreate(struct EwramData_EntityData *entity);
extern void FlickeringFlameCreate(struct EwramData_EntityData *entity);
extern void FlickeringFlameUpdate(struct EwramData_EntityData *entity);
extern void LegionWalkingCorpseUpdate(struct EwramData_EntityData *entity);
extern void LegionWalkingCorpseCreate(struct EwramData_EntityData *entity);
extern void PopUpTrapCreate(struct EwramData_EntityData *entity);
extern void PopUpTrapUpdate(struct EwramData_EntityData *entity);
extern void PopUpTrapSmallCreate(struct EwramData_EntityData *entity);
extern void PopUpTrapSmallUpdate(struct EwramData_EntityData *entity);
extern void PopUpTrapLargeCreate(struct EwramData_EntityData *entity);
extern void PopUpTrapLargeUpdate(struct EwramData_EntityData *entity);
extern void GardenAnimatedPropCreate(struct EwramData_EntityData *entity);
extern void GardenAnimatedPropUpdate(struct EwramData_EntityData *entity);
extern void DriftingCloudsCreate(struct EwramData_EntityData *entity);
extern void DriftingCloudsUpdate(struct EwramData_EntityData *entity);
extern void ReactivePropCreate(struct EwramData_EntityData *entity);
extern void ReactivePropUpdate(struct EwramData_EntityData *entity);
extern void ProximityBurstCreate(struct EwramData_EntityData *entity);
extern void ProximityBurstUpdate(struct EwramData_EntityData *entity);
extern void JuliusArenaPropCreate(struct EwramData_EntityData *entity);
extern void JuliusArenaPropUpdate(struct EwramData_EntityData *entity);
extern void LeafSpawnerCreate(struct EwramData_EntityData *entity);
extern void LeafSpawnerUpdate(struct EwramData_EntityData *entity);
extern void JuliusArenaDebrisCreate(struct EwramData_EntityData *entity);
extern void JuliusArenaDebrisUpdate(struct EwramData_EntityData *entity);
extern void CrumblingPlatformCreate(struct EwramData_EntityData *entity);
extern void CrumblingPlatformUpdate(struct EwramData_EntityData *entity);
extern void SparkEmitterCreate(struct EwramData_EntityData *entity);
extern void SparkEmitterUpdate(struct EwramData_EntityData *entity);
extern void PendulumPlatformCreate(struct EwramData_EntityData *entity);
extern void PendulumPlatformUpdate(struct EwramData_EntityData *entity);

#endif /* AREA_PROPS_H */
