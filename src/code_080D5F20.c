/* Five cumulative timer thresholds select one of two palette sequences. */
#include <string.h>
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern u8 sGrahamPartSteps[],sGrahamPalette2[];
static inline s32 AfterThreshold(s32 index) { return index+1; }
/**
 * @brief Use five cumulative timer thresholds to load one of two palette
 * sequences selected by kind. Advance the substate and return nonzero
 * after the final step; preserve the byte timer comparison and indexing.
 */
s32 GrahamPart_UpdatePaletteSequence(struct EwramData_EntityData *entity,s32 kind)
{
    u8 steps[15];
    s32 finished=0;
    s32 timer;
    s32 index;
    memcpy(steps,sGrahamPartSteps,15);
    timer=entity->timer++;
    index=entity->subState*3;
    if((u8)timer>=steps[index])
    {
        s32 next;
        ObjPalette_LoadRange(sGrahamPalette2,steps[kind+AfterThreshold(index)],1,entity->palette);
        next=entity->subState+1;
        if(next<=4)
            entity->subState=next;
        else
            finished=1;
    }
    return finished;
}
