/* Format a signed value in eleven digit slots, with leading blanks.
 * Frame 10 is minus, 11 is the kind-1 prefix, and 12 is blank. */
#include "code_08009A0.h"
#include "structs/ewram.h"

/*
 * Glyph slots of the number indicator: 11 bytes from unk_4FC (slot 0 =
 * sign / prefix frame, slot 1 = first digit, see NumberSlot) and the number of
 * zero-padded trailing digits (unk_508 byte).
 */
#define NUMBER_PREFIX(e) e->unk_4FC.part8.unk_4FC
#define NUMBER_FIRST_DIGIT(e) e->unk_4FC.part8.unk_4FD
#define NUMBER_PAD_DIGITS(e) e->unk_508.part8.unk_508
/* Preserve the lane, base and group temporaries: folding the fixed offsets
 * into one displacement changes agbcc's addressing and register allocation. */
static inline u8 *NumberSlot(struct EwramData_EntityData *entity, u8 i)
{
    s32 lane = i & 3;
    u8 *base = (u8 *)&entity->unk_4F4;
    s32 group = (i >> 2) + 2;
    s32 offset = lane + group * 4;
    return base + offset;
}
#define DIGIT(e,i) (*NumberSlot(e, i))
/**
 * @brief Format a signed value in eleven packed glyph slots with leading blanks and optional minus/kind-1 prefix, preserving the original zero and minimum-digit behavior.
 */
void NumberIndicator_Format(struct EwramData_EntityData *entity, s32 value)
{
    s32 started = 0;
    s32 divisor = 1000000000;
    u8 i;
    if (value == 0)
    {
        for (i = 0; i <= 10; i++) DIGIT(entity,i) = 12;
        NUMBER_FIRST_DIGIT(entity) = 0;
    }
    else
    {
        if (value < 0)
        {
            NUMBER_PREFIX(entity) = 10;
            value = -value;
        }
        else
        {
            if (entity->state == 1) NUMBER_PREFIX(entity) = 11;
            else NUMBER_PREFIX(entity) = 12;
        }
        for (i = 1; i <= 10; i++)
        {
            u8 digit;
            DIGIT(entity,i) = 12;
            if (10 - NUMBER_PAD_DIGITS(entity) == i) started = 1;
            for (digit = 0; digit <= 9; digit++)
            {
                if (value < divisor)
                {
                    if (digit != 0 && started == 0) started = 1;
                    if (started != 0) DIGIT(entity,i) = digit;
                    break;
                }
                value -= divisor;
            }
            divisor /= 10;
        }
    }
}
