#include "save_data.h"
#include "sram.h"
#include "intro_cutscene.h"
#include "map_flags.h"
#include "player_core.h"
#include "soul_inventory.h"
#include "player_level.h"
#include "gba.h"
#include "agb_sram.h"

#include "constants/main.h"

#include "data/data_0E0334.h"
#include "data/data_0E3464.h"

#include "structs/agb_sram.h"
#include "structs/ewram.h"
#include "structs/main.h"
#include "data/data_4F0B18.h"

static inline void SaveData_StoreHeaderSummary(struct GameState *gameState)
{
    gameState->saveGold = gEwramData->playerStatus.currentGold;
    gameState->saveLevel = gEwramData->playerStatus.currentLevel;
    gameState->saveMapCompletionTenths = Map_GetCompletionTenths(gameState, 2);
    gameState->saveCharacter = gEwramData->playerStatus.currentCharacter;
    gameState->unk_A5 = gEwramData->playerStatus.unk_13267;
}

/* Keep the address of the global pointer, rather than a snapshot of its
 * value, so buffer accesses still reload it after SRAM calls. The local
 * also preserves agbcc's early symbol-address materialization. */
static inline u32 SaveData_SaveSlotToSram_inline(s32 saveSlot)
{
    struct EwramData **ewramAddress = &gEwramData;
    void *sramSaveSlot;
    u32 error_addr;

    sramSaveSlot = SRAM_BASE + 0x10 + saveSlot * 0x47C;
    SramLockSaveSlot(saveSlot);
    error_addr = WriteAndVerifySramFast(&(*ewramAddress)->playerStatus, sramSaveSlot, 0x190);
    if (error_addr == 0)
    {
        error_addr = WriteAndVerifySramFast(&(*ewramAddress)->gameState.playerName, sramSaveSlot + 0x190, 0x20);
        if (error_addr == 0)
        {
            error_addr = WriteAndVerifySramFast(&(*ewramAddress)->gameState.mapCells, sramSaveSlot + 0x190 + 0x20, 0x2CC);
        }
    }
    SramUnlockSaveSlot(saveSlot);
    return error_addr;
}

static inline s32 SaveData_ClearSlotInSram_inline(s32 saveSlot)
{
    u32 error_addr;
    void *zeroBuf;
    void *sramSaveSlot;

    zeroBuf = gEwramData->sharedScratchBuffer;
    DMA_FILL_32(3, 0, zeroBuf, 0x47C);
    sramSaveSlot = SRAM_BASE + 0x10 + saveSlot * 0x47C;
    SramLockSaveSlot(saveSlot);
    error_addr = WriteAndVerifySramFast(zeroBuf, sramSaveSlot, 0x47C);
    SramUnlockSaveSlot(saveSlot);
    return error_addr;
}

static inline s32 SaveSlot_ReadHeader(s32 saveSlot, s32 bufferIndex)
{
    s32 saveValid;
    s32 sramSaveSlotOffset;

    saveValid = SramCheckInitialized();
    if (saveValid)
    {
        sramSaveSlotOffset = (saveSlot * 0x47C);
        saveValid = SramCheckSaveSlotUnlocked(saveSlot);
        if (saveValid)
        {
            gReadSramFast(SRAM_BASE + 0x10 + sramSaveSlotOffset + 0x190, &gEwramData->saveHeaders[bufferIndex], sizeof(gEwramData->saveHeaders[bufferIndex]));
        }
        else
        {
            DMA_FILL_32(3, 0, &gEwramData->saveHeaders[bufferIndex], sizeof(gEwramData->saveHeaders[bufferIndex]));
        }
    }
    return saveValid;
}

// load save file
/**
 * @brief 12744 | To document
 * 
 * @param saveSlot Save slot index into SRAM
 * @return s32 To document
 */
s32 SaveData_LoadSlotFromSram(s32 saveSlot)
{
    struct EwramData **ewramAddress = &gEwramData;
    s32 saveValid;
    void *sramSaveSlot;

    saveValid = SramCheckInitialized();
    if (saveValid)
    {
        sramSaveSlot = SRAM_BASE + 0x10 + saveSlot * 0x47C;
        saveValid = SramCheckSaveSlotUnlocked(saveSlot);
        if (saveValid)
        {
            gReadSramFast(sramSaveSlot, &(*ewramAddress)->playerStatus, 0x190);
            gReadSramFast(sramSaveSlot + 0x190, &(*ewramAddress)->gameState.playerName, 0x20);
            gReadSramFast(sramSaveSlot + 0x190 + 0x20, &(*ewramAddress)->gameState.mapCells, 0x2CC);

            if (((*ewramAddress)->gameState.savedPlayerX | (*ewramAddress)->gameState.savedPlayerY) != 0)
               return saveValid;
        }
        saveValid = FALSE;
    }
    return saveValid;
}

const u32 sFlagsClearedAfterEnding[] = {
    (1 << 5) | 12, (1 << 5) | 13, 0
};

/**
 * @brief Reload the slot (or the default first save room when empty), keep
 * the current play time, restore HP/MP, apply the ending-specific flag
 * resets (afterClear: listed game flags; else flag 0x28 and bossEventFlags
 * bit 0), refresh the summary and save. Return the SRAM error.
 */
s32 SaveData_ResaveSlotAfterEnding(s32 saveSlot, s32 fullClear)
{
    struct SaveHeader *header;
    struct GameState *gameState;
    s32 inGameTimerBackup;
    s32 i;
    u32 error_addr;

    gameState = &gEwramData->gameState;
    header = &gEwramData->saveHeaders[1];
    inGameTimerBackup = gameState->inGameTimer;

    if (SaveSlot_ReadHeader(saveSlot, 1) && header->playTime)
    {
        SaveData_LoadSlotFromSram(saveSlot);
    }
    else
    {
        gameState->currentArea = 0;
        gameState->currentRoom = 0x24; // First save room
        gameState->savedCameraX = 0;
        gameState->savedCameraY = 0x30;
        gameState->savedPlayerX = 0x78;
        gameState->savedPlayerY = 0x9F;
    }

    gameState->inGameTimer = inGameTimerBackup;
    Player_RestoreHPAndMP();

    if (fullClear != 0)
    {
        gameState->gameCleared = 1;
        gEwramData->gameState.bossEventFlags &= ~0x80; // clear Chaos death flag?

        for (i = 0; sFlagsClearedAfterEnding[i] != 0; i++)
        {
            GameFlag_Clear(sFlagsClearedAfterEnding[i]);
        }
    }
    else
    {
        gEwramData->gameState.bossEventFlags &= ~1; // clear Graham death flag?
        GameFlag_Clear((1 << 5) | 8);
    }

    SaveData_StoreHeaderSummary(&gEwramData->gameState);
    error_addr = SaveData_SaveSlotToSram_inline(saveSlot);
    return error_addr;
}

// deletes a file?
// copies a file?
/**
 * @brief Delete a save slot from the file menu. If another slot is linked to
 * it, rewrite that slot without the link (also unlinking other slots
 * that referred to it); otherwise clear the slot. Return the SRAM error.
 */
s32 SaveData_DeleteSlot(s32 saveSlot)
{
    s32 newSource;
    s32 otherSlot;
    u32 error_addr;
    u32 valid;
    s32 i;
    s32 sourceLink;
    struct SaveHeader *header;
    struct SaveHeader *header0;
    s32 used;
    struct SaveHeader *header1;
    struct SaveHeader *header0b;

    header0 = &gEwramData->saveHeaders[0];
    valid = SaveSlot_ReadHeader(saveSlot, 0);
    used = valid && header0->playTime;
    if (used == 0)
    {
        return 0;
    }

    header = gEwramData->saveHeaders;
    sourceLink = header->sourceSlotLink;

    if (sourceLink != 0)
    {
        otherSlot = sourceLink - 1;

        SaveData_ClearSlotInSram_inline(saveSlot);
        SaveData_LoadSlotFromSram(otherSlot);
        SaveLink_ClearCopySlot();

        for (i = 0; i < 6; i += 2)
        {
            if ((i != saveSlot) && (otherSlot != i))
            {
                header1 = &gEwramData->saveHeaders[1];
                valid = SaveSlot_ReadHeader(i, 0);
                used = valid && header1->playTime;
                if (used != 0)
                {
                    header = &gEwramData->saveHeaders[1];
                    if (header->sourceSlotLink == (otherSlot + 1))
                    {
                        SaveLink_SetSlot(1, i + 1);
                    }
                }
            }
        }

        error_addr = SaveData_SaveSlotToSram_inline(otherSlot);
        return error_addr;
    }

    else
    {
        otherSlot = header->copySlotLink;

        error_addr = SaveData_ClearSlotInSram_inline(saveSlot);
        if (otherSlot == 0)
        {
            return error_addr;
        }
        otherSlot -= 1;
        newSource = -1;

        for (i = 0; i < 6; i += 2)
        {
            if ((i != saveSlot) && (otherSlot != i))
            {
                header0b = &gEwramData->saveHeaders[0];
                valid = SaveSlot_ReadHeader(i, 0);
                used = valid && header0b->playTime;
                if (used != 0)
                {
                    header = &gEwramData->saveHeaders[0];
                    if (header->sourceSlotLink == (saveSlot + 1))
                    {
                        newSource = i;
                    }
                }
            }
        }

        SaveData_LoadSlotFromSram(otherSlot);
        SaveLink_DetachFromSource();
        if (newSource >= 0)
        {
            SaveLink_SetSlot(1, newSource + 1);
        }

        error_addr = SaveData_SaveSlotToSram_inline(otherSlot);
        if (newSource >= 0)
        {
            SaveData_LoadSlotFromSram(newSource);
            SaveLink_SetSlots(otherSlot + 1, 0);
            error_addr = SaveData_SaveSlotToSram_inline(newSource);
        }
        return error_addr;
    }
}

// copies file?
/**
 * @brief Copy save slot src to dst: delete dst, relink src (and its odd
 * companion slot when used) to dst, then write src data to dst.
 * Return the SRAM error of the final write.
 */
s32 SaveData_CopySlot(s32 srcSaveSlot, s32 dstSaveSlot)
{
    s32 saveSlot;
    struct SaveHeader *header;
    u32 valid;
    s32 hasBackup;
    u32 error_addr;

    SaveData_DeleteSlot(dstSaveSlot);
    SaveData_LoadSlotFromSram(srcSaveSlot);
    SaveLink_SetSlot(1, dstSaveSlot + 1);

    SaveData_SaveSlotToSram_inline(srcSaveSlot);
    saveSlot = srcSaveSlot + 1;
    header = &gEwramData->saveHeaders[0];
    valid = SaveSlot_ReadHeader(saveSlot, 0);
    hasBackup = valid && header->playTime;
    if (hasBackup)
    {
        SaveData_LoadSlotFromSram(srcSaveSlot + 1);
        SaveLink_SetSlot(1, dstSaveSlot + 1);
        SaveData_SaveSlotToSram_inline(srcSaveSlot + 1);
    }

    SaveData_LoadSlotFromSram(srcSaveSlot);
    SaveLink_AttachToSource(srcSaveSlot + 1);
    error_addr = SaveData_SaveSlotToSram_inline(dstSaveSlot);
    return error_addr;
}

/**
 * @brief When the slot holds data, load it, subtract one soul of the given
 * group/type/id, unequip the soul if no longer owned, save it and
 * return 1; otherwise return 0.
 */
s32 SaveData_RemoveSoulInSlot(s32 saveSlot, s32 soulGroupOffset, s32 soulType, s32 soulIndex)
{
    struct SaveHeader *header;
    u32 valid;
    s32 used;

    header = &gEwramData->saveHeaders[0];
    valid = SaveSlot_ReadHeader(saveSlot, 0);
    used = valid && header->playTime;
    if (used != 0)
    {
        SaveData_LoadSlotFromSram(saveSlot);
        SoulInventory_AddAmountToGroupTotal(soulGroupOffset, soulType, soulIndex, -1);
        SoulInventory_UnequipIfUnowned(soulType, soulIndex);
        SaveData_SaveSlotToSram_inline(saveSlot);
        return 1;
    }
    return 0;
}

/**
 * @brief Remove one soul (type, id) given away in a trade from the save slot,
 * resaving it and every slot linked to it.
 */
s32 SaveData_GiveAwaySoul(s32 saveSlot, s32 soulType, s32 soulId)
{
    struct GameState *gameState;
    struct SaveHeader *slotData;
    struct EwramData **ewramAddress;
    u32 loaded;
    u32 errorAddress;
    s32 slot;
    s32 linkedSlot;
    struct SaveHeader *metadata;
    s32 undefinedResult;
    s32 occupied;

    gameState = &gEwramData->gameState;
    if ((SoulInventory_GetGroupSoulTotal(0, soulType, soulId) != 0) && (gameState->slotLinks[1] == 0))
    {
        SoulInventory_AddAmountToSoulTotal(soulType, soulId, -1);
        SoulInventory_UnequipIfUnowned(soulType, soulId);
        errorAddress = SaveData_SaveSlotToSram_inline(saveSlot);
        return errorAddress;
    }

    saveSlot = saveSlot + 1;
    while(TRUE)
    {
        ewramAddress = &gEwramData;
        SaveSlot_ReadHeader(saveSlot - 1, 0);
        metadata = &(*ewramAddress)->saveHeaders[0];
        if (metadata->sourceSlotLink == 0)
            break;
        saveSlot = metadata->sourceSlotLink;
    }

    saveSlot = saveSlot - 1;
    SaveData_LoadSlotFromSram(saveSlot);
    SoulInventory_AddAmountToGroupTotal(0, soulType, soulId, -1);
    SoulInventory_UnequipIfUnowned(soulType, soulId);
    SaveData_SaveSlotToSram_inline(saveSlot);
    SaveData_RemoveSoulInSlot(saveSlot + 1, 0, soulType, soulId);
    
    for(slot = 0; slot < 6; slot += 2)
    {
        if (slot != saveSlot)
        {
            slotData = &gEwramData->saveHeaders[0];
            loaded = SaveSlot_ReadHeader(slot, 0);
            occupied = loaded && slotData->playTime;
            if (occupied)
            {
                metadata = &gEwramData->saveHeaders[0];
                if (metadata->sourceSlotLink == (saveSlot + 1))
                {
                    SaveData_RemoveSoulInSlot(slot, 1, soulType, soulId);
                    SaveData_RemoveSoulInSlot(slot + 1, 1, soulType, soulId);
                    linkedSlot = metadata->copySlotLink;
                    if (linkedSlot != 0)
                    {
                        SaveData_RemoveSoulInSlot(linkedSlot - 1, 1, soulType, soulId);
                        SaveData_RemoveSoulInSlot(linkedSlot, 1, soulType, soulId);
                    }
                }
            }
        }
    }

    // The original returns an undefined value on this path; preserve it for matching.
    return undefinedResult;
}

/**
 * @brief Clear the progress block (preserving gameFlagsTail bits 0x100000 and
 * 0x200000), set game flag 0x3B, zero soul totals and visitedThisRun, clear
 * abilities, set level 1, restore HP/MP and unequip all souls.
 */
void Game_ResetProgressForNewGame(void)
{
    s32 j;
    s32 i;
    struct SoulKey *soul;
    u32 keepFlag20;
    u32 keepFlag21;

    soul = sKeySouls;

    if (gEwramData->gameState.gameFlagsTail[0] & 0x100000)
        keepFlag20 = 1;
    else
        keepFlag20 = 0;

    if (gEwramData->gameState.gameFlagsTail[0] & 0x200000)
        keepFlag21 = 1;
    else
        keepFlag21 = 0;

    DMA_FILL_32(3, 0, &gEwramData->gameState.gameFlags, 0x44);
    GameFlag_Set((1 << 5) | 0x1B);

    if (keepFlag20 != 0)
    {
        gEwramData->gameState.gameFlagsTail[0] |= 0x100000;
    }
    if (keepFlag21 != 0)
    {
        gEwramData->gameState.gameFlagsTail[0] |= 0x200000;
    }

    for (i = 0; i < 11; i++, soul += 1)
    {
        SoulInventory_SetGroupTotalAmount(0, soul->soulType, soul->oneBasedSoulId - 1, 0);
        SoulInventory_SetGroupTotalAmount(1, soul->soulType, soul->oneBasedSoulId - 1, 0);
    }

    for (j = 0; j < 40; j++)
    {
        for (i = 0; i < 2; i++)
        {
            gEwramData->gameState.mapCells[i][j].visitedThisRun = 0;
        }
    }

    for (i = 0; i < 8; i++)
    {
        Player_SetAbility(i, 0);
    } 

    Player_SetLevel(1);
    Player_RestoreHPAndMP();
    gEwramData->playerStatus.equippedRedSoul = 0;
    gEwramData->playerStatus.equippedBlueSoul = 0;
    gEwramData->playerStatus.equippedYellowSoul = 0;
}

/**
 * @brief Unreferenced: set the start position, start the play timer and save
 * the current slot (no return value).
 */
s32 SaveData_CreateNewGameSlot(void)
{
    struct GameState *gameState;

    gameState = &gEwramData->gameState;
    Game_SetStartPosition(0);
    gameState->inGameTimer = 1;
    gameState->saveMapCompletionTenths = 0;
    gameState->saveLevel = 1;
    SaveData_SaveSlotToSram_inline(gameState->currentSaveSlot);
    // BUG: no return value
}

// creates quick save?
// called on save?
/**
 * @brief Store gold, level, map completion tenths, character and unk_13267
 * in the slot summary fields of the given gameState block.
 */
void SaveData_UpdateSlotSummary(struct GameState *gameState)
{
    // TODO: SaveData_UpdateSlotSummary is the same as SaveData_StoreHeaderSummary
    gameState->saveGold = gEwramData->playerStatus.currentGold;
    gameState->saveLevel = gEwramData->playerStatus.currentLevel;
    gameState->saveMapCompletionTenths = Map_GetCompletionTenths(gameState, 2);
    gameState->saveCharacter = gEwramData->playerStatus.currentCharacter;
    gameState->unk_A5 = gEwramData->playerStatus.unk_13267;
}

/**
 * @brief 13620 | To document
 * 
 * @param saveSlot Save slot index into SRAM
 * @return s32 To document
 */
u32 SaveData_SaveSlotToSram(s32 saveSlot)
{
    struct EwramData **ewramAddress = &gEwramData;
    // todo: SaveData_SaveSlotToSram is the same as SaveData_SaveSlotToSram_inline
    void *sramSaveSlot;
    u32 error_addr;

    sramSaveSlot = SRAM_BASE + 0x10 + saveSlot * 0x47C;
    SramLockSaveSlot(saveSlot);
    error_addr = WriteAndVerifySramFast(&(*ewramAddress)->playerStatus, sramSaveSlot, 0x190);
    if (error_addr == 0)
    {
        error_addr = WriteAndVerifySramFast(&(*ewramAddress)->gameState.playerName, sramSaveSlot + 0x190, 0x20);
        if (error_addr == 0)
        {
            error_addr = WriteAndVerifySramFast(&(*ewramAddress)->gameState.mapCells, sramSaveSlot + 0x190 + 0x20, 0x2CC);
        }
    }
    SramUnlockSaveSlot(saveSlot);
    return error_addr;
}

/**
 * @brief 13698 | Clear the save slot in SRAM
 * 
 * @param saveSlot Save slot index into SRAM
 * @return s32 To document
 */
s32 SaveData_ClearSlotInSram(s32 saveSlot)
{
    // todo: SaveData_ClearSlotInSram is the same as SaveData_ClearSlotInSram_inline
    u32 error_addr;
    void *zeroBuf;
    void *sramSaveSlot;

    zeroBuf = gEwramData->sharedScratchBuffer;
    DMA_FILL_32(3, 0, zeroBuf, 0x47C);
    sramSaveSlot = SRAM_BASE + 0x10 + saveSlot * 0x47C;
    SramLockSaveSlot(saveSlot);
    error_addr = WriteAndVerifySramFast(zeroBuf, sramSaveSlot, 0x47C);
    SramUnlockSaveSlot(saveSlot);
    return error_addr;
}

/**
 * @brief Read the SRAM header of saveSlot into menu buffer unk_20[bufferIndex], or
 * clear that buffer when the slot is locked.
 *
 * @param saveSlot Save slot index into SRAM
 * @param bufferIndex Destination menu-buffer index
 * @return Nonzero when SRAM is initialized and the slot is unlocked
 */
s32 SaveData_ReadSlotHeader(s32 saveSlot, s32 bufferIndex)
{
    struct EwramData **ewramAddress = &gEwramData;
    // todo: SaveData_ReadSlotHeader is the same as SaveSlot_ReadHeader
    s32 saveValid;
    s32 sramSaveSlotOffset;

    saveValid = SramCheckInitialized();
    if (saveValid)
    {
        sramSaveSlotOffset = (saveSlot * 0x47C);
        saveValid = SramCheckSaveSlotUnlocked(saveSlot);
        if (saveValid)
        {
            gReadSramFast(SRAM_BASE + 0x10 + 0x190 + sramSaveSlotOffset, &(*ewramAddress)->saveHeaders[bufferIndex], sizeof((*ewramAddress)->saveHeaders[bufferIndex]));
        }
        else
        {
            DMA_FILL_32(3, 0, &(*ewramAddress)->saveHeaders[bufferIndex], sizeof((*ewramAddress)->saveHeaders[bufferIndex]));
        }
    }
    return saveValid;
}

/**
 * @brief Write the 4-byte system flags gameState.systemFlags to system slot 6.
 * Return the SRAM error.
 */
u32 SaveData_SaveSystemFlags(void)
{
    struct EwramData **ewramAddress = &gEwramData;
    u32 error_addr;
    void *dst;

    dst = SRAM_BASE + 0x10 + 0x47C * 6 + 0;
    SramLockSaveSlot(6);
    error_addr = WriteAndVerifySramFast(&(*ewramAddress)->gameState, dst, 4);
    SramUnlockSaveSlot(6);
    return error_addr;
}

/**
 * @brief When SRAM is initialized and system slot 6 is unlocked, read its
 * system flags into gameState.systemFlags. Return the validity result.
 */
u32 SaveData_LoadSystemFlags(void)
{
    struct EwramData **ewramAddress = &gEwramData;
    s32 saveValid;
    void *sramAddr;

    saveValid = SramCheckInitialized();
    if (saveValid)
    {
        sramAddr = SRAM_BASE + 0x10 + 0x47C * 6 + 0;
        saveValid = SramCheckSaveSlotUnlocked(6);
        if (saveValid)
        {
            gReadSramFast(sramAddr, &(*ewramAddress)->gameState.systemFlags, sizeof((*ewramAddress)->gameState.systemFlags));
        }
    }
    return saveValid;
}

/**
 * @brief Return the linked slot field sourceSlotLink of a save summary.
 */
s32 SaveSummary_GetLinkedSlot(struct SaveHeader *header)
{
    return header->sourceSlotLink;
}

/**
 * @brief Set system flags 1|2 in gameState.systemFlags, write them to system slot 6
 * and resave the current slot with SaveData_ResaveSlotAfterEnding(..,1).
 */
s32 SaveData_RecordFullClear(void)
{
    s32 resaveError;
    u32 error_addr;

    gEwramData->gameState.systemFlags |= 3;

    SramLockSaveSlot(6);
    error_addr = WriteAndVerifySramFast(&gEwramData->gameState.systemFlags, SRAM_BASE + 0x10 + 0x47C * 6 + 0, sizeof(gEwramData->gameState.systemFlags));
    SramUnlockSaveSlot(6);
    if (error_addr != 0)
    {
        return 0;
    }

    resaveError = SaveData_ResaveSlotAfterEnding(gEwramData->gameState.currentSaveSlot, 1);
    if (resaveError != 0)
    {
        return 0;
    }
    // BUG: no return value?
}

/**
 * @brief Read the slot header into menu buffer 0 (cleared when locked) and
 * return 1 when SRAM is valid and buffer[index].unk_38 marks data.
 * No source caller.
 */
s32 SaveData_IsSlotUsed(s32 saveSlot, s32 headerIndex)
{
    struct SaveHeader *header;
    u32 saveValid;
    s32 sramSaveSlotOffset;

    header = &gEwramData->saveHeaders[headerIndex];

    saveValid = SramCheckInitialized();
    if (saveValid)
    {
        sramSaveSlotOffset = saveSlot * 0x47C;
        saveValid = SramCheckSaveSlotUnlocked(saveSlot);
        if (saveValid)
        {
            gReadSramFast(SRAM_BASE + 0x10 + sramSaveSlotOffset + 0x190, &gEwramData->saveHeaders[0], sizeof(gEwramData->saveHeaders[0]));
        }
        else
        {
            DMA_FILL_32(3, 0, &gEwramData->saveHeaders[0], sizeof(gEwramData->saveHeaders[0]));
        }

        if (saveValid && (header->playTime != 0))
        {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief 138E8 | Save the language to SRAM
 * 
 * @return u32 To document
 */
u32 SaveData_SaveLanguage(void)
{
    struct EwramData **ewramAddress = &gEwramData;
    u32 error_addr;
    void *sramLanguageOffset;

    sramLanguageOffset = SRAM_BASE + 0x10 + 0x47C * 6 + 4;
    SramLockSaveSlot(6);
    error_addr = WriteAndVerifySramFast(&(*ewramAddress)->gameState.language, sramLanguageOffset, sizeof((*ewramAddress)->gameState.language));
    SramUnlockSaveSlot(6);
    return error_addr;
}

/**
 * @brief 1391C | Load the language from SRAM
 * 
 * @return u32 To document
 */
s32 SaveData_LoadLanguage(void)
{
    struct EwramData **ewramAddress = &gEwramData;
    s32 saveValid;
    void *sramLanguageOffset;

    saveValid = SramCheckInitialized();
    if (saveValid)
    {
        sramLanguageOffset = SRAM_BASE + 0x10 + 0x47C * 6 + 4;
        saveValid = SramCheckSaveSlotUnlocked(6);
        if (saveValid)
        {
            gReadSramFast(sramLanguageOffset, &(*ewramAddress)->gameState.language, sizeof((*ewramAddress)->gameState.language));
        }
    }
    return saveValid;
}
