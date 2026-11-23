/* Palette fade step. */
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 Entity_SteerToPoint(struct EwramData_EntityData *, void *);
extern u8 sBaloreHandSteeringParams1[];

/**
 * @brief B46FC | Palette fade-in step: state 0 sets unk_514 = 0x40000 and a
 * 0x40-frame counter in timer.  While the counter runs, palette unk_510
 * (slot unk_510 - 10 in sharedObjPaletteVariants/sharedObjPaletteSources) gets brightness level
 * counter / 8 + 8 via ObjPalette_LoadRange whenever the level changes.  Every frame
 * then continues with Entity_SteerToPoint(entity, sBaloreHandSteeringParams1).
 */
s32 BaloreHand_FadeIn(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        entity->unk_514.unk_514_32 = (struct EwramData_EntityData *)0x40000;
        entity->timer = 0x40;
        entity->subState = 1;
    case 1:
        break;
    default:
        return 0;
    }
    if (entity->timer != 0)
    {
        u8 pal, idx;
        u16 level;

        entity->timer--;
        pal = entity->unk_510.part8.basePalette;
        /* the original reloads the decremented counter */
        level = (*(volatile u8 *)&entity->timer >> 3) + 8; /* FAKEMATCH: volatile read forces the reload of the original */
        idx = pal - 10;
        if (gEwramData->gameState.sharedObjPaletteVariants[idx] != level)
        {
            gEwramData->gameState.sharedObjPaletteVariants[idx] = level;
            ObjPalette_LoadRange((u8 *)gEwramData->gameState.sharedObjPaletteSources[idx], level, 1, pal);
        }
    }
    return Entity_SteerToPoint(entity, sBaloreHandSteeringParams1);
}
