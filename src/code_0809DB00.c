#include "code_08009A0.h"
#include "code_0800CB00.h"
#include "code_08033CAC.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 Sin(s32);
extern u8 sLegionObject25Palette[], sLegionPaletteAnim0[], sLegionPaletteAnim1[];

#define STAGE entity->ext.legion.introStage
#define POSE entity->ext.legion.introStep
#define CYCLE entity->ext.legion.introArrived

/**
 * @brief Intro sequence run by Legion_UpdateCircle before the fight; returns
 * nonzero when it has finished.
 */
s32 Legion_UpdateIntro(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    u8 state = STAGE;
    struct EwramData **ew = &gEwramData;
    switch (state) {
    case 0:
        if ((u8)entity->timer++ > 7) {
            STAGE++;
            entity->timer = 0;
        }
        break;
    case 1: {
        u32 *flags = &(*ew)->gameState.stateFlags;
        u32 value = *flags;
        s32 one = 1;
        if (!(value & one)) {
            STAGE++;
            *flags |= one;
            (*ew)->startPauseEnabled = 0;
            (*ew)->gameState.stateFlags |= 2;
            Player_SetSelectedTarget(entity);
        }
        break;
    }
    case 2:
        if ((*ew)->playerState.player->posX.whole + bg->xPos.whole > 0xFFFFFF) {
            STAGE++;
            entity->timer = 0;
            SetPlayerInput(2, 0);
            return 0;
        }
        SetPlayerInput(2, 0x10);
        break;
    case 3:
        SetPlayerInput(2, 0);
        if ((u8)entity->timer++ > 9) {
            s32 zero = 0;
            entity->timer = zero;
            STAGE++;
            entity->stage = zero;
            gEwramData->gameState.cameraScrollMinPriority = 1;
            CYCLE = zero;
        }
        break;
    case 4:
        Camera_ScrollWithPriority(1, 0, -0x8000);
        SetPlayerInput(2, 0);
        if (entity->stage == 0 && (s16)bg->yPos.part16.integer <= 0x120) {
            entity->stage++;
            POSE = 1;
        }
        if (entity->stage == 1 && (s16)bg->yPos.part16.integer <= 0x118) {
            entity->stage++;
            POSE = 2;
        }
        if (entity->stage == 2 && (s16)bg->yPos.part16.integer <= 0x110) {
            entity->stage++;
            POSE = 3;
        }
        if (entity->stage == 3 && (s16)bg->yPos.part16.integer <= 0x10C) {
            entity->stage++;
            POSE = 4;
        }
        if (entity->stage == 4 && (s16)bg->yPos.part16.integer <= 0x108) {
            entity->stage++;
            POSE = 5;
        }
        if (entity->stage == 5 && (u8)CYCLE > 4) {
            PaletteAnim_Start(sLegionObject25Palette, sLegionPaletteAnim0, (s8)entity->ext.legion.animPalette);
            entity->stage++;
            entity->timer = 0;
        }
        entity->timer++;
        if (bg->yPos.whole <= 0xC00000) {
            s32 zero;
            STAGE++;
            zero = 0;
            entity->stage = zero;
            POSE = 6;
            CYCLE = zero;
        }
        break;
    case 5:
        Camera_ScrollWithPriority(1, 0, 0);
        SetPlayerInput(2, 0);
        if ((u8)CYCLE != 0) {
            s32 zero;
            STAGE++;
            zero = 0;
            entity->timer = zero;
            entity->stage = zero;
            POSE = 7;
            CYCLE = zero;
        }
        break;
    case 6:
        Camera_ScrollWithPriority(1, 0, 0);
        SetPlayerInput(2, 0);
        if ((u8)entity->timer++ > 29) {
            STAGE++;
            entity->timer = 0;
        }
        break;
    case 7:
        Camera_ScrollWithPriority(1, 0, 0);
        SetPlayerInput(2, 0);
        if ((u8)CYCLE != 0) {
            STAGE++;
            entity->timer = 0;
            PaletteAnim_Start(sLegionObject25Palette, sLegionPaletteAnim1, (s8)entity->ext.legion.animPalette);
        }
        break;
    case 8:
        Camera_ScrollWithPriority(1, 0, 0);
        SetPlayerInput(2, 0);
        if ((u8)entity->timer++ > 19) {
            STAGE++;
            entity->timer = 0;
        }
        break;
    case 9:
        SetPlayerInput(2, 0);
        if (bg->yPos.whole > 0x12FFFFF) {
            s32 zero;
            STAGE++;
            zero = 0;
            entity->timer = zero;
            gEwramData->gameState.stateFlags &= ~1;
            gEwramData->startPauseEnabled = 1;
            gEwramData->gameState.cameraScrollMinPriority = zero;
            return 1;
        }
        Camera_ScrollWithPriority(1, 0, 0x10000);
        break;
    }
    entity->velX.whole += 0x200;
    entity->posY.whole += Sin(entity->velX.whole) / 16;
    return 0;
}
