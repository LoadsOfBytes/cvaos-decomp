/* BestiaryMenu_ReleaseResources. */
#include "gba.h"
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void Hit_Disable(struct EwramData_EntityData *entity);

/**
 * @brief 4CCC8 | Release the four palette slots sharedObjPaletteSources/sharedObjPaletteVariants, the
 * graphics slots 9..23 (ObjAffine_Free) and ObjAffine_ResetAll, delete and clear
 * the 16 entities entityData[208..223] (spriteSlot = 0xFF), then
 * PaletteAnim_Stop(10..13). Separate counters per loop and a count-up loop
 * (reversed by agbcc) are needed for the original allocation.
 */
void BestiaryMenu_ReleaseResources(void)
{
    s32 i;
    s32 bg;
    s32 n;
    struct EwramData_EntityData *entity;

    for (i = 0; i <= 3; i++)
    {
        if (gEwramData->gameState.sharedObjPaletteSources[i] != 0)
        {
            gEwramData->gameState.sharedObjPaletteSources[i] = 0;
            gEwramData->gameState.sharedObjPaletteVariants[i] = 0;
        }
    }
    for (bg = 9; bg <= 0x17; bg++)
        ObjAffine_Free(bg);
    ObjAffine_ResetAll();
    entity = &gEwramData->entityData[208];
    for (n = 0; n < 16; n++, entity++)
    {
        if (entity->updateFunc != NULL)
        {
            EntityGetSlot(entity);
            Sprite_Free(entity, 1);
            Hit_Disable(entity);
            DMA_FILL_32(3, 0, entity, sizeof(struct EwramData_EntityData));
            entity->spriteSlot = 0xFF;
        }
    }
    PaletteAnim_Stop(10);
    PaletteAnim_Stop(11);
    PaletteAnim_Stop(12);
    PaletteAnim_Stop(13);
}
