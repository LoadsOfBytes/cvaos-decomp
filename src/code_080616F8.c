#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void TextWindow_Begin(s32);
extern s32 TextWindow_Update(void);
extern void Interaction_End(struct EwramData_EntityData *, s32);
extern s32 Entity_IsInRect(struct EwramData_EntityData *, s16, s16, u16, u16);

/**
 * @brief Update of event scene variant 16 (sEventSceneUpdateFuncs[16]); returns the
 * player input mask. NPC object: when the player stands near it and
 * presses up / A while no conversation runs, stops the player and starts
 * conversation 0x1D (0x1E after the fourth time), facing the player.
 */
s32 EventScene16_Update(struct EwramData_EntityData *entity)
{
    s32 ret;
    u16 new;
    struct EwramData_EntityData *player;

    ret = 0;
    new = gEwramData->inputData.newInput;
    switch (entity->state)
    {
    case 0:
        Interaction_End(entity, 0);
        entity->state++;
        /* fall through */
    case 1:
        if (!Entity_IsInRect(gEwramData->playerState.player, entity->posX.part16.integer - 0x20, entity->posY.part16.integer - 0x20, 0x40, 0x40))
            break;
        if (!(new & 0x42))
            break;
        if (gEwramData->gameState.timeStopTimer != 0)
            break;
        gEwramData->playerState.player->velX.whole = 0;
        gEwramData->playerState.player->accelX.whole = 0;
        player = gEwramData->playerState.player;
        if (player->velY.whole < 0)
            player->velY.whole = 0;
        if (entity->timer++ > 3)
            TextWindow_Begin(0x1E);
        else
            TextWindow_Begin(0x1D);
        entity->flipX = entity->posX.whole < gEwramData->playerState.player->posX.whole;
        gEwramData->gameState.stateFlags |= 1;
        entity->state++;
        break;
    case 2:
        if (TextWindow_Update())
        {
            entity->state = 0x80;
            Interaction_End(entity, 0);
            entity->state = ret;
        }
        break;
    }
    return ret;
}
