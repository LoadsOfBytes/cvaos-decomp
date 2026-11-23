/* Giant Ghost's graphics, collision effect and death particles. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_080211F0.h"
extern s8 ObjPalette_FindOrLoadVariant(void *, u8);
extern void Effect_UpdateTimedParticle(struct EwramData_EntityData *);
extern u8 sGiantGhostGfx[], sGiantGhostPalette[], sGiantGhostPaletteAnim[], sGiantGhostSpriteSheet[];
extern u8 sCommonObjectGfx[], sCommonObjectSpriteSheet[];
/**
 * @brief Load the Giant Ghost graphics and palette and start its palette
 * animation (sGiantGhostPaletteAnim); return 0 when no graphics slot is free, else 1.
 */
u8 GiantGhost_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 graphics = ObjGfx_LoadResource(sGiantGhostGfx, 0);
    u8 result;
    if (graphics < 0) goto failed;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sGiantGhostPalette, 0);
    PaletteAnim_Start(sGiantGhostPalette, sGiantGhostPaletteAnim, entity->basePalette);
    Sprite_Attach(entity, sGiantGhostSpriteSheet, graphics, entity->basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    result = 1;
    goto finished;
failed:
    result = 0;
finished:
    return result;
}
