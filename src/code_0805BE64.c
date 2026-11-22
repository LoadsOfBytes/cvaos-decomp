/* EventScene01_Update. */
#include "code_08009A0.h"
#include "structs/ewram.h"
#include "structs/main.h"
#include "gba/display.h"

/* Declared with an int result as in code_0804F690.c: the original compares
 * the returned value without sign extension. */
extern s32 GetEntityRoomXPositionInteger(struct EwramData_EntityData *entity);
extern void Interaction_End(struct EwramData_EntityData *entity, s32 deleteEntity);
extern void TextWindow_Begin(s32 textId);
extern s32 TextWindow_Update(void);

/**
 * @brief Update of event scene variant 1 (sEventSceneUpdateFuncs[1]); returns the player
 * input mask. Event step (state state): once the player stands at room X
 * 0x81..0x11F set stateFlags bit 0; then TextWindow_Begin(0xC); then wait
 * for TextWindow_Update and finish via Interaction_End(entity, 1).
 * States 0x80/0x81 hide BG0 and show it again on finishing.
 */
s32 EventScene01_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
    case 0:
        if (GetEntityRoomXPositionInteger(gEwramData->playerState.player) > 0x80
            && GetEntityRoomXPositionInteger(gEwramData->playerState.player) <= 0x11F)
        {
            gEwramData->gameState.stateFlags |= 1;
            entity->state++;
        }
        break;
    case 1:
        TextWindow_Begin(0xC);
        entity->state++;
        break;
    case 2:
        if (TextWindow_Update())
            Interaction_End(entity, 1);
        break;
    case 0x80:
        gUnk_03002CB0.dispCnt &= ~DCNT_BG0;
        entity->state++;
        break;
    case 0x81:
        Interaction_End(entity, 1);
        gUnk_03002CB0.dispCnt |= DCNT_BG0;
        break;
    }
    return 0;
}
