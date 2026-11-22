/* Create an effect at (x, y) from graphics set 080E13BC[index] (palette
 * slot 0, palette 2) with animation animation, priority 13; updated by
 * RedSoulEffect_UpdateAccelerated. GRAPHICS_CONTEXT as in code_0802D604.c keeps the original
 * reloads of the 0x131CC offset. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "macros.h"
#define GRAPHICS_CONTEXT ((struct PlayerState *)((u8 *)gEwramData + OFFSET_OF(struct EwramData, playerState)))
struct GraphicsSet {
    u8 *graphics;
    void *palette;
    void *sprites;
};
extern const struct GraphicsSet sSkillGraphicsSets[];
extern void RedSoulEffect_UpdateAccelerated(struct EwramData_EntityData *);

/**
 * @brief Unreferenced: spawn a RedSoulEffect_UpdateAccelerated entity at (x, y)
 * with the graphics set n of sSkillGraphicsSets and the given animation.
 */
struct EwramData_EntityData *Skill_SpawnAcceleratedEffect(s32 x, s32 y, s32 index, s32 animation)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)RedSoulEffect_UpdateAccelerated);
    const struct GraphicsSet *set;
    u8 *graphics;
    void *palette, *sprites;

    if (entity == NULL)
        return NULL;
    set = &sSkillGraphicsSets[index];
    graphics = set->graphics;
    palette = set->palette;
    sprites = set->sprites;
    GRAPHICS_CONTEXT->redSoulGfx = ObjGfx_LoadResource(graphics, 0x10000);
    ObjPalette_LoadRange(palette, 0, 1, 2);
    Sprite_Attach(entity, sprites, GRAPHICS_CONTEXT->redSoulGfx, 2);
    Anim_StartIndexed(entity, animation, 0, 0);
    entity->spriteLayer = 0xD;
    entity->posX.whole = x;
    entity->posY.whole = y;
    entity->scrollWithBg = 1;
    return entity;
}
