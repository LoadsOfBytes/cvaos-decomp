/* Object 0x20 0x0805B42C..0x0805B5E7. */
#include "code_08009A0.h"
#include "code_0800B700.h"
#include "code_0800CB00.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void TextWindow_AdjustOpening(s32 delta);
extern void Conversation_Reset(void);

/* Create / update of the variants of object 0x20. */
extern void (*const sEventSceneCreateFuncs[])(struct EwramData_EntityData *entity);
extern u32 (*const sEventSceneUpdateFuncs[])(struct EwramData_EntityData *entity);

/**
 * @brief 5B42C | Object 0x20 create. Deleted unless (HP > 0 or stateFlags bit
 * 0x200000), unk_8E_7 clear and (Soma or stateFlags bit 0x8000); runs the create
 * of its variant, then (unless 0x53D bit 3 is set) sets stateFlags bit 0x200,
 * stores itself in sceneController and releases the player input.
 */
void Object20Create(struct EwramData_EntityData *entity)
{
    if (gEwramData->playerStatus.currentHP <= 0 && !(gEwramData->gameState.stateFlags & 0x200000))
    {
        EntityDelete(entity);
        return;
    }
    if (gEwramData->gameState.unk_8E_7
        || (gEwramData->playerStatus.currentCharacter && !(gEwramData->gameState.stateFlags & 0x8000)))
    {
        EntityDelete(entity);
        return;
    }
    sEventSceneCreateFuncs[entity->spawnParam0](entity);
    if (entity->deleteRequested)
    {
        EntityDelete(entity);
        return;
    }
    gEwramData->gameState.stateFlags |= 0x200;
    Hud_CloseMessageWindow();
    RedSoul_CancelAll();
    BlueSoul_StopAll();
    Game_SetMenusEnabled(0);
    gEwramData->gameState.sceneController = entity;
    if (gEwramData->gameState.stateFlags & 1)
        SetPlayerInput(1, 0);
    Conversation_Reset();
}

/**
 * @brief 5B510 | Object 0x20 update: run the update of its variant and feed
 * its result to SetPlayerInput while stateFlags bit 0 is set.
 */
void Object20Update(struct EwramData_EntityData *entity)
{
    u32 playerInputs;
    struct EwramData *ewram = gEwramData;

    if (ewram->gameState.stateFlags & 0x80)
        ewram->hBlankEffect.requestStop = 1;
    if ((s8)entity->state < 0)
    {
        if (ewram->gameState.textWindowOpening)
            TextWindow_AdjustOpening(-3);
    }
    else if ((gEwramData->inputData.newInput & 8) && (ewram->gameState.systemFlags & 2)
             && !(gEwramData->gameState.stateFlags & 0x10000))
    {
        ewram->gameState.textWindowState = 5;
    }
    playerInputs = sEventSceneUpdateFuncs[entity->spawnParam0](entity);
    if (gEwramData->gameState.stateFlags & 1)
    {
        SetPlayerInput(1, playerInputs);
        Game_SetMenusEnabled(0);
    }
    else
    {
        Game_SetMenusEnabled(1);
    }
}
