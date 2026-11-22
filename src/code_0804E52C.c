#include "code_08009A0.h"
#include "code_08001194.h"
#include "code/code_08039340.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"
extern s32 Entity_IsInRect(struct EwramData_EntityData *, s16, s16, u16, u16);

/**
 * @brief 4E52C | Door object (object 0x03): hidden off screen; in state 0
 * it walks the player towards it (SetPlayerInput) when the event flag
 * linkPlay is set and the player (and both followers linkPartner / linkTargetProxy)
 * stand within 0x13 px, then waits 4 frames (state 1) and opens (state 2):
 * clears the door tiles (Collision_ClearBlock at three heights, sound 0x113), clears
 * stateFlags bits 0 and 8 after 48 frames and keeps the player walking (state 3).
 */
void Object03Update(struct EwramData_EntityData *entity)
{
    struct EwramData *ew;
    struct EwramData_EntityData *p2;
    struct BgLayerState *bg;
    s32 x;
    s32 y;

    entity->hidden = Entity_IsInRect(entity, -0x20, 0, 0x130, 0xD0) ? 0 : 1;
    switch (entity->state)
    {
    case 0:
        if (gEwramData->playerState.selectedTarget)
            break;
        if (!gEwramData->gameState.linkPlay)
            goto open;
        if ((u32)((s16)entity->posX.part16.integer - (s16)gEwramData->playerState.player->posX.part16.integer + 0x18) <= 0x2F)
            SetPlayerInput(1, entity->flipX ? 0x20 : 0x10);
        if ((u32)((s16)entity->posX.part16.integer - (s16)gEwramData->playerState.player->posX.part16.integer + 0x13) > 0x26)
        {
            entity->timer = 0;
            break;
        }
        ew = gEwramData;
        if (!ew->gameState.linkPlay)
            break;
        p2 = ew->playerState.linkPartner;
        if ((u32)((s16)entity->posX.part16.integer - (s16)p2->posX.part16.integer + 0x13) > 0x26)
            break;
        if ((s16)ew->playerState.linkTargetProxy->posX.part16.integer != (s16)p2->posX.part16.integer)
            break;
        entity->timer = 4;
        SetPlayerInput(1, entity->flipX ? 0x20 : 0x10);
        entity->state = 1;
        break;
    case 1:
        SetPlayerInput(1, entity->flipX ? 0x20 : 0x10);
        if (entity->timer)
        {
            entity->timer--;
            break;
        }
    open:
        entity->state = 2;
        break;
    case 2:
        if (gEwramData->gameState.linkPlay)
            SetPlayerInput(1, entity->flipX ? 0x20 : 0x10);
        if (entity->unk_4F8.whole == 0)
        {
            x = (s16)entity->posX.part16.integer;
            bg = &gEwramData->bgInfo[1];
            x += (s16)bg->xPos.part16.integer;
            y = (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer;
            Collision_ClearBlock(x, y - 8);
            Collision_ClearBlock(x, y - 0x18);
            Collision_ClearBlock(x, y - 0x28);
            PlaySong(0x113);
        }
        if (++entity->unk_4F8.whole > 0x2F)
        {
            gEwramData->gameState.stateFlags &= 0xFFFFFEFE;
            if (gEwramData->gameState.linkPlay)
                entity->state++;
        }
        break;
    case 3:
        SetPlayerInput(1, entity->flipX ? 0x20 : 0x10);
        break;
    }
}
