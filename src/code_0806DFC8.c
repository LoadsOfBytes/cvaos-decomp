/* Entity helpers 0x0806DFC8..0x0806E13F. */
#include "gba/dma.h"
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);

/* Start of the 0x800-byte buffer at entityScratchBuffer (two halfwords; 0 and 0x1FF
 * after a reset). */
struct EntityHeapHeader {
    u16 tag;
    u16 words;
};

/**
 * @brief Contact callback: Player_ApplyContactDamage with the entity damage
 * attack at its position.
 */
void Entity_OnPlayerContactDamage(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attributes = 1;
    contactDamage.unused = 0;
    contactDamage.attack = entity->attack;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief 6DFF8 | Create an entity with update function updateFunc in slots
 * 0x1B..0x48, or 0xD0..0xDF when unk_8E bit 6 is set.
 */
struct EwramData_EntityData *Entity_CreateObject(u32 *updateFunc)
{
    struct EwramData_EntityData *child;

    if (!gEwramData->gameState.bestiaryPreview)
        child = EntityCreateInRange(0x1B, 0x48, updateFunc);
    else
        child = EntityCreateInRange(0xD0, 0xDF, updateFunc);
    return child;
}

/**
 * @brief Zero the shared 0x800-byte entity arena and initialize its first block header to tag 0 and word count 0x1FF.
 */
void Heap_Reset(void)
{
    struct EntityHeapHeader *header = (struct EntityHeapHeader *)gEwramData->entityScratchBuffer;

    DMA_FILL_32(3, 0, header, 0x800);
    header->tag = 0;
    header->words = 0x1FF;
}

/**
 * @brief Copy the entire entity arena to its backup and reset only the active first block header to tag 0 and word count 0x1FF.
 */
void Heap_SaveAndReset(void)
{
    struct EntityHeapHeader *header = (struct EntityHeapHeader *)gEwramData->entityScratchBuffer;

    DMA_COPY_32(3, header, gEwramData->entityHeapBackup, 0x800);
    header->tag = 0;
    header->words = 0x1FF;
}

/**
 * @brief Copy the 0x800-byte entity arena from its backup without changing allocator metadata separately.
 */
void Heap_Restore(void)
{
    DMA_COPY_32(3, gEwramData->entityHeapBackup, gEwramData->entityScratchBuffer, 0x800);
}

/**
 * @brief 6E0D0 | Load graphics resource, palette palette (row variant) and
 * animation set sheet for the entity; 0 when a slot is missing.
 */
u8 Entity_LoadGraphics(struct EwramData_EntityData *entity, u8 *resource, void *palette, u8 variant, void *sheet)
{
    s32 gfx = ObjGfx_LoadResource(resource, 0);
    u8 paletteIndex;

    if (gfx < 0)
        return 0;
    paletteIndex = ObjPalette_FindOrLoadVariant(palette, variant);
    if ((s8)paletteIndex < 0)
        return 0;
    entity->basePalette = paletteIndex;
    Sprite_Attach(entity, sheet, gfx, entity->basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief Set horizontal velocity to the supplied value when flipX is set, otherwise to its negation.
 */
void Entity_SetVelocityXFromFacing(struct EwramData_EntityData *entity, s32 velocity)
{
    if (entity->flipX)
        entity->velX.whole = velocity;
    else
        entity->velX.whole = -velocity;
}
