#include "code_08009A0.h"
#include "code_08001194.h"
#include "code/code_080211F0.h"
#include "code/code_08060B98.h"
#include "structs/main.h"
#include "structs/ewram.h"

extern void Door_SetupGraphics(struct EwramData_EntityData *entity);

/**
 * @brief 4E770 | Object 4 init (one-way door, as Object02Create): deleted
 * when unk_514 <= 1 and bit unk_516 of bossEventFlags is set; Door_SetupGraphics, window
 * outside 0x3F / 0x2F. The player counts as passing when at most 0x18 px
 * right of it (unk_4FE == 0) or at most 0x18 px left of it (otherwise):
 * then unk_4F8 = 0x30, state 10, clear player input, RedSoul_CancelAll and stop
 * the player when moving towards the room centre; otherwise mark three
 * tiles above it and pick the state from unk_514.
 */
void Object04Create(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *player;
    struct EwramData_EntityData **pp;
    struct BgLayerState *bg;
    s32 found;
    s32 x, y;

    if (entity->unk_514.part16.spawnParam0 <= 1
        && ((gEwramData->gameState.bossEventFlags >> entity->unk_514.part16.spawnParam1) & 1))
    {
        EntityDelete(entity);
        return;
    }
    Door_SetupGraphics(entity);
    gDisplayRegisters.winOut_L = 0x3F;
    gDisplayRegisters.winOut_H = 0x2F;
    if (entity->unk_4FC.part8.unk_4FE == 0)
    {
        if ((s16)gEwramData->playerState.player->posX.part16.integer <= (s16)entity->posX.part16.integer + 0x18)
            goto passed;
        goto blocked;
    }
    else
    {
        s32 px = (s16)gEwramData->playerState.player->posX.part16.integer;
        s32 ex = (s16)entity->posX.part16.integer - 0x18;
        found = 0;
        if (px >= ex)
        {
        passed:
            found = 1;
        }
    }
    if (found)
    {
        entity->unk_4F8.whole = 0x30;
        found = 0;
        entity->state = 10;
        SetPlayerInput(1, 0);
        RedSoul_CancelAll();
        pp = &gEwramData->playerState.player;
        player = *pp;
        if ((s16)player->posX.part16.integer <= 0x77)
        {
            if (player->velX.whole >= 0)
                return;
        }
        else if (player->velX.whole <= 0)
            return;
        player->velX.whole = found;
        (*pp)->accelX.whole = found;
        return;
    }
blocked:
    x = (s16)entity->posX.part16.integer;
    bg = &gEwramData->bgInfo[1];
    x += (s16)bg->xPos.part16.integer;
    y = (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer;
    Collision_SetBlock(x, y - 8);
    Collision_SetBlock(x, y - 0x18);
    Collision_SetBlock(x, y - 0x28);
    entity->unk_4F8.whole = 0;
    switch (entity->unk_514.part16.spawnParam0)
    {
    case 1:
        entity->state = 0x14;
        break;
    case 3:
        entity->state = 0x63;
        break;
    default:
        entity->state = 0;
        break;
    }
}
