#ifndef MATH_INLINE_H
#define MATH_INLINE_H

/* Small arithmetic helpers shared by several C files. */

#include "types.h"
#include "core.h"

static inline s32 Abs(s32 x)
{
    if (x < 0)
        x = -x;
    return x;
}

/* Signed whole-pixel difference a - b of two 16.16 coordinates (rounded toward zero). */
static inline s32 PixelDiff(s32 a, s32 b)
{
    s32 diff = a - b;

    if (diff < 0)
        return -((b - a) >> 16);
    return diff >> 16;
}

static inline s32 Add16(s16 a, u16 b)
{
    return a + b;
}

/* 16.16 multiply that drops the low 8 bits of both operands first. */
static inline s32 FixedMulFast(s32 a, s32 b)
{
    return (a >> 8) * (b >> 8);
}

static inline s32 SignExtendOffset(u16 value)
{
    return (s16)value;
}

/* Random value in [-range / 2, range / 2). */
static inline s32 RandomOffset(s32 range)
{
    return RandomNumberGenerator() % range - range / 2;
}

#endif // MATH_INLINE_H
