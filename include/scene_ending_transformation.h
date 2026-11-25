#ifndef SCENE_ENDING_TRANSFORMATION_H
#define SCENE_ENDING_TRANSFORMATION_H

/* Functions of event/scene_ending_transformation.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void EventSceneEnding_UpdateTransformation(struct EwramData_EntityData *entity);

#endif /* SCENE_ENDING_TRANSFORMATION_H */
