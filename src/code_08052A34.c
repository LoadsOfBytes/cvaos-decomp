/* Object 15 interaction: capture player input, animate, show one of two
 * messages, switch UI state and restore the palette transition. */
#include "code_08009A0.h"
#include "room_object_entity.h"
#include "code_0800B700.h"
#include "code_080109F4.h"
#include "code_08011DD0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
extern s32 Player_IsBusy(void);
extern u8 sObject15DestinationRoom1[], sObject15DestinationRoom0[];
static inline struct EwramData_EntityData *Player(struct EwramData *e) { return e->playerState.player; }
void Object15Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
    case 1:
        if (!Entity_IsInRect(gEwramData->playerState.player, 0x60, 0x7F, 0x20, 0x10)) break;
        if (!(gEwramData->inputData.playerHeldInput & 0x40)) break;
        if (Player_IsBusy()) break;
        entity->state = 2;
        gEwramData->gameState.stateFlags |= 1;
        Game_SetMenusEnabled(0);
        SetPlayerInput(1, 0);
        break;
    case 2:
        if (OBJECT15_RADIUS(entity) > 0x90000) SetPlayerInput(1, 0);
        else SetPlayerInput(1, gEwramData->playerStatus.jumpButton);
        OBJECT15_ANGLE(entity) += 0x40;
        if (OBJECT15_RADIUS(entity) != 0)
        {
            if (OBJECT15_RADIUS(entity) == 0x100000)
            {
                BgPalette_StartColorFade(31, 7, 7, 0x10000, 0, 64);
                ObjPalette_StartColorFade(31, 7, 7, 0x10000, 0, 64);
            }
            OBJECT15_RADIUS(entity) -= 0x4000;
        }
        else entity->state = 3;
        break;
    case 3:
        SetPlayerInput(1, gEwramData->playerStatus.jumpButton);
        if (entity->unk_514.part16.spawnParam0 == 0) RoomJump_SetDestination(sObject15DestinationRoom1, 0, 0, 0x78, 0x50);
        else RoomJump_SetDestination(sObject15DestinationRoom0, 0, 0, 0x78, 0x50);
        RoomJump_Begin(&gEwramData->gameState);
        gEwramData->gameState.stateFlags &= ~1;
        BgPalette_StartColorFade(31, 31, 31, 0, 0x10000, 8);
        ObjPalette_StartColorFade(31, 31, 31, 0, 0x10000, 8);
        break;
    case 4:
    {
        s32 input, playerX, entityX;
        Game_SetMenusEnabled(0);
        playerX = Player(gEwramData)->posX.whole;
        entityX = entity->posX.whole;
        input = 0x20;
        if (entityX > playerX) input = 0x10;
        SetPlayerInput(1, input);
        if (Entity_IsInRect(Player(gEwramData), 0x70, 0x7F, 0x10, 0x10)) entity->state = 2;
        break;
    }
    }
}
