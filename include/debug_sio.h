#ifndef DEBUG_SIO_H
#define DEBUG_SIO_H

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

// Preserve the packed byte-stage / halfword-timer overlap used by the two tests.
union __attribute__((packed)) DebugSioStageOrTimer {
    u8 stage;
    u16 timer;
};
struct DebugSioTestState {
    union DebugSioStageOrTimer stageOrTimer;
    u8 state;
    u8 subState;
    u8 titleChoice;
    u8 unk_5;
    u8 pad_6[0x8 - 0x6];
    u8 sendBytes[1]; // 12-byte use includes padding and sendMarker; retain declared extent
    u8 pad_9[0x10 - 0x9];
    s32 sendMarker;
    u8 receivedBytes[1]; // 12-byte use includes padding and receivedMarker; retain declared extent
    u8 pad_15[0x1C - 0x15];
    s32 receivedMarker;
    s8 editIndex;
    u8 pad_21[0x24 - 0x21];
}; /* size = 0x24 */
s32 DebugSioTest_Run(s32 testIndex);
s32 DebugSioTest0_Update(struct DebugSioTestState *test);
s32 DebugSioTest1_Update(struct DebugSioTestState *test);
s32 DebugSioTest1_UpdateTitle(struct DebugSioTestState *test);
s32 DebugSioTest1_UpdateConnectCheck(struct DebugSioTestState *test);
s32 DebugSioTest1_UpdateMain(struct DebugSioTestState *test);
void Debug_ClearBgMaps(void);
void DebugSioTest0_Init(struct DebugSioTestState *test);
void DebugSioTest0_Exit(struct DebugSioTestState *test);
void DebugSioTest1_Init(struct DebugSioTestState *test);
void DebugSioTest1_Exit(struct DebugSioTestState *test);

#endif /* DEBUG_SIO_H */
