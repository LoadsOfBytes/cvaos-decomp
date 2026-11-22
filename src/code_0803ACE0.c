/* Sprite_BuildSingle: single-part variant of Sprite_DrawParts that writes one OAM entry to
 * gEwramData->singleSpriteOam and its offset to singleSpriteX/4; matched. */
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern const s32 sUnk_080E29A0[3][4][2];

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

/**
 * @brief 3ACE0 | Build the OAM template of a one-part sprite
 *
 * Stores the attributes of the first part of the current frame in
 * singleSpriteOam and its offset in singleSpriteX/Y for Sprite_DrawSingle.
 *
 * @param entity Entity with a sprite
 * @return s32 1 if built, 0 without a sheet
 */
s32 Sprite_BuildSingle(struct EwramData_EntityData *entity)
{
    s32 origin[2];
    s32 position[2];
    s32 graphics;
    u32 baseAttributes;
    s32 paletteOffset = 0;
    struct SpriteSheet *animation;
    struct SpriteFrame *frame;
    struct SpritePart *part;
    u16 *output;
    s32 shape, size;
    s32 flipX, flipY;
    struct EwramData *ew;
    s32 id;

    if (entity->spriteSlot >= 0xF0)
        return 0;
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
        paletteOffset = ew->objTileSlots[(s16)graphics].size;
        break;
    }
    if (entity->animFrame < animation->frameCount)
        frame = &animation->frames[entity->animFrame];
    else
        frame = animation->frames;
    part = frame->parts;
    origin[0] = frame->originX << 16;
    origin[1] = frame->originY << 16;
    baseAttributes = (entity->mosaic << 12) | (entity->objMode << 10);
    output = gEwramData->singleSpriteOam;
    shape = part->shape;
    size = part->size;
    flipX = entity->flipX;
    flipY = entity->flipY;
    position[0] = part->x << 16;
    position[1] = part->y << 16;
    if (flipX | flipY)
        Mirror(position, position, origin, shape, size, flipX, flipY);
    gEwramData->singleSpriteX = position[0];
    gEwramData->singleSpriteY = position[1];
    *output++ = (shape << 14) | baseAttributes | (WHOLE(position[1] + entity->posY.whole) & 0xFF);
    *output++ = (size << 14) | ((flipY ^ (part->flip & 1)) << 13) | (((part->flip & 2) ? flipX ^ 1 : flipX) << 12) | (WHOLE(position[0] + entity->posX.whole) & 0x1FF);
    *output = ((entity->palette + (part->paletteOffset & entity->paletteMask)) << 12)
        | ObjTiles_GetTile(part->gfxFrame ? graphics + paletteOffset : graphics, part->tileX, part->tileY);
    return 1;
}
