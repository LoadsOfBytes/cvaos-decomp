#include "code_08009A0.h"
#include "code_08012744.h"
#include "structs/ewram.h"

extern void ItemInventory_AddOneByGlobalId(u32 item);

/**
 * @brief 511F0 | Object 0x10 (save point?): when stateFlags bit 10 is set,
 * wait 128 frames with the player input cleared, then reload the current
 * save slot (SaveData_LoadSlotFromSram), call ItemInventory_AddOneByGlobalId(unk_4FC) and
 * SaveData_UpdateSlotSummary, store entity 26's unk_4F8 into bestBossRushTimeFrames when bestBossRushTimeFrames is 0 or
 * larger, and write the slot back (SaveData_SaveSlotToSram); finally set
 * subMode = 2, subModeStep = 0 (state 3).
 */
void Object10Update(struct EwramData_EntityData *entity)
{
    struct GameState *data = &gEwramData->gameState;
    u8 state = entity->state;
    u32 value;

    switch (state)
    {
    case 0:
        if (gEwramData->gameState.stateFlags & 0x400)
        {
            entity->timer = 0x80;
            entity->state = 1;
            entity->hidden = 0;
        }
        break;
    case 1:
        SetPlayerInput(1, 0);
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        entity->state = 2;
        SaveData_LoadSlotFromSram(gEwramData->gameState.currentSaveSlot);
        ItemInventory_AddOneByGlobalId(entity->unk_4FC.whole);
        SaveData_UpdateSlotSummary(&gEwramData->gameState);
        {
            struct EwramData_EntityData *slot26 = &gEwramData->entityData[26];
            value = slot26->unk_4F8.whole;
        }
        if (data->bestBossRushTimeFrames > value || data->bestBossRushTimeFrames == 0)
            data->bestBossRushTimeFrames = value;
        SaveData_SaveSlotToSram(gEwramData->gameState.currentSaveSlot);
        break;
    case 2:
        SetPlayerInput(1, 0);
        gEwramData->gameState.subMode = state;
        gEwramData->gameState.subModeStep = 0;
        entity->state = 3;
        break;
    }
}
