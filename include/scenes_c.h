#ifndef SCENES_C_H
#define SCENES_C_H

/* Functions of event/scenes_c.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void EventSceneEnding_UploadFrame(void);
extern void EventSceneEndingFlash_Update(struct EwramData_EntityData *entity);
extern void EventSceneEndingEffect_UpdateBallistic(struct EwramData_EntityData *entity);
extern void EventSceneEndingEffect_UpdateSpinning(struct EwramData_EntityData *entity);
extern void EventSceneEndingEffect_UpdateSpinningKeepSlot(struct EwramData_EntityData *entity);
extern s32 Conversation_LoadNextText(void);
extern void Conversation_Reset(void);

#endif /* SCENES_C_H */
