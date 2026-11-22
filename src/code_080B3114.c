/* Falling debris and the short-lived emitter that spawns it. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08060B98.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
struct HeadStackParticleGraphicsView {
    u8 *graphics;
    void *palette;
    void *spriteSheet;
    u32 unused;
};
extern struct HeadStackParticleGraphicsView sUnk_081193DC[];
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, u8 *, void *, u8, void *);
extern void HeadStackEffect_UpdateFallingPiece(struct EwramData_EntityData *);
/* Keep the table index as an inline parameter: agbcc then reproduces
 * the original base-plus-0x10 calculation instead of a relocated pointer. */
static inline u8 LoadDebrisGraphics(struct EwramData_EntityData *e, struct HeadStackParticleGraphicsView *base, s32 idx)
{
    struct HeadStackParticleGraphicsView *g = base + idx;
    return Entity_LoadGraphics(e, g->graphics, g->palette, 0, g->spriteSheet);
}
extern s32 Collision_IsPointSolid(s32, s32);
/**
 * @brief Load variant-1 frame 27, move with acceleration and count down while the collision probe succeeds; delete when it fails or the timer expires.
 */
void HeadStackEffect_UpdateFallingPiece(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg;

    switch (entity->state)
    {
    case 0:
        if (!LoadDebrisGraphics(entity, sUnk_081193DC, 1))
        {
            EntityDelete(entity);
            return;
        }
        entity->animFrame = 0x1B;
        entity->spriteLayer = 9;
        entity->timer = 0x80;
        entity->velY.whole = -0x4000;
        entity->state++;
    case 1:
        EntityAccelerateAndMove(entity, 0x80000);
        {
            s32 x = (s16)entity->posX.part16.integer;
            bg = &gEwramData->bgInfo[1];
            if (Collision_IsPointSolid(x + (s16)bg->xPos.part16.integer,
                    entity->posY.part16.integer + (s16)bg->yPos.part16.integer)
                && entity->timer != 0)
            {
                if (entity->timer == 0x40)
                    entity->drawSkip = 1;
                entity->timer--;
            }
            else
                entity->deleteRequested = 1;
        }
        break;
    }
    if (EntityIsOutsideScreen(entity, 1))
        entity->deleteRequested = 1;
}

/**
 * @brief Play variant-1 animation 6, then hide and emit falling pieces during a 32-tick countdown.
 */
void HeadStackEffect_UpdateEmitter(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
    case 0:
        {
            if (!LoadDebrisGraphics(entity, sUnk_081193DC, 1))
            { EntityDelete(entity); return; }
            entity->palette = 6;
            Anim_StartIndexed(entity, 6, 0, 0);
            entity->spriteLayer = 9;
            entity->state++;
        }
    case 1:
        if (Anim_Update(entity) == 3)
        {
            entity->hidden = 1;
            entity->timer = 0x20;
            entity->state = 2;
        }
        break;
    case 2:
        if (entity->timer != 0)
        {
            if (!(entity->timer & 0xF))
            {
                struct EwramData_EntityData *effect = EntityCreateInRange(0x49, 0xCF, (u32 *)HeadStackEffect_UpdateFallingPiece);
                if (effect != NULL)
                {
                    effect->posX.part16.integer = entity->posX.part16.integer + RandomSigned(0x10);
                    effect->posY.part16.integer = entity->posY.part16.integer + RandomSigned(8) - 0x10;
                    effect->scrollWithBg = 1;
                }
            }
            entity->timer--;
        }
        else entity->deleteRequested = 1;
        break;
    }
}

