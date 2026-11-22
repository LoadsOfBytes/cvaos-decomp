/* Debris particle spawned from asm/code/code_080B3114.s. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

/* Graphics of one debris variant (passed to Entity_LoadGraphics). */
struct HeadStackParticleGraphicsView {
    u8 *graphics;
    void *palette;
    void *spriteSheet;
    u32 unk_C;
};

extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, u8 *, void *, u8, void *);
extern struct HeadStackParticleGraphicsView sUnk_081193DC[];

/**
 * @brief Load the variant selected by subState, choose random flips and a 16..31 countdown, then move with acceleration until deletion.
 */
void HeadStackEffect_UpdateParticle(struct EwramData_EntityData *entity)
{
    struct HeadStackParticleGraphicsView *gfx;
    u32 rnd, bit;

    switch (entity->state)
    {
    case 0:
        gfx = &sUnk_081193DC[entity->subState];
        if (!Entity_LoadGraphics(entity, gfx->graphics, gfx->palette, 0, gfx->spriteSheet))
        {
            EntityDelete(entity);
            return;
        }
        if (entity->subState == 0)
            entity->animFrame = 0x14;
        else
            entity->animFrame = 0x1A;
        entity->spriteLayer = 9;
        entity->objMode = 1;
        /* the mask is kept in a register across both random calls */
        rnd = RandomNumberGenerator();
        bit = 1;
        entity->flipX = rnd & bit;
        entity->flipY = RandomNumberGenerator() & bit;
        entity->timer = (RandomNumberGenerator() & 0xF) + 0x10;
        entity->state++;
    case 1:
        EntityAccelerateAndMove(entity, 0x80000);
        if (entity->timer != 0)
            entity->timer--;
        else
            entity->deleteRequested = 1;
        break;
    }
}
