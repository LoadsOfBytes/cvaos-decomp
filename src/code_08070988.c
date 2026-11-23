/* Load shared Killer Fish graphics and start a part's animation. */
#include "code/code_08039340.h"
extern u8 sKillerFishGfx[], sKillerFishSpriteSheet[];
/**
 * @brief Give part the Killer Fish graphics in the palette of parent, animation 0;
 * return 0 when the graphics are not loaded, else 1.
 */
u8 KillerFishSwarm_LoadGraphics(struct EwramData_EntityData *parent, struct EwramData_EntityData *part)
{
    s32 graphics = ObjGfx_LoadResource(sKillerFishGfx, 0);
    if (graphics < 0) return 0;
    Sprite_Attach(part, sKillerFishSpriteSheet, graphics, parent->basePalette);
    Anim_StartIndexed(part, 0, 0, 1);
    Anim_Update(part);
    part->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}
