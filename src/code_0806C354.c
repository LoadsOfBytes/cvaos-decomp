/* Reset entity-related state and initialize the small allocation arena. */
#include "structs/ewram.h"
#include "gba/dma.h"
#include "code_08013960.h"
#include "code_08033CAC.h"
#include "code_08011DD0.h"
#include "code_0800CB00.h"
#include "code/code_08039340.h"
#include "code/code_080211F0.h"
#include "code/code_08060B98.h"

struct EntityHeapBlock {
    u16 tag;
    u16 words;
};

/**
 * @brief Per-room reset: clear tracked sounds and the music override, stop
 * finished colour fades, clear the selected target, solid object and
 * shared palettes, reset the entity heap, screen shake and message
 * window.
 */
void Room_ResetTransientState(void)
{
    struct EntityHeapBlock *heap;
    s32 zero;
    gEwramData->gameState.conveyorSpeed = -0x8000;
    gEwramData->gameState.cachedPickupIcon = 0xFF;
    gEwramData->gameState.stateFlags &= ~0x440800;
    Sound_ClearTracked();
    Music_StopOverride();
    if (!BgPalette_GetPendingFades())
        BgPalette_StopColorFade();
    if (!ObjPalette_GetPendingFades())
        ObjPalette_StopColorFade();
    Player_SetSelectedTarget(NULL);
    Player_ClearSolidObject();
    SharedObjPalette_ResetSlots();
    heap = (struct EntityHeapBlock *)gEwramData->entityScratchBuffer;
    zero = 0;
    DMA_FILL_32(3, zero, heap, 0x800);
    heap->tag = zero;
    heap->words = 511;
    ScreenShake_Clear();
    Hud_CloseMessageWindow();
}
