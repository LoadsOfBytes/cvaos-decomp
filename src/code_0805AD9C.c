/* Helpers 0x0805AD9C..0x0805AFBF (object 0x37 and screen setup). */
#include "gba.h"
#include "room_object_entity.h"
#include "code_08009A0.h"
#include "code_0800CB00.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);
extern void Object37_UpdateState(struct EwramData_EntityData *entity);

/* Graphics of object 0x37. */
extern u8 sObject37Gfx[];
extern u8 sObject37Palette[];
extern u8 sObject37SpriteSheet[];

/* Screen data of Shop_SetupBackgrounds (per language). */
extern u8 sShopBgPalette[];
extern u8 sShopBgGfx[];
extern u8 sShopBgGfxLanguage3[];
extern u8 sShopBgGfxLanguage3Extra[];
extern u8 sShopBgGfxLanguage2[];
extern u8 sLocalizedWindowGfxLanguage3[];
extern u8 sLocalizedWindowGfxLanguage2[];
extern u8 sLocalizedWindowGfx[];
extern u8 sTextBgPalette[];

/**
 * @brief 5AD9C | Copy subState from the parent (unk_4F8); move unk_4FC toward
 * 0 (subState 0) or -0x2000 (subState 1) by 0xAA per frame and use it as the
 * rotation of ObjAffine_Set.
 */
void Object37Child_Update(struct EwramData_EntityData *entity)
{
    entity->subState = ENTITY_OWNER(entity)->subState;
    switch (entity->subState)
    {
        case 0:
            if ((s32)OBJECT37_CHILD_ANGLE(entity) < 0)
            {
                OBJECT37_CHILD_ANGLE(entity) += 0xAA;
                if ((s32)OBJECT37_CHILD_ANGLE(entity) > 0)
                    OBJECT37_CHILD_ANGLE(entity) = 0;
            }
            break;
        case 1:
            if ((s32)OBJECT37_CHILD_ANGLE(entity) > -0x2000)
            {
                OBJECT37_CHILD_ANGLE(entity) -= 0xAA;
                if ((s32)OBJECT37_CHILD_ANGLE(entity) < -0x2000)
                    OBJECT37_CHILD_ANGLE(entity) = -0x2000;
            }
            break;
    }
    ObjAffine_Set(OBJECT37_CHILD_ANGLE(entity), 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
}

/**
 * @brief 5AE00 | Object 0x37 update.
 */
void Object37Update(struct EwramData_EntityData *entity)
{
    u32 var_0[4];

    if (entity->state == 0)
        Object37_UpdateState(entity);
}

/**
 * @brief 5AE18 | Load the graphics of object 0x37; 0 when no slot is free.
 */
u8 Object37_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sObject37Gfx, 0);

    if (gfx < 0)
        return 0;
    entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sObject37Palette, 0);
    Sprite_Attach(entity, sObject37SpriteSheet, gfx, entity->unk_510.part8.basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief Set up the shop BG layers, palette and language-specific graphics
 * and the text window.
 */
void Shop_SetupBackgrounds(void)
{
    u8 *localizedGfx;

    gUnk_03002CB0.dispCnt &= 0x13FF;
    gDisplayRegisters.bgCnt[0] = 0x1C04;
    gDisplayRegisters.bgCnt[1] = 0x1D01;
    gDisplayRegisters.bgOfs[0].hOfs = 0;
    gDisplayRegisters.bgOfs[0].vOfs = 0;
    BgPalette_Load(sShopBgPalette);
    switch (gEwramData->gameState.language)
    {
        case 2:
            Gfx_CopyResource(sShopBgGfxLanguage2, (void *)VRAM_BASE, 0);
            localizedGfx = sLocalizedWindowGfxLanguage2;
            break;
        case 3:
            Gfx_CopyResource(sShopBgGfxLanguage3, (void *)VRAM_BASE, 0);
            Gfx_CopyResource(sShopBgGfxLanguage3Extra, (void *)(VRAM_BASE + 0x2000), 0);
            localizedGfx = sLocalizedWindowGfxLanguage3;
            break;
        default:
            Gfx_CopyResource(sShopBgGfx, (void *)VRAM_BASE, 0);
            localizedGfx = sLocalizedWindowGfx;
            break;
    }
    Gfx_CopyResource(localizedGfx, (void *)(VRAM_BASE + 0xA000), 0);
    Text_SetTileBuffer((void *)(VRAM_BASE + 0x4000));
    BgPalette_LoadRange(sTextBgPalette, 0, 1, 0xF);
    Text_SetForegroundColor(8);
    Text_SetBackgroundColor(0);
    Text12_SetRegion(0, 0, 0x29, 4);
    Text12_ClearRegion();
    Text12_SetRegion(0, 1, 0x25, 3);
    Text12_SetCursor(0, 1);
}

/**
 * @brief Save the display, palette and BG1 map state, pause the game and
 * enter in-game sub-mode 9 (the shop) at step 0.
 */
void Shop_Open(void)
{
    struct GameState *gameState = &gEwramData->gameState;

    Display_SaveState();
    Palette_SaveState();
    BgMap_SaveSelection(1);
    gEwramData->bgControlMinWriteLevel = gEwramData->unk_A074_4 = 3;
    gDisplayRegisters.bldCnt = 0xFF;
    gEwramData->paused = 1;
    gameState->subMode = 9;
    gameState->subModeStep = 0;
}
