#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void BgPalette_StartColorFade(u16 r, u16 g, u16 b, s32 from, s32 to, s32 frames);
extern void ObjPalette_StartColorFade(u16 r, u16 g, u16 b, s32 from, s32 to, s32 frames);
extern void Chaos_StartPaletteSequence(struct EwramData_EntityData *, void *);
extern void Chaos2_StepPaletteSequence(struct EwramData_EntityData *);
extern void Chaos2_CreatePart(struct EwramData_EntityData *core, s32 x, s32 y, s32 mirror);
extern void BgPalette_StopColorFade(void);
extern void ObjPalette_StopColorFade(void);
extern s32 ObjPalette_GetPendingFades(void);
extern void Chaos2_CreateSnake(struct EwramData_EntityData *, s32, s32, s32);
extern u8 sChaos2PaletteSequence1[];

/**
 * @brief A4728 | Chaos intro sequence (subState, player input frozen):
 * fades the screen (BgPalette_StartColorFade / ObjPalette_StartColorFade), plays the sequence
 * sChaos2PaletteSequence1 while Chaos2_StepPaletteSequence animates the core for 60 + 120 frames,
 * then fades back in, places the four parts (Chaos2_CreatePart) and after 40
 * frames ends the fades, releases the input (stateFlags bit 0), sets
 * startPauseEnabled and starts the fight (state 1, Chaos2_CreateSnake). Step 10 does
 * the same once the fade ObjPalette_GetPendingFades has finished.
 */
void Chaos2_UpdateIntro(struct EwramData_EntityData *core)
{
    SetPlayerInput(2, 0);
    switch (core->subState)
    {
    case 0:
        core->subState++;
        BgPalette_StartColorFade(0, 0, 0, 0x10000, 0, 1);
        ObjPalette_StartColorFade(0, 0, 0, 0, 0x10000, 0x3C);
        core->stage = 0;
        gEwramData->startPauseEnabled = 0;
        core->timer = 0;
        Chaos_StartPaletteSequence(core, sChaos2PaletteSequence1);
        gEwramData->gameState.stateFlags |= 0x20;
        break;
    case 1:
        gEwramData->startPauseEnabled = 0;
        BgPalette_StartColorFade(0, 0, 0, 0x10000, 0, 1);
        Chaos2_StepPaletteSequence(core);
        if (core->timer++ > 0x3B)
        {
            core->subState++;
            core->timer = 0;
        }
        break;
    case 2:
        BgPalette_StartColorFade(0, 0, 0, 0x10000, 0, 1);
        Chaos2_StepPaletteSequence(core);
        if (core->timer++ > 0x77)
        {
            core->subState++;
            core->timer = 0;
            ObjPalette_StartColorFade(0, 0, 0, 0x10000, 0, 0x28);
        }
        break;
    case 3:
        BgPalette_StartColorFade(0, 0, 0, 0x10000, 0, 1);
        if (core->timer++ > 0x27)
        {
            core->subState++;
            core->timer = 0;
            BgPalette_StartColorFade(0, 0, 0, 0, 0x10000, 0x28);
            ObjPalette_StartColorFade(0, 0, 0, 0, 0x10000, 0x28);
            Chaos2_CreatePart(core, 0x180000, 0x280000, 0);
            Chaos2_CreatePart(core, 0xD80000, 0x280000, 1);
            Chaos2_CreatePart(core, 0x180000, 0x780000, 2);
            Chaos2_CreatePart(core, 0xD80000, 0x780000, 3);
            gEwramData->gameState.stateFlags &= ~0x20;
        }
        break;
    case 4:
        if (core->timer++ > 0x27)
        {
            BgPalette_StopColorFade();
            ObjPalette_StopColorFade();
            core->timer = 0;
            core->state = 1;
            core->subState = 0;
            core->stage = 0;
            gEwramData->gameState.stateFlags &= ~1;
            gEwramData->startPauseEnabled = 1;
            Chaos2_CreateSnake(core, 0, 0, 0);
        }
        break;
    case 10:
        if (ObjPalette_GetPendingFades() == 0)
        {
            BgPalette_StopColorFade();
            ObjPalette_StopColorFade();
            core->state = 1;
            core->subState = 0;
            core->stage = 0;
            gEwramData->gameState.stateFlags &= ~1;
            gEwramData->startPauseEnabled = 1;
            Chaos2_CreateSnake(core, 0, 0, 0);
        }
        break;
    }
}
