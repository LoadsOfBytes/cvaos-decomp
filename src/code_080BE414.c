#include "code_08009A0.h"
#include "death_entity.h"
#include "code_0800B700.h"
#include "code_08013960.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"
#include "syscalls.h"

extern void Afterimage_Create(struct EwramData_EntityData *, s32, s32, s32);
extern s32 Entity_AccelerateAlongAngle(struct EwramData_EntityData *entity, u16 angle, s32 speed, s32 target, s16 acceleration, u16 time);

/* Move towards (tx, ty) (16.16) at a speed proportional to the distance;
 * velX/velY get the step taken. Returns the speed. */
static inline s32 MoveToward(struct EwramData_EntityData *entity, s32 tx, s32 ty)
{
    s32 x = entity->posX.whole;
    s32 y = entity->posY.whole;
    u32 dist = Sqrt(((x - tx) >> 16) * ((x - tx) >> 16) + ((y - ty) >> 16) * ((y - ty) >> 16)) << 16;
    u16 angle = ArcTan2(((tx < 0) ? -(-tx >> 16) : (tx >> 16)) - (s16)entity->posX.part16.integer,
                        ((ty < 0) ? -(-ty >> 16) : (ty >> 16)) - entity->posY.part16.integer);
    dist >>= 4;
    Entity_AccelerateAlongAngle(entity, angle, dist, 0, 0, 0);
    entity->velX.whole = entity->posX.whole - x;
    entity->velY.whole = entity->posY.whole - y;
    return dist;
}

/* Cutscene step: wait until the player walks left of x 0xC0 on screen, then
 * spawn two effects and pull the entity towards a point above the player. */
/**
 * @brief Intro state (0): gate menus and supply player input, wait for
 * the room-position threshold, signal introductory children, then move
 * above the player and enter the first phase.
 */
void Death_UpdateIntro(struct EwramData_EntityData *entity)
{
    struct EwramData *ew;
    switch (entity->subState)
    {
    case 0:
        if (gEwramData->gameState.stateFlags & 1)
            break;
        entity->subState = 1;
    case 1:
        gEwramData->gameState.stateFlags |= 1;
        Game_SetMenusEnabled(0);
        if ((s16)(ew = gEwramData)->playerState.player->posX.part16.integer + (s16)((struct BgLayerState *)((u8 *)ew + 0xA094))->xPos.part16.integer > 0xC0)
            break;
        DEATH_FLAGS(entity) |= 0x20;
        DEATH_WEAPON_TIMER(entity) = 0x60;
        entity->subState = 2;
        break;
    case 2:
        if (DEATH_WEAPON_TIMER(entity) != 0)
        {
            DEATH_WEAPON_TIMER(entity)--;
            break;
        }
        DEATH_WEAPON_TIMER(entity) = 0x30;
        entity->subState = 3;
        Afterimage_Create(entity, 0, 0x10, 0x31);
        Afterimage_Create(entity, 0, 8, 0x30);
        PlaySong(0x121);
        Music_SetCurrent(0x10);
        gEwramData->gameState.stateFlags &= ~1;
        Game_SetMenusEnabled(1);
        break;
    case 3:
    {
        MoveToward(entity, entity->posX.whole, gEwramData->playerState.player->posY.whole - 0x280000);
        if (DEATH_WEAPON_TIMER(entity) != 0)
        {
            DEATH_WEAPON_TIMER(entity)--;
            break;
        }
        entity->state = 1;
        entity->stage = 0;
        entity->subState = 0;
        break;
    }
    }
    if ((u8)(entity->subState - 1) <= 1)
    {
        s32 input = 0;
        if ((s16)(ew = gEwramData)->playerState.player->posX.part16.integer + (s16)((struct BgLayerState *)((u8 *)ew + 0xA094))->xPos.part16.integer > 0x7F)
            input = 0x20;
        SetPlayerInput(2, input);
    }
}
