/* Bouncing debris using graphics/palette information from two tables. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
struct DebrisGraphics { void *graphics, *palette, *sprite; };
struct DebrisKind { u16 graphicsIndex; u8 palette; u8 pad[9]; };
extern struct DebrisGraphics sBreakableBlockGraphics[];
extern struct DebrisKind sBreakableBlockVariants[];
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
/**
 * @brief Object 0x09 debris piece: load the graphics of its kind from
 * sBreakableBlockVariants, fly ballistically and delete off screen.
 */
void Object09Debris_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
    case 0:
    {
        struct DebrisKind *kind = &sBreakableBlockVariants[entity->spawnParam0];
        struct DebrisGraphics *graphics = &sBreakableBlockGraphics[kind->graphicsIndex];
        if (!Entity_LoadGraphics(entity, graphics->graphics, graphics->palette, kind->palette, graphics->sprite))
        {
            EntityDelete(entity);
            return;
        }
        entity->animFrame = (RandomNumberGenerator() & 1) + 1;
        entity->velX.whole = ((RandomNumberGenerator() & 0x1FFF) << 4) + 0x20000;
        entity->velY.whole = -0x10000 - ((RandomNumberGenerator() & 0x3FFF) << 4);
        entity->accelY.whole = 0x3000;
        entity->state++;
    }
    /* fall through */
    case 1:
        EntityAccelerateAndMove(entity, 0x80000);
        break;
    }
    if (!Entity_IsInRect(entity, -16, -16, 272, 192)) entity->deleteRequested = 1;
}
