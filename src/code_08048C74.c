#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void CursorSprite_Update(struct EwramData_EntityData *);
extern u8 sMenuCursorGfx[], sMenuCursorGfxLanguage2[], sMenuCursorGfxLanguage3[], sCursorSpritePalette[], sMenuSpriteSheet[];
/**
 * @brief Delete the entity, load the language dependent cursor graphics and
 * palette, attach the cursor animation set and install CursorSprite_Update.
 *
 * @return 1 on success, 0 when the graphics could not be loaded
 */
s32 CursorSprite_Create(struct EwramData_EntityData *entity, u16 palette)
{
    s32 graphics, result;
    EntityDelete(entity);
    switch (gEwramData->gameState.language)
    {
    case 2: graphics = ObjGfx_LoadResource(sMenuCursorGfxLanguage2, 0); break;
    case 3: graphics = ObjGfx_LoadResource(sMenuCursorGfxLanguage3, 0); break;
    default: graphics = ObjGfx_LoadResource(sMenuCursorGfx, 0); break;
    }
    if (graphics >= 0)
    {
        ObjPalette_LoadRange(sCursorSpritePalette, 1, 1, palette);
        Sprite_Attach(entity, sMenuSpriteSheet, graphics, palette);
        entity->animFrame = 0x1E;
        entity->updateFunc = (u32 *)CursorSprite_Update;
        result = 1;
    }
    else result = 0;
    return result;
}
