/* Two-hand spawn and fade-out phase (Balore area). */
#include "code/code_08039340.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern void Balore_CreateHand(struct EwramData_EntityData *, u16, u16, u8, s32);

/**
 * @brief B569C | Phase of the entity whose hands Balore_CreateHand creates
 * (code next to EnemyBaloreCreate).  State 0/1 counts 0x40 frames down in
 * timer and spawns a hand at (0x40, 0x80) flipped at 0x38 and one at
 * (0x1C0, 0x80) at 0x18; then sound 0x1D4 and state 2.  State 2 sinks by
 * 0x4000 per frame while the palette unk_510 fades (level counter / 8 via
 * ObjPalette_LoadRange, cached in sharedObjPaletteVariants); at the end it clears bit 2 of flags
 * and switches to state = 2.
 */
void Balore_UpdateEnter(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        entity->subState = 1;
        entity->timer = 0x40;
    case 1:
        if (entity->timer != 0)
        {
            if (entity->timer == 0x38)
                Balore_CreateHand(entity, 0x40, 0x80, 1, 0);
            if (entity->timer == 0x18)
                Balore_CreateHand(entity, 0x1C0, 0x80, 0, 0);
            entity->timer--;
        }
        else
        {
            entity->subState = 2;
            entity->timer = 0x40;
            PlaySong(0x1D4);
        }
        break;
    case 2:
        entity->posY.whole += 0x4000;
        if (entity->timer != 0)
        {
            u8 pal, idx;
            u8 level;

            entity->timer--;
            pal = entity->unk_510.part8.basePalette;
            level = *(volatile u8 *)&entity->timer >> 3; /* FAKEMATCH: volatile read forces the reload of the original */
            idx = pal - 10;
            if (gEwramData->gameState.sharedObjPaletteVariants[idx] != level)
            {
                gEwramData->gameState.sharedObjPaletteVariants[idx] = level;
                ObjPalette_LoadRange((u8 *)gEwramData->gameState.sharedObjPaletteSources[idx], level, 1, pal);
            }
        }
        else
        {
            entity->hit.flags &= ~4;
            entity->state = 2;
            entity->subState = 0;
        }
        break;
    }
}
