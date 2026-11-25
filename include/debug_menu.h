#ifndef DEBUG_MENU_H
#define DEBUG_MENU_H

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

struct NestedDebugMenuEntry {
    u8 *label;
    struct NestedDebugMenuEntry *submenu;
    s32 selectionId;
}; /* size = 0xC */
struct NestedDebugMenuState {
    struct NestedDebugMenuEntry *rootList;
    s8 depth;
    s8 cursors[1]; // TODO: length?
    u8 pad_6[0xC - 0x6];
    s32 selectionResult;
    u8 testStage;
    u8 pad_11[0x14 - 0x11];
}; /* size = 0x14 */
s32 GameModeDebugNestedMenuUpdate(void);
s32 DebugTest_GetStage(void);
void DebugTest_SetStage(u8 stage);
s32 DebugMenu_Update(struct NestedDebugMenuState *menu, u16 tileIndex, u16 rowOffset);
void DebugMenu_Init(struct NestedDebugMenuState *menu, struct NestedDebugMenuEntry *root);
s32 DebugMenu_CountEntries(struct NestedDebugMenuEntry *entries);
struct NestedDebugMenuEntry* DebugMenu_GetCurrentList(struct NestedDebugMenuState *menu);

#endif /* DEBUG_MENU_H */
