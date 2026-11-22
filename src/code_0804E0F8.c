#include "code_08009A0.h"
#include "code_08001194.h"
#include "code_08013960.h"
#include "code/code_08039340.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"
#include "structs/main.h"
#include "code/code_08040A38.h"
extern s32 RandomSigned(s32 range);
extern s32 Entity_IsInRect(struct EwramData_EntityData *, s16, s16, u16, u16);

#define BG &gEwramData->bgInfo[1]

/**
 * @brief 4E0F8 | Shutter door (object 0x02): like Object04Update, but
 * variant 7 throws sparks (Effect_SpawnShared) every fourth frame while closed,
 * state 10 closes once the player is out of the 0x38 x 0x22 box and state 11
 * also clears bit 15 of DISPCNT.
 */
void Object02Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *player = gEwramData->playerState.player;

    entity->hidden = Entity_IsInRect(entity, -0x20, 0, 0x130, 0xD0) ? 0 : 1;
    switch (entity->state)
    {
    case 0:
    {
        s32 ex, x, y, t;

        if (entity->unk_514.part16.spawnParam1 == 7 && entity->unk_4F8.whole == 0)
        {
            entity->timer++;
            if (!(entity->timer & 3))
            {
                s32 sx = entity->posX.whole + RandomSigned(0x80000);
                Effect_SpawnShared(sx, entity->posY.whole + ((RandomNumberGenerator() % 0x30 - 0x30) << 16), 0x10, 9);
            }
        }
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
            if (entity->unk_4F8.whole == 0 && Entity_IsInRect(entity, -0x10, 0, 0x110, 0xC0))
                PlaySong(0x113);
            if ((entity->unk_4F8.whole += 2) > 0x2F)
                entity->unk_4F8.whole = 0x30;
            break;
        }
        t = entity->unk_4F8.whole;
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
            entity->unk_4F8.whole = 0;
            break;
        }
        if (t == 0x30 && Entity_IsInRect(entity, -0x10, 0, 0x110, 0xC0))
            PlaySong(0x113);
        entity->unk_4F8.whole -= 2;
        break;
    }
    case 21:
    {
        struct BgLayerState *bg;
        s32 x, y;

        if (++entity->unk_4F8.whole <= 0x2F)
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
        SetPlayerInput(1, entity->unk_4FC.part8.unk_4FE ? 0x10 : 0x20);
        break;
    case 10:
    {
        struct BgLayerState *bg;
        s32 x, y;

        SetPlayerInput(1, entity->unk_4FC.part8.unk_4FE ? 0x20 : 0x10);
        {
            struct EwramData_EntityData *player = gEwramData->playerState.player;
            if (Entity_IsInRect(entity, player->posX.part16.integer - 0x1C, player->posY.part16.integer, 0x38, 0x22))
                break;
        }
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
        if (--entity->unk_4F8.whole != 0)
            break;
        gEwramData->gameState.stateFlags &= 0xFFFFFEFE;
        gUnk_03002CB0.dispCnt &= 0x7FFF;
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
