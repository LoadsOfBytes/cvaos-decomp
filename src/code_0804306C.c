#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

/* HUD HP number: the smoothed HUD HP value last shown. Token-identical. */
#define HUD_HP_SHOWN(e) e->unk_510.whole
extern void NumberIndicator_Format(struct EwramData_EntityData *, s32);
extern u8 sCommonObjectGfx[];
extern u8 sCommonObjectSpriteSheet[];
void NumberIndicator_DrawCentered(struct EwramData_EntityData *entity);

/**
 * @brief Set the existing entity flag and reformat the number when the smoothed HUD HP value changes.
 */
void HudHPNumber_Update(struct EwramData_EntityData *entity)
{
    struct EwramData **ewram;
    entity->roomActive = 1;
    ewram = &gEwramData;
    if ((*ewram)->gameState.hudHP != HUD_HP_SHOWN(entity))
    {
        NumberIndicator_Format(entity, (*ewram)->gameState.hudHP);
        HUD_HP_SHOWN(entity) = (*ewram)->gameState.hudHP;
    }
}

/**
 * @brief Create the HUD HP number in slots 0x16..0x19 at pixel position 18,10 using the centered draw callback and cached smoothed HP.
 */
void HudHPNumber_Create(void)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x16, 0x19, (u32 *)HudHPNumber_Update);
    s32 gfx;
    s32 value;

    gfx = ObjGfx_Find(sCommonObjectGfx);
    Sprite_Attach(entity, sCommonObjectSpriteSheet, gfx, 7);
    value = gEwramData->gameState.hudHP;
    HUD_HP_SHOWN(entity) = value;
    NumberIndicator_Format(entity, value);
    entity->drawFunc = (u32 *)NumberIndicator_DrawCentered;
    entity->spriteLayer = 0;
    entity->posX.part16.integer = 0x12;
    entity->posY.part16.integer = 0xA;
}
