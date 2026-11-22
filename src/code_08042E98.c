#include "structs/ewram.h"
#include "code/code_08039340.h"

/* Center and draw eleven entries, skipping value 12.
 * Retain the signed remainder/row arithmetic and short register lifetimes:
 * they reproduce agbcc's allocation and address calculation. The output
 * pointer is formed before the first loop for the same reason. A match
 * does not establish the original source's spelling or organization. */
/**
 * @brief When flag 0x20 is clear, center and draw nonblank entries from eleven packed glyph slots at five-pixel spacing, then restore X.
 */
void NumberIndicator_DrawCentered(struct EwramData_EntityData *entity)
{
    s32 savedX, i;

    if (!(gEwramData->gameState.stateFlags & 0x20))
    {
        u8 *entries;
        u8 *output;
        savedX = entity->posX.whole;
        i = 0;
        entries = (u8 *)&entity->unk_4F4;
        output = &entity->animFrame;
        {
            u8 *base = entries;
            s32 delta = -0x28000;
            for (; i < 11; i++)
            {
                register s32 remainder asm("r0") = i % 4; /* FAKEMATCH: pinned register */
                s32 row = (i >> 2) + 2;
                remainder += row * 4;
                if (*(base + remainder) != 12)
                    entity->posX.whole += delta;
            }
        }
        for (i = 0; i < 11; i++)
        {
            register s32 remainder asm("r1") = i % 4; /* FAKEMATCH: pinned register */
            s32 row = (i >> 2) + 2;
            u8 *entry = &entries[remainder + row * 4];
            *output = *entry;
            if (*entry != 12)
            {
                Sprite_Draw(entity);
                entity->posX.whole += 0x50000;
            }
        }
        entity->posX.whole = savedX;
    }
}

