/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   HarpyFeather_LoadGraphics = MudmanShot_LoadGraphics */
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u8 sHarpyGfx[];
extern u8 sHarpySpriteSheet[];

/**
 * @brief 763DC | Same as MudmanShot_LoadGraphics.
 */
u8 HarpyFeather_LoadGraphics(struct EwramData_EntityData *entity, struct EwramData_EntityData *projectile)
{
    s32 gfx = ObjGfx_Find(sHarpyGfx);

    if (gfx < 0)
        return 0;
    Sprite_Attach(projectile, sHarpySpriteSheet, gfx, entity->basePalette);
    Anim_StartIndexed(projectile, 5, 0, 1);
    Anim_Update(projectile);
    projectile->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}
