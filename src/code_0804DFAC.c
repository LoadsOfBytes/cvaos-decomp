#include "code_08009A0.h"
#include "code_08001194.h"
#include "code/code_080211F0.h"
#include "code/code_08060B98.h"
#include "structs/main.h"
#include "structs/ewram.h"

extern void Door_SetupGraphics(struct EwramData_EntityData *entity);

/**
 * @brief 4DFAC | Object 2 init (door, as Object06Create): deleted when
 * unk_514 <= 1 and bit unk_516 of bossEventFlags is set; Door_SetupGraphics, window
 * outside 0x3F / 0x2F, variant 7 uses palette 9 (unk_4FF, palette). When
 * the player is inside the 0x38 x 0x22 area 0x1C px to its left: unk_4F8 =
 * 0x30, state 10, clear player input, RedSoul_CancelAll, and stop the player
 * (velX, accelX) when moving towards the room centre (x 0x78); otherwise
 * mark three tiles above it and pick the state from unk_514.
 */
void Object02Create(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *player;
    struct EwramData_EntityData **pp;
    struct BgLayerState *bg;
    s32 zero;
    s32 found;
    s32 x, y;

    if (entity->unk_514.part16.spawnParam0 <= 1
        && ((gEwramData->gameState.bossEventFlags >> entity->unk_514.part16.spawnParam1) & 1))
    {
        EntityDelete(entity);
        return;
    }
    Door_SetupGraphics(entity);
    {
        struct DisplayRegisters *regs = &gDisplayRegisters;
        u8 *win = &regs->winOut_L;
        zero = 0;
        *win = 0x3F;
        regs->winOut_H = 0x2F;
    }
    if (entity->unk_514.part16.spawnParam1 == 7)
    {
        entity->unk_4FC.part8.unk_4FF = 9;
        entity->palette = entity->unk_4FC.part8.unk_4FF;
    }
    found = Entity_IsInRect(entity, (s16)(gEwramData->playerState.player->posX.part16.integer - 0x1C),
                         gEwramData->playerState.player->posY.part16.integer, 0x38, 0x22);
    if (found)
    {
        entity->unk_4F8.whole = 0x30;
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
        player->velX.whole = zero;
        (*pp)->accelX.whole = zero;
        return;
    }
    x = (s16)entity->posX.part16.integer;
    bg = &gEwramData->bgInfo[1];
    x += (s16)bg->xPos.part16.integer;
    y = (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer;
    Collision_SetBlock(x, y - 8);
    Collision_SetBlock(x, y - 0x18);
    Collision_SetBlock(x, y - 0x28);
    entity->unk_4F8.whole = found;
    switch (entity->unk_514.part16.spawnParam0)
    {
    case 1:
        entity->state = 0x14;
        break;
    case 3:
        entity->state = 0x63;
        break;
    default:
        entity->state = found;
        break;
    }
}
