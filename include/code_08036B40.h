#ifndef CODE_08036B40_H
#define CODE_08036B40_H

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

union DebugDisplayTarget {
    void *roomDefinition;
    s32 bgIndex;
}; /* size = 0x4 */
struct DebugDisplayTestState {
    union DebugDisplayTarget target; // BG index or room-definition pointer, according to the test
    struct EwramData_EntityData *player;
    s32 clearBgIndex;
    u16 transitionState;
    u8 pad_E[0x10 - 0xE];
}; /* size = 0x10 */
s32 DebugDispTest_Run(s32 testIndex);
void DebugBgDispTest_Update(struct DebugDisplayTestState *test);
void DebugLinkTest_Init(struct DebugDisplayTestState *test);
void DebugHitTest_Init(struct DebugDisplayTestState *test);
void DebugHitTestCursor_Update(struct EwramData_EntityData *entity);
void DebugLinkTest_CenterCamera(struct EwramData_EntityData *entity);
void DebugLinkTestWalker_Update(struct EwramData_EntityData *entity);
void DebugTestWalkerUnused_Update(struct EwramData_EntityData *entity);
void DebugBgDispTest_Init(struct DebugDisplayTestState *test);
void DebugBgDispTest_Exit(struct DebugDisplayTestState *test);
void DebugLinkTest_Update(struct DebugDisplayTestState *test);
void DebugLinkTest_Exit(struct DebugDisplayTestState *test);
void DebugHitTest_Update(struct DebugDisplayTestState *test);
void DebugHitTest_Exit(struct DebugDisplayTestState *test);

#endif /* CODE_08036B40_H */
