#include "code_08009A0.h"
#include "gate_entity.h"
#include "code_08001194.h"
#include "code_0800B700.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern void Gate_SetupGraphics(struct EwramData_EntityData *entity, s32 opened);

/**
 * @brief 4D9DC | Object 0 init: Gate_SetupGraphics with unk_514; when the player
 * is inside the 0x28 x 0x28 area 0x14 px to its left (Entity_IsInRect), set
 * bits 0 and 8 of stateFlags, call Game_SetMenusEnabled(0), set unk_4F8 to 0x4000,
 * state 10 and force player input 0x10 (0x20 when flipped); otherwise
 * mark three tiles (Collision_SetBlock) 8 / 0x18 / 0x28 px above its room
 * position and reset unk_4F8 and the state.
 */
void Object00Create(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *player;
    s32 found;
    s32 x, y;
    struct BgLayerState *bg;

    Gate_SetupGraphics(entity, entity->unk_514.part16.spawnParam0);
    player = gEwramData->playerState.player;
    found = Entity_IsInRect(entity, (s16)(player->posX.part16.integer - 0x14), player->posY.part16.integer, 0x28, 0x28);
    if (found)
    {
        gEwramData->gameState.stateFlags |= 0x101;
        Game_SetMenusEnabled(0);
        GATE_OPEN_ANGLE(entity) = 0x4000;
        entity->state = 10;
        SetPlayerInput(1, entity->flipX ? 0x20 : 0x10);
        return;
    }
    x = (s16)entity->posX.part16.integer;
    bg = &gEwramData->bgInfo[1];
    x += (s16)bg->xPos.part16.integer;
    y = (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer;
    Collision_SetBlock(x, y - 8);
    Collision_SetBlock(x, y - 0x18);
    Collision_SetBlock(x, y - 0x28);
    GATE_OPEN_ANGLE(entity) = found;
    entity->state = found;
}
