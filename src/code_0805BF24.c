#include "structs/ewram.h"

extern void TextWindow_Begin(s32 textId);

/**
 * @brief Init of event scene variant 0 (Object 0x20 unk_514 = 0,
 * sEventSceneCreateFuncs[0]).
 */
void EventScene00_Init(void)
{
    gEwramData->gameState.stateFlags |= 1;
    TextWindow_Begin(0xC);
}

/**
 * @brief Init of event scene variant 1 (Object 0x20 unk_514 = 1,
 * sEventSceneCreateFuncs[1]).
 */
void EventScene01_Init(void)
{
}
