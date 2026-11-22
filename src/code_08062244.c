/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   EventScene22_Init = EventScene13_Init */
#include "structs/ewram.h"

/**
 * @brief Init of event scene variant 22 (Object 0x20 unk_514 = 22,
 * sEventSceneCreateFuncs[22]). Same as EventScene13_Init.
 */
void EventScene22_Init(void)
{
    gEwramData->gameState.stateFlags |= 1;
}
