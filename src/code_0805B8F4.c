/* EventScene30_Update, decompiled from the original assembly. The second random
 * number has its own block variable and its offset is summed in a statement
 * expression before the player Y is added, as in the target. */
#include "code_08009A0.h"
#include "event_scene30.h"
#include "code_080109F4.h"
#include "code_08011DD0.h"
#include "code_08013960.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "gba/dma.h"
#include "structs/ewram.h"
#include "structs/main.h"
extern s32 EventScene30_PlayTransformation(struct EwramData_EntityData *);
extern void TextWindow_Begin(s32);
extern s32 TextWindow_Update(void);
extern s32 Sin(s32);

extern void Player_StartScriptedAnimation(s32, s32, s32, s32);
extern struct RoomDefinition sEndingRoomDefinition;

/* Unresolved original return value: the assembly returns the incoming r9. */
/**
 * @brief Update of event scene variant 30 (sEventSceneUpdateFuncs[30]); returns the
 * player input mask.
 */
s32 EventScene30_Update(struct EwramData_EntityData *entity)
{
    s32 result;
    switch (entity->state) {
    case 0:
        if (entity->stage == 0) {
            entity->timer = 8;
            entity->stage++;
        }
        if ((u8)--entity->timer == 255) {
            entity->state++;
            entity->stage = 0;
            entity->subState = 0;
        }
        entity->state++;
        entity->stage = 0;
        entity->subState = 0;
        break;
    case 1:
        if (entity->stage == 0)
            entity->stage++;
        if (EventScene30_PlayTransformation(entity)) {
            EVENT30_FLOAT_PHASE(entity) = 0;
            gEwramData->playerState.player->hidden = 0;
            Music_SetCurrent(32);
            entity->state++;
            entity->stage = 0;
            entity->subState = 0;
            ObjPalette_LoadRange((void *)0x082099FC, 5, 1, 14);
        }
        break;
    case 2:
        if (entity->subState == 0) {
            gUnk_03002CB0.dispCnt &= 0xF1FF;
            entity->subState++;
            entity->stage = 0;
        } else if (entity->subState == 1) {
            DMA_FILL_16(3, 0xFFFF, 0x05000000, 32);
            /* The original DMA source is the literal address 0xFFFF. */
            DMA_COPY_16(3, 0xFFFF, &gEwramData->bgPalette, 32);
            TextWindow_Begin(37);
            entity->subState++;
            entity->stage = 0;
            entity->timer = 0;
        } else if (entity->subState == 2) {
            ObjPalette_StartColorFade(31, 31, 31, 0, 0x10000, 128);
            entity->subState++;
            entity->stage = 0;
        } else if (ObjPalette_GetPendingFades() == 0) {
            gUnk_03002CB0.dispCnt &= 0xF1FF;
            entity->state++;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    case 3:
        if (TextWindow_Update()) {
            entity->state++;
            entity->stage = 0;
            entity->subState = 0;
        }
        if (entity->subState == 1) {
            if (entity->stage == 0) {
                gEwramData->gameState.textWindowPause = 1;
                entity->timer = 64;
                entity->stage++;
            }
            if ((u8)--entity->timer == 255) {
                gEwramData->gameState.textWindowPause = 0;
                entity->subState++;
                entity->stage = 0;
            }
        }
        break;
    case 4:
        if (entity->subState == 0) {
            ObjPalette_StartColorFade(31, 31, 31, 0x10000, 0, 128);
            entity->subState++;
            entity->stage = 0;
            entity->timer = 32;
            Music_SubmitCommandOnce(0x1040);
        } else if (entity->subState == 1 && ObjPalette_GetPendingFades() == 0) {
            ObjPalette_StopBlend(14);
            BgPalette_StopBlend(0);
            gEwramData->startPauseEnabled = 0;
            gEwramData->gameState.stateFlags |= 0x800000;
            RoomJump_SetDestination(&sEndingRoomDefinition, 0, 0, 0, 0);
            RoomJump_Begin(&gEwramData->gameState);
            gDisplayRegisters.bldCnt = 0;
            entity->subState++;
            entity->stage = 0;
            gEwramData->gameState.stateFlags |= 0x4000;
        } else if (*(volatile u8 *)&entity->timer == 0 && entity->stage == 0) { /* FAKEMATCH: volatile read forces the reload of the original */
            Music_SubmitCommandOnce(0x1040);
            entity->stage++;
        }
        break;
    }
    if (entity->state > 1) {
        s32 sine = Sin((u16)EVENT30_FLOAT_PHASE(entity) + 0x4000);
        Player_StartScriptedAnimation(51, 0, 0, sine / 4);
        EVENT30_FLOAT_PHASE(entity) += 0x200;
        { u8 previous = entity->timer--;
        if ((previous % 16) == 0) {
            u32 random = RandomNumberGenerator();
            s32 x = gEwramData->playerState.player->posX.whole - 0x100000;
            s32 y;
            x += (random & 0x7FFFF) * 4;
            {
                u32 r2 = RandomNumberGenerator();
                y = gEwramData->playerState.player->posY.whole + ({ s32 t = (r2 & 0x7FFFF) * 2 - 0xC0000; t; });
            }
            Effect_CreateFromTable(x, y, 0, -0x2000, 7, 9);
        }
        }
    } else {
        struct EwramData_EntityData **player = &gEwramData->playerState.player;
        (*player)->posX.part16.integer = 120;
        (*player)->posY.part16.integer = 80;
        (*player)->velX.whole = 0;
        (*player)->velY.whole = 0;
        Player_StartScriptedAnimation(51, 0, 0, 0);
    }
    gEwramData->gameState.stateFlags |= 32;
    return result;
}
