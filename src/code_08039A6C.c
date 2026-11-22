/* Draw one entity's sprite parts through an affine matrix (rotated/scaled OAM). */
#include "code/code_08039340.h"
#include "structs/ewram.h"

struct Matrix2 { s32 m[4]; };
struct ObjEntry;
extern const s32 sUnk_080E29A0[3][4][2];
extern void (*gObjAffineGetMatrixFunc)(struct Matrix2 *, s32);
extern s32 (*gObjAffineGetFlipXFunc)(s32);
extern s32 (*gObjAffineGetFlipYFunc)(s32);
extern u16 *(*gObjAllocManyFunc)(u32);
extern void (*gObjAddToLayerFunc)(u16 *, u32);
extern void (*gMatrixMultiplyVec2Func)(s32 *, struct Matrix2 *, s32 *);
extern s32 (*gObjTilesGetTileFunc)(s32, s32, s32);

#define WHOLE(v) ((v) < 0 ? -(-(v) >> 16) : (v) >> 16)

static inline void Mirror(s32 *dst, s32 *src, s32 *origin, s32 shape, s32 size, s32 flipX, s32 flipY)
{
        if (flipX)
        {
            s32 extent = sUnk_080E29A0[shape][size][0];
            dst[0] = origin[0] * 2 - src[0] - extent * 2;
        }
        else
            dst[0] = src[0];
        if (flipY)
        {
            s32 extent = sUnk_080E29A0[shape][size][1];
            dst[1] = origin[1] * 2 - src[1] - extent * 2;
        }
        else
            dst[1] = src[1];
}

static inline void Rotate(s32 *dst, s32 *src, s32 *origin, struct Matrix2 *matrix, s32 shape, s32 size, s32 doubled)
{
    s32 temp[2];
    s32 extentX = sUnk_080E29A0[shape][size][0];
    s32 extentY = sUnk_080E29A0[shape][size][1];

    if (doubled)
    {
        temp[0] = src[0] - extentX;
        temp[1] = src[1] - extentY;
        extentX <<= 1;
        extentY <<= 1;
    }
    else
    {
        temp[0] = src[0];
        temp[1] = src[1];
    }
    temp[0] = extentX - origin[0] + temp[0];
    temp[1] = extentY - origin[1] + temp[1];
    gMatrixMultiplyVec2Func(temp, matrix, temp);
    dst[0] = temp[0] - extentX;
    dst[1] = temp[1] - extentY;
}

/**
 * @brief 39A6C | Draw a multi-part sprite through an affine set (Thumb version)
 *
 * Builds one OAM object per part of the current frame (entity->animFrame) with
 * the entity's flips, palette, OBJ mode and mosaic, and links them into layer
 * entity->spriteLayer. Part positions are rotated/scaled by the matrix of entity->affineIndex.
 * The ARM copy Sprite_DrawPartsAffineArm is used at runtime.
 *
 * @param entity Entity with a sprite
 * @return s32 1 if drawn, 0 without a sheet or when out of objects
 */
s32 Sprite_DrawPartsAffine(struct EwramData_EntityData *entity)
{
    struct Matrix2 matrix;
    s32 origin[2];
    s32 position[2];
    s32 result[2];
    s32 graphics;
    s32 i;
    u32 baseAttributes;
    s32 flipX, flipY;
    s32 paletteOffset = 0;
    s32 affine;
    u16 *slot;
    s32 shape, size;
    struct SpriteSheet *animation;
    struct SpriteFrame *frame;
    struct SpritePart *part;
    u16 *output;
    s32 id;
    struct EwramData *ew;
    s32 count;

    ew = gEwramData;
    id = entity->spriteSlot;
    graphics = ew->spriteSlots[id].tileSlot;
    animation = ew->spriteSlots[(u8)id].sheet;
    if (animation == NULL)
        return 0;
    switch (entity->animMode)
    {
    case 0:
    case 1:
        break;
    case 2:
        paletteOffset = gEwramData->objTileSlots[(s16)graphics].size;
        break;
    }
    if (entity->animFrame < animation->frameCount)
        frame = &animation->frames[entity->animFrame];
    else
        frame = animation->frames;
    count = frame->partCount;
    part = frame->parts;
    origin[0] = frame->originX << 16;
    origin[1] = frame->originY << 16;
    affine = entity->affineIndex;
    gObjAffineGetMatrixFunc(&matrix, affine);
    flipX = gObjAffineGetFlipXFunc(affine);
    flipY = gObjAffineGetFlipYFunc(affine);
    affine <<= 9;
    baseAttributes = ((entity->doubleSize << 1) | 1) << 8;
    baseAttributes |= entity->mosaic << 12;
    baseAttributes |= entity->objMode << 10;
    slot = gObjAllocManyFunc(count);
    if (slot == NULL)
        return 0;
    for (i = 0; i < count; i++, part++, slot += 6)
    {
        {
            output = slot;
            shape = part->shape;
            size = part->size;
            position[0] = part->x << 16;
            position[1] = part->y << 16;
            if (entity->shrinkPartOffsets)
            {
                position[0] = (position[0] >> 8) * 0xE8;
                position[1] = (position[1] >> 8) * 0xE8;
            }
            if (flipX | flipY)
                Mirror(position, position, origin, shape, size, flipX, flipY);
            Rotate(result, position, origin, &matrix, shape, size, entity->doubleSize);
            if (entity->shrinkTransformedOffsets)
            {
                result[0] = (result[0] >> 8) * 0xF6;
                result[1] = (result[1] >> 8) * 0xF6;
            }
            *output++ = (shape << 14) | baseAttributes | (WHOLE(result[1] + entity->posY.whole) & 0xFF);
            *output++ = (size << 14) | affine | (WHOLE(result[0] + entity->posX.whole) & 0x1FF);
            *output = ((entity->palette + (part->paletteOffset & entity->paletteMask)) << 12)
                | gObjTilesGetTileFunc(part->gfxFrame ? graphics + paletteOffset : graphics, part->tileX, part->tileY);
            gObjAddToLayerFunc(slot, entity->spriteLayer);
        }
    }
    return 1;
}
