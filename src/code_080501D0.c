/* Object09Create (breakable block). */
#include "code_08009A0.h"
#include "room_object_entity.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern u8 Entity_LoadGraphics(struct EwramData_EntityData *entity, void *resource, void *palette, u8 variant, void *sheet);
extern void BreakableBlock_SetCollision(struct EwramData_EntityData *entity);

/* Per variant (see also code_0804FCA0.c): graphics set, palette, frame,
 * area offset / size, layer and mode. */
struct BreakableBlockVariant {
    u16 graphicsIndex; // index into sBreakableBlockGraphics
    u8 paletteIndex; // palette
    u8 animFrame; // animation frame (animFrame)
    s8 collisionOffsetX;
    s8 collisionOffsetY;
    u8 collisionWidth;
    u8 collisionHeight;
    u8 spriteLayer; // layer (spriteLayer)
    u8 collisionMode; // mode (unk_4F8)
    u8 pad_A[0xC - 0xA];
};

struct BreakableBlockGraphics {
    void *graphics; // graphics
    void *palette; // palette
    void *spriteSheet; // animation set
};

extern struct BreakableBlockVariant sBreakableBlockVariants[];
extern struct BreakableBlockGraphics sBreakableBlockGraphics[];

/* Event flag param_1 of the bit array gameFlagsTail (as in Object35Create). */
#define OBJECT_FLAG(ewram, flag) ((ewram)->gameState.gameFlagsTail[(flag) >> 5] & (1 << ((flag) & 0x1F)))

/**
 * @brief 501D0 | Object 0x09 create: deleted when its flag unk_516 is set or
 * the graphics cannot be loaded; otherwise frame / mode from its variant
 * and BreakableBlock_SetCollision.
 */
void Object09Create(struct EwramData_EntityData *entity)
{
    struct EwramData *ewram = gEwramData;
    struct BreakableBlockVariant *def;
    struct BreakableBlockGraphics *gfxDef;

    if (OBJECT_FLAG(ewram, entity->unk_514.part16.spawnParam1))
    {
        EntityDelete(entity);
        return;
    }
    def = &sBreakableBlockVariants[entity->unk_514.part16.spawnParam0];
    gfxDef = &sBreakableBlockGraphics[def->graphicsIndex];
    entity->flipX = 0;
    entity->spriteLayer = def->spriteLayer;
    if (!Entity_LoadGraphics(entity, gfxDef->graphics, gfxDef->palette, def->paletteIndex, gfxDef->spriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    entity->animFrame = def->animFrame;
    OBJECT09_COLLISION_MODE(entity) = def->collisionMode;
    BreakableBlock_SetCollision(entity);
}
