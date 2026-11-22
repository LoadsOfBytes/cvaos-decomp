/* Damage/status indicator; palette and vertical position depend on kind. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void EntityDamageIndicator_Update(struct EwramData_EntityData *);
extern void NumberIndicator_Format(struct EwramData_EntityData *, s32);
extern void NumberIndicator_Draw(struct EwramData_EntityData *);
extern u8 sCommonObjectGfx[], sCommonObjectGfxLanguage2[], sCommonObjectGfxLanguage3[], sCommonObjectSpriteSheet[];

void Entity_ShowDamage(struct EwramData_EntityData *owner, s32 value, u8 kind)
{
    s32 useFrame = 0;
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)EntityDamageIndicator_Update);
    u8 *sprite;
    s32 graphics, palette;
    entity->state = kind;
    switch (gEwramData->gameState.language)
    {
    case 2: sprite = sCommonObjectGfxLanguage2; break;
    case 3: sprite = sCommonObjectGfxLanguage3; break;
    default: sprite = sCommonObjectGfx; break;
    }
    graphics = ObjGfx_LoadResource(sprite, 0);
    entity->posX.whole = owner->posX.whole;
    entity->posY.whole = owner->posY.whole;
    switch (entity->state)
    {
    case 0:
        palette = 6;
        if (value == 0) useFrame = 1;
        break;
    case 1:
        palette = 7;
        entity->posY.whole -= 0x80000;
        break;
    case 2:
        palette = 9;
        entity->posY.whole -= 0x80000;
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
