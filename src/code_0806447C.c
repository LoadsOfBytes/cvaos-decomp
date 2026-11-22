/* Progress through six timed palette transitions. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
struct PaletteTransition { void *data; u16 count,duration; };
extern struct PaletteTransition sEventScene30PaletteTransitions[];
/**
 * @brief Advance the six-step BG/OBJ palette transition of sEventScene30PaletteTransitions by its
 * durations.
 */
void EventScene30_StepPaletteTransition(struct EwramData_EntityData *entity)
{
    u8 *index=(u8 *)&entity->unk_518.part16.hp;
    struct PaletteTransition *step=&sEventScene30PaletteTransitions[*index];
    struct PaletteTransition *table=sEventScene30PaletteTransitions;
    if(*index<=5 && (s16)entity->unk_518.part16.unk_51A++>step->duration)
    {
        s32 zero=0;
        entity->unk_518.part16.unk_51A=zero;
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
            ObjPalette_StartBlend(entity->unk_510.part8.basePalette,step->data,step->count,scale,zero,step->duration);
        }
    }
}
