/* Legion_UpdatePreview. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u8 sLegionObject25Palette[];
extern u8 sLegionPaletteAnim1[];

/**
 * @brief A005C | Substate 0: PaletteAnim_Start with palette (s8)unk_504; then
 * advance the phase in velX by 0x200 and bob posY by sin(phase) / 16.
 */
void Legion_UpdatePreview(struct EwramData_EntityData *entity)
{
    if (entity->subState == 0)
    {
        PaletteAnim_Start(sLegionObject25Palette, sLegionPaletteAnim1, (s8)entity->unk_504.part8.unk_504);
        entity->subState++;
    }
    entity->velX.whole += 0x200;
    entity->posY.whole += Sin(entity->velX.whole) / 16;
}
