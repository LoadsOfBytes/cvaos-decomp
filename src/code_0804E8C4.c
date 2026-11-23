#include "code_08009A0.h"
#include "code_08001194.h"
#include "code_08013960.h"
#include "code/code_08039340.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"
extern s32 Entity_IsInRect(struct EwramData_EntityData *, s16, s16, u16, u16);

#define BG &gEwramData->bgInfo[1]

static inline s32 IsBeside(struct EwramData_EntityData *entity)
{
    if (!entity->ext.door.onRight)
    {
        if ((s16)gEwramData->playerState.player->posX.part16.integer > (s16)entity->posX.part16.integer + 0x18)
            return 0;
    }
    else if ((s16)gEwramData->playerState.player->posX.part16.integer < (s16)entity->posX.part16.integer - 0x18)
        return 0;
    return 1;
}

/**
 * @brief 4E8C4 | Shutter door (object 0x04): hidden off screen. State 0
 * opens it while the player is within 0x28 px (tiles cleared with
 * Collision_ClearBlock, sound 0x113, counter unk_4F8 up to 0x30) and closes it again
 * (Collision_SetBlock) when the player has left; states 10/11 close it behind the
 * player and go to 0x63 / 0x14 (unk_514); state 20 waits for its boss flag
 * (bit unk_516 of bossEventFlags, map reveal 0x1040) and state 21 opens it for good.
 */
void Object04Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *player = gEwramData->playerState.player;

    entity->hidden = Entity_IsInRect(entity, -0x20, 0, 0x130, 0xD0) ? 0 : 1;
    switch (entity->state)
    {
    case 0:
    {
        s32 ex, x, y, t;

        ex = (s16)entity->posX.part16.integer;
        if ((u32)(ex - (s16)player->posX.part16.integer + 0x28) <= 0x4F)
        {
            struct BgLayerState *bg = BG;
            x = (s16)bg->xPos.part16.integer;
            x = ex - -x; /* "ex + x" with ex as the first operand */
            y = (s16)entity->posY.part16.integer;
            y += (s16)bg->yPos.part16.integer;
            Collision_ClearBlock(x, y - 8);
            Collision_ClearBlock(x, y - 0x18);
            y -= 0x28;
            Collision_ClearBlock(x, y);
            if (entity->ext.door.openHeight == 0 && Entity_IsInRect(entity, -0x10, 0, 0x110, 0xC0))
                PlaySong(0x113);
            if ((entity->ext.door.openHeight += 2) > 0x2F)
                entity->ext.door.openHeight = 0x30;
            break;
        }
        t = entity->ext.door.openHeight;
        if (t <= 2)
        {
            struct BgLayerState *bg2 = BG;
            x = ex + (s16)bg2->xPos.part16.integer;
            y = (s16)entity->posY.part16.integer + (s16)bg2->yPos.part16.integer;
            if (t != 0)
            {
                Collision_SetBlock(x, y - 8);
                Collision_SetBlock(x, y - 0x18);
                Collision_SetBlock(x, y - 0x28);
            }
            entity->ext.door.openHeight = 0;
            break;
        }
        if (t == 0x30 && Entity_IsInRect(entity, -0x10, 0, 0x110, 0xC0))
            PlaySong(0x113);
        entity->ext.door.openHeight -= 2;
        break;
    }
    case 21:
    {
        struct BgLayerState *bg;
        s32 x, y;

        if (++entity->ext.door.openHeight <= 0x2F)
            break;
        x = (s16)entity->posX.part16.integer;
        bg = BG;
        x += (s16)bg->xPos.part16.integer;
        y = (s16)entity->posY.part16.integer;
        y += (s16)bg->yPos.part16.integer;
        Collision_ClearBlock(x, y - 8);
        Collision_ClearBlock(x, y - 0x18);
        y -= 0x28;
        Collision_ClearBlock(x, y);
        entity->state++;
        break;
    }
    case 2:
        SetPlayerInput(1, entity->ext.door.onRight ? 0x10 : 0x20);
        break;
    case 10:
    {
        struct BgLayerState *bg;
        s32 x, y;

        SetPlayerInput(1, entity->ext.door.onRight ? 0x20 : 0x10);
        if (IsBeside(entity))
            break;
        x = (s16)entity->posX.part16.integer;
        bg = BG;
        x += (s16)bg->xPos.part16.integer;
        y = (s16)entity->posY.part16.integer;
        y += (s16)bg->yPos.part16.integer;
        Collision_SetBlock(x, y - 8);
        Collision_SetBlock(x, y - 0x18);
        y -= 0x28;
        Collision_SetBlock(x, y);
        PlaySong(0x113);
        entity->state++;
        break;
    }
    case 11:
        SetPlayerInput(1, 0);
        if (--entity->ext.door.openHeight != 0)
            break;
        gEwramData->gameState.stateFlags &= 0xFFFFFEFE;
        if (entity->unk_514.part16.spawnParam0 > 1)
            entity->state = 0x63;
        else
            entity->state = 0x14;
        break;
    case 20:
        if ((gEwramData->gameState.bossEventFlags >> entity->unk_514.part16.spawnParam1) & 1)
        {
            if (!(gEwramData->gameState.stateFlags & 0x400000))
                Music_SubmitCommandOnce(0x1040);
            PlaySong(0x113);
            entity->state++;
        }
        break;
    }
}
