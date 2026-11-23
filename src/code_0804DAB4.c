#include "code_08009A0.h"
#include "player_entity.h"
#include "code_08001194.h"
#include "code_0800B700.h"
#include "code/code_08039340.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"
extern s32 Entity_IsInRect(struct EwramData_EntityData *, s16, s16, u16, u16);

/**
 * @brief 4DAB4 | Gate object (object 0x00): hidden off screen, state 0x63
 * once roomActive is clear (releases the player input, frees the two affine
 * slots unk_4FC / unk_4FD). State 0 waits for the player to stand in the
 * 0x28 x 2 box facing it (not while bit 9 of soulStateFlags or bit 28 of the
 * player's unk_4F4), takes the input (stateFlags |= 0x101, sound 0x115);
 * state 1 counts unk_4F8 up to 0x4000 and clears the gate tiles
 * (Collision_ClearBlock at three heights); state 2 walks the player through.
 * States 10/11 close it again behind the player (sound 0x114,
 * Collision_SetBlock) and return to state 0.
 */
void Object00Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *player;
    struct BgLayerState *bg;
    s32 near;
    /* the original keeps &gEwramData and 0x13110 in registers from the start */
    struct EwramData **ew = &gEwramData;
    u32 off = 0x13110;

    entity->hidden = Entity_IsInRect(entity, -0x20, 0, 0x130, 0xD0) ? 0 : 1;
    if (!entity->roomActive)
        entity->state = 0x63;
    if (entity->state <= 9)
    {
        player = *(struct EwramData_EntityData **)((u8 *)*ew + off);
        near = Entity_IsInRect(entity, player->posX.part16.integer - 0x14, player->posY.part16.integer, 0x28, 2);
    }
    else
    {
        player = *(struct EwramData_EntityData **)((u8 *)*ew + off);
        near = Entity_IsInRect(entity, player->posX.part16.integer - 0x14, player->posY.part16.integer, 0x28, 0x28);
    }
    switch (entity->state)
    {
    case 0:
        if (!near)
            break;
        if (entity->flipX == gEwramData->playerState.player->flipX)
            break;
        if (gEwramData->playerStatus.soulStateFlags & 0x200)
            break;
        if (gEwramData->playerState.player->ext.player.actionFlags & 0x10000000)
            break;
        SetPlayerInput(1, 0);
        gEwramData->gameState.stateFlags |= 0x101;
        Game_SetMenusEnabled(0);
        PlaySong(0x115);
        entity->state++;
        break;
    case 1:
    {
        s32 x1, y1;
        SetPlayerInput(1, 0);
        if ((entity->ext.gate.openAngle += 0x100) <= 0x3FFF)
            break;
        x1 = (s16)entity->posX.part16.integer;
        /* &gEwramData->bgInfo[1], loaded straight into the bg register */
        bg = (struct BgLayerState *)gEwramData;
        bg = (struct BgLayerState *)((u8 *)bg + 0xA094);
        x1 += (s16)bg->xPos.part16.integer;
        y1 = (s16)entity->posY.part16.integer;
        y1 += (s16)bg->yPos.part16.integer;
        Collision_ClearBlock(x1, y1 - 8);
        Collision_ClearBlock(x1, y1 - 0x18);
        y1 -= 0x28;
        Collision_ClearBlock(x1, y1);
        entity->state++;
        break;
    }
    case 2:
        SetPlayerInput(1, entity->flipX ? 0x10 : 0x20);
        break;
    case 10:
        Game_SetMenusEnabled(0);
        SetPlayerInput(1, entity->flipX ? 0x20 : 0x10);
        if (near)
            break;
        PlaySong(0x114);
        entity->state++;
        break;
    case 11:
    {
        s32 x2, y2;
        SetPlayerInput(1, 0);
        if ((entity->ext.gate.openAngle += -0x1A0) > 0)
            break;
        entity->ext.gate.openAngle = 0;
        gEwramData->gameState.stateFlags &= 0xFFFFFEFE;
        Game_SetMenusEnabled(1);
        x2 = (s16)entity->posX.part16.integer;
        bg = (struct BgLayerState *)gEwramData;
        bg = (struct BgLayerState *)((u8 *)bg + 0xA094);
        x2 += (s16)bg->xPos.part16.integer;
        y2 = (s16)entity->posY.part16.integer;
        y2 += (s16)bg->yPos.part16.integer;
        Collision_SetBlock(x2, y2 - 8);
        Collision_SetBlock(x2, y2 - 0x18);
        y2 -= 0x28;
        Collision_SetBlock(x2, y2);
        entity->ext.gate.openAngle = 0;
        entity->state = 0;
        break;
    }
    case 0x63:
        SetPlayerInput(1, 0);
        gEwramData->gameState.stateFlags &= 0xFFFFFEFE;
        entity->deleteRequested = 1;
        ObjAffine_Free((u8)entity->ext.gate.affine0);
        ObjAffine_Free(entity->ext.gate.affine1);
        break;
    }
}
