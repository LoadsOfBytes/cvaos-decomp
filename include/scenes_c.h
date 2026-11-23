#ifndef SCENES_C_H
#define SCENES_C_H

/* Functions of event/scenes_c.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void EventScene30_UploadFrame(void);
extern void EventScene30Flash_Update(struct EwramData_EntityData *entity);
extern void EventScene30Effect_UpdateBallistic(struct EwramData_EntityData *entity);
extern void EventScene30Effect_UpdateSpinning(struct EwramData_EntityData *entity);
extern void EventScene30Effect_UpdateSpinningKeepSlot(struct EwramData_EntityData *entity);
extern s32 Conversation_LoadNextText(void);
extern void Conversation_Reset(void);

#endif /* SCENES_C_H */
