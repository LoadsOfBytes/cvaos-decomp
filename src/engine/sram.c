#include "sram.h"
#include "gba.h"
#include "agb_sram.h"

#include "data/interrupt_table.h"
#include "data/debug_font.h"

#include "structs/agb_sram.h"
#include "structs/ewram.h"
#include "structs/main.h"

static u8 sSramSignature[0x10] = "CASTLEVANIA2-010";

/**
 * @brief 1004 | Clears SRAM and writes SRAM header
 * 
 * @return u32 0 if SRAM operations successful, otherwise the address it failed at
 */
u32 SramInitialize(void)
{
    s32 i;
    u8 *dst;
    u32 error_addr;

    dst = SRAM_BASE;
    DMA_FILL_32(3, 0, gEwramData->sharedScratchBuffer, 0x1000);

    for (i = 0; i < SRAM_SIZE; i += 0x1000)
    {
        error_addr = WriteAndVerifySramFast(gEwramData->sharedScratchBuffer, dst + i, 0x1000);
        if (error_addr != 0)
            return error_addr;
    }

    dst = gEwramData->sharedScratchBuffer;
    for (i = 0; i < 0x10; i += 1)
    {
        dst[i] = sSramSignature[i];
    }

    error_addr = WriteAndVerifySramFast(gEwramData->sharedScratchBuffer, SRAM_BASE, 0x10);
    return error_addr;
}

/**
 * @brief 1094 | Check SRAM header to determine if SRAM is initialized
 * 
 * @return bool, SRAM is initialized
 */
s32 SramCheckInitialized(void)
{
    s32 i;
    s32 sramInitialized;
    u8 *sramPtr;
    u8 *headerStrPtr;

    sramInitialized = TRUE;
    sramPtr = gEwramData->sharedScratchBuffer;
    gReadSramFast(SRAM_BASE, &gEwramData->sharedScratchBuffer, 0x10);

    i = 0x10;
    headerStrPtr = sSramSignature;

    for (; i > 0; i--, sramPtr++, headerStrPtr++)
    {
        if (*sramPtr != *headerStrPtr)
        {
            sramInitialized = FALSE;
            break;
        }
    }
    return sramInitialized;
}

/**
 * @brief 10E4 | Set SRAM save slot as locked (i.e. being operated on)
 * 
 * @param saveSlot Save slot index to SRAM
 * @return u32 0 if the SRAM verified, otherwise the address it failed at
 */
u32 SramLockSaveSlot(u32 saveSlot)
{
    u8 saveSlots; // each bit corresponds to a save slot, 0/1 is unlocked/locked

    gReadSramFast(SRAM_BASE + 0x10, &saveSlots, 1);

    saveSlots |= (1 << saveSlot);

    return WriteAndVerifySramFast(&saveSlots, SRAM_BASE + 0x10, 1);
}

/**
 * @brief 1124 | Set SRAM save slot as unlocked (i.e. not being operated on)
 * 
 * @param saveSlot Save slot index to SRAM
 * @return u32 0 if the SRAM verified, otherwise the address it failed at
 */
u32 SramUnlockSaveSlot(u32 saveSlot)
{
    u8 saveSlots; // each bit corresponds to a save slot, 0/1 is unlocked/locked

    gReadSramFast(SRAM_BASE + 0x10, &saveSlots, 1);

    saveSlots &= ~(1 << saveSlot);

    return WriteAndVerifySramFast(&saveSlots, SRAM_BASE + 0x10, 1);
}

/**
 * @brief 1164 | Check if SRAM save slot is unlocked (something has gone wrong if it's locked)
 * 
 * @param saveSlot Save slot index to SRAM
 * @return bool, save slot is unlocked
 */
u32 SramCheckSaveSlotUnlocked(u32 saveSlot)
{
    u8 saveSlots; // each bit corresponds to a save slot, 0/1 is unlocked/locked

    gReadSramFast(SRAM_BASE + 0x10, &saveSlots, 1);

    return (saveSlots & (1 << saveSlot)) == 0;
}
