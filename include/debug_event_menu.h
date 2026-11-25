#ifndef DEBUG_EVENT_MENU_H
#define DEBUG_EVENT_MENU_H

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

s32 GameModeDebugEventMenuUpdate(void);
s32 DebugEventMenu_UpdateEventMenu(void);
s32 DebugEventMenu_RunEventGame(void);
s32 DebugEventMenu_SelectSaveSlot(void);

#endif /* DEBUG_EVENT_MENU_H */
