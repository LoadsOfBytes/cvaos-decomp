/* Progress through six timed palette transitions. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "event_scene30.h"
struct PaletteTransition { void *data; u16 count,duration; };
extern struct PaletteTransition sEventScene30PaletteTransitions[];
/**
 * @brief Advance the six-step BG/OBJ palette transition of sEventScene30PaletteTransitions by its
 * durations.
 */
void EventScene30_StepPaletteTransition(struct EwramData_EntityData *entity)
{
    u8 *index=&EVENT30_PALETTE_STEP(entity);
    struct PaletteTransition *step=&sEventScene30PaletteTransitions[*index];
    struct PaletteTransition *table=sEventScene30PaletteTransitions;
    if(*index<=5 && (s16)EVENT30_PALETTE_TIMER(entity)++>step->duration)
    {
        s32 zero=0;
        EVENT30_PALETTE_TIMER(entity)=zero;
        BgPalette_LoadRange(step->data,step->count,1,0);
        ObjPalette_LoadRange(step->data,step->count,1,14);
        if(++*index<=5)
        {
            s32 scale;
            void *data;
            s32 count;
            step=&table[*index];
            data=step->data;
            count=step->count;
            scale=0x10000;
            BgPalette_StartBlend(0,data,count,scale,zero,step->duration);
            ObjPalette_StartBlend(entity->basePalette,step->data,step->count,scale,zero,step->duration);
        }
    }
}
