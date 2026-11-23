#include "main_menu.h"
#include "core.h"
#include "bg_and_collision.h"
#include "bg_effects.h"
#include "save_data.h"
#include "graphics.h"
#include "gameplay.h"
#include "sound.h"
#include "gba.h"
#include "macros.h"
#include "syscalls.h"

#include "constants/main.h"
#include "constants/songs.h"

#include "data/data_0E0334.h"
#include "data/data_0E3464.h"

#include "structs/agb_sram.h"
#include "structs/ewram.h"
#include "structs/main.h"
#include "math_anim.h"
#include "menu_common.h"
#include "obj_gfx.h"
#include "palette.h"
#include "pause_menu.h"
#include "text12.h"
#include "data/data_0E5364.h"

// TODO: HudMap_WriteTiles is implicitly called in other files, except this one
extern void HudMap_WriteTiles(s32 x, s32 y, u16 tile, s32 count);

/**
 * @brief Unreferenced: enable window 0 over (x1..x2, y1..y2) with a pulsing
 * brightness increase outside it.
 */
void MainMenu_SetHighlightWindow(u8 arg0, u8 arg1, u8 arg2, u8 arg3)
{
    gUnk_03002CB0.dispCnt |= DCNT_WIN0;
    gDisplayRegisters.bldCnt = BLDCNT_SCREEN_SECOND_TARGET | BLDCNT_BRIGHTNESS_INCREASE_EFFECT | BLDCNT_SCREEN_FIRST_TARGET;

    gDisplayRegisters.winIn_L = WIN0_ALL;
    gDisplayRegisters.winOut_L = WIN0_ALL_NO_COLOR_EFFECT;

    gDisplayRegisters.win0H = C_16_2_8(arg0, arg1);
    gDisplayRegisters.win0V = C_16_2_8(arg2, arg3);

    gDisplayRegisters.bldY = (Sin(gEwramData->frameCounter << 0xA) >> 0xE) + 4;
    if (gDisplayRegisters.bldY < 2)
    {
        gDisplayRegisters.bldY = 2;
    }
}

/**
 * @brief Clear the 12-pixel text region and, for a nonnegative id, print the
 * localized text entry.
 */
void MainMenu_PrintMessage(s32 entry)
{
    Text12_ClearRegion();
    if (entry >= 0)
    {
        Text12_Print(Text_GetLocalizedEntry(entry), 0);
    }
}

struct SaveMenuSpriteResources {
    u32 *spriteSheet; // type?
    u32 *graphics; // type?
    u32 paletteIndex; // type?
};
// extern struct SaveMenuSpriteResources sSaveMenuSpriteResources[18];

const struct SaveMenuSpriteResources sSaveMenuSpriteResources[] = {
    [0] = {
        .spriteSheet = (u32*)0x082571E0,
        .graphics = (u32*)0x082052D8,
        .paletteIndex = 1
    },
    [1] = {
        .spriteSheet = (u32*)0x082571E0,
        .graphics = (u32*)0x082052D8,
        .paletteIndex = 2
    },
    [2] = {
        .spriteSheet = (u32*)0x08259120,
        .graphics = (u32*)0x082052E0,
        .paletteIndex = 7
    },
    [3] = {
        .spriteSheet = (u32*)0x08267A38,
        .graphics = (u32*)0x080E5BC0,
        .paletteIndex = 5
    },
    [4] = {
        .spriteSheet = (u32*)0x08267A38,
        .graphics = (u32*)0x080E5BC0,
        .paletteIndex = 15
    },
    [5] = {
        .spriteSheet = (u32*)0x082571E0,
        .graphics = (u32*)0x082052D8,
        .paletteIndex = 14
    },
    [6] = {
        .spriteSheet = (u32*)0x082571E0,
        .graphics = (u32*)0x0827798C,
        .paletteIndex = 1
    },
    [7] = {
        .spriteSheet = (u32*)0x082571E0,
        .graphics = (u32*)0x0827798C,
        .paletteIndex = 2
    },
    [8] = {
        .spriteSheet = (u32*)0x08259120,
        .graphics = (u32*)0x0827AA14,
        .paletteIndex = 7
    },
    [9] = {
        .spriteSheet = (u32*)0x08267A38,
        .graphics = (u32*)0x08277994,
        .paletteIndex = 5
    },
    [10] = {
        .spriteSheet = (u32*)0x08267A38,
        .graphics = (u32*)0x08277994,
        .paletteIndex = 15
    },
    [11] = {
        .spriteSheet = (u32*)0x082571E0,
        .graphics = (u32*)0x0827798C,
        .paletteIndex = 14
    },
    [12] = {
        .spriteSheet = (u32*)0x082571E0,
        .graphics = (u32*)0x0826C840,
        .paletteIndex = 1
    },
    [13] = {
        .spriteSheet = (u32*)0x082571E0,
        .graphics = (u32*)0x0826C840,
        .paletteIndex = 2
    },
    [14] = {
        .spriteSheet = (u32*)0x08259120,
        .graphics = (u32*)0x0827AA0C,
        .paletteIndex = 7
    },
    [15] = {
        .spriteSheet = (u32*)0x08267A38,
        .graphics = (u32*)0x0826C850,
        .paletteIndex = 5
    },
    [16] = {
        .spriteSheet = (u32*)0x08267A38,
        .graphics = (u32*)0x0826C850,
        .paletteIndex = 15
    },
    [17] = {
        .spriteSheet = (u32*)0x082571E0,
        .graphics = (u32*)0x0826C840,
        .paletteIndex = 14
    },
};

static inline void SaveMenu_PlaceNameSprites(struct EwramData_EntityData *entity, s32 x, s32 y, u8 *name)
{
    s32 i;

    entity->posX.part16.integer = x;
    entity->posY.part16.integer = y;

    for (i = 0; i < 8; i++)
    {
        if (*name >= 0x2 && *name <= 0x22)
        {
            entity->animFrame = *name;
            Sprite_Draw(entity);
        }
        name += 1;
        entity->posX.part16.integer += 0xC;
    }
}

static inline void SaveMenu_AttachLanguageSprite(struct EwramData_EntityData *entity, s32 sprite)
{
    struct SaveMenuSpriteResources table[18];

    memcpy(table, sSaveMenuSpriteResources, sizeof(sSaveMenuSpriteResources));
    switch (gEwramData->gameState.language)
    {
        case 2:
            sprite = sprite + 6;
            break;
        case 3:
            sprite = sprite + 12;
            break;
    }
    Sprite_Free(entity, 0);
    Sprite_Attach(entity, (u8*)table[sprite].spriteSheet, ObjGfx_Find((u8*)table[sprite].graphics), table[sprite].paletteIndex);
}

/* The frame counter is unsigned; phase is the shifted intermediate.
 * Keeping them separate preserves agbcc's load and shift registers. */
static inline s32 SaveMenuCursorOffset(u32 frame)
{
    s32 offset;
    s32 phase = frame >> 1;
    if (phase & 8)
        offset = -((phase & 7) - 3);
    else
        offset = (phase & 7) - 3;
    return offset;
}

/**
 * @brief Draw the save slot list with the cursor and slot summaries for the
 * game start (state 2) and data delete (state 5) screens.
 */
void MainMenu_DrawSlotList(struct EwramData_EntityData *entity)
{
    struct SaveHeader *menuData;
    s32 cursorOffset;
    s32 slot;
    s32 copy;
    s32 selectedSlot;
    
    menuData = &gEwramData->saveHeaders[0];
    if (entity->subState == 0)
    {
        MainMenu_DrawSelectionMarkers(entity, 8, 0x10);
    }
    else
    {
        MainMenu_DrawSelectionMarkers(entity, 8, 0x18);
    }
    SaveMenu_AttachLanguageSprite(entity, 0);
    entity->animFrame = 0x10;

    if (entity->stage == 0)
    {
        if (entity->ext.mainMenu.confirmOpen != 0)
        {
            cursorOffset = SaveMenuCursorOffset(gEwramData->frameCounter);
            entity->posX.part16.integer = (entity->ext.mainMenu.confirmCursor * 0x60) + 0x24 + cursorOffset;
            entity->posY.part16.integer = 0x8C;
            Sprite_Draw(entity);
            if (gEwramData->frameCounter & 2)
            {
                if (entity->ext.mainMenu.cursor < 6)
                {
                    entity->posX.part16.integer = ((entity->ext.mainMenu.cursor * 0x10) + 0x10);
                    entity->posY.part16.integer = 0x48;
                    if (entity->ext.mainMenu.cursor & 1)
                    {
                        entity->posX.part16.integer -= 0x10;
                        entity->posY.part16.integer += 0x18;
                    }
                }
                else
                {
                    entity->posX.part16.integer = (entity->ext.mainMenu.cursor * 0x10) - 0x50;
                    entity->posY.part16.integer = 0x30;
                }
                Sprite_Draw(entity);
            }
        }
        else {
            if (entity->ext.mainMenu.cursor < 6)
            {
                cursorOffset = SaveMenuCursorOffset(gEwramData->frameCounter);
                entity->posX.part16.integer = (entity->ext.mainMenu.cursor * 0x10) + 0x10 + cursorOffset;
                entity->posY.part16.integer = 0x48;
                if (entity->ext.mainMenu.cursor & 1)
                {
                    entity->posX.part16.integer -= 0x10;
                    entity->posY.part16.integer += 0x18;
                }
            }
            else
            {
                cursorOffset = SaveMenuCursorOffset(gEwramData->frameCounter);
                entity->posX.part16.integer = (entity->ext.mainMenu.cursor * 0x10) - 0x50 + cursorOffset;
                entity->posY.part16.integer = 0x30;
            }
            Sprite_Draw(entity);
        }
    }
    else
    {
        cursorOffset = SaveMenuCursorOffset(gEwramData->frameCounter);
        entity->posX.part16.integer = ((entity->ext.mainMenu.cursor * 0x40) + 0x46) + cursorOffset;
        entity->posY.part16.integer = 0x90;
        Sprite_Draw(entity);

        if (gEwramData->frameCounter & 2)
        {
            entity->posX.part16.integer = (u16) ((entity->ext.mainMenu.copySource * 0x10) + 0x10);
            entity->posY.part16.integer = 0x48;
            Sprite_Draw(entity);
        }
    }

    if (entity->ext.mainMenu.confirmOpen == 0)
    {
        SaveMenu_AttachLanguageSprite(entity, 2);
        SaveMenu_PlaceNameSprites(entity, 0x80, 0x20, menuData->name);
        
    }
    SaveMenu_AttachLanguageSprite(entity, 1);

    for (slot = 0; slot < 3; slot++)
    {
        for (copy = 0; copy < 2; copy++)
        {
            if (copy != 1 || entity->subState != 2)
            {
                if (copy == 0)
                {
                    SaveData_ReadSlotHeader(slot * 2, 1);
                    if (SaveSummary_GetLinkedSlot(menuData + 1) != 0)
                    {
                        entity->animFrame = slot + 3;
                    }
                    else
                    {
                        entity->animFrame = slot;
                    }
                    entity->posX.part16.integer = (slot << 5) + 0x10;
                    entity->posY.part16.integer = 0x40;
                    Sprite_Draw(entity);
                    if (menuData->playTime == 0)
                    {
                        entity->posX.part16.integer = 0x90;
                        entity->posY.part16.integer = 0x38;
                        if (entity->subState == 2)
                        {
                            entity->animFrame = 9;
                        }
                        else
                        {
                            entity->animFrame = 0xA;
                        }
                        Sprite_Draw(entity);
                    }
                    else if (entity->ext.mainMenu.cursor > 5)
                    {
                        entity->posX.part16.integer = 0x90;
                        entity->posY.part16.integer = 0x38;
                        entity->animFrame = 0xA;
                        Sprite_Draw(entity);
                    }
                    if ((entity->subState != 2) && (gEwramData->saveHeaders[1].clearAndDifficulty & 0xF))
                    {
                        entity->animFrame = 8;
                        entity->posX.part16.integer = (slot << 5) + 0x10;
                        entity->posY.part16.integer = 0x28;
                        Sprite_Draw(entity);
                        entity->animFrame = 6;
                        entity->posX.part16.integer = (slot << 5) + 0x18;
                        entity->posY.part16.integer = 0x38;
                        Sprite_Draw(entity);
                    }
                    if (entity->stage == 0)
                    {
                        selectedSlot = entity->ext.mainMenu.cursor;
                    }
                    else
                    {
                        selectedSlot = entity->ext.mainMenu.copySource;
                    }
                    if ((menuData->clearAndDifficulty & 0xF) && (menuData->playTime != 0) && !(selectedSlot & 1) && (selectedSlot < 6))
                    {
                        entity->animFrame = 0xB;
                        entity->posX.part16.integer = 0x80;
                        entity->posY.part16.integer = 0x60;
                        Sprite_Draw(entity);
                    }
                }
                else
                {
                    SaveData_ReadSlotHeader((slot * 2) + copy, 1);
                    if (gEwramData->saveHeaders[1].playTime != 0)
                    {
                        entity->animFrame = 6;
                        entity->posX.part16.integer = (slot << 5) + 0x18;
                        entity->posY.part16.integer = 0x50;
                        Sprite_Draw(entity);
                        entity->animFrame = 7;
                        entity->posX.part16.integer = (slot << 5) + 0x10;
                        entity->posY.part16.integer = 0x58;
                        Sprite_Draw(entity);
                    }
                }
            }
        }
    }
}

/**
 * @brief Draw the save slot list of the data copy screen (state 4) with the
 * source and destination cursors.
 */
void MainMenu_DrawCopySlotList(struct EwramData_EntityData *entity)
{
    struct SaveHeader unk_20_0;
    struct SaveHeader *header;
    s32 bob;
    u32 phase;
    s32 i;

    header = &gEwramData->saveHeaders[0];
    MainMenu_DrawSelectionMarkers(entity, 8, 0x10);
    SaveMenu_AttachLanguageSprite(entity, 0);
    entity->animFrame = 0x10;
    if (entity->stage != 2)
    {
        phase = gEwramData->frameCounter >> 1;
        if (phase & 8)
        {
            bob = -((phase & 7) - 3);
        }
        else
        {
            bob = (phase & 7) - 3;
        }
        entity->posX.part16.integer = (entity->ext.mainMenu.cursor * 0x10) + 0x88 + bob;
        entity->posY.part16.integer = 0x10;
        Sprite_Draw(entity);
    }
    else
    {
        phase = gEwramData->frameCounter >> 1;
        if (phase & 8)
        {
            bob = -((phase & 7) - 3);
        }
        else
        {
            bob = (phase & 7) - 3;
        }
        entity->posX.part16.integer = (entity->ext.mainMenu.cursor << 6) + 0x46 + bob;
        entity->posY.part16.integer = 0x90;
        Sprite_Draw(entity);
        if (gEwramData->frameCounter & 2)
        {
            entity->posX.part16.integer = (entity->ext.mainMenu.copyDest * 0x10) + 0x88;
            entity->posY.part16.integer = 0x10;
            Sprite_Draw(entity);
        }
    }
    SaveMenu_AttachLanguageSprite(entity, 2);
    SaveMenu_PlaceNameSprites(entity, 0x8, 0x20, header->name);
    if ((header->clearAndDifficulty & 0xF) && (header->playTime != 0))
    {
        SaveMenu_AttachLanguageSprite(entity, 1);
        entity->animFrame = 0xB;
        entity->posX.part16.integer = 0x8;
        entity->posY.part16.integer = 0x60;
        Sprite_Draw(entity);
    }

    SaveMenu_AttachLanguageSprite(entity, 1);
    for (i = 0; i <= 2; i += 1)
    {
        unk_20_0 = *header;
        SaveData_ReadSlotHeader(i * 2, 0);
        if (SaveSummary_GetLinkedSlot(header) != 0)
        {
            entity->animFrame = i + 3;
        }
        else
        {
            entity->animFrame = i;
        }
        entity->posX.part16.integer = i * 0x20 + 0x88;
        entity->posY.part16.integer = 8;
        Sprite_Draw(entity);
        *header = unk_20_0;
    }

    header++;
    if (entity->ext.mainMenu.slotEmpty == 0)
    {
        SaveMenu_AttachLanguageSprite(entity, 2);
        SaveMenu_PlaceNameSprites(entity, 0x88, 0x20, header->name);

        if ((header->clearAndDifficulty & 0xF))
        {
            if (header->playTime != 0)
            {
                SaveMenu_AttachLanguageSprite(entity, 1);
                entity->animFrame = 0xB;
                entity->posX.part16.integer = 0x88;
                entity->posY.part16.integer = 0x60;
                Sprite_Draw(entity);
            }
        }
    }
    else
    {
        if (entity->stage != 0)
        {
            SaveMenu_AttachLanguageSprite(entity, 1);
            entity->posX.part16.integer = 0x98;
            entity->posY.part16.integer = 0x38;
            entity->animFrame = 9;
            Sprite_Draw(entity);
        }
    }
}

/**
 * @brief Draw the name entry screen (state 6): frame, blinking text cursor,
 * keyboard cursor, the entered name and the 9x4 character grid.
 */
void MainMenu_DrawNameEntry(struct EwramData_EntityData *entity)
{
    struct SaveHeader *header;
    s32 i;
    s32 j;
    s32 glyph;

    header = gEwramData->saveHeaders;
    SaveMenu_AttachLanguageSprite(entity, 5);

    entity->animFrame = entity->ext.mainMenu.baseFrame;
    Anim_Update(entity);

    entity->posX.part16.integer = 0x37;
    entity->posY.part16.integer = 0x18;
    Sprite_Draw(entity);

    entity->posX.part16.integer += 0x82;
    Sprite_Draw(entity);
    SaveMenu_AttachLanguageSprite(entity, 2);

    if (gEwramData->frameCounter & 0x10)
    {
        entity->posX.part16.integer = (entity->ext.mainMenu.nameCursor * 0xC) + 0x48;
        entity->posY.part16.integer = 0x10;
        entity->animFrame = 0x23;
        Sprite_Draw(entity);
    }

    SaveMenu_AttachLanguageSprite(entity, 2);
    entity->posX.part16.integer = (Mod(entity->ext.mainMenu.cursor, 9) * 0x18) + 0x18;
    entity->posY.part16.integer = (Div(entity->ext.mainMenu.cursor, 9) * 0x18) + 0x40;
    entity->animFrame = 1;

    Sprite_Draw(entity);
    SaveMenu_PlaceNameSprites(entity, 0x48, 0x10, header->name);
    for (i = 0; i < 4; i++)
    {
        entity->posY.part16.integer = (i * 0x18) + 0x38;
        for (j = 0; j < 9; j++)
        {
            entity->posX.part16.integer = j * 0x18 + 0x12;
            glyph = (i * 9) + j + 2;
            if (glyph != 0x23)
            {
                entity->animFrame = glyph;
                Sprite_Draw(entity);
            }
        }
    }
}

/**
 * @brief Draw the save select screen (state 7): the slot cursor and the slot
 * markers, including linked slots and the soul trade variant.
 */
void SaveSelect_Draw(struct EwramData_EntityData *entity)
{
    struct SaveHeader *header;
    s32 offset;
    s32 i;
    s32 j;
    u32 phase;

    header = gEwramData->saveHeaders;
    MainMenu_DrawSelectionMarkers(entity, 8, 0x10);
    SaveMenu_AttachLanguageSprite(entity, 0);
    entity->animFrame = 0x10;
    phase = gEwramData->frameCounter >> 1;
    if (phase & 8)
    {
        offset = -((phase & 7) - 3);
    }
    else
    {
        offset = (phase & 7) - 3;
    }
    entity->posX.part16.integer = (u16) ((entity->ext.mainMenu.cursor * 0x10) + 0x10 + offset);
    entity->posY.part16.integer = 0x48;
    Sprite_Draw(entity);
    SaveMenu_AttachLanguageSprite(entity, 2);
    SaveMenu_PlaceNameSprites(entity, 0x80, 0x20, header->name);
    if ((header->clearAndDifficulty & 0xF) && (header->playTime != 0))
    {
        if (!(entity->ext.mainMenu.cursor & 1) && (entity->ext.mainMenu.cursor < 6))
        {
            SaveMenu_AttachLanguageSprite(entity, 1);
            entity->animFrame = 0xB;
            entity->posX.part16.integer = 0x80;
            entity->posY.part16.integer = 0x60;
            Sprite_Draw(entity);
        }
    }

    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 2; j++)
        {
            if (j == 0)
            {
                SaveMenu_AttachLanguageSprite(entity, 1);
                SaveData_ReadSlotHeader(i * 2, 1);
                
                if (SaveSummary_GetLinkedSlot(header + 1) != 0)
                {
                    offset = 3;
                }
                else
                {
                    offset = 0;
                }
                entity->animFrame = offset + i;
                entity->posX.part16.integer = (i << 5) + 0x10;
                entity->posY.part16.integer = 0x40;
                Sprite_Draw(entity);
                if (header->playTime == 0)
                {
                    SaveMenu_AttachLanguageSprite(entity, 1);
                    entity->posX.part16.integer = 0x90;
                    entity->posY.part16.integer = 0x38;
                    entity->animFrame = 9;
                    Sprite_Draw(entity);
                }
            }
            else
            {
                if (gEwramData->gameMode == GAME_MODE_SOUL_TRADE_MENU)
                {
                    SaveData_ReadSlotHeader((i * 2) + j, 1);
                    if (gEwramData->saveHeaders[1].playTime != 0)
                    {
                        entity->animFrame = 6;
                        entity->posX.part16.integer = (i << 5) + 0x18;
                        entity->posY.part16.integer = 0x50;
                        Sprite_Draw(entity);
                        entity->animFrame = 7;
                        entity->posX.part16.integer = (i << 5) + 0x10;
                        entity->posY.part16.integer = 0x58;
                        Sprite_Draw(entity);
                    }
                }
            }
        }
    }
}

/**
 * @brief Draw the animated selection marker at (x - 1, y) and again 0x62
 * pixels to the right.
 */
void MainMenu_DrawSelectionMarkers(struct EwramData_EntityData *entity, s16 x, s16 y)
{
    SaveMenu_AttachLanguageSprite(entity, 5);
    entity->animFrame = entity->ext.mainMenu.baseFrame;
    Anim_Update(entity);
    entity->posX.part16.integer = x - 1;
    entity->posY.part16.integer = y;
    Sprite_Draw(entity);
    entity->posX.part16.integer += 0x62;;
    Sprite_Draw(entity);
}

/**
 * @brief Draw callback: in state 1 draw the three menu entries with the
 * selected one highlighted and its markers; in later states delegate
 * to the slot and submenu draw helpers.
 */
void MainMenu_Draw(struct EwramData_EntityData *entity)
{
    s32 i;

    SaveMenu_AttachLanguageSprite(entity, 0);
    entity->animFrame = 0x10;
    switch (entity->state - 1)
    {
        case 0:
            for (i = 0; i < 3; i++)
            {
                if (entity->subState == i)
                {
                    SaveMenu_AttachLanguageSprite(entity, 3);
                }
                else
                {
                    SaveMenu_AttachLanguageSprite(entity, 4);
                }
                entity->posX.part16.integer = i * 0x30 + 0x20;
                entity->posY.part16.integer = (i << 5) + 0x10;
                entity->animFrame = i;
                Sprite_Draw(entity);
                if (entity->subState == i)
                {
                    MainMenu_DrawSelectionMarkers(entity, (i * 0x300000 + 0x180000) >> 0x10, (i << 5) + 0x18);
                }
            }
            break;

        case 1:
        case 4:
            MainMenu_DrawSlotList(entity);
            break;

        case 3:
            MainMenu_DrawCopySlotList(entity);
            break;

        case 5:
            MainMenu_DrawNameEntry(entity);
            break;

        case 6:
            SaveSelect_Draw(entity);
            break;
    }
}

/**
 * @brief Load the language-specific menu graphics and palette, attach the
 * menu sprite with draw callback MainMenu_Draw and record in unk_4F5 a
 * bit for each of the six save slots that holds data.
 */
void MainMenu_InitEntity(struct EwramData_EntityData *entity)
{
    s32 gfx;
    s32 i;
    struct SaveHeader *header;

    EntityDelete(entity);
    switch (gEwramData->gameState.language)
    {
        default:
            ObjGfx_LoadResource((u8 *)0x080E5BC0, 0);
            break;
        case 2:
            ObjGfx_LoadResource((u8 *)0x08277994, 0);
            break;
        case 3:
            ObjGfx_LoadResource((u8 *)0x0826C850, 0);
            break;
    }
    switch (gEwramData->gameState.language)
    {
        default:
            ObjGfx_LoadResource((u8 *)0x082052E0, 0);
            break;
        case 2:
            ObjGfx_LoadResource((u8 *)0x0827AA14, 0);
            break;
        case 3:
            ObjGfx_LoadResource((u8 *)0x0827AA0C, 0);
            break;
    }
    switch (gEwramData->gameState.language)
    {
        default:
            gfx = ObjGfx_LoadResource((u8 *)0x082052D8, 0);
            break;
        case 2:
            gfx = ObjGfx_LoadResource((u8 *)0x0827798C, 0);
            break;
        case 3:
            gfx = ObjGfx_LoadResource((u8 *)0x0826C840, 0);
            break;
    }
    entity->paletteMask = 0xFF;
    ObjPalette_Load((u8 *)0x080E5E24);
    Sprite_Attach(entity, (u8 *)0x082571E0, gfx, 0);
    entity->animFrame = 0;
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    entity->ext.mainMenu.baseFrame = entity->animFrame;
    entity->drawFunc = (u32 *) MainMenu_Draw;
    header = gEwramData->saveHeaders;

    for (i = 0; i < 6; i++)
    {
        SaveData_ReadSlotHeader(i, 0);
        if (header->playTime != 0)
        {
            entity->ext.mainMenu.savedSlots |= 1 << i;
        }
    }
    entity->updateFunc = (u32 *) EntityUpdateNothing;
    entity->state = 1;
}

/**
 * @brief Unload the language-specific menu graphics and request deletion
 * of the menu entity.
 */
void MainMenu_FreeEntity(struct EwramData_EntityData *entity)
{
    switch (gEwramData->gameState.language)
    {
        case 2:
            ObjGfx_Unload(ObjGfx_Find((u8 *)0x0827798C));
            ObjGfx_Unload(ObjGfx_Find((u8 *)0x08277994));
            ObjGfx_Unload(ObjGfx_Find((u8 *)0x0827AA14));
            break;
        case 3:
            ObjGfx_Unload(ObjGfx_Find((u8 *)0x0826C840));
            ObjGfx_Unload(ObjGfx_Find((u8 *)0x0826C850));
            ObjGfx_Unload(ObjGfx_Find((u8 *)0x0827AA0C));
            break;
        default:
            ObjGfx_Unload(ObjGfx_Find((u8 *)0x082052D8));
            ObjGfx_Unload(ObjGfx_Find((u8 *)0x080E5BC0));
            ObjGfx_Unload(ObjGfx_Find((u8 *)0x082052E0));
            break;
    }
    entity->deleteRequested = 1;
}

const u32 sUnk_080E0D98[0xD] = {
    0xD, 0x10, 0x22, 0x23, 0x29, 0x24, 0x25, 0x34, 0x33, 0x35, 0x36, 0x37, 0x16
};

static inline void MainMenu_PrintPlayTime(s32 time, s32 x, s32 y)
{
    HudMap_PrintNumberAttr(x + 0xB, y + 4, Div(0x64 * Mod(time, 0x3C), 0x3C), 2, 0xD100, 1);
    HudMap_PrintNumberAttr(x + 8, y + 4, Mod(Div(time, 0x3C), 0x3C), 2, 0xD100, 1);
    HudMap_PrintNumberAttr(x + 5, y + 4, Div(time, 0xE10), 2, 0xD100, 1);
}

static inline s32 SaveHeader_GetDisplayArea(struct SaveHeader *header)
{
    s32 i;
    s32 area;
    u32 rooms[0xD];

    memcpy(rooms, sUnk_080E0D98, 0x34);
    area = header->area;
    if (area == 7)
    {
        for (i = 0; i < 0xD; i++)
        {
            if (header->room == rooms[i])
            {
                area = 0xC;
                break;
            }
        }
    }
    return area;
}

/**
 * @brief Draw a save-slot summary (header buffer) at the given tile position
 * into the given text buffer.
 */
void MainMenu_DrawSlotSummary(struct EwramData_EntityData *entity, struct SaveHeader *header, s32 tileX, s32 tileY, s32 panel)
{
    s32 seconds;
    s32 minutes;
    s32 i;
    s32 showClear;
    s32 areaIndex;
    s32 area;
    s32 frames;
    s32 y;
    s32 completion;
    s32 completionX;
    s32 x;

    HudMap_PrintNumberAttr(tileX + 3, tileY, (header->level < 0x64) ? header->level : 0, 2, 0xD100, 0);
    completion = header->mapCompletionTenths;
    completionX = tileX + 9;
    HudMap_PrintNumberAttr(completionX, tileY, Mod(completion, 0xA), 3, 0xD100, 0);
    HudMap_PrintNumberAttr(completionX - 2, tileY, Div(completion, 0xA), 1, 0xD100, 0);

    frames = header->playTime;
    x = tileX + 0xB;
    y = tileY + 1;
    seconds = Div(frames, 0x3C);
    HudMap_PrintNumberAttr(x, y, Mod(seconds, 0x3C), 2, 0xD100, 1);
    minutes = Div(seconds, 0x3C);
    HudMap_PrintNumberAttr(tileX + 8, tileY + 1, Mod(minutes, 0x3C), 2, 0xD100, 1);
    HudMap_PrintNumberAttr(tileX + 5, tileY + 1, Div(minutes, 0x3C), 2, 0xD100, 1);
    HudMap_PrintNumberAttr(tileX + 9, tileY + 2, header->gold, 6, 0xD100, 0);

    switch (gEwramData->gameState.language)
    {
        case 2:
            HudMap_WriteTiles(tileX, tileY - 1, (panel << 0x5) + 0xC0C0, 0xC);
            Text8_SetRegion(0, panel + 6, 0x1E, 1);
            Text8_ClearRegion();
            area = SaveHeader_GetDisplayArea(header);
            Text8_Print((struct unk_08506B38 *) Text_GetLocalizedEntry(area + 0x3B7), 0);
            break;

        case 3:
            if (SaveHeader_GetDisplayArea(header) == 7 || SaveHeader_GetDisplayArea(header) == 0xC)
            {
                HudMap_WriteTiles(tileX, tileY - 2, (panel << 5) - 0x3F40, 8);
                HudMap_WriteTiles(tileX + 6, tileY - 1, (panel << 5) - 0x3F38, 0xC);
            }
            else
            {
                HudMap_WriteTiles(tileX, tileY - 1, (panel << 5) - 0x3F40, 0xC);
            }

            Text8_SetRegion(0, panel + 6, 0x1E, 1);
            Text8_ClearRegion();
            area = SaveHeader_GetDisplayArea(header);
            Text8_Print((struct unk_08506B38 *) Text_GetLocalizedEntry(area + 0x3B7), 0);
            break;

        default:
            switch (header->area)
            {
                case 7:
                case 12:
                    HudMap_WriteTiles(tileX, tileY - 2, (panel << 5) - 0x3F40, 7);
                    HudMap_WriteTiles(tileX + 4, tileY - 1, (panel << 5) - 0x3F39, 0xC);
                    break;
                default:
                    HudMap_WriteTiles(tileX, tileY - 1, (panel << 0x5) + 0xC0C0, 0xC);
                    break;
            }

            Text8_SetRegion(0, panel + 6, 0x1E, 1);
            Text8_ClearRegion();
            areaIndex = SaveHeader_GetDisplayArea(header);
            Text8_Print((struct unk_08506B38 *) Text_GetLocalizedEntry(areaIndex + 0x3B7), 0);
            break;
    }

    showClear = 0;
    if (entity->subState == 0)
    {
        if ((header->clearAndDifficulty & 0xF) && (header->playTime != 0))
        {
            if (!(entity->ext.mainMenu.cursor & 1) && (entity->ext.mainMenu.cursor < 6))
            {
                showClear = 1;
            }
        }
    }
    else if ((header->clearAndDifficulty & 0xF) && (header->playTime != 0))
    {
        showClear = 1;
    }
    if (showClear != 0)
    {
        for (i = 0; i < 8; i++)
        {
            if ((i == 2) || (i == 5))
            {
                WRITE_32(0x0600A200 + i * 4, 0x10000);
            }
            else
            {
                WRITE_32(0x0600A200 + i * 4, 0);
            }
        }
        MainMenu_PrintPlayTime(header->bestBossRushTimeFrames, tileX, tileY);
        HudMap_WriteTiles(tileX + 9, tileY + 4, 0xD110U, 1);
        HudMap_WriteTiles(tileX + 6, tileY + 4, 0xD110U, 1);
    }
}

/**
 * @brief Select save slot n, read its header and draw its summary when it
 * holds data; record whether the slot is empty for the menu logic and
 * update the slot cursor display.
 */
void MainMenu_ShowSlotInfo(struct EwramData_EntityData *entity, s32 slot)
{
    struct SaveHeader *header;
    struct GameState *gameState;
    s16 *dst;
    s32 scroll;
    s32 valid;
    s32 i;
    s32 index;
    s32 arg;

    gameState = &gEwramData->gameState;
    header = gEwramData->saveHeaders;
    gEwramData->gameState.currentSaveSlot = (u8) slot;
    valid = SaveData_ReadSlotHeader(slot, 0);
    DMA_FILL_32(3, 0, 0x0600E000, 0x340);

    if ((valid != 0) && (header->playTime != 0))
    {
        if (entity->ext.mainMenu.cursor < 6)
        {
            MainMenu_DrawSlotSummary(entity, header, 0x10, 8, 0);
            entity->ext.mainMenu.slotEmpty = 0;
            gameState->unk_4CC_3 = 2;
        }
    }
    else
    {
        entity->ext.mainMenu.slotEmpty = 1;
        gameState->unk_4CC_3 = 1;
    }

    if (entity->ext.mainMenu.cursor > 5)
    {
        if (header->clearAndDifficulty & 0xF)
        {
            entity->ext.mainMenu.slotEmpty = 1;
            gameState->unk_4CC_3 = 1;
        }
    }

    if (entity->subState == 0)
    {
        Text12_SetCursor(0, 1);
        dst = &gEwramData->hBlankEffect.hBlankBuffer[1 - gEwramData->hBlankEffect.currentBuffer][0x18];
        
        if (entity->ext.mainMenu.slotEmpty != 0)
        {
            scroll = 0x88;
        }
        else
        {
            scroll = 0;
        }

        for (i = 0; i < 0x50; i++)
        {
            *dst++ = scroll;
        }
        *dst = 0;
        gEwramData->hBlankEffect.requestStart = 1;

        if (entity->ext.mainMenu.cursor < 6)
        {
            if (entity->ext.mainMenu.cursor & 1)
            {
                MenuText_ClearAndPrint(0x381);
                return;
            }
        }
        else
        {
            if (entity->ext.mainMenu.cursor == 6)
            {
                arg = 1;
            }
            else if (entity->ext.mainMenu.cursor == 8)
            {
                arg = 2;
            }
            else
            {
                arg = 3;
            }
            MenuText_PrintWithInsertedEntry(0x38B, arg);
            return;
        }

        if (entity->ext.mainMenu.slotEmpty != 0)
        {
            index = 0x35B;
        }
        else
        {
            index = 0x35C;
        }
        MenuText_ClearAndPrint(index);
    }
}

/**
 * @brief Draw the summary of the source slot and, once a destination is
 * being chosen, of the destination slot; mark whether it is empty and
 * fill the highlight scanline buffer accordingly.
 */
void MainMenu_ShowCopySlots(struct EwramData_EntityData *entity, s32 srcSlot, s32 dstSlot)
{
    struct SaveHeader *header;
    s16 *dst;
    s16 scroll;
    s32 i;

    header = gEwramData->saveHeaders;
    DMA_FILL_32(3, 0, 0x0600E000, 0x340);
    SaveData_ReadSlotHeader(srcSlot, 0);
    MainMenu_DrawSlotSummary(entity, header, 1, 8, 0);

    if (entity->stage != 0)
    {
        SaveData_ReadSlotHeader(dstSlot, 1);
        header++;
        if (header->playTime != 0)
        {
            MainMenu_DrawSlotSummary(entity, header, 0x11, 8, 1);
            entity->ext.mainMenu.slotEmpty = 0;
        }
        else
        {
            entity->ext.mainMenu.slotEmpty = 1;
        }
    }
    else
    {
        entity->ext.mainMenu.slotEmpty = 1;
    }

    dst = &gEwramData->hBlankEffect.hBlankBuffer[1 - gEwramData->hBlankEffect.currentBuffer][0x18];
    if (entity->ext.mainMenu.slotEmpty == 0)
    {
        scroll = 0x88;
    }
    else
    {
        scroll = 0;
    }

    for (i = 0; i < 0x40; i++)
    {
        *dst++ = scroll;
    }
    gEwramData->hBlankEffect.requestStart = 1;
}

/**
 * @brief Init entry 0 of sMainMenuSubmenuInitFuncs: read the current slot header and
 * show the slot information for the game-start submenu.
 */
void MainMenu_InitGameStart(struct EwramData_EntityData *entity)
{
    BgMap_Setup(1, (struct BgMapMetadata *)0x080E7314, 0, 0);
    BgMap_Setup(2, (struct BgMapMetadata *)0x080E7044, 0, 0);
    HBlankEffectSetup(160, (s32)(SCREEN_SIZE_Y * 0.65f), 2, REG_BG1VOFS);

    entity->ext.mainMenu.cursor = 0;
    if ((SaveData_ReadSlotHeader(1, 1) != 0) && (gEwramData->saveHeaders[1].playTime != 0))
    {
        entity->ext.mainMenu.cursor = 1;
    }

    MainMenu_ShowSlotInfo(entity, (s32) entity->ext.mainMenu.cursor);
    entity->state = 2;
    entity->ext.mainMenu.blinkPhase = 0;
    entity->ext.mainMenu.blinkTimer = 0;
}

/**
 * @brief Update entry 0 of sMainMenuSubmenuUpdateFuncs: choose a slot, then load it or
 * start a new game (Game_ResetProgressForNewGame). Return -1 to go
 * back, 1 or a negative mode code to leave the main menu.
 */
s32 MainMenu_UpdateGameStart(struct EwramData_EntityData *entity)
{
    s32 result;
    s32 repeatedInput;
    s32 saveSlot;
    s32 backSlot;
    s32 shownSlot;
    s32 cursor;
    u8 blinkTimer;
    struct GameState *gameState;

    repeatedInput = gEwramData->inputData.repeatedInput;
    cursor = entity->ext.mainMenu.cursor;
    result = 0;
    if (entity->ext.mainMenu.confirmOpen != 0)
    {
        cursor = entity->ext.mainMenu.confirmCursor;
        if (repeatedInput & KEY_B)
        {
            backSlot = entity->ext.mainMenu.cursor;
            if (backSlot > 5)
            {
                backSlot -= 6;
            }
            PlaySong(SE_240);
            MainMenu_ShowSlotInfo(entity, backSlot);
            entity->ext.mainMenu.confirmOpen = 0;
            entity->ext.mainMenu.confirmCursor = 0;
        }
        else if (repeatedInput & KEY_A)
        {
            if (cursor != 0)
            {
                gEwramData->gameState.difficultyMode = 1;
            }
            else
            {
                gEwramData->gameState.difficultyMode = 0;
            }
            
            if (entity->ext.mainMenu.cursor < 6)
            {
                result = 1;
            }
            else
            {
                gEwramData->gameState.roomData = GetRoomPointer(0, 0);
                Game_ResetProgressForNewGame();
                result = -3;
            }
            entity->ext.mainMenu.confirmOpen = 0;
        }
        else if (repeatedInput & KEY_RIGHT)
        {
            cursor = 1;
        }
        else if (repeatedInput & KEY_LEFT)
        {
            cursor = 0;
        }
        if (entity->ext.mainMenu.confirmCursor != cursor)
        {
            entity->ext.mainMenu.confirmCursor = cursor;
            PlaySong(SE_241);
        }
    }
    else
    {
        if (repeatedInput & KEY_B)
        {
            result = -1;
        }
        else if (repeatedInput & KEY_A)
        {
            gameState = &gEwramData->gameState;

            if (cursor >= 6)
            {
                saveSlot = cursor - 6;
            }
            else
            {
                saveSlot = cursor;
            }
            SaveData_LoadSlotFromSram(saveSlot);
            if (entity->ext.mainMenu.slotEmpty != 0)
            {
                gEwramData->gameState.roomData = NULL;
                if (gameState->systemFlags & 2)
                {
                    entity->ext.mainMenu.confirmOpen = 1;
                }
                else if ((entity->ext.mainMenu.cursor > 5) && (gameState->systemFlags & 1))
                {
                    gEwramData->gameState.roomData = GetRoomPointer(0, 0);
                    Game_ResetProgressForNewGame();
                    result = -3;
                }
                else
                {
                    result = 1;
                }
            }
            else if (entity->ext.mainMenu.cursor < 6)
            {
                if (cursor & 1)
                {
                    SaveData_ClearSlotInSram(cursor);
                    entity->ext.mainMenu.savedSlots &= ~(1 << cursor);
                    gEwramData->gameState.currentSaveSlot = cursor - 1;
                }
                gEwramData->gameState.roomData = GetRoomPointer(gEwramData->gameState.currentArea, gEwramData->gameState.currentRoom);
                result = -2;
            }
            else
            {
                gEwramData->gameState.roomData = GetRoomPointer(0, 0);
                Game_ResetProgressForNewGame();
                entity->ext.mainMenu.confirmOpen = 1;
            }
        }
        else if (repeatedInput & KEY_RIGHT)
        {
            cursor = Mod(cursor + 2, 6);
        }
        else if (repeatedInput & KEY_LEFT)
        {
            cursor = Mod(cursor + 4, 6);
        }
        else if (repeatedInput & KEY_DOWN)
        {
            if (cursor > 5)
            {
                cursor -= 6;
            }
            else if (!(cursor & 1))
            {
                cursor += 1;
            }
        }
        else if (repeatedInput & KEY_UP)
        {
            if (cursor & 1)
            {
                cursor -= 1;
            }
            else if (cursor < 6)
            {
                cursor += 6;
            }
        }

        if (cursor < 6)
        {
            if (!(cursor & 1))
            {
                if ((entity->ext.mainMenu.cursor != (cursor + 1)) && (entity->ext.mainMenu.cursor != cursor) && (SaveData_ReadSlotHeader(cursor + 1, 1) != 0) && (gEwramData->saveHeaders[1].playTime != 0))
                {
                    cursor = cursor + 1;
                }
            }
            if (cursor < 6)
            {
                if ((cursor & 1) && ((SaveData_ReadSlotHeader(cursor, 1) == 0) || (gEwramData->saveHeaders[1].playTime == 0)))
                {
                    cursor -= 1;
                }
            }
        }
        if (cursor >= 6)
        {
            if (!(cursor & 1) && ((SaveData_ReadSlotHeader(cursor - 6, 1) == 0) || !(gEwramData->saveHeaders[1].clearAndDifficulty & 0xF)))
            {
                cursor = entity->ext.mainMenu.cursor;
            }
        }
        if (entity->ext.mainMenu.cursor != cursor)
        {
            entity->ext.mainMenu.cursor = cursor;
            shownSlot = entity->ext.mainMenu.cursor;
            if (shownSlot > 5)
            {
                shownSlot -= 6;
            }
            PlaySong(SE_241);
            MainMenu_ShowSlotInfo(entity, shownSlot);
            entity->ext.mainMenu.blinkPhase = 1;
            entity->ext.mainMenu.blinkTimer = 0;
        }
        else if (entity->ext.mainMenu.confirmOpen != 0)
        {
            Text12_SetCursor(0, 1);
            MenuText_ClearAndPrint(0x385);
            PlaySong(SE_243);
        }
        if ((gEwramData->gameState.systemFlags & 1) && (entity->ext.mainMenu.slotEmpty != 0) && (entity->ext.mainMenu.cursor < 6))
        {
            blinkTimer = entity->ext.mainMenu.blinkTimer++;
            if (!(blinkTimer & 0x7F))
            {
                if (entity->ext.mainMenu.blinkPhase != 0)
                {
                    Text12_SetCursor(0, 1);
                    MenuText_ClearAndPrint(0x3B0);
                }
                else
                {
                    MainMenu_PrintMessage(0x35B);
                }
                entity->ext.mainMenu.blinkPhase = 1 - entity->ext.mainMenu.blinkPhase;
            }
        }
    }
    return result;
}

/**
 * @brief Init entry 1 of sMainMenuSubmenuInitFuncs: show the source/destination slot
 * information for the data-copy submenu.
 */
void MainMenu_InitDataCopy(struct EwramData_EntityData *entity)
{
    s16 *dst;
    s16 scroll;
    s32 k;
    s32 i;
    s32 j;

    entity->ext.mainMenu.copyDest = 0;
    entity->ext.mainMenu.copySource = 0;
    for (i = 0; i < 6; i += 2)
    {
        if ((entity->ext.mainMenu.savedSlots >> i) & 1)
        {
            entity->ext.mainMenu.copySource = i;
            break;
        }
    }
    for (j = 0; j < 6; j += 2)
    {
        if (!((entity->ext.mainMenu.savedSlots >> j) & 1))
        {
            entity->ext.mainMenu.copyDest = j;
            break;
        }
    }
    entity->ext.mainMenu.cursor = entity->ext.mainMenu.copySource;
    BgMap_Setup(1, (struct BgMapMetadata*)&sMainMenuCopyBg1Map, 0, 0);
    BgMap_Setup(2, (struct BgMapMetadata*)&sMainMenuCopyBg2Map, 0, 0);
    HBlankEffectSetup(160, (s32)(SCREEN_SIZE_Y * 0.95f), 2, REG_BG1VOFS);
    MainMenu_ShowCopySlots(entity, entity->ext.mainMenu.copySource, entity->ext.mainMenu.copyDest);
    MenuText_ClearAndPrint(0x358);
    entity->state = 4;
    entity->stage = 0;
    dst = &gEwramData->hBlankEffect.hBlankBuffer[1 - gEwramData->hBlankEffect.currentBuffer][0x18];
    if (entity->ext.mainMenu.slotEmpty == 0)
    {
        scroll = 0x88;
    }
    else
    {
        scroll = 0;
    }
    for (k = 0; k < 0x40; k++)
    {
        *dst++ = scroll;
    }
    gEwramData->hBlankEffect.requestStart = 1;
}

/**
 * @brief Update entry 1 of sMainMenuSubmenuUpdateFuncs: choose source and destination
 * slots, confirm with messages and copy with SaveData_CopySlot.
 */
s32 MainMenu_UpdateDataCopy(struct EwramData_EntityData *entity)
{
    s16 *dst;
    s16 scroll;
    s32 next;
    s32 next2;
    s32 i;
    s32 result;
    u32 repeatedInput;
    s32 cursor;

    cursor = entity->ext.mainMenu.cursor;
    repeatedInput = gEwramData->inputData.repeatedInput;
    result = 0;
    switch (entity->stage)
    {
        case 0:
            if (repeatedInput & KEY_A)
            {
                PlaySong(SE_243);
                MenuText_ClearAndPrint(0x359);
                if (cursor == entity->ext.mainMenu.copyDest)
                {
                    entity->ext.mainMenu.copyDest = Mod(cursor + 2, 6);
                }
                cursor = entity->ext.mainMenu.copyDest;
                entity->ext.mainMenu.cursor = 0xFE;
                entity->stage += 1;
            }
            else if (repeatedInput & KEY_B)
            {
                result = -1;
            }
            else if (repeatedInput & (KEY_LEFT | KEY_RIGHT))
            {
                do
                {
                    if (repeatedInput & KEY_RIGHT)
                    {
                        next = cursor + 2;
                    }
                    else
                    {
                        next = cursor + 4;
                    }
                    cursor = Mod(next, 6);
                } while (!((entity->ext.mainMenu.savedSlots >> cursor) & 1));
            }
            break;

        case 1:
            if (repeatedInput & KEY_A)
            {
                PlaySong(SE_243);
                MainMenu_PrintMessage(-1);
                entity->ext.mainMenu.cursor = 1;
                cursor = 1;
                entity->stage += 1;
                Text12_ClearRegion();
                Text12_SetCursor(0, 1);
                Text12_Print(Text_GetLocalizedEntry(0x379), 0);
                Text12_SetCursor(9, 2);
                Text12_Print(Text_GetLocalizedEntry(0x34B), 0);
                Text12_SetCursor(0x14, 2);
                Text12_Print(Text_GetLocalizedEntry(0x34D), 0);
                Text12_SetCursor(0, 1);
            }
            else if (repeatedInput & KEY_B)
            {
                PlaySong(SE_240);
                MainMenu_PrintMessage(0x358);
                cursor = entity->ext.mainMenu.cursor = entity->ext.mainMenu.copySource;
                entity->stage -= 1;
                MainMenu_ShowCopySlots(entity, entity->ext.mainMenu.copySource, entity->ext.mainMenu.copyDest);
            }
            else if (repeatedInput & (KEY_LEFT | KEY_RIGHT))
            {
                do
                {
                    if (repeatedInput & KEY_RIGHT)
                    {
                        next2 = cursor + 2;
                    }
                    else
                    {
                        next2 = cursor + 4;
                    }
                    cursor = Mod(next2, 6);
                } while (cursor == entity->ext.mainMenu.copySource);
            }
            break;

        case 2:
            if ((repeatedInput & KEY_A) && (cursor == 0))
            {
                PlaySong(SE_244);
                SaveData_CopySlot(entity->ext.mainMenu.copySource, entity->ext.mainMenu.copyDest);
                entity->ext.mainMenu.savedSlots |= 1 << entity->ext.mainMenu.copyDest;
                MainMenu_PrintMessage(0x358);
                cursor = entity->ext.mainMenu.cursor = entity->ext.mainMenu.copySource;
                MainMenu_ShowCopySlots(entity, entity->ext.mainMenu.copySource, entity->ext.mainMenu.copyDest);
                entity->stage = 0;
                SaveData_ClearSlotInSram(entity->ext.mainMenu.copyDest + 1);
                entity->ext.mainMenu.savedSlots &= ~(1 << (entity->ext.mainMenu.copyDest + 1));
            }
            else if (repeatedInput & (KEY_A | KEY_B))
            {
                PlaySong(SE_240);
                MainMenu_PrintMessage(0x359);
                cursor = entity->ext.mainMenu.cursor = entity->ext.mainMenu.copyDest;
                entity->stage -= 1;
            }
            else if (repeatedInput & (KEY_LEFT | KEY_RIGHT))
            {
                cursor = (repeatedInput >> 4) & 1; // repeatedInput & KEY_RIGHT
            }
            break;
    }

    if (entity->ext.mainMenu.cursor != cursor)
    {
        entity->ext.mainMenu.cursor = cursor;
        PlaySong(SE_241);
        if (entity->stage == 0)
        {
            entity->ext.mainMenu.copySource = cursor;
        }
        else if (entity->stage == 1)
        {
            entity->ext.mainMenu.copyDest = cursor;
        }
        MainMenu_ShowCopySlots(entity, entity->ext.mainMenu.copySource, entity->ext.mainMenu.copyDest);
    }
    dst = &gEwramData->hBlankEffect.hBlankBuffer[1 - gEwramData->hBlankEffect.currentBuffer][0x18];
    if (entity->ext.mainMenu.slotEmpty == 0)
    {
        scroll = 0x88;
    }
    else
    {
        scroll = 0;
    }
    for (i = 0; i < 0x40; i++)
    {
        *dst++ = scroll;
    }
    gEwramData->hBlankEffect.requestStart = 1;
    return result;
}

/**
 * @brief Update entry 2 of sMainMenuSubmenuUpdateFuncs: choose a slot, confirm with
 * messages and delete it with SaveData_DeleteSlot.
 */
s32 MainMenu_UpdateDataDelete(struct EwramData_EntityData *entity)
{
    s16 *dst;
    s16 scroll;
    s32 next;
    s32 i;
    s32 result;
    u32 repeatedInput;
    s32 cursor;

    repeatedInput = gEwramData->inputData.repeatedInput;
    cursor = entity->ext.mainMenu.cursor;
    result = 0;
    switch (entity->stage)
    {
        case 0:
            if (repeatedInput & KEY_B)
            {
                result = -1;
            }
            else if (repeatedInput & KEY_A)
            {
                if (entity->ext.mainMenu.slotEmpty != 0)
                {
                    PlaySong(SE_239);
                }
                else
                {
                    PlaySong(SE_243);
                    MainMenu_PrintMessage(-1);
                    entity->ext.mainMenu.copySource = entity->ext.mainMenu.cursor;
                    entity->ext.mainMenu.cursor = 1;
                    cursor = 1;
                    entity->stage += 1;
                    Text12_ClearRegion();
                    Text12_SetCursor(0, 1);
                    Text12_Print(Text_GetLocalizedEntry(0x377), 0);
                    Text12_SetCursor(0xA, 2);
                    Text12_Print(Text_GetLocalizedEntry(0x34B), 0);
                    Text12_SetCursor(0x15, 2);
                    Text12_Print(Text_GetLocalizedEntry(0x34D), 0);
                    Text12_SetCursor(0, 1);
                }
            }
            else if (repeatedInput & (KEY_LEFT | KEY_RIGHT))
            {
                if (repeatedInput & KEY_RIGHT)
                {
                    next = cursor + 2;
                }
                else
                {
                    next = cursor + 4;
                }
                cursor = Mod(next, 6);
            }
            break;

        case 1:
            if ((repeatedInput & KEY_A) && (cursor == 0))
            {
                PlaySong(SE_244);
                if (!(entity->ext.mainMenu.copySource & 1))
                {
                    SaveData_DeleteSlot(entity->ext.mainMenu.copySource);
                }
                else
                {
                    SaveData_ClearSlotInSram(entity->ext.mainMenu.copySource);
                }
                entity->ext.mainMenu.savedSlots &= ~(1 << entity->ext.mainMenu.copySource);
                MenuText_ClearAndPrint(0x35A);
                cursor = entity->ext.mainMenu.cursor = entity->ext.mainMenu.copySource;
                MainMenu_ShowSlotInfo(entity, entity->ext.mainMenu.copySource);
                entity->stage -= 1;
                if (!(entity->ext.mainMenu.copySource & 1))
                {
                    SaveData_ClearSlotInSram(entity->ext.mainMenu.copySource + 1);
                    entity->ext.mainMenu.savedSlots &= ~(1 << (entity->ext.mainMenu.copySource + 1));
                }
            }
            else if (repeatedInput & (KEY_A | KEY_B))
            {
                PlaySong(SE_240);
                MenuText_ClearAndPrint(0x35A);
                cursor = entity->ext.mainMenu.cursor = entity->ext.mainMenu.copySource;
                entity->stage -= 1;
            }
            else if (repeatedInput & (KEY_LEFT | KEY_RIGHT))
            {
                cursor = (repeatedInput >> 4) & 1; // repeatedInput & KEY_RIGHT
            }
            break;
    }
    if (entity->ext.mainMenu.cursor != cursor)
    {
        entity->ext.mainMenu.cursor = cursor;
        PlaySong(SE_241);
        if (entity->stage == 0)
        {
            MainMenu_ShowSlotInfo(entity, entity->ext.mainMenu.cursor);
        }
    }
    dst = &gEwramData->hBlankEffect.hBlankBuffer[1 - gEwramData->hBlankEffect.currentBuffer][0x18];
    if (entity->ext.mainMenu.slotEmpty != 0)
    {
        scroll = 0x88;
    }
    else
    {
        scroll = 0;
    }
    for (i = 0; i < 0x50; i++)
    {
        *dst++ = scroll;
    }
    gEwramData->hBlankEffect.requestStart = 1;
    return result;
}

/**
 * @brief Select save slot n, draw its summary or mark it empty and print the
 * matching prompt (the soul trade menu checks the paired slot).
 */
void SaveSelect_ShowSlot(struct EwramData_EntityData *entity, s32 slot)
{
    s16 *dst;
    s16 scroll;
    s32 valid;
    s32 i;
    s32 message;
    s32 message2;
    struct SaveHeader *header;
    struct GameState *gameState;

    gameState = &gEwramData->gameState;
    header = gEwramData->saveHeaders;
    gEwramData->gameState.currentSaveSlot = slot;
    valid = SaveData_ReadSlotHeader(slot, 0);
    DMA_FILL_32(3, 0, 0x0600E000, 0x340);
    if ((valid != 0) && (header->playTime != 0))
    {
        MainMenu_DrawSlotSummary(entity, header, 0x10, 8, 0);
        entity->ext.mainMenu.slotEmpty = 0;
        gameState->unk_4CC_3 = 2;
    }
    else
    {
        entity->ext.mainMenu.slotEmpty = 1;
        gameState->unk_4CC_3 = 1;
    }
    if (entity->subState == 0)
    {
        Text12_SetCursor(0, 1);
        dst = &gEwramData->hBlankEffect.hBlankBuffer[1 - gEwramData->hBlankEffect.currentBuffer][0x18];
        
        if (entity->ext.mainMenu.slotEmpty != 0)
        {
            scroll = 0x88;
        }
        else
        {
            scroll = 0;
        }
        for (i = 0; i < 0x50; i++)
        {
            *dst++ = scroll;
        }
        *dst = 0;
        gEwramData->hBlankEffect.requestStart = 1;
        if (gEwramData->gameMode == GAME_MODE_SOUL_TRADE_MENU)
        {
            valid = SaveData_ReadSlotHeader(slot + 1, 1);
            if ((header[1].playTime != 0) && (valid != 0))
            {
                MenuText_ClearAndPrint(0x38F);
                return;
            }
            if (entity->ext.mainMenu.slotEmpty != 0)
            {
                message = 0x37B;
            }
            else
            {
                message = 0x37C;
            }
            MenuText_ClearAndPrint(message);
            return;
        }
        
        if (entity->ext.mainMenu.slotEmpty != 0)
        {
            message2 = 0x37B;
        }
        else
        {
            message2 = 0x37C;
        }
        MenuText_ClearAndPrint(message2);
    }
}

/**
 * @brief Set up the BG0 text layer and window and print message 0x354 +
 * index.
 */
void SaveSelect_SetupMessageWindow(s32 choice)
{
    s16 *tilemap;
    s32 i;

    tilemap = (s16 *)0x0600E3C4;
    DMA_FILL_32(3, 0, 0x0600E000, 0x800);
    DMA_FILL_32(3, 0, 0x06008000, 0x2000);
    gDisplayRegisters.bgCnt[0] = CREATE_BGCNT(2, 28, BGCNT_HIGH_PRIORITY, BGCNT_SIZE_256x256);
    BgPalette_LoadRange((u8*)0x081183F4, 0, 1, 0xC);
    Text_SetTileBuffer(VRAM_BASE + 0x8000);
    Text_SetForegroundColor(8);
    Text_SetBackgroundColor(0);
    BgPalette_LoadRange((u8*)0x080E5C20, 3, 1, 0xD);
    switch (gEwramData->gameState.language)
    {
        case 2:
            Gfx_CopyResource((u8 *)0x08277984, VRAM_BASE + 0xA000, 0);
            break;
        case 3:
            Gfx_CopyResource((u8 *)0x0826C848, VRAM_BASE + 0xA000, 0);
            break;
        default:
            Gfx_CopyResource((u8 *)0x0827B200, VRAM_BASE + 0xA000, 0);
            break;
    }
    Text12_SetRegion(0, 0, 0x29, 3);
    Text12_ClearRegion();
    Text12_SetRegion(0, 1, 0x25, 3);
    Text12_SetCursor(0, 1);
    for (i = 0; i < 0x80; i++)
    {
        *tilemap++ = i - 0x3FE0;
    }
    Text12_Print(Text_GetLocalizedEntry(choice + 0x354), 0);
}

/**
 * @brief Clear the BG maps and display registers, load the menu backgrounds,
 * init the menu entity and show slot 0.
 */
void SaveSelect_InitScreen(struct EwramData_EntityData *entity)
{

    DMA_FILL_32(3, 0, 0x0600E000, 0x200 * 4);
    DMA_FILL_32(3, 0, 0x0600E800, 0x200 * 4);
    DMA_FILL_32(3, 0, 0x0600F000, 0x200 * 4);
    DMA_FILL_32(3, 0, 0x0600F800, 0x200 * 4);
    DMA_FILL_32(3, 0, &gDisplayRegisters, sizeof(gDisplayRegisters));

    gDisplayRegisters.bg3PD = 0x100;
    gDisplayRegisters.bg3PA = 0x100;
    gDisplayRegisters.bg2PD = 0x100;
    gDisplayRegisters.bg2PA = 0x100;

    gDisplayRegisters.bgCnt[0] = CREATE_BGCNT(0, 28, BGCNT_HIGH_PRIORITY, BGCNT_SIZE_256x256);
    gDisplayRegisters.bgCnt[1] = CREATE_BGCNT(0, 29, BGCNT_HIGH_MID_PRIORITY, BGCNT_SIZE_256x256);
    gDisplayRegisters.bgCnt[2] = CREATE_BGCNT(0, 30, BGCNT_LOW_MID_PRIORITY, BGCNT_SIZE_256x256);
    gDisplayRegisters.bgCnt[3] = CREATE_BGCNT(0, 31, BGCNT_LOW_PRIORITY, BGCNT_SIZE_256x256);

    gEwramData->bgControlMinWriteLevel = gEwramData->unusedWriteLevel = 0;
    gUnk_03002CB0.dispCnt = DCNT_OBJ | DCNT_BG3 | DCNT_BG2 | DCNT_BG1 | DCNT_BG0;
    gDisplayRegisters.bldCnt = BLDCNT_BG3_SECOND_TARGET_PIXEL | BLDCNT_ALPHA_BLENDING_EFFECT | BLDCNT_BG2_FIRST_TARGET_PIXEL;
    gDisplayRegisters.bldAlpha = C_16_2_8(10, 6);
    gDisplayRegisters.bldY = 0;
    gEwramData->gameState.bossRushMode = 0;

    Gfx_CopyResource((u8 *)0x080E5BB0, VRAM_BASE, 0);
    Gfx_CopyResource((u8 *)0x080E5BB8, VRAM_BASE + 0x2000, 0);

    switch (gEwramData->gameState.language)
    {
        case 2:
            Gfx_CopyResource((u8 *)0x08277994, VRAM_BASE + 0x4000, 0);
            break;
        case 3:
            Gfx_CopyResource((u8 *)0x0826C850, VRAM_BASE + 0x4000, 0);
            break;
        default:
            Gfx_CopyResource((u8 *)0x080E5BC0, VRAM_BASE + 0x4000, 0);
            break;
    }

    BgPalette_Load((u8 *)0x080E5E24);
    BgMap_Setup(3, (struct BgMapMetadata *)0x080E74C4, 0, 0);
    BgMap_Setup(2, (struct BgMapMetadata *)0x080E7044, 0, 0);
    BgMap_Setup(1, (struct BgMapMetadata *)0x080E7314, 0, 0);
    MainMenu_InitEntity(entity);
    SaveSelect_SetupMessageWindow(0);
    HBlankEffectSetup(160, (s32)(SCREEN_SIZE_Y * 0.65f), 2, REG_BG1VOFS);
    entity->ext.mainMenu.cursor = 0;
    SaveSelect_ShowSlot(entity, 0);
    entity->state = 7;
}

/**
 * @brief Choose a save slot: B returns -1; A loads it and returns 1 when it
 * suits the caller (soul trade or boss rush), else plays the error
 * sound.
 */
s32 SaveSelect_Update(struct EwramData_EntityData *entity)
{
    s32 hasBackup;
    s32 result;
    s32 repeatedInput;
    s32 cursor;
    s32 canLoad;

    repeatedInput = gEwramData->inputData.repeatedInput;
    cursor = entity->ext.mainMenu.cursor;
    result = 0;
    if (repeatedInput & KEY_B)
    {
        result = -1;
    }
    else if (repeatedInput & KEY_A)
    {
        SaveData_LoadSlotFromSram(cursor);
        if (entity->ext.mainMenu.slotEmpty != 0)
        {
            canLoad = 0;
        }
        else
        {
            hasBackup = SaveData_ReadSlotHeader(cursor + 1, 1);
            if (gEwramData->gameMode == GAME_MODE_SOUL_TRADE_MENU)
            {
                if (!(gEwramData->saveHeaders[0].character == 0 && ((gEwramData->saveHeaders[1].playTime == 0) || (hasBackup == 0))))
                {
                    canLoad = 0;
                }
                else
                {
                    canLoad = 1;
                }
            }
            else if ((gEwramData->saveHeaders[0].clearAndDifficulty & 0xF))
            {
                canLoad = 1;
            }
            else
            {
                canLoad = 0;
            }
        }

        if (canLoad == 0)
        {
            PlaySong(SE_239);
        }
        else
        {
            if (cursor & 1)
            {
                cursor -= 1;
            }
            gEwramData->gameState.currentSaveSlot = cursor;
            gEwramData->gameState.roomData = NULL;
            PlaySong(SE_243);
            result = 1;
        }
    }
    else if (repeatedInput & KEY_RIGHT)
    {
        cursor = Mod(cursor + 2, 6);
    }
    else if (repeatedInput & KEY_LEFT)
    {
        cursor = Mod(cursor + 4, 6);
    }

    if (entity->ext.mainMenu.cursor != cursor)
    {
        entity->ext.mainMenu.cursor = cursor;
        PlaySong(SE_241);
        SaveSelect_ShowSlot(entity, entity->ext.mainMenu.cursor);
    }
    Entity_UpdateAll();
    return result;
}

/**
 * @brief Init entry 2 of sMainMenuSubmenuInitFuncs: show the slot information for the
 * data-delete submenu.
 */
void MainMenu_InitDataDelete(struct EwramData_EntityData *entity)
{
    s32 i;

    for (i = 0; i < 6; i += 2)
    {
        if ((entity->ext.mainMenu.savedSlots >> i) & 1)
        {
            entity->ext.mainMenu.cursor = i;
            break;
        }
    }
    BgMap_Setup(1, (struct BgMapMetadata *)&sMainMenuDeleteBg1Map, 0, 0);
    BgMap_Setup(2, (struct BgMapMetadata *)&sMainMenuDeleteBg2Map, 0, 0);
    HBlankEffectSetup(160, (s32)(SCREEN_SIZE_Y * 0.95f), 2, REG_BG1VOFS);
    MainMenu_ShowSlotInfo(entity, entity->ext.mainMenu.cursor);
    MenuText_ClearAndPrint(0x35A);
    entity->state = 5;
}

/**
 * @brief Save the display state and open the save select screen (boss rush,
 * soul trade, debug mode 4531).
 */
void SaveSelect_Open(struct EwramData_EntityData *entity)
{
    Display_SaveState();
    SaveSelect_InitScreen(entity);
}

/**
 * @brief Free the menu entity and restore the display state.
 */
void SaveSelect_Close(struct EwramData_EntityData *entity)
{
    MainMenu_FreeEntity(entity);
    Display_RestoreState();
}

/**
 * @brief Load the name entry backgrounds and windows and enter menu state 6.
 */
void MainMenu_OpenNameEntry(struct EwramData_EntityData *entity)
{
    s16 *hBlankBuf;
    s16 scroll;
    s32 scanline;

    BgMap_Setup(1, (struct BgMapMetadata *)0x080E70D4, 0, 0);
    BgMap_Setup(2, (struct BgMapMetadata *)0x080E6E94, 0, 0);
    HBlankEffectSetup(160, (s32)(SCREEN_SIZE_Y * 0.1f), 2, REG_BG1VOFS);
    hBlankBuf = gEwramData->hBlankEffect.hBlankBuffer[1 - gEwramData->hBlankEffect.currentBuffer];
    if (entity->subState != 0)
    {
        scroll = 0xA0;
    }
    else
    {
        scroll = 0;
    }
    for (scanline = 0; scanline < (s32)(SCREEN_SIZE_Y * 0.1f); scanline++)
    {
        *hBlankBuf++ = scroll;
    }
    gUnk_03002CB0.dispCnt = DCNT_WIN1 | DCNT_WIN0 | DCNT_OBJ | DCNT_BG3 | DCNT_BG1;
    gDisplayRegisters.bldCnt = BLDCNT_SCREEN_SECOND_TARGET | BLDCNT_ALPHA_BLENDING_EFFECT | BLDCNT_OBJ_FIRST_TARGET_PIXEL | BLDCNT_BG1_FIRST_TARGET_PIXEL;
    entity->ext.mainMenu.nameCursor = 0;
    entity->ext.mainMenu.cursor = 0;
    entity->state = 6;
}

u8 sDefaultPlayerName[8] = {
    0x14, 0x10, 0xE, 0x2, 0, 0, 0, 0
};

const u8 sNameCodeJulius[8] = {
    0xB, 0x16, 0xD, 0xA, 0x16, 0x14, 0, 0
};

const u8 sNameCodeNoUse[8] = {
    0xF, 0x10, 0x16, 0x14, 0x6, 0, 0, 0
};

const u8 sNameCodeNoSoul[8] = {
    0xF, 0x10, 0x14, 0x10, 0x16, 0xD, 0, 0
};

/* gameState.playerName is declared as u8 * in ewram.h but holds the 8-byte player name;
 * indexing it through this view gives the original base + 0x94 + i order. */
struct EwramPlayerName { u8 pad[0x94]; u8 name[8]; };

// Matches (from decomp.me scratch vf8yR, 96.51 %: goto loops, name view)
/**
 * @brief Name entry keyboard: move on the 9x4 grid, A types a character (up
 * to eight), L/R move the text cursor, B deletes; return 1 when done,
 * -1 when cancelled.
 */
s32 MainMenu_UpdateNameEntry(struct EwramData_EntityData *entity)
{
    s32 row;
    s32 secret;
    s32 nextColumn;
    s32 nextKey;
    s32 delEmpty;
    s32 backEmpty;
    s32 blank;
    s32 delIndex;
    s32 delShift;
    s32 backIndex;
    s32 backShift;
    s32 i;
    s32 j;
    s32 k;
    s32 index;
    s32 flagMatch;
    s32 characterMatch;
    s32 action;
    s32 result;
    struct SaveHeader *header;
    u8 *delScan;
    u8 *backScan;
    s32 input;
    u16 song;
    u8 *cmpC;
    u8 *cmpA;
    u8 *cmpB;
    u8 textCursor;
    s32 key;
    u8 *ch;

    header = gEwramData->saveHeaders;
    input = gEwramData->inputData.repeatedInput;
    key = entity->ext.mainMenu.cursor;
    action = 0;
    result = 0;

    if (input & 8)
    {
        action = 1;
    }
    else if (input & 1)
    {
        if (key < 0x22)
        {
            PlaySong(0xF3);
            header->name[entity->ext.mainMenu.nameCursor] = key + 2;
            entity->ext.mainMenu.nameCursor += 1;
            if (entity->ext.mainMenu.nameCursor > 7)
            {
                entity->ext.mainMenu.cursor = 0x23;
                key = 0x23;
            }
        }
        else if (key == 0x22)
        {
            action = 4;
        }
        else
        {
            action = 1;
        }
    }
    else if (input & 2)
    {
        action = 3;
    }
    else if (input & 0x300)
    {
        PlaySong(0xF2);
        textCursor = entity->ext.mainMenu.nameCursor;
        entity->ext.mainMenu.nameCursor = (input & 0x100) ? (textCursor + 1) : (textCursor - 1);
    }
    else if (input & 0x30)
    {
        row = Div(key, 9);
        
        if (input & 0x10)
        {
            nextColumn = key + 1;
        }
        else
        {
            nextColumn = key + 8;
        }
        key = (row * 9) + Mod(nextColumn, 9);
    }
    else if (input & 0xC0)
    {
        if (input & 0x40)
        {
            nextKey = key + 27;
        }
        else
        {
            nextKey = key + 9;
        }
        key = Mod(nextKey, 0x24);
    }

    switch (action)
    {
        case 1:
            song = 0xF4;

            ch = header->name;
            for (i = 0; i < 8; i++)
            {
                if (*ch >= 0x2 && *ch <= 0x22)
                {
                    break;
                }
                ch++;
            }
            if (i > 7)
            {
                blank = 1;
            }
            else
            {
                blank = 0;
            }
            if (blank != 0)
            {
                for (j = 0; j < 8; j++)
                {
                    header->name[j] = sDefaultPlayerName[j];
                }
            }

            for (k = 0; k < 8; k++)
            {
                ((struct EwramPlayerName *)gEwramData)->name[k] = header->name[k];
            }

            if (gEwramData->gameState.systemFlags & 1)
            {
                if (entity->subState == 0)
                {
                    gEwramData->playerStatus.currentCharacter = 0;
                    characterMatch = 1;
                    index = 0;
                    cmpA = (u8*)&gEwramData->gameState.playerName;
                    for (; index < 8; )
                    {
                        if (*cmpA != sNameCodeJulius[index])
                        {
                            characterMatch = 0;
                            break;
                        }
                        index += 1;
                        cmpA += 1;
                    }
                    if (characterMatch != 0)
                    {
                        song = 0x1E5;
                        gEwramData->playerStatus.currentCharacter = 1;
                    }
                    else
                    {
                        flagMatch = 1;
                        index = 0;
                        cmpB = (u8*)&gEwramData->gameState.playerName;
                        goto t9_;
                    e9_:
                        if (index == 7)
                        {
                            secret = 2;
                            goto sw_;
                        }
                        index += 1;
                        cmpB += 1;
                    t9_:
                        if (index > 7) goto f9_;
                        if (*cmpB == sNameCodeNoSoul[index]) goto e9_;
                    f9_:
                        index = 0;
                        cmpC = (u8*)&gEwramData->gameState.playerName;
                        goto t_; i_: index += 1; cmpC += 1; t_:
                        if (index > 7) goto d_;
                        if (*cmpC == sNameCodeNoUse[index]) goto i_;
                        flagMatch = 0;
                    d_:
                        secret = flagMatch;
                    sw_:
                        switch (secret)
                        {
                            case 1:
                                song = 0x1B5;
                                gEwramData->gameState.gameFlags[2] |= 0x100000;
                                break;

                            case 2:
                                song = 0x199;
                                gEwramData->gameState.gameFlags[2] |= 0x200000;
                                break;
                        }
                    }
                }
            }
            PlaySong(song);
            result = 1;
            break;

        case 4:
            if (entity->ext.mainMenu.nameCursor != 0)
            {
                entity->ext.mainMenu.nameCursor -= 1;
            }
            delScan = header->name;
            delIndex = 0;
            for (; delIndex < 8; )
            {
                if ((*delScan >= 0x2 && *delScan <= 0x22))
                {
                    break;
                }
                delScan += 1;
                delIndex += 1;
            }
            if (delIndex > 7)
            {
                delEmpty = 1;
            }
            else
            {
                delEmpty = 0;
            }
            if (delEmpty == 0)
            {
                PlaySong(0xF0);
                delShift = entity->ext.mainMenu.nameCursor + 1;
                for (; delShift < 8; delShift++)
                {
                    header->name[delShift - 1] = header->name[delShift];
                }
                header->name[7] = 0;
            }
            break;
        
        case 3:
            if (entity->ext.mainMenu.nameCursor != 0)
            {
                entity->ext.mainMenu.nameCursor -= 1;
            }
            /* fallthrough */
        case 2:
            backScan = header->name;
            backIndex = 0;
            for (; backIndex < 8; )
            {
                if (*backScan >= 0x2 && *backScan <= 0x22)
                {
                    break;
                }
                backScan += 1;
                backIndex += 1;
            }
            if (backIndex > 7)
            {
                backEmpty = 1;
            }
            else
            {
                backEmpty = 0;
            }
            if (backEmpty != 0)
            {
                result = -1;
            }
            else
            {
                PlaySong(0xF0);
                backShift = entity->ext.mainMenu.nameCursor + 1;
                for (; backShift < 8; backShift++)
                {
                    header->name[backShift - 1] = header->name[backShift];
                }
                header->name[7] = 0;
            }
            break;
    }

    entity->ext.mainMenu.nameCursor &= 7;
    if (entity->ext.mainMenu.cursor != key)
    {
        entity->ext.mainMenu.cursor = key;
        PlaySong(0xF1);
    }
    return result;
}

/**
 * @brief Unreferenced: set every game flag except flags 32-63 (word 1),
 * which are cleared.
 */
void GameFlags_SetAllExceptWord1(void)
{
    struct GameState *gameState;
    s32 i;

    gameState = &gEwramData->gameState;

    for (i = 0; i < 8; i++)
    {
        gameState->gameFlags[i] = (i != 1) ? -1 : 0;
    }
}
