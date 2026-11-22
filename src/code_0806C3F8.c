/* Flipped offsets of sprite parts, decompiled from original assembly. */
#include "structs/ewram.h"

/* Offset to the far corner of a part, counting parts in reverse order. */
void Sprite_GetPartCornerOffset(struct EwramData_EntityData *entity, s32 index, s16 *offset)
{
    u8 *graphics = &entity->spriteSlot;
    struct SpriteSheet *animation = gEwramData->spriteSlots[*graphics].sheet;
    u8 frameIndex = entity->animFrame;
    struct SpriteFrame *frames = animation->frames;
    struct SpriteFrame *frame = (struct SpriteFrame *)(frameIndex * sizeof(*frames) + (u32)frames);
    u8 count = frame->partCount;
    struct SpritePart *part = frame->parts;
    if (index >= count)
        index = count - 1;
    part += count - index - 1;
    if (entity->flipX)
        offset[0] = -part->x - part->width;
    else
        offset[0] = part->x + part->width;
    if (entity->flipY)
        offset[1] = -part->y - part->height;
    else
        offset[1] = part->y + part->height;
}

/* Offset to the centre of a part. */
/**
 * @brief Return the flipped center offset of the reverse-indexed sprite
 * part, clamping indices above the part count to its last index.
 * Preserve the existing behavior for empty frames and negative indices.
 */
void Sprite_GetPartCenterOffset(struct EwramData_EntityData *entity, s32 index, s16 *offset)
{
    u8 *graphics = &entity->spriteSlot;
    struct SpriteSheet *animation = gEwramData->spriteSlots[*graphics].sheet;
    u8 frameIndex = entity->animFrame;
    struct SpriteFrame *frames = animation->frames;
    struct SpriteFrame *frame = (struct SpriteFrame *)(frameIndex * sizeof(*frames) + (u32)frames);
    u8 count = frame->partCount;
    struct SpritePart *part = frame->parts;
    if (index >= count)
        index = count - 1;
    part += count - index - 1;
    if (entity->flipX)
        offset[0] = -part->x - (part->width >> 1);
    else
        offset[0] = part->x + (part->width >> 1);
    if (entity->flipY)
        offset[1] = -part->y - (part->height >> 1);
    else
        offset[1] = part->y + (part->height >> 1);
}

/* Offset to the part origin, with both entity flips applied. */
/**
 * @brief Return the flipped origin offset of the reverse-indexed sprite
 * part, clamping indices above the part count to its last index.
 * Preserve the existing behavior for empty frames and negative indices.
 */
void Sprite_GetPartOriginOffset(struct EwramData_EntityData *entity, s32 index, s16 *offset)
{
    u8 *graphics = &entity->spriteSlot;
    struct SpriteSheet *animation = gEwramData->spriteSlots[*graphics].sheet;
    u8 frameIndex = entity->animFrame;
    struct SpriteFrame *frames = animation->frames;
    struct SpriteFrame *frame = (struct SpriteFrame *)(frameIndex * sizeof(*frames) + (u32)frames);
    u8 count = frame->partCount;
    struct SpritePart *part = frame->parts;
    if (index >= count)
        index = count - 1;
    part += count - index - 1;
    if (entity->flipX)
        offset[0] = -part->x;
    else
        offset[0] = part->x;
    if (entity->flipY)
        offset[1] = -part->y;
    else
        offset[1] = part->y;
}
