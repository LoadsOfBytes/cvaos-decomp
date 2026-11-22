#ifndef CODE_0803889C_H
#define CODE_0803889C_H

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

struct DebugObjTestState {
    struct EwramData_EntityData *entities[2];
}; /* size = 0x8 */
s32 DebugObjDispTest_Run(s32 test);
void DebugObjDispTestSprite_Update(struct EwramData_EntityData *entity);
void DebugObjDispTest_Update(struct DebugObjTestState *state);
void DebugObjDispTest_Init(struct DebugObjTestState *state);
void DebugObjDispTest_Exit(struct DebugObjTestState *state);
void DebugObjDispTestDummy_Update(struct EwramData_EntityData *entity);

#endif /* CODE_0803889C_H */
