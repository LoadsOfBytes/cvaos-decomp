#ifndef DEBUG_4531_H
#define DEBUG_4531_H

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

s32 GameModeDebug4531Update(void);
s32 Debug4531_UpdateEventMenu(void);
s32 Debug4531_RunEventGame(void);
s32 Debug4531_SelectSaveSlot(void);

#endif /* DEBUG_4531_H */
