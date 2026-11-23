#include "code_08009A0.h"
#include "code_08012744.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"

struct MenuPage
{
    u8 pad_0[0xA];
    u8 state;
    u8 subState;
    u8 pad_C[0x15 - 0xC];
    u8 cursor;
};

/**
 * @brief 4BD38 | Menu save page: when saving is not possible show message
 * 0x360 / 0x361 and play the error sound, otherwise show the save prompt
 * (texts 0x34B / 0x34D) and place the cursor.
 */
void SaveMenu_Open(struct MenuPage *page)
{
    struct SaveHeader *slot;
    struct EwramData *ew;
    struct EwramData_EntityData *ent;

    ew = gEwramData;
    slot = &ew->saveHeaders[0];
    SaveData_ReadSlotHeader(ew->gameState.currentSaveSlot, 0);
    if (ew->gameState.bossRushMode)
    {
        MenuText_ClearAndPrint(0x360);
        page->state = 0xFF;
        PlaySong(0xEF);
    }
    else if (gEwramData->gameState.stateFlags & 2)
    {
        MenuText_ClearAndPrint(0x360);
        page->state = 0xFF;
        PlaySong(0xEF);
    }
    else if (slot->playTime == 0)
    {
        MenuText_ClearAndPrint(0x361);
        page->state = 0xFF;
        PlaySong(0xEF);
    }
    else
    {
        MenuText_ClearAndPrint(0x36A);
        Text12_SetCursor(4, 1);
        Text12_Print(Text_GetLocalizedEntry(0x34B), 0);
        Text12_SetCursor(0xC, 1);
        Text12_Print(Text_GetLocalizedEntry(0x34D), 0);
        Text12_SetCursor(0, 0);
        page->cursor = 1;
        page->state = 0;
        page->subState = 0;
        ent = &gEwramData->entityData[18];
        ent->unk_518.part16.hp = 0x95;
        ent->unk_518.part16.unk_51A = 0x89;
    }
}
