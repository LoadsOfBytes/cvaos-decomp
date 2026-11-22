/* Graphics setup helpers, 0x0802D604..0x0802D723. */
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "macros.h"

/* Keep the graphics context access explicit, matching the original loaders. */
#define GRAPHICS_CONTEXT ((struct PlayerState *)((u8 *)gEwramData + OFFSET_OF(struct EwramData, playerState)))

struct GraphicsSet {
    u8 *graphics;
    void *palette;
    void *sprites;
};

struct GraphicsSetWithPalette {
    struct GraphicsSet set;
    u8 pad_C[2];
    u8 paletteIndex;
    u8 pad_F;
};

extern const struct GraphicsSet sSkillGraphicsSets[];

/**
 * @brief Unreferenced: load red-soul graphics (into redSoulGfx), palette row and
 * attach the sprite with palette 2.
 */
void Skill_LoadGraphics(struct EwramData_EntityData *entity, u8 *resource, void *palette, void *sheet, u8 first)
{
    gEwramData->playerState.redSoulGfx = ObjGfx_LoadResource(resource, 0x10000);
    ObjPalette_LoadRange(palette, first, 1, 2);
    Sprite_Attach(entity, sheet, gEwramData->playerState.redSoulGfx, 2);
}

/**
 * @brief Unreferenced: as Skill_LoadGraphics from a graphics set with palette
 * index.
 */
void Skill_LoadGraphicsFromSet(struct EwramData_EntityData *entity, const struct GraphicsSetWithPalette *def)
{
    u8 paletteIndex = def->paletteIndex;
    u8 *graphics = def->set.graphics;
    void *palette = def->set.palette;
    void *sprites = def->set.sprites;
    GRAPHICS_CONTEXT->redSoulGfx = ObjGfx_LoadResource(graphics, 0x10000);
    ObjPalette_LoadRange(palette, paletteIndex, 1, 2);
    Sprite_Attach(entity, sprites, GRAPHICS_CONTEXT->redSoulGfx, 2);
}

/**
 * @brief Unreferenced: as Skill_LoadGraphics from entry n of sSkillGraphicsSets with
 * the given palette row.
 */
void Skill_LoadGraphicsByIndex(struct EwramData_EntityData *entity, s32 setIndex, s32 paletteSlot)
{
    const struct GraphicsSet *set = &sSkillGraphicsSets[setIndex];
    u8 paletteIndex = paletteSlot;
    u8 *graphics = set->graphics;
    void *palette = set->palette;
    void *sprites = set->sprites;
    GRAPHICS_CONTEXT->redSoulGfx = ObjGfx_LoadResource(graphics, 0x10000);
    ObjPalette_LoadRange(palette, paletteIndex, 1, 2);
    Sprite_Attach(entity, sprites, GRAPHICS_CONTEXT->redSoulGfx, 2);
}
