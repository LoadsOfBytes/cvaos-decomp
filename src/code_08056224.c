#include "code_08009A0.h"
#include "room_object_entity.h"
#include "code_0800B700.h"
#include "code_0800F1FC.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
#include "structs/main.h"

void Object1FUpdate(struct EwramData_EntityData *entity)
{
    struct GameState *g = &gEwramData->gameState;
    struct EwramData_EntityData *player = gEwramData->playerState.player;
    u8 t;

    switch (entity->state)
    {
    case 0:
        if (!(gEwramData->inputData.heldInput & 0x40))
            return;
        if (g->subMode != 1)
            return;
        if (Entity_IsInRect(player, 0x55, 0x7F, 0x49, 2) == 0)
            return;
        Game_SetMenusEnabled(0);
        player->objMode = 1;
        gDisplayRegisters.bldAlpha = 0x10;
        entity->ext.object1F.fadeTimer = 0x80;
        break;
    case 1:
        gEwramData->gameState.stateFlags |= 1;
        SetPlayerInput(1, 0);
        t = entity->timer;
        if ((s8)entity->timer >= 0)
        {
            entity->timer = t + 1;
        }
        else
        {
            player->hidden = 1;
            player->objMode = 0;
            gDisplayRegisters.bldAlpha = 0x808;
        }
        gDisplayRegisters.bldAlpha = (0x10 - (entity->timer >> 3)) | ((entity->timer >> 3) << 8);
        if (--entity->ext.object1F.fadeTimer != 0xFFFF)
            return;
        RoomFadeTransition_Begin(g);
        break;
    default:
        return;
    }
    entity->state++;
}
