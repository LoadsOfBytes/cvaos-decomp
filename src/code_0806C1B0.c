/* Target box tests, decompiled from the original assembly. */
#include "structs/ewram.h"
#include "gba/dma.h"

extern s32 Entity_IsInRect(struct EwramData_EntityData *, s16, s16, u16, u16);

/**
 * @brief Test the selected target in a centered width/height pixel rectangle; the alternate-target flag overrides player selection and a null target returns zero.
 */
s32 Entity_IsTargetInCenteredBox(struct EwramData_EntityData *entity, u16 width, u16 height)
{
    struct EwramData_EntityData *target;
    u16 x;
    u16 y;
    if (entity->enemyFlags & 4)
        target = gEwramData->playerState.player;
    else
        target = gEwramData->playerState.enemyTarget;
    x = entity->posX.part16.integer - (width >> 1);
    y = entity->posY.part16.integer - (height >> 1);
    if (gEwramData->gameState.linkPlay)
        target = gEwramData->playerState.linkTargetProxy;
    if (target == NULL)
        return 0;
    else
        return Entity_IsInRect(target, x, y, width, height);
}

/* Headers for the word-sized blocks in the shared 0x800-byte arena.
 * A free block has tag 0; words excludes the header itself. */
struct EntityHeapBlock {
    u16 tag;
    u16 words;
};

/* Free an allocation, then coalesce the first adjacent run of free blocks. */
void Heap_Free(void *allocation)
{
    struct EntityHeapBlock *block = (struct EntityHeapBlock *)gEwramData->entityScratchBuffer;
    s32 index;
    if (allocation == NULL)
        return;
    ((struct EntityHeapBlock *)allocation)[-1].tag = 0;
    for (index = 0; index <= 510; )
    {
        u16 words = block->words;
        if (block->tag == 0 && words + index <= 511)
        {
            struct EntityHeapBlock *next = block;
            s32 span = 0;
            while (next->tag == 0)
            {
                span += next->words + 1;
                next = block + span;
                if (index + span > 510)
                    break;
            }
            if (span > block->words + 1)
            {
                block->words = span - 1;
                return;
            }
        }
        words++;
        index += words;
        block += words;
    }
}

/* Allocate words from the first free block with room for a new header. */
void *Heap_Alloc(u16 words)
{
    struct EntityHeapBlock *block = (struct EntityHeapBlock *)gEwramData->entityScratchBuffer;
    s32 index = 0;
    s32 limit = 508;
    for (; index <= 510; )
    {
        u16 size = block->words;
        s32 tag = block->tag;
        if (tag == 0 && size >= words + 2)
        {
            void *data;
            s32 bytes;
            s32 end;
            block->tag = index + 1;
            block->words = words;
            end = index + words;
            bytes = words << 2;
            if (end <= limit)
            {
                struct EntityHeapBlock *tail = block + words;
                tail[1].tag = tag;
                tail[1].words = size - words - 1;
            }
            data = block + 1;
            DMA_FILL_32(3, tag, data, bytes);
            return data;
        }
        size++;
        index += size;
        block += size;
    }
    return NULL;
}
