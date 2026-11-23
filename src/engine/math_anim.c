/* Matching arithmetic/block routines and entity frame-sequence steppers
 * (0x0803E820..0x0803FEA3, one upstream file split) from upstream
 * engine/obj_gfx.c. Anim_StartIndexed matches (FAKEMATCH); BgMap_FillText matches.
 * These preserve agbcc signed arithmetic, truncation and access order.
 * They are target C, not yet a portable arithmetic layer for the PC port. */
#include "core.h"
#include "structs/render_slots.h"
#include "gba.h"
#include "bg_and_collision.h"
#include "syscalls.h"
#include "gameplay.h"
#include "hitbox.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"

// file split

/**
 * @brief 3E820 | Multiply two 8.8 fixed-point numbers
 *
 * @return s16 a * b (8.8)
 */
s16 Fixed8Multiply(s16 a, s16 b)
{
    s32 product;

    product = (a * b);
    product /= 0x100;
    return product;
}

/**
 * @brief 3E83C | Divide two 8.8 fixed-point numbers
 *
 * @return s16 a / den (8.8)
 */
s16 Fixed8Divide(s16 a, s16 den)
{
    s32 num = a << 8;
    return num / den;
}

/**
 * @brief 3E854 | Reciprocal of an 8.8 fixed-point number
 *
 * @return s16 1 / x (8.8)
 */
s16 Fixed8Reciprocal(s16 x)
{
    s32 num = 0x100 << 8;
    return num / x;
}

/**
 * @brief 3E86C | Multiply two 16.16 fixed-point numbers
 *
 * Both operands are truncated to 8 fractional bits (toward zero); 0 and
 * 0x10000 (1.0) are handled exactly.
 *
 * @param a Factor (16.16)
 * @param b Factor (16.16)
 * @return s32 a * b (16.16)
 */
s32 FixedMultiply(s32 a, s32 b)
{
    s32 b8;
    s32 result;

    if ((a == 0) || (b == 0))
    {
        return 0;
    }

    if (a == 0x10000)
    {
        return b;
    }
    
    if (b == 0x10000)
    {
        return a;
    }

    if (a < 0)
    {
        result = -(-a >> 8);
    }
    else
    {
        result = a >> 8;
    }

    if (b < 0)
    {
        b8 = -(-b >> 8);
    }
    else
    {
        b8 = b >> 8;
    }

    result *= b8;
    result /= 0x100;
    result <<= 8;
    return result;
}

/**
 * @brief 3E8BC | Divide two 16.16 fixed-point numbers
 *
 * Both operands are truncated to 8 fractional bits (toward zero), so the
 * result has 8 fractional bits of precision.
 *
 * @param num Dividend (16.16)
 * @param den Divisor (16.16); 0 returns 0, 0x10000 returns num unchanged
 * @return s32 num / den (16.16)
 */
s32 FixedDivide(s32 num, s32 den)
{
    s32 result;

    if (den == 0)
    {
        return 0;
    }

    if (den == 0x10000)
    {
        return num;
    }

    if (num < 0)
    {
        result = -(-num >> 8);
    }
    else
    {
        result = num >> 8;
    }
    result = result << 8;

    if (den < 0)
    {
        den = -den;
        den = den >> 8;
        den = -den;
    }
    else
    {
        den = den >> 8;
    }

    return (result / den) << 8;
}

/**
 * @brief 3E8FC | Reciprocal of a 16.16 fixed-point number
 *
 * @param x Value (16.16); 0x10000 returns 0x10000
 * @return s32 1 / x (16.16, 8-bit precision)
 */
s32 FixedReciprocal(s32 x)
{
    s32 x8;
    s32 result;

    x8 = 0x10000;
    if (x == 0x10000)
    {
        return 0x10000;
    }

    result = 0x10000;
    if (x < 0)
    {
        x8 = -x;
        x8 = x8 >> 8;
        x8 = -x8;
    }
    else
    {
        x8 = x >> 8;
    }

    result /= x8;
    result <<= 8;
    return result;
}

/**
 * @brief 3E92C | Multiply two 16.16 numbers without rounding toward zero
 *
 * @return s32 (a >> 8) * (b >> 8)
 */
s32 FixedMultiplyFast(s32 a, s32 b)
{
    return (a >> 8) * (b >> 8);
}

/**
 * @brief 3E934 | Divide two 16.16 numbers without special cases
 *
 * @return s32 (num / (den >> 8)) << 8
 */
s32 FixedDivideFast(s32 num, s32 den)
{
    return (num / (den >> 8)) << 8;
}

/**
 * @brief 3E944 | Reciprocal of a 16.16 number without special cases
 *
 * @return s32 1 / x (16.16)
 */
s32 FixedReciprocalFast(s32 x)
{
    s32 num = 0x100 << 8;
    return (num / (x >> 8)) << 8;
}

/**
 * @brief 3E958 | Set a matrix to the identity
 *
 * @param m Matrix
 */
void Matrix_SetIdentity(struct Matrix2 *m)
{
    m->d = 0x10000;
    m->a = 0x10000;
    m->c = 0;
    m->b = 0;
}

/**
 * @brief 3E968 | Set a matrix to rotate by an angle and scale
 *
 * Multiples of a quarter turn are built exactly; other angles use Sin.
 *
 * @param m Matrix
 * @param angle Angle (0x10000 = full turn)
 * @param scaleX Horizontal scale (16.16)
 * @param scaleY Vertical scale (16.16)
 */
void Matrix_SetRotationScale(struct Matrix2 *m, s32 angle, s32 scaleX, s32 scaleY)
{
    s32 cos;
    s32 sin;

    switch (angle)
    {
        case 0x0:
        case 0x10000:
            m->a = scaleX;
            m->b = 0;
            m->c = 0;
            m->d = scaleY;
            break;

        case 0x4000:
            m->a = 0;
            m->b = -scaleX;
            m->c = scaleY;
            m->d = 0;
            break;

        case 0x8000:
            m->a = -scaleX;
            m->b = 0;
            m->c = 0;
            m->d = -scaleY;
            break;

        case 0xC000:
            m->a = 0;
            m->b = scaleX;
            m->c = -scaleY;
            m->d = 0;
            break;

        default:
            sin = Sin(angle);
            cos = Sin(angle + 0x4000);
            m->a = (scaleX >> 8) * (cos >> 8);
            m->b = (scaleX >> 8) * (-sin >> 8);
            m->c = (scaleY >> 8) * (sin >> 8);
            m->d = (scaleY >> 8) * (cos >> 8);
            break;
    }
}

/**
 * @brief 3EA1C | Copy the two 32-bit coordinates of a position into a vector
 *
 * @param v Destination vector (x, y)
 * @param pos Position
 */
void Vec2_FromPosition(s32 *v, struct WhipChainNode *pos)
{
    v[0] = pos->posX;
    v[1] = pos->posY;
}

/**
 * @brief 3EA28 | Copy a matrix
 *
 * @param dst Destination
 * @param src Source
 */
void Matrix_Copy(struct Matrix2 *dst, struct Matrix2 *src)
{
    /* Preserve interleaved word loads/stores; do not assume disjoint blocks. */
    dst->a = src->a;
    dst->b = src->b;
    dst->c = src->c;
    dst->d = src->d;
}

/**
 * @brief 3EA3C | Add two matrices (dst may alias an operand)
 *
 * @param dst Result (4 words)
 * @param m1 Operand
 * @param m2 Operand
 */
void Matrix_Add(s32 *dst, s32 *m1, s32 *m2)
{
    s32 sum[4];

    sum[0] = m1[0] + m2[0];
    sum[1] = m1[1] + m2[1];
    sum[2] = m1[2] + m2[2];
    sum[3] = m1[3] + m2[3];

    dst[0] = sum[0];
    dst[1] = sum[1];
    dst[2] = sum[2];
    dst[3] = sum[3];
}

/**
 * @brief 3EA70 | Subtract two matrices (dst may alias an operand)
 *
 * @param dst Result m1 - m2
 * @param m1 Operand
 * @param m2 Operand
 */
void Matrix_Subtract(s32 *dst, s32 *m1, s32 *m2)
{
    s32 diff[4];

    diff[0] = m1[0] - m2[0];
    diff[1] = m1[1] - m2[1];
    diff[2] = m1[2] - m2[2];
    diff[3] = m1[3] - m2[3];

    dst[0] = diff[0];
    dst[1] = diff[1];
    dst[2] = diff[2];
    dst[3] = diff[3];
}

/**
 * @brief 3EAA4 | Add two vectors
 *
 * @param dst Result v1 + v2
 * @param v1 Operand
 * @param v2 Operand
 */
void Vec2_Add(s32 *dst, s32 *v1, s32 *v2)
{
    dst[0] = v1[0] + v2[0];
    dst[1] = v1[1] + v2[1];
}

/**
 * @brief 3EABC | Subtract two vectors
 *
 * @param dst Result v1 - v2
 * @param v1 Operand
 * @param v2 Operand
 */
void Vec2_Subtract(s32 *dst, s32 *v1, s32 *v2)
{
    dst[0] = v1[0] - v2[0];
    dst[1] = v1[1] - v2[1];
}

static inline s32 MulFast(s32 a, s32 b)
{
    return (a >> 8) * (b >> 8);
}

/**
 * @brief 3EAD4 | Scale a vector
 *
 * @param dst Result v * s
 * @param v Vector (16.16)
 * @param s Scale (16.16)
 */
void Vec2_Scale(s32 *dst, s32 *v, s32 s)
{
    dst[0] = MulFast(v[0], s);
    dst[1] = MulFast(v[1], s);
}

/**
 * @brief 3EAE8 | Multiply a matrix by a vector
 *
 * @param dst Result m * v
 * @param m Matrix (4 words: a b / c d)
 * @param v Vector (x, y)
 */
void Matrix_MultiplyVec2(s32 *dst, s32 *m, s32 *v)
{
    s32 result[2];

    result[0] = MulFast(m[0], v[0]) + MulFast(m[1], v[1]);
    result[1] = MulFast(m[2], v[0]) + MulFast(m[3], v[1]);

    dst[0] = *&result[0];
    dst[1] = *&result[1];
}

/**
 * @brief 3EB24 | Multiply two matrices
 *
 * @param dst Result m1 * m2
 * @param m1 Left matrix
 * @param m2 Right matrix
 */
void Matrix_Multiply(s32 *dst, s32 *m1, s32 *m2)
{
    s32 result[4];

    result[0] = MulFast(m1[0], m2[0]) + MulFast(m1[1], m2[2]);
    result[1] = MulFast(m1[0], m2[1]) + MulFast(m1[1], m2[3]);
    result[2] = MulFast(m1[2], m2[0]) + MulFast(m1[3], m2[2]);
    result[3] = MulFast(m1[2], m2[1]) + MulFast(m1[3], m2[3]);

    dst[0] = result[0];
    dst[1] = result[1];
    dst[2] = result[2];
    dst[3] = result[3];
}

/**
 * @brief 3EBA8 | Advance an animation script of type 0 by one frame
 *
 * Sets entity->animFrame from the current step; counts animTimer up to
 * the step duration, then moves to the next step. At the end the script loops
 * (animFlags bit 0) or stops on its last step (animFlags |= 4); animDone is
 * set in both cases. Nothing advances while animFlags bit 1 (pause) is set.
 *
 * @param script Animation script
 * @param entity Entity
 * @return s32 1 same step, 2 next step, 3 finished, 4 looped
 */
s32 Anim_StepList(struct AnimScript *script, struct EwramData_EntityData *entity)
{
    s32 result;
    struct AnimStep *step;

    result = 1;
    step = script->steps;
    step = &step[entity->animStep];
    entity->animFrame = step->frame;
    if (entity->animFlags & 4)
    {
        return 3;
    }

    if ((entity->animFlags & (4 | 2)) == 0)
    {
        entity->animTimer += 1;
        if (entity->animTimer >= step->duration)
        {
            result = 2;
            entity->animTimer = 0;
            entity->animStep += 1;

            if (entity->animStep >= script->stepCount)
            {
                if (entity->animFlags & 1)
                {
                    entity->animStep = 0;
                    result = 4;
                }
                else
                {
                    entity->animStep = script->stepCount - 1;
                    result = 3;
                    entity->animFlags |= 4;
                }
                entity->animDone = 1;
            }
        }
    }
    return result;
}

/**
 * @brief 3EC34 | Advance an animation script of type 1 by one frame
 *
 * Sets entity->animFrame from the current step; counts animTimer up to
 * the step duration, then moves to the next step. At the end the script loops
 * (animFlags bit 0) or stops on its last step (animFlags |= 4); animDone is
 * set in both cases. Nothing advances while animFlags bit 1 (pause) is set.
 *
 * @param script Animation script
 * @param entity Entity
 * @return s32 1 same step, 2 next step, 3 finished, 4 looped
 */
s32 Anim_StepInline(struct AnimScript *script, struct EwramData_EntityData *entity)
{
    s32 result;
    struct AnimStep **step; // TODO: what is happening here with step?

    result = 1;
    step = &script->steps;
    step = &step[entity->animStep];
    entity->animFrame = ((struct AnimStep *)step)->frame;
    if (entity->animFlags & 4)
    {
        return 3;
    }

    if ((entity->animFlags & (4 | 2)) == 0)
    {
        entity->animTimer += 1;
        if (entity->animTimer >= ((struct AnimStep *)step)->duration)
        {
            result = 2;
            entity->animTimer = 0;
            entity->animStep += 1;

            if (entity->animStep >= script->stepCount)
            {
                if (entity->animFlags & 1)
                {
                    entity->animStep = 0;
                    result = 4;
                }
                else
                {
                    entity->animStep = script->stepCount - 1;
                    result = 3;
                    entity->animFlags |= 4;
                }
                entity->animDone = 1;
            }
        }
    }
    return result;
}

/**
 * @brief 3ECC0 | Advance an animation script of type 2 by one frame
 *
 * The step also sets the entity's attack box (HitBox_SetAttackRaw).
 * Sets entity->animFrame from the current step; counts animTimer up to
 * the step duration, then moves to the next step. At the end the script loops
 * (animFlags bit 0) or stops on its last step (animFlags |= 4); animDone is
 * set in both cases. Nothing advances while animFlags bit 1 (pause) is set.
 *
 * @param script Animation script
 * @param entity Entity
 * @return s32 1 same step, 2 next step, 3 finished, 4 looped
 */
s32 Anim_StepInlineBox(struct AnimScript *script, struct EwramData_EntityData *entity)
{
    s32 result;
    struct AnimStep **step; // TODO: what is happening here with step?

    result = 1;
    step = &script->steps;
    step = &step[entity->animStep * 2];
    entity->animFrame = READ_16(&((struct AnimStep *)step)->frame); // TODO: what????
    HitBox_SetAttackRaw(entity, ((struct AnimStep *)step)->attackBox);
    if (entity->animFlags & 4)
    {
        return 3;
    }

    if ((entity->animFlags & (4 | 2)) == 0)
    {
        entity->animTimer = entity->animTimer + 1;
        if (entity->animTimer >= ((struct AnimStep *)step)->wideDuration)
        {
            result = 2;
            entity->animTimer = 0;
            entity->animStep += 1;

            if (entity->animStep >= script->stepCount)
            {
                if (entity->animFlags & 1)
                {
                    entity->animStep = 0;
                    result = 4;
                }
                else
                {
                    entity->animStep = script->stepCount - 1;
                    result = 3;
                    entity->animFlags |= 4;
                }
                entity->animDone = 1;
            }
        }
    }
    return result;
}

/**
 * @brief 3ED6C | Advance an animation script of type 3 by one frame
 *
 * The step also sets the attack and body boxes (HitBox_SetAttackRaw, HitBox_SetBodyRaw).
 * Sets entity->animFrame from the current step; counts animTimer up to
 * the step duration, then moves to the next step. At the end the script loops
 * (animFlags bit 0) or stops on its last step (animFlags |= 4); animDone is
 * set in both cases. Nothing advances while animFlags bit 1 (pause) is set.
 *
 * @param script Animation script
 * @param entity Entity
 * @return s32 1 same step, 2 next step, 3 finished, 4 looped
 */
s32 Anim_StepInlineBoxes(struct AnimScript *script, struct EwramData_EntityData *entity)
{
    s32 result;
    struct AnimStep **step; // TODO: what is happening here with step?

    result = 1;
    step = &script->steps;
    step = &step[entity->animStep * 3];
    entity->animFrame = READ_16(&((struct AnimStep *)step)->frame); // TODO: what????
    HitBox_SetAttackRaw(entity, ((struct AnimStep *)step)->attackBox);
    HitBox_SetBodyRaw(entity, ((struct AnimStep *)step)->bodyBox);
    if (entity->animFlags & 4)
    {
        return 3;
    }

    if ((entity->animFlags & (4 | 2)) == 0)
    {
        entity->animTimer = entity->animTimer + 1;
        if (entity->animTimer >= ((struct AnimStep *)step)->wideDuration)
        {
            result = 2;
            entity->animTimer = 0;
            entity->animStep += 1;

            if (entity->animStep >= script->stepCount)
            {
                if (entity->animFlags & 1)
                {
                    entity->animStep = 0;
                    result = 4;
                }
                else
                {
                    entity->animStep = script->stepCount - 1;
                    result = 3;
                    entity->animFlags |= 4;
                }
                entity->animDone = 1;
            }
        }
    }
    return result;
}

extern const void *sAnimStepFuncs[];

/* Sprite sheet of the sprite slot *index (entity->spriteSlot). */
static inline struct SpriteSheet *SpriteSheetOf(u8 *index)
{
    return gEwramData->spriteSlots[*index].sheet;
}

/* Run the current animation script command of the entity; returns its result (0 = none). */
static inline s32 RunAnimationScript(struct EwramData_EntityData *entity)
{
    struct SpriteSheet *graphics = SpriteSheetOf(&entity->spriteSlot);
    struct AnimScript *script;

    if (graphics == NULL)
        return 0;
    if (entity->animFlags & 8)
        script = entity->animScript;
    else
        script = graphics->scripts + entity->animScriptIndex;
    if (script == NULL)
        return 0;
    return ((s32 (*)(struct AnimScript *, struct EwramData_EntityData *))sAnimStepFuncs[script->type])(script, entity);
}

/**
 * @brief 3EE24 | Animation mode 1: run the script and stream graphics frame 0
 *
 * When the frame changes, the gfxFrame of its first part is loaded into
 * loaded frame 0 of the entity's graphics (ObjGfx_SetFrame).
 *
 * @param entity Entity
 * @return s32 Script result
 */
s32 Anim_UpdateGfxFrame0(struct EwramData_EntityData *entity)
{
    u8 frame = entity->animFrame;
    s32 result = RunAnimationScript(entity);

    if (frame != entity->animFrame)
    {
        u8 *index = &entity->spriteSlot;
        struct SpriteSheet *graphics = gEwramData->spriteSlots[*index].sheet;
        s32 palette, sprite;

        if (graphics == NULL)
            return 0;
        if (entity->animFrame < graphics->frameCount)
            palette = graphics->frames[entity->animFrame].parts->gfxFrame;
        else
            palette = graphics->frames->parts->gfxFrame;
        sprite = Sprite_GetTileSlot(entity);
        if (ObjGfx_GetFrame(sprite, 0) != palette)
            ObjGfx_SetFrame(sprite, 0, palette, 1);
    }
    return result;
}

/**
 * @brief 3EEFC | Animation mode 2: run the script and stream graphics frame 1
 *
 * When the frame changes, the first nonzero gfxFrame of its parts is loaded
 * into loaded frame 1 (parts with gfxFrame use the second frame).
 *
 * @param entity Entity
 * @return s32 Script result
 */
s32 Anim_UpdateGfxFrame1(struct EwramData_EntityData *entity)
{
    u8 frame = entity->animFrame;
    s32 result = RunAnimationScript(entity);

    if (frame != entity->animFrame)
    {
        u8 *index = &entity->spriteSlot;
        struct SpriteSheet *graphics = gEwramData->spriteSlots[*index].sheet;
        struct SpriteFrame *frameData;
        s32 i, count, palette, sprite;

        if (graphics == NULL)
            return 0;
        if (entity->animFrame < graphics->frameCount)
            frameData = graphics->frames + entity->animFrame;
        else
            frameData = graphics->frames;
        count = frameData->partCount;
        for (i = 0; i < count; i++)
        {
            palette = frameData->parts[i].gfxFrame;
            if (palette != 0)
                break;
        }
        if (palette != 0)
        {
            sprite = Sprite_GetTileSlot(entity);
            if (ObjGfx_GetFrame(sprite, 1) != palette)
                ObjGfx_SetFrame(sprite, 1, palette, 1);
        }
    }
    return result;
}

/**
 * @brief 3EFF0 | Animation step that also streams the tile graphics of the current frame
 *
 * @param entity Entity
 * @return s32 Script command result
 */
s32 Anim_UpdateStreamed(struct EwramData_EntityData *entity)
{
    u8 *sheet;
    void **tiles;
    u8 *frame;
    u8 tile;
    u8 bank;

    if (SpriteSheetOf(&entity->spriteSlot) == NULL)
        return 0;
    sheet = gEwramData->objGfx[gEwramData->spriteSlots[entity->spriteSlot].gfxEntry].source;
    if (sheet == NULL)
        return 0;
    if (*sheet != 2)
        return 0;
    tiles = (void **)(sheet + 4);
    frame = (u8 *)&entity->animScript->steps;
    frame += entity->animStep * 4;
    tile = *frame;
    bank = tile >> 2;
    if (entity->spawnIndex != tile)
    {
        entity->spawnIndex = tile;
        ObjTiles_LoadFrame(Sprite_GetTileSlot(entity), tiles[bank], *frame, 0, 1, 1);
    }
    return RunAnimationScript(entity);
}

/**
 * @brief 3F0E4 | Start an animation script and run its first command
 *
 * @param entity Entity
 * @param script Animation script
 * @param mode Animation mode (animMode)
 * @param flags Script flags
 * @return s32 1 if started, 0 if the entity has no graphics
 */
s32 Anim_Start(struct EwramData_EntityData *entity, struct AnimScript *script, u8 mode, u8 flags)
{
    entity->animMode = mode;
    if (SpriteSheetOf(&entity->spriteSlot) == NULL)
        return 0;

    entity->animScript = script;
    entity->animFlags = flags | 8;
    entity->animScriptIndex = 0;
    entity->animStep = 0;
    entity->animTimer = 0;
    Anim_Update(entity);
    entity->animFlags = flags | 8;
    entity->animScriptIndex = 0;
    entity->animStep = 0;
    entity->animTimer = 0;
    entity->animDone = 0;
    return 1;
}

/**
 * @brief 3F17C | Advance the entity animation according to its mode
 *
 * @param entity Entity
 * @return s32 Script command result
 */
s32 Anim_Update(struct EwramData_EntityData *entity)
{
    s16 result = 0;

    switch (entity->animMode)
    {
        case 0:
            result = RunAnimationScript(entity);
            break;

        case 1:
            result = Anim_UpdateGfxFrame0(entity);
            break;

        case 2:
            result = Anim_UpdateGfxFrame1(entity);
            break;

        case 3:
            result = Anim_UpdateStreamed(entity);
            break;
    }

    return result;
}

/**
 * @brief 3F224 | Value of the current command of the animation script
 *
 * @param entity Entity
 * @return s32 Command value (depends on the script type)
 */
s32 Anim_GetStepDuration(struct EwramData_EntityData *entity)
{
    struct AnimStep *command;
    struct AnimScript *script;
    s32 value = 0;
    u8 *bytes;
    u16 *halves;
    struct SpriteSheet *graphics = SpriteSheetOf(&entity->spriteSlot);

    if (graphics == NULL)
        return 0;

    if (entity->animFlags & 8)
        script = entity->animScript;
    else
        script = graphics->scripts + entity->animScriptIndex;

    switch (script->type)
    {
        case 0:
            command = script->steps;
            command = &command[entity->animStep];
            value = command->duration;
            break;

        case 1:
            bytes = (u8 *)script + entity->animStep * 4;
            value = bytes[5];
            break;

        case 2:
            halves = (u16 *)script + entity->animStep * 4;
            value = halves[3];
            break;

        case 3:
            halves = (u16 *)script + entity->animStep * 6;
            value = halves[3];
            break;
    }

    return value;
}

// Upstream nonmatching: https://decomp.me/scratch/650Uz (matched with two FAKEMATCH lines)
s32 Anim_StartIndexed(struct EwramData_EntityData *entity, u16 script, u8 mode, u8 flags)
{
    struct SpriteSheet *graphics;

    asm("" : "+r"(flags)); /* FAKEMATCH: second set of flags defeats reg_is_remote_constant_p in regmove */
    { register struct SpriteSheet *g_ asm("r0") = SpriteSheetOf(&entity->spriteSlot); asm("" : "+r"(g_)); graphics = g_; } /* FAKEMATCH: load into r0, then copy */

    if (graphics->scriptTable != NULL && script < graphics->scriptCount)
    {
        entity->animMode = mode;
        entity->animScript = graphics->scriptTable[script];
        entity->animFlags = flags | 8;
        entity->animScriptIndex = script;
        entity->animStep = 0;
        entity->animTimer = 0;
        entity->animDone = 0;
        return 1;
    }
    return 0;
}

/**
 * @brief 3F348 | Select another script of the current graphics set
 *
 * @param entity Entity
 * @param script Script index
 * @param flags Script flags
 * @return s32 1 if selected, 0 if the index is out of range
 */
s32 Anim_SelectScript(struct EwramData_EntityData *entity, u8 script, u8 flags)
{
    struct SpriteSheet *graphics = SpriteSheetOf(&entity->spriteSlot);

    if (graphics != NULL && script < graphics->scriptCount)
    {
        entity->animFlags = flags;
        entity->animScriptIndex = script;
        entity->animStep = 0;
        entity->animTimer = 0;
        return 1;
    }
    return 0;
}

/**
 * @brief 3F39C | Switch to another animation script, keeping the current position
 *
 * @param entity Entity
 * @param script Animation script
 * @param flags Script flags (1 = loop)
 * @return s32 1 if switched, 0 if the entity has no graphics
 */
s32 Anim_SwitchScript(struct EwramData_EntityData *entity, struct AnimScript *script, u8 flags)
{
    if (SpriteSheetOf(&entity->spriteSlot) == NULL)
        return 0;

    entity->animScript = script;
    entity->animFlags &= ~(4 | 1);
    entity->animFlags |= flags | 8;
    entity->animDone = 0;
    return 1;
}

/**
 * @brief Limit the fixed-point horizontal step by the map edge and eight pixels, queue an entering tile column when needed and return the updated X position.
 */
s32 BgMap_ScrollRight(s32 bgIndex, struct BgLayerState *bg, s32 deltaX)
{
    u16 *column;
    s32 i;
    u16 tileX;
    u32 vramAddr;
    u16 tileY;
    s32 margin;
    s32 vramBase = 0x0600E000;

    if ((((u32) (bg->xPos.whole + deltaX) >> 0x13) + 0x1E) >= (bg->pBgMetadata->widthCells << 5))
    {
        margin = 0xF0;
        deltaX = (bg->pBgMetadata->widthCells << 0x18) - (bg->xPos.whole + (margin << 0x10));
    }

    if (deltaX > 0x80000)
    {
        deltaX = 0x80000;
    }
    bg->xVel.whole = deltaX;

    if (((u32) (bg->xPos.whole - 0x10000) >> 0x13) != ((u32) (bg->xPos.whole + deltaX - 0x10000) >> 0x13))
    {
        column = (u16*)&gEwramData->sharedScratchBuffer[0x20];
        tileX = (bg->xPos.part16.integer + 0xF7) >> 3;
        tileY = bg->yPos.part16.integer >> 3;
        vramAddr = vramBase + (bgIndex << 11) + ((tileX & 0x1F) * 2) + ((tileY & 0x1F) << 6);

        for (i = 0; i < 0x15; i++, tileY++)
        {
            column[i] = BgMap_GetTileEntry(bg->pBgMetadata, tileX, tileY);
        }

        BgCmdBuffer_WriteData(0x80, 0x15, (u8*)column, vramAddr);
    }

    bg->xPos.whole += deltaX;
    return bg->xPos.whole;
}

/**
 * @brief Limit the fixed-point horizontal step by zero and minus eight pixels, queue an entering tile column when needed and return the updated X position.
 */
s32 BgMap_ScrollLeft(s32 bgIndex, struct BgLayerState *bg, s32 deltaX)
{
    u16 *column;
    s32 i;
    u16 tileX;
    u32 vramAddr;
    u16 tileY;
    s32 vramBase = 0x0600E000;

    bg++,bg--; // FAKE
    if ((bg->xPos.whole + deltaX) < 0)
    {
        deltaX = -bg->xPos.whole;
    }

    if (deltaX < -0x80000)
    {
        deltaX = -0x80000;
    }
    bg->xVel.whole = deltaX;

    if (((u32) bg->xPos.whole >> 0x13) != ((u32) (bg->xPos.whole + deltaX) >> 0x13))
    {
        column = (u16*)&gEwramData->sharedScratchBuffer[0x20];
        tileX = (bg->xPos.part16.integer - 8) >> 3;
        tileY = bg->yPos.part16.integer >> 3;
        vramAddr = vramBase + (bgIndex << 11) + ((tileX & 0x1F) * 2) + ((tileY & 0x1F) << 6);

        for (i = 0; i < 0x15; i++, tileY++)
        {
            column[i] = BgMap_GetTileEntry(bg->pBgMetadata, tileX, tileY);
        }

        BgCmdBuffer_WriteData(0x80, 0x15, (u8*)column, vramAddr);
    }

    bg->xPos.whole += deltaX;
    return bg->xPos.whole;
}

/**
 * @brief Limit downward scrolling using a 208-pixel bottom extent and an eight-pixel step, queue an entering tile row and return the updated Y position.
 */
s32 BgMap_ScrollDownWithMargin(s32 bgIndex, struct BgLayerState *bg, s32 deltaY)
{
    u16 *row;
    s32 i;
    u16 tileY;
    u32 vramAddr;
    u16 tileX;
    s32 margin;
    s32 vramBase = 0x0600E000;

    if ((((u32) (bg->yPos.whole + deltaY) >> 0x13) + 0x1A) >= (bg->pBgMetadata->heightCells << 5))
    {
        margin = 0xD0;
        deltaY = (bg->pBgMetadata->heightCells << 0x18) - (bg->yPos.whole + (margin << 0x10));
    }

    if (deltaY > 0x80000)
    {
        deltaY = 0x80000;
    }
    bg->yVel.whole = deltaY;

    if (((u32) (bg->yPos.whole - 0x10000) >> 0x13) != ((u32) (bg->yPos.whole + deltaY - 0x10000) >> 0x13))
    {
        row = (u16*)&gEwramData->sharedScratchBuffer[0x20];
        tileX = bg->xPos.part16.integer >> 3;
        tileY = (bg->yPos.part16.integer + 0xA7) >> 3;
        vramAddr = vramBase + (bgIndex << 0xB) + ((tileX & 0x1F) * 2) + ((tileY & 0x1F) << 6);

        for (i = 0; i < 0x1F; i++, tileX++)
        {
            row[i] = BgMap_GetTileEntry(bg->pBgMetadata, tileX, tileY);
        }

        BgCmdBuffer_WriteData(0, 0x1F, (u8*)row, vramAddr);
    }

    bg->yPos.whole += deltaY;
    return bg->yPos.whole;
}

/**
 * @brief Limit upward scrolling to a 48-pixel top position and a minus-eight-pixel step, queue an entering tile row and return the updated Y position.
 */
s32 BgMap_ScrollUpWithMargin(s32 bgIndex, struct BgLayerState *bg, s32 deltaY)
{
    u16 *row;
    s32 i;
    u16 tileY;
    u32 vramAddr;
    u16 tileX;
    s32 vramBase = 0x0600E000;

    if ((bg->yPos.whole + deltaY) < 0x300000)
    {
        deltaY = 0x300000 - bg->yPos.whole;
    }

    if (deltaY < -0x80000)
    {
        deltaY = -0x80000;
    }
    bg->yVel.whole = deltaY;

    if (((u32) bg->yPos.whole >> 0x13) != ((u32) (bg->yPos.whole + deltaY) >> 0x13))
    {
        row = (u16*)&gEwramData->sharedScratchBuffer[0x20];
        tileX = bg->xPos.part16.integer >> 3;
        tileY = (bg->yPos.part16.integer - 8) >> 3;
        vramAddr = vramBase + (bgIndex << 11) + ((tileX & 0x1F) * 2) + ((tileY & 0x1F) << 6);

        for (i = 0; i < 0x1F; i++, tileX++)
        {
            row[i] = BgMap_GetTileEntry(bg->pBgMetadata, tileX, tileY);
        }

        BgCmdBuffer_WriteData(0, 0x1F, (u8*)row, vramAddr);
    }

    bg->yPos.whole += deltaY;
    return bg->yPos.whole;
}

// Matching (upstream scratch: https://decomp.me/scratch/hV8oX)
/**
 * @brief Set scroll registers and fill the wrapped 32x32 text tilemap at 0x0600E000 plus background index times 0x800.
 */
void BgMap_FillText(s32 bgIndex)
{
    u16 startX;
    u16 *p;
    struct BgLayerState *bg;
    u16 i;
    u16 col;
    u16 j;
    u16 tileY;
    void* vramBase = (void*)0x0600E000;

    bg = &gEwramData->bgInfo[bgIndex];

    /* one pointer variable for the scroll registers and the map entries (r4 in both) */
    p = (u16 *)&gDisplayRegisters.bgOfs[bgIndex];
    p[0] = bg->xPos.part16.integer;
    p[1] = bg->yPos.part16.integer;

    startX = bg->xPos.part16.integer >> 3;
    tileY = bg->yPos.part16.integer >> 3;

    for (i = 0; i < 0x20; i++, tileY++)
    {
        for (j = startX, col = 0; col < 0x20; col++, j++)
        {
            p = vramBase + (bgIndex << 11) + ((j & 0x1F) * 2) + ((tileY & 0x1F) << 6);
            *p = BgMap_GetTileEntry(bg->pBgMetadata, j, tileY);
        }
    }
}

/**
 * @brief For background 2 only, expand cached map blocks into the affine map at 0x0600F000 and initialize the BG2 affine transform.
 */
void BgMap_FillAffineBG2(s32 bgIndex)
{
    struct BgAffineSrcData subroutine_arg0;
    struct BgLayerState *bg;
    s32 block;
    u16 i;
    u16 j;
    u16 *blocks;
    u32 *dst;
    u32 *tiles;

    bg = &gEwramData->bgInfo[bgIndex];
    blocks = bg->pBgMetadata->blockMap;

    if (bgIndex == 2)
    {
        gDisplayRegisters.bgOfs[2].hOfs = bg->xPos.part16.integer;
        gDisplayRegisters.bgOfs[2].vOfs = bg->yPos.part16.integer;

        for (i = 0; i < 0x20; i++)
        {
            for (j = 0; j < 8; j++)
            {
                dst = (u32*)0x0600F000 + (j) + (i << 3);
                tiles = (u32*)gEwramData->sharedBgTileCache;
                block = blocks[(u8)j + ((u8)(i >> 2) << 3)];
                if (block != 0)
                {
                    *dst = tiles[block * 4 + (i & 3) - 4];
                }
                else
                {
                    *dst = 0;
                }
                
            }
        }

        subroutine_arg0.texX = 0x8000;
        subroutine_arg0.texY = 0x8000;
        subroutine_arg0.scrX = 0x78;
        subroutine_arg0.scrY = 0x50;
        subroutine_arg0.sx = 0x100;
        subroutine_arg0.sy = 0x100;
        subroutine_arg0.alpha = 0;
        BgAffineSet((struct BgAffineSrcData *) &subroutine_arg0, (struct BgAffineDstData *) &gDisplayRegisters.bg2PA, 1);
    }
}

/**
 * @brief Set metadata and integer scroll, zero fractions/velocities, copy or decompress primary map data and optional BG1 auxiliary data, then fill the selected map mode.
 */
void BgMap_Setup(s32 bgIndex, struct BgMapMetadata *metadata, u16 scrollX, u16 scrollY)
{
    struct BgLayerState *bg;

    bg = &gEwramData->bgInfo[bgIndex];
    bg->pBgMetadata = metadata;
    bg->xPos.part16.integer = scrollX;
    bg->yPos.part16.integer = scrollY;
    bg->yPos.part16.fraction = 0;
    bg->xPos.part16.fraction = 0;
    bg->yVel.whole = 0;
    bg->xVel.whole = 0;

    if (metadata == NULL)
        return;

    if (metadata->formatFlags & 2)
    {
        LZ77UnCompWram((u32 *) metadata->tileData, ((bgIndex == 1) ? &gEwramData->bg1TileCache - 2 : &gEwramData->sharedBgTileCache - 2));
        if ((metadata->collisionData != NULL) && (bgIndex == 1))
        {
            LZ77UnCompWram((u32 *) metadata->collisionData, &gEwramData->bg1CollisionCache - 4);
        }
    }
    else
    {
        DMA_COPY_16(3, metadata->tileData, ((bgIndex == 1) ? &gEwramData->bg1TileCache : &gEwramData->sharedBgTileCache), 0x800);
    }

    if (!(metadata->formatFlags & 1))
    {
        BgMap_FillText(bgIndex);
    }
    else
    {
        BgMap_FillAffineBG2(bgIndex);
    }
}

/**
 * @brief Set metadata and scroll state and load primary map data; fill text mode only, leaving auxiliary decompression and affine-map fill out of this variant.
 */
void BgMap_SetupPrimary(s32 bgIndex, struct BgMapMetadata *metadata, u16 scrollX, u16 scrollY)
{
    struct BgLayerState *bg;

    bg = &gEwramData->bgInfo[bgIndex];
    bg->pBgMetadata = metadata;
    bg->xPos.part16.integer = scrollX;
    bg->yPos.part16.integer = scrollY;
    bg->yPos.part16.fraction = 0;
    bg->xPos.part16.fraction = 0;
    bg->yVel.whole = 0;
    bg->xVel.whole = 0;

    if (metadata == NULL)
        return;

    if (metadata->formatFlags & 2)
    {
        LZ77UnCompWram((u32 *) metadata->tileData, ((bgIndex == 1) ? &gEwramData->bg1TileCache - 2 : &gEwramData->sharedBgTileCache - 2));
    }
    else
    {
        DMA_COPY_16(3, metadata->tileData, ((bgIndex == 1) ? &gEwramData->bg1TileCache : &gEwramData->sharedBgTileCache), 0x800);
    }

    if (!(metadata->formatFlags & 1))
    {
        BgMap_FillText(bgIndex);
    }
}

/**
 * @brief Limit downward scrolling using a 160-pixel bottom extent and an eight-pixel step, queue an entering tile row and return the updated Y position.
 */
s32 BgMap_ScrollDown(s32 bgIndex, struct BgLayerState *bg, s32 deltaY)
{
    u16 *row;
    s32 i;
    u16 tileY;
    u32 vramAddr;
    u16 tileX;
    s32 margin;
    s32 vramBase = 0x0600E000;

    if ((((u32) (bg->yPos.whole + deltaY) >> 0x13) + 0x14) >= (bg->pBgMetadata->heightCells << 5))
    {
        margin = 0xA0;
        deltaY = (bg->pBgMetadata->heightCells << 0x18) - (bg->yPos.whole + (margin << 0x10));
    }

    if (deltaY > 0x80000)
    {
        deltaY = 0x80000;
    }
    bg->yVel.whole = deltaY;

    if (((u32) (bg->yPos.whole - 0x10000) >> 0x13) != ((u32) (bg->yPos.whole + deltaY - 0x10000) >> 0x13))
    {
        row = (u16*)&gEwramData->sharedScratchBuffer[0x20];
        tileX = bg->xPos.part16.integer >> 3;
        tileY = (bg->yPos.part16.integer + 0xA7) >> 3;
        vramAddr = vramBase + (bgIndex << 0xB) + ((tileX & 0x1F) * 2) + ((tileY & 0x1F) << 6);

        for (i = 0; i < 0x1F; i++, tileX++)
        {
            row[i] = BgMap_GetTileEntry(bg->pBgMetadata, tileX, tileY);
        }

        BgCmdBuffer_WriteData(0, 0x1F, (u8*)row, vramAddr);
    }

    bg->yPos.whole += deltaY;
    return bg->yPos.whole;
}

/**
 * @brief Limit upward scrolling to zero and a minus-eight-pixel step, queue an entering tile row and return the updated Y position.
 */
s32 BgMap_ScrollUp(s32 bgIndex, struct BgLayerState *bg, s32 deltaY)
{
    u16 *row;
    s32 i;
    u16 tileY;
    u32 vramAddr;
    u16 tileX;
    s32 vramBase = 0x0600E000;

    bg++,bg--; // FAKE
    if ((bg->yPos.whole + deltaY) < 0)
    {
        deltaY = -bg->yPos.whole;
    }

    if (deltaY < -0x80000)
    {
        deltaY = -0x80000;
    }
    bg->yVel.whole = deltaY;

    if (((u32) bg->yPos.whole >> 0x13) != ((u32) (bg->yPos.whole + deltaY) >> 0x13))
    {
        row = (u16*)&gEwramData->sharedScratchBuffer[0x20];
        tileX = bg->xPos.part16.integer >> 3;
        tileY = (bg->yPos.part16.integer - 8) >> 3;
        vramAddr = vramBase + (bgIndex << 11) + ((tileX & 0x1F) * 2) + ((tileY & 0x1F) << 6);

        for (i = 0; i < 0x1F; i++, tileX++)
        {
            row[i] = BgMap_GetTileEntry(bg->pBgMetadata, tileX, tileY);
        }

        BgCmdBuffer_WriteData(0, 0x1F, (u8*)row, vramAddr);
    }

    bg->yPos.whole += deltaY;
    return bg->yPos.whole;
}

// TODO: inline likely used in many functions here

static inline s32 BgMap_GetVramEntryAddressInline(s32 bgIndex, s32 tileX, s32 tileY)
{
    s32 vramBase = 0x0600E000;
    return vramBase + (bgIndex << 0xB) + ((tileX & 0x1F) * 2) + ((tileY & 0x1F) << 6);
}

/**
 * @brief Return the wrapped text tilemap VRAM address for the background index and tile X/Y coordinates.
 */
s32 BgMap_GetVramEntryAddress(s32 bgIndex, u16 tileX, u16 tileY)
{
    return BgMap_GetVramEntryAddressInline(bgIndex, tileX, tileY);
}

/**
 * @brief Dispatch signed horizontal/vertical steps for multi-screen maps using the 48-pixel top and 208-pixel bottom variant; update scroll registers.
 */
void BgMap_ScrollWithMargin(s32 bgIndex, s32 deltaX, s32 deltaY)
{
    struct BgLayerState *bg;
    u16 *hScroll;
    u16 *vScroll;

    bg = &gEwramData->bgInfo[bgIndex];
    hScroll = &gDisplayRegisters.bgOfs[bgIndex].hOfs;
    vScroll = &gDisplayRegisters.bgOfs[bgIndex].vOfs;

    if (bg->pBgMetadata->widthCells > 1)
    {
        if (deltaX > 0)
        {
            BgMap_ScrollRight(bgIndex, bg, deltaX);
        }
        else if (deltaX < 0)
        {
            BgMap_ScrollLeft(bgIndex, bg, deltaX);
        }
        else
        {
            bg->xVel.whole = 0;
        }
    }
    else
    {
        bg->xVel.whole = 0;
        bg->xPos.whole = 0;
    }

    if (bg->pBgMetadata->heightCells > 1)
    {
        if (deltaY > 0)
        {
            BgMap_ScrollDownWithMargin(bgIndex, bg, deltaY);
        }
        else if (deltaY < 0)
        {
            BgMap_ScrollUpWithMargin(bgIndex, bg, deltaY);
        }
        else
        {
            bg->yVel.whole = 0;
        }
    }
    else
    {
        bg->yPos.whole = 0x300000;
        bg->yVel.whole = 0;
    }

    *hScroll = bg->xPos.part16.integer;
    *vScroll = bg->yPos.part16.integer;
}

/**
 * @brief Dispatch signed horizontal/vertical steps for multi-screen maps using the zero top and 160-pixel bottom variant; update scroll registers.
 */
void BgMap_Scroll(s32 bgIndex, s32 deltaX, s32 deltaY)
{
    struct BgLayerState *bg;
    u16 *hScroll;
    u16 *vScroll;

    bg = &gEwramData->bgInfo[bgIndex];
    hScroll = &gDisplayRegisters.bgOfs[bgIndex].hOfs;
    vScroll = &gDisplayRegisters.bgOfs[bgIndex].vOfs;

    if (bg->pBgMetadata->widthCells > 1)
    {
        if (deltaX > 0)
        {
            BgMap_ScrollRight(bgIndex, bg, deltaX);
        }
        else if (deltaX < 0)
        {
            BgMap_ScrollLeft(bgIndex, bg, deltaX);
        }
        else
        {
            bg->xVel.whole = 0;
        }
    }
    else
    {
        bg->xVel.whole = 0;
        bg->xPos.whole = 0;
    }

    if (bg->pBgMetadata->heightCells > 1)
    {
        if (deltaY > 0)
        {
            BgMap_ScrollDown(bgIndex, bg, deltaY);
        }
        else if (deltaY < 0)
        {
            BgMap_ScrollUp(bgIndex, bg, deltaY);
        }
        else
        {
            bg->yVel.whole = 0;
        }
    }
    else
    {
        bg->yVel.whole = 0;
        bg->yPos.whole = 0;
    }

    *hScroll = bg->xPos.part16.integer;
    *vScroll = bg->yPos.part16.integer;
}

/**
 * @brief Copy selected background metadata and integer X/Y scroll into the existing shared saved-selection fields.
 */
void BgMap_SaveSelection(u8 bgIndex)
{
    struct BgLayerState *bg;

    bg = &gEwramData->bgInfo[bgIndex];

    gEwramData->savedBgMetadata = bg->pBgMetadata;
    gEwramData->savedBgScrollX = bg->xPos.part16.integer;
    gEwramData->savedBgScrollY = bg->yPos.part16.integer;
}

/**
 * @brief Replace priority bits of all four cached BG control registers with Obj_GetBgPriority results.
 */
void Bg_SyncPriorities(void)
{
    s16 i;
    u16 *control;
    s32 tmp;
    s32 tmp2;

    control = &gDisplayRegisters.bgCnt[0];

    for (i = 0; i < 4; i++)
    {
        control[0] = control[0] & ~3;
        tmp2 = Obj_GetBgPriority(i);
        tmp = control[0];
        control[0] = tmp | tmp2;
        control++;
    }
}

/**
 * @brief Select raw payload or shared decompression scratch and copy the header-selected size to the destination, immediately or through the DMA queue.
 */
void Gfx_CopyResource(void *resource, void *dest, u16 queued)
{
    u32 size;
    u8 *src;
    u8 *header;

    header = resource;
    src = NULL;

    if (header[0] != 0)
    {
        if (header[0] == 1)
        {
            ObjGfx_Decompress(header);
            src = gEwramData->gfxDecompTiles;
        }
    }
    else
    {
        src = header + 4;
    }

    if ((header[1] == 4) || (header[1] == 0))
    {
        size = header[3] << 9;
    }
    else
    {
        size = 0x4000;
    }

    if (queued == 0)
    {
        DMA_COPY_16(3, src, dest, size);
    }
    else
    {
        DmaQueue_IndirectCopy(size, (u32 *) src, dest);
    }
}

/**
 * @brief Select raw payload or shared decompression scratch and DMA-copy the specified source/destination blocks to VRAM using 0x800-byte units.
 */
void Gfx_CopyResourceBlocks(void *resource, s32 destinationIndex, s32 sourceIndex, s32 blockCount)
{
    u8 *src;
    u8 *header;
    void *dst;
    s32 size;

    header = resource;
    src = NULL;

    if (header[0] != 0)
    {
        if (header[0] == 1)
        {
            ObjGfx_Decompress(resource);
            src = gEwramData->gfxDecompTiles;
        }
    }
    else
    {
        src = resource + 4;
    }

    src = src + (sourceIndex << 0xB);
    dst = (void*)0x06000000 + (destinationIndex << 0xB);
    size = blockCount << 11;
    DMA_COPY_16(3, src, dst, size);
}

/**
 * @brief Return zero when unk_A074_2 exceeds the supplied level; otherwise write the cached BG control entry selected by register offset and return one.
 */
s32 Bg_SetControlIfLevelAllows(s32 level, s32 reg, u16 value)
{
    if (gEwramData->bgControlMinWriteLevel > level)
        return 0;

    // TODO: what?
    // gDisplayRegisters.bgCnt[(reg - 8) >> 1] = value;
    ((reg - 8) >> 1)[gDisplayRegisters.bgCnt] = value;
    return 1;
}
