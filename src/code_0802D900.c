/* Create an effect at (x, y) from graphics set 080E13BC[index] with the
 * given frame, priority 13; updated by SoulBoneFragment_Update. */
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
extern void SoulBoneFragment_Update(struct EwramData_EntityData *);

/**
 * @brief Unreferenced: spawn a SoulBoneFragment_Update entity at (x, y) with
 * the graphics set n of sSkillGraphicsSets and the given frame.
 */
struct EwramData_EntityData *Skill_SpawnBoneFragment(s32 x, s32 y, s32 index, s32 frame)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)SoulBoneFragment_Update);
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
    entity->animFrame = frame;
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    entity->spriteLayer = 0xD;
    entity->posX.whole = x;
    entity->posY.whole = y;
    entity->scrollWithBg = 1;
    return entity;
}
