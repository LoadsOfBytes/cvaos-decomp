#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern s32 BlueSoul_ShouldEnd(struct EwramData_EntityData *entity);
extern void BlueSoul_ReleaseActiveResources(struct EwramData_EntityData *entity);

#define PLAYER (gEwramData->playerState.player)

/**
 * @brief 2EB84 | Giant Bat blue soul (bat transformation; ends via
 * BlueSoul_ShouldEnd, or BlueSoul_ReleaseActiveResources while the player has unk_4F4 & 0x10001A0):
 * on start clamps the player's speed to +-0x40000, stops its acceleration,
 * song 0x8F; for 16 frames spawns Effect_CreateFromTable bats (type 6) at random
 * angles 16 px above the player. Keeps the player in state 8, soulStateFlags
 * bit 9 and actionLockFlags bits 0-1.
 */
void SkillGiantBatUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *player;
    struct EwramData_EntityData *bat;
    s32 a, s, c, vx, vy;

    if (BlueSoul_ShouldEnd(entity))
        goto delete;
    if (PLAYER->unk_4F4.whole & 0x010001A0)
    {
        BlueSoul_ReleaseActiveResources(entity);
    delete:
        EntityDelete(entity);
        return;
    }
    if (entity->state == 0)
    {
        if (PLAYER->velX.whole > 0x40000)
            PLAYER->velX.whole = 0x40000;
        else if (PLAYER->velX.whole < -0x40000)
            PLAYER->velX.whole = -0x40000;
        if (PLAYER->velY.whole > 0x40000)
            PLAYER->velY.whole = 0x40000;
        else if (PLAYER->velY.whole < -0x40000)
            PLAYER->velY.whole = -0x40000;
        PLAYER->accelX.whole = 0;
        PLAYER->accelY.whole = 0;
        PLAYER->unk_4F4.whole &= ~0x20;
        PlaySong(0x8F);
        entity->state = 1;
        PLAYER->subState = 0;
        entity->timer = 0x10;
    }
    if (entity->timer != 0)
    {
        entity->timer--;
        player = PLAYER;
        a = (RandomNumberGenerator() & 0xFF) << 8;
        s = Sin(a + 0x4000);
        vx = s * 2;
        c = Sin(a);
        vy = c * 2;
        {
            s32 x = player->posX.whole + s * 4;
            s32 y = player->posY.whole - 0x100000;
            y += c * 4;
            bat = Effect_CreateFromTable(x, y, vx / 16, vy / 16, 4, 6);
        }
        if (bat != NULL)
        {
            bat->velX.whole = vx;
            bat->velY.whole = vy;
        }
    }
    PLAYER->state = 8;
    gEwramData->playerStatus.soulStateFlags |= 0x200;
    gEwramData->playerState.actionLockFlags |= 3;
}
