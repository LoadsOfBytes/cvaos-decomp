/* Copy the parent's current frame parts into this entity's part list (mirrored when unk_4F7 is set) and pick layer 0x1D/0x1E. */
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern struct SpriteSheet sBaloreSpriteSheet;

void BaloreOverlay_UpdateSprite(struct EwramData_EntityData *entity)
{
    s32 count;
    struct EwramData_EntityData *parent = ENTITY_OWNER(entity);
    u8 idx_ = parent->animFrame;
    struct SpriteFrame *frames = sBaloreSpriteSheet.frames;
    struct SpritePart *src = frames[idx_].parts;
    struct SpriteFrame *frame = (struct SpriteFrame *)entity->unk_500.whole;
    struct SpritePart *dst = (struct SpritePart *)entity->unk_504.whole;
    s32 i;
    
    if ((s8)parent->unk_4F4.part8.unk_4F6 <= 3)
    {
        if (entity->unk_4F4.part8.unk_4F7 != 0)
            entity->spriteLayer = 0x1E;
        else
            entity->spriteLayer = 0x1D;
    }
    else
    {
        if (entity->unk_4F4.part8.unk_4F7 != 0)
            entity->spriteLayer = 0x1D;
        else
            entity->spriteLayer = 0x1E;
    }
    count = 0;
    if (entity->unk_4F4.part8.unk_4F7 != 0)
    {
        for (i = frames->partCount - 1; i >= 0; src++, i--)
        {
            if (src->flip != 0)
            {
                *dst = *src;
                dst->x = -dst->x - dst->width;
                dst++;
                count++;
            }
        }
    }
    else
    {
        for (i = frames->partCount - 1; i >= 0; src++, i--)
        {
            if (src->flip == 0)
            {
                *dst = *src;
                dst++;
                count++;
            }
        }
    }
    frame->partCount = count;
}
