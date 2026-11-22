/* Draw eleven packed glyph entries, skipping empty entry 0x0C. */
#include "code/code_08039340.h"
static inline u8 *Glyph(u8 *base, s32 index) { return base + index; }
/**
 * @brief Draw nonblank entries from eleven packed glyph slots at five-pixel spacing, then restore X.
 */
void NumberIndicator_Draw(struct EwramData_EntityData *entity)
{
    s32 savedX = entity->posX.whole;
    s32 i;
    u8 *glyph;
    for (i = 0; i <= 10; i++)
    {
        s32 index = i;
        index %= 4;
        index += ((i >> 2) + 2) * 4;
        glyph = Glyph((u8 *)&entity->unk_4F4, index);
        entity->animFrame = *glyph;
        if (*glyph != 0xC)
        {
            Sprite_Draw(entity);
            entity->posX.whole += 0x50000;
        }
    }
    entity->posX.whole = savedX;
}
