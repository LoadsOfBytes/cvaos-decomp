/* MudmanShot_LoadGraphics (Mudman graphics for another entity). */
#include "code/code_08039340.h"
#include "structs/ewram.h"

/* Graphics of Mudman. */
extern u8 sMudmanGfx[];
extern u8 sMudmanSpriteSheet[];

/**
 * @brief 72608 | Give projectile the already loaded Mudman graphics (palette
 * of entity), animation 5; 0 when they are not loaded.
 */
u8 MudmanShot_LoadGraphics(struct EwramData_EntityData *entity, struct EwramData_EntityData *projectile)
{
    s32 gfx = ObjGfx_Find(sMudmanGfx);

    if (gfx < 0)
        return 0;
    Sprite_Attach(projectile, sMudmanSpriteSheet, gfx, entity->unk_510.part8.basePalette);
    Anim_StartIndexed(projectile, 5, 0, 1);
    Anim_Update(projectile);
    projectile->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}
