/* Damage/status indicator; palette and vertical position depend on kind. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 Player_GetOverheadY(void);
extern void PlayerDamageIndicator_Update(struct EwramData_EntityData *);
extern void NumberIndicator_Format(struct EwramData_EntityData *, s32);
extern void NumberIndicator_Draw(struct EwramData_EntityData *);
extern u8 sCommonObjectGfx[], sCommonObjectGfxLanguage2[], sCommonObjectGfxLanguage3[], sCommonObjectSpriteSheet[];

/**
 * @brief Create the localized player damage/healing indicator with kind-dependent position and palette; value zero in kind 0 selects frame 0x58.
 */
void Player_ShowDamage(s32 value, u8 kind)
{
    s32 useFrame = 0;
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)PlayerDamageIndicator_Update);
    s32 graphics, palette;
    entity->state = kind;
    switch (gEwramData->gameState.language)
    {
    case 2: graphics = ObjGfx_LoadResource(sCommonObjectGfxLanguage2, 0); break;
    case 3: graphics = ObjGfx_LoadResource(sCommonObjectGfxLanguage3, 0); break;
    default: graphics = ObjGfx_LoadResource(sCommonObjectGfx, 0); break;
    }
    entity->posX.whole = gEwramData->playerState.cachedPlayerX;
    entity->posY.part16.integer = Player_GetOverheadY();
    switch (entity->state)
    {
    case 0:
        palette = 0;
        entity->posY.whole = gEwramData->playerState.player->posY.whole - 0x1A0000;
        if (value == 0) { useFrame = 1; palette = 6; }
        break;
    case 1:
        palette = 7;
        break;
    case 2:
        palette = 8;
        break;
    default: palette = 9; break;
    }
    Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, palette);
    if (useFrame == 0)
    {
        NumberIndicator_Format(entity, value);
        entity->drawFunc = (u32 *)NumberIndicator_Draw;
    }
    else
    {
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->animFrame = 0x58;
    }
    entity->spriteLayer = 2;
    entity->scrollWithBg = 1;
}
