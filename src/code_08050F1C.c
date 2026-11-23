/* BossRushResult_Draw: matched (three block-local v instead of one variable set three times;
   literal constants, no helper variables). */
#include "code_08009A0.h"
#include "room_object_entity.h"
#include "code_0800B700.h"
#include "code/code_08039340.h"
#include "syscalls.h"
#include "structs/ewram.h"
#include "structs/entity_render_flags.h"

/**
 * @brief Draw function of the result object (0x10): call Game_SetMenusEnabled(0), keep
 * the player input set in state 3 and, unless hidden, draw the labels, the
 * record frame when the timer (entity 26) beats the stored best bestBossRushTimeFrames or
 * none is stored, and the time with hundredths, seconds and minutes.
 */
void BossRushResult_Draw(struct EwramData_EntityData *entity)
{
    struct GameState *g;
    struct EwramData_EntityData *t;
    u32 time;
    g = &gEwramData->gameState;
    t = &gEwramData->entityData[26];
    time = BOSS_RUSH_TIME(t);
    Game_SetMenusEnabled(0);
    if (entity->state == 3)
        SetPlayerInput(1, 0);
    if (*(ENTITY_RENDER_FLAGS_ADDRESS(entity)) & 0x20)
        return;
    entity->animFrame = 1;

    entity->posX.part16.integer = 0x78;
    entity->posY.part16.integer = 0x20;
    Sprite_Draw(entity);
    entity->animFrame = 0;
    entity->posY.part16.integer = 0x38;
    Sprite_Draw(entity);
    if (g->bestBossRushTimeFrames > time || g->bestBossRushTimeFrames == 0)
    {
        entity->animFrame = 2;
        entity->posY.part16.integer = 0x78;
        Sprite_Draw(entity);
    }
    entity->posY.part16.integer = 0x50;
    {
    s32 v = Div(Mod(time, 60) * 100, 60);
    entity->posX.part16.integer = 0x98;
    entity->animFrame = Mod(v, 10) + 3;
    Sprite_Draw(entity);
    entity->posX.part16.integer -= 8;
    entity->animFrame = Div(v, 10) + 3;
    Sprite_Draw(entity);

    entity->animFrame = 0xD;
    entity->posX.part16.integer -= 8;
    Sprite_Draw(entity);
    }
    {
    s32 v = Mod(Div(time, 60), 60);
    entity->posX.part16.integer = 0x80;
    entity->animFrame = Mod(v, 10) + 3;
    Sprite_Draw(entity);
    entity->posX.part16.integer -= 8;
    entity->animFrame = Div(v, 10) + 3;
    Sprite_Draw(entity);
    entity->animFrame = 0xD;
    entity->posX.part16.integer -= 8;
    Sprite_Draw(entity);
    }
    {
    s32 v = Div(time, 3600);
    entity->posX.part16.integer = 0x68;
    entity->animFrame = Mod(v, 10) + 3;
    Sprite_Draw(entity);
    entity->posX.part16.integer -= 8;
    entity->animFrame = Div(v, 10) + 3;
    Sprite_Draw(entity);
    }
}
