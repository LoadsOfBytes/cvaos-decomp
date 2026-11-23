#ifndef SRAM_H
#define SRAM_H

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

u32 SramInitialize(void);
s32 SramCheckInitialized(void);
u32 SramLockSaveSlot(u32 saveSlot);
u32 SramUnlockSaveSlot(u32 saveSlot);
u32 SramCheckSaveSlotUnlocked(u32 saveSlot);

#endif /* SRAM_H */
