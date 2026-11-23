/* ARM (IWRAM) twin of Sprite_DrawParts: draw one entity's sprite parts with flips.
 * Built with the bundled agbcc_arm; matched. */
#include "structs/ewram.h"

struct Matrix2 { s32 m[4]; };
struct ObjEntry;
extern const s32 sUnk_080E33FC[3][4][2];
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
            s32 extent = sUnk_080E33FC[shape][size][0];
            dst[0] = origin[0] * 2 - src[0] - extent * 2;
        }
        else
            dst[0] = src[0];
        if (flipY)
        {
            s32 extent = sUnk_080E33FC[shape][size][1];
            dst[1] = origin[1] * 2 - src[1] - extent * 2;
        }
        else
            dst[1] = src[1];
}

/**
 * @brief 4311C | ARM build of Sprite_DrawParts (copied to IWRAM)
 *
 * Builds one OAM object per part of the current frame (entity->animFrame) with
 * the entity's flips, palette, OBJ mode and mosaic, and links them into layer
 * entity->spriteLayer.
 *
 * @param entity Entity with a sprite
 * @return s32 1 if drawn, 0 without a sheet or when out of objects
 */
s32 Sprite_DrawPartsArm(struct EwramData_EntityData *entity)
{
    s32 origin[2];
    s32 position[2];
    s32 graphics;
    s32 i;
    u32 baseAttributes;
    s32 paletteOffset = 0;
    u16 *slot;
    struct SpriteSheet *animation;
    struct SpriteFrame *frame;
    struct SpritePart *part;
    u16 *output;
    s32 shape, size;
    s32 flipX, flipY;
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
        {
            u8 *p = (u8 *)ew + (s16)graphics * 8;
            p += 0x12FF1;
            paletteOffset = *p;
        }
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
    baseAttributes = entity->mosaic << 12;
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
            flipX = entity->flipX;
            flipY = entity->flipY;
            position[0] = part->x << 16;
            position[1] = part->y << 16;
            if (flipX | flipY)
                Mirror(position, position, origin, shape, size, flipX, flipY);
            *output++ = (shape << 14) | baseAttributes | (WHOLE(position[1] + entity->posY.whole) & 0xFF);
            *output++ = (size << 14) | ((flipY ^ (part->flip & 1)) << 13) | (((part->flip & 2) ? flipX ^ 1 : flipX) << 12) | (WHOLE(position[0] + entity->posX.whole) & 0x1FF);
            *output = ((entity->palette + (part->paletteOffset & entity->paletteMask)) << 12)
                | gObjTilesGetTileFunc(part->gfxFrame ? graphics + paletteOffset : graphics, part->tileX, part->tileY);
            gObjAddToLayerFunc(slot, entity->spriteLayer);
        }
    }
    return 1;
}
