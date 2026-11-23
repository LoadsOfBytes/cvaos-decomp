#include "code_08009A0.h"
#include "code_08001194.h"
#include "code/code_08060B98.h"
#include "structs/main.h"
#include "structs/ewram.h"

extern void Door_SetupGraphics(struct EwramData_EntityData *entity);

/**
 * @brief 4EC3C | Object 6 init (as Object00Create): Door_SetupGraphics, palette
 * palette = 5, window outside 0x3F / 0x2F; when the player is inside the
 * 0x38 x 0x22 area 0x1C px to its left or bossEventFlags bit 2 is set, set unk_4F8
 * to 0x30, state 10 and clear player input; otherwise mark three tiles
 * above it (Collision_SetBlock), reset unk_4F8 and pick the state from spawnParam0
 * (1: 0x14, 3: 0x63, else 0).
 */
void Object06Create(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *player;
    struct BgLayerState *bg;
    u16 flag;
    s32 x, y;

    Door_SetupGraphics(entity);
    entity->palette = 5;
    gDisplayRegisters.winOut_L = 0x3F;
    gDisplayRegisters.winOut_H = 0x2F;
    player = gEwramData->playerState.player;
    if (Entity_IsInRect(entity, (s16)(player->posX.part16.integer - 0x1C), player->posY.part16.integer, 0x38, 0x22)
        || (flag = gEwramData->gameState.bossEventFlags & 4) != 0)
    {
        entity->ext.door.openHeight = 0x30;
        entity->state = 10;
        SetPlayerInput(1, 0);
        return;
    }
    x = (s16)entity->posX.part16.integer;
    bg = &gEwramData->bgInfo[1];
    x += (s16)bg->xPos.part16.integer;
    y = (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer;
    Collision_SetBlock(x, y - 8);
    Collision_SetBlock(x, y - 0x18);
    Collision_SetBlock(x, y - 0x28);
    entity->ext.door.openHeight = flag;
    switch (entity->spawnParam0)
    {
    case 1:
        entity->state = 0x14;
        break;
    case 3:
        entity->state = 0x63;
        break;
    default:
        entity->state = flag;
        break;
    }
}
