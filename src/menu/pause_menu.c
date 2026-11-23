/*
 * Pause menu: text helpers, enter / status page / graphics / update / exit, page list and cursors, the soul type selector.
 * Built without -f2003-patch (SoulTypeSelector_SetDirection needs it off; the rest compiles the same either way).
 */
#include "graphics.h"
#include "gameplay.h"
#include "structs/ewram.h"
#include "bg_map.h"
#include "bg_effects.h"
#include "room_entities.h"
#include "map_flags.h"
#include "music.h"
#include "player_level.h"
#include "syscalls.h"
#include "core.h"
#include "sound.h"
#include "menu_entity.h"
#include "gba.h"
#include "structs/entity_render_flags.h"
#include "blue_souls.h"
#include "equipment_menu.h"
#include "math_anim.h"
#include "menu_common.h"
#include "oam.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "palette.h"
#include "palette_state.h"
#include "pause_menu.h"
#include "soul_menu.h"
#include "text12.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"

/* Menu text redraw and transient entity-byte reset helpers.
 * for source evidence. */

/**
 * @brief Clear the configured Text12 region, then print the selected localized menu entry.
 */
void MenuText_ClearAndPrint(s32 index)
{
    Text12_ClearRegion();
    MenuText_Print(index);
}

/**
 * @brief Clear the menu page cursor bytes unk_4F7..unk_4FB.
 */
void MenuPage_Reset(struct EwramData_EntityData *entity)
{
    entity->ext.menuPage.category = 0;
    entity->ext.menuPage.count = 0;
    entity->ext.menuPage.cursor = 0;
    entity->ext.menuPage.top = 0;
    entity->ext.menuPage.item = 0;
}

/* Write count consecutive tile numbers from map byte offset xOffset on row y.
 * Modifying y in place keeps loop.c from strength-reducing the caller's row
 * address (the original recomputes it every row). */
static inline void FillMapRow(s32 xOffset, s32 y, u16 tile, s32 count)
{
    u16 *map;
    s32 i;

    y <<= 6;
    y += BG_MAP_VRAM;
    map = (u16 *)(xOffset + y);

    for (i = 0; i < count; i++)
    {
        *map = tile;
        tile++;
        map++;
    }
}

/* Fill a width x height (in 6x12 px text cells) block of the menu tilemap with
 * consecutive tiles (32 per row), then reset the text drawing region. */
void Text12_SetupTileRegion(s32 x, s32 y, u16 tile, s32 width, s32 height)
{
    s32 columns = (width * 6) >> 3;
    s32 rows = (height * 12) >> 3;
    s32 row;
    for (row = 0; row < rows; row++)
    {
        u16 t = tile + row * 32;
        FillMapRow(x * 2, y + row, t, columns);
    }
    Text12_SetRegion(0, 0, width, height);
    Text12_ClearRegion();
}

/* Menu helpers 0x08047330..0x080473F3. */
/**
 * @brief Clear the 0x1E x 2 Text12 region and print the name of soul (index + 1)
 * of the given kind, or the default text 0x34F when soul is zero. Kinds
 * outside 0..3 keep the default base.
 */
void Menu_PrintSoulName(s32 soulType, s32 soulId)
{
    s32 text = 0x34F;

    if (soulId != 0)
    {
        switch (soulType)
        {
            case 0:
                text = 0x1E4;
                break;
            case 1:
                text = 0x21C;
                break;
            case 2:
                text = 0x234;
                break;
            case 3:
                text = 0x257;
                break;
        }
        text += soulId - 1;
    }

    Text12_SetRegion(0, 0, 0x1E, 2);
    Text12_ClearRegion();
    MenuText_Print(text);
}

/**
 * @brief Print the first segment of localized entry id at (0, row) and the
 * following segment at (0, row + 1).
 */
void Text12_PrintTwoLineEntryAt(s32 index, s32 row)
{
    u8 *text = Text_GetLocalizedEntry(index);
    s32 offset;

    Text12_SetCursor(0, row);
    offset = Text12_Print(text, 0);
    Text12_SetCursor(0, row + 1);
    Text12_Print(text, offset + 1);
}

/**
 * @brief Fill count entries of the BG map with the same tile from (x, y) rightwards.
 */
void HudMap_FillTiles(s32 x, s32 y, u16 tile, s32 count)
{
    u16 *map;
    s32 i;

    x <<= 1;
    y <<= 6;
    y += BG_MAP_VRAM;
    map = (u16 *)(x + y);

    for (i = 0; i < count; i++)
    {
        *map = tile;
        map++;
    }
}

/**
 * @brief 473F4 | Enter the menu: unless stateFlags bit 0 is set, reset the
 * game screen, set up blending / BG offsets and remember the equipped
 * blue and yellow souls in sharedScratchBuffer[0x14..0x15].
 */
void PauseMenu_Enter(struct GameState *gameState)
{
    if (gEwramData->gameState.stateFlags & 1)
        return;
    AreaName_Dismiss(gameState);
    Display_SaveState();
    ObjAffine_SaveState();
    ObjGfx_SaveState();
    ItemIconCache_Save();
    Palette_SaveState();
    gUnk_03002CB0.dispCnt &= ~0x2000;
    gEwramData->bgControlMinWriteLevel = gEwramData->unusedWriteLevel = 3;
    gDisplayRegisters.bldCnt = 0xFF;
    gEwramData->startPauseEnabled = 0;
    gEwramData->paused = 1;
    gameState->subMode = 6;
    gameState->subModeStep = 0;
    gDisplayRegisters.bgOfs[0].hOfs = 0;
    gDisplayRegisters.bgOfs[0].vOfs = 0;
    gEwramData->hBlankEffect.requestStop = 1;
    Sound_StopTrackedWithVolume();
    BgMap_SaveSelection(1);
    gEwramData->sharedScratchBuffer[0x14] = gEwramData->playerStatus.equippedBlueSoul;
    gEwramData->sharedScratchBuffer[0x15] = gEwramData->playerStatus.equippedYellowSoul;
    Obj_SetBgLayer(2, 0);
    Obj_SetBgLayer(1, 1);
    Bg_SyncPriorities();
    gEwramData->screenShakeOffsetX = 0;
    gEwramData->screenShakeOffsetY = 0;
    gDisplayRegisters.bldY = 0x10;
    gEwramData->pauseMenuPage = 0;
}

extern void Menu_DrawPlayTime(s32 x, s32 y, s32 a, u32 value);
extern void HudMap_PrintNumber(s32 x, s32 y, s32 value, s32 digits, s32 color, s32 flag);
extern void HudMap_WriteTiles(s32 x, s32 y, u16 tile, s32 count);
extern void PauseMenu_DrawPageList(void);
extern void MenuItemIcon_SetHidden(s32 value);

/**
 * @brief 47520 | Draw the status page of the menu: play time, level,
 * gold, experience and next level, plus the static frame tiles.
 */
void PauseMenu_DrawStatusPage(struct GameState *gameState)
{
    struct PlayerStatus *stats;
    s32 value;
    s32 i;

    stats = &gEwramData->playerStatus;
    BgMap_Setup(1, (struct BgMapMetadata *)0x0815E940, 0, 0);
    HudMap_ClearAll();
    Player_RefreshEquipmentStats();
    YellowSoul_Restart(gEwramData->playerStatus.equippedYellowSoul, 1);
    Menu_DrawHpMpBars(0xE, 2);
    Menu_DrawCurrentStats(0xE, 5);
    Menu_DrawAttackDefense(0x18, 5);
    Menu_DrawPlayTime(0x1C, 8, 3, gEwramData->gameState.inGameTimer);
    value = Map_GetCompletionTenths(gameState, gameState->unk_8D_6);
    HudMap_PrintNumber(0x17, 9, Div(value, 10), 3, 3, 0);
    HudMap_PrintNumber(0x19, 9, Mod(value, 10), 1, 3, 1);
    HudMap_PrintNumber(0x1A, 0xA, stats->currentGold, 6, 6, 0);
    HudMap_PrintNumber(0x19, 0xC, stats->currentExperience, 8, 3, 0);
    HudMap_PrintNumber(0x19, 0xD, Player_GetExperienceToNextLevel(stats), 8, 3, 0);
    HudMap_PrintLargeNumber(5, 9, stats->currentLevel, 2, 3);
    HudMap_DrawStatusConditionIcon(0xC, 9);
    Text_Initialize();
    Text_SetTileBuffer((void *)0x06008000);
    Text_SetForegroundColor(1);
    Text_SetBackgroundColor(0);
    for (i = 0; i < 3; i++)
        HudMap_WriteTiles(10, i + 0xF, i * 0x20 + 0x3200, 0x13);
    Text12_SetRegion(0, 0, 0x1A, 3);
    Text12_ClearRegion();
    PauseMenu_DrawPageList();
    MenuItemIcon_SetHidden(1);
}

extern s32 GameFlag_Is3ASet(void);

/**
 * @brief 47684 | Load the menu background graphics for the current
 * language (2 = French, 3 = German) and set up BG0/BG1.
 */
void PauseMenu_LoadGraphics(void)
{
    void *a;
    void *b;
    void *c;

    a = (void *)0x0815E02C;
    b = (void *)0x08264398;
    c = (void *)0x0827B200;
    switch (gEwramData->gameState.language)
    {
    case 2:
        a = (void *)0x0827397C;
        b = (void *)0x08275980;
        c = (void *)0x08277984;
        break;
    case 3:
        a = (void *)0x08268838;
        b = (void *)0x0826A83C;
        c = (void *)0x0826C848;
        break;
    }
    if (GameFlag_Is3ASet())
    {
        BgPalette_Load((u8 *)0x0826664C);
        Gfx_CopyResource(b, (void *)0x06000000, 0);
    }
    else
    {
        BgPalette_Load((u8 *)0x080E5C20);
        Gfx_CopyResource(a, (void *)0x06000000, 0);
    }
    Gfx_CopyResource(c, (void *)0x0600A000, 0);
    gDisplayRegisters.bgCnt[0] = 0x1C04;
    gDisplayRegisters.bgCnt[1] = 0x1D00;
    gUnk_03002CB0.dispCnt &= 0x13FF;
    MenuCursor_LoadGraphics(&gEwramData->entityData[18]);
    Cursor_StartAnimation();
}

/* PauseMenu_Update, decompiled from the original assembly: menu page selection
 * (fade in/out, cursor over seven entries, page callbacks). The cursor read
 * through the inline CurSel() keeps the base and reloads the 0x25553 offset
 * as in the target. */

typedef s32 (*MenuPageFunc)(struct EwramData_EntityData *ent);

extern void PauseMenu_Exit(void);
struct SoulTypeIcons;

extern const MenuPageFunc sPauseMenuPageOpenFuncs[];
extern const MenuPageFunc sPauseMenuPageUpdateFuncs[];

static inline void DrawItem(s32 x, s32 y, s32 tile, s32 hl)
{
    tile += 0x300;
    if (hl)
        tile |= 0x3000;
    else
        tile |= 0x7000;
    HudMap_WriteTiles(x, y, tile, 6);
}

static inline s32 CurSel(void)
{
    return gEwramData->pauseMenuPage;
}

/**
 * @brief Pause menu state machine (state subModeStep): fade in and draw the status page,
 * move the cursor over the seven page entries (pauseMenuPage, wrapping), open a
 * page through the open/update tables sPauseMenuPageOpenFuncs/sPauseMenuPageUpdateFuncs (entry 4,
 * save, opens without the fade), and on B fade out and leave via
 * PauseMenu_Exit. Item icons are drawn while entry 1 or 2 is selected.
 *
 * @return 1 once the menu has closed, 0 otherwise
 */
s32 PauseMenu_Update(struct GameState *gameState)
{
    struct EwramData_EntityData *ent;
    s32 ret;
    s32 repeated;
    s32 new;
    s32 i;
    s32 sel;
    s32 tile;
    s32 x;
    s32 y;

    ent = &gEwramData->entityData[18];
    ret = 0;
    repeated = gEwramData->inputData.repeatedInput;
    new = gEwramData->inputData.newInput;
    switch (gameState->subModeStep)
    {
    case 0:
    case 1:
        gEwramData->bgControlMinWriteLevel = gEwramData->unusedWriteLevel = 3;
        if ((u16)(gDisplayRegisters.bldY += 3) <= 0xF)
            break;
        gDisplayRegisters.bldY = 0x10;
        PauseMenu_LoadGraphics();
        PauseMenu_DrawStatusPage(gameState);
        gameState->subModeStep = 2;
        ent->stage = 0;
        ent->subState = 0;
        ent->state = 0;
        Cursor_SetTilePosition(1, gEwramData->pauseMenuPage + 0xC);
        Cursor_Draw();
        break;
    case 2:
        if ((s16)(gDisplayRegisters.bldY -= 3) <= 0)
        {
            gameState->subModeStep = 3;
            gDisplayRegisters.bldY = 0;
        }
        Cursor_Draw();
        break;
    case 3:
        if (repeated & 2)
        {
            PlaySong(0xF0);
            gameState->subModeStep = 7;
        }
        else if (new & 1)
        {
            PlaySong(0xF3);
            if (gEwramData->pauseMenuPage == 4)
            {
                gameState->subModeStep = 6;
                sPauseMenuPageOpenFuncs[gEwramData->pauseMenuPage](ent);
            }
            else
            {
                gameState->subModeStep = 4;
            }
        }
        else if (repeated & 0xC0)
        {
            PlaySong(0xF1);
            if (repeated & 0x40)
            {
                gEwramData->pauseMenuPage--;
                if (gEwramData->pauseMenuPage < 0)
                    gEwramData->pauseMenuPage = 6;
            }
            else
            {
                gEwramData->pauseMenuPage++;
                if (gEwramData->pauseMenuPage > 6)
                    gEwramData->pauseMenuPage = 0;
            }
            Cursor_SetTilePosition(1, gEwramData->pauseMenuPage + 0xC);
            x = 2;
            y = 0xC;
            sel = CurSel();
            for (i = 0, tile = 0x40; i <= 6; tile += 0x10, i++)
            {
                s32 t = tile + 0x300;
                if (i == sel)
                    t |= 0x3000;
                else
                    t |= 0x7000;
                HudMap_WriteTiles(x, y + i, t, 6);
            }
            MenuText_ClearAndPrint(gEwramData->pauseMenuPage + 0x362);
        }
        Cursor_Draw();
        break;
    case 4:
        if ((u16)(gDisplayRegisters.bldY += 3) <= 0xF)
            break;
        if (gEwramData->pauseMenuPage != 6)
        {
            BgMap_Setup(1, sPauseMenuBgMaps[gEwramData->pauseMenuPage + 1], 0, 0);
            HudMap_ClearAll();
        }
        sPauseMenuPageOpenFuncs[gEwramData->pauseMenuPage](ent);
        gameState->subModeStep = 5;
        gDisplayRegisters.bldY = 0x10;
        break;
    case 5:
        if ((s16)(gDisplayRegisters.bldY -= 3) <= 0)
        {
            gameState->subModeStep = 6;
            gDisplayRegisters.bldY = 0;
        }
        switch (gEwramData->pauseMenuPage)
        {
        case 0:
            SoulTypeSelector_Update((struct SoulTypeIcons *)&gEwramData->entityData[21]);
            break;
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
            Cursor_Draw();
            break;
        }
        break;
    case 6:
        if (sPauseMenuPageUpdateFuncs[gEwramData->pauseMenuPage](ent))
        {
            if (gEwramData->pauseMenuPage == 4)
                gameState->subModeStep = 3;
            else
                gameState->subModeStep = 1;
        }
        break;
    case 7:
        if ((u16)(gDisplayRegisters.bldY += 3) <= 0xF)
            break;
        PauseMenu_Exit();
        Entity_UpdateAll();
        gDisplayRegisters.bldY = 0;
        gDisplayRegisters.bldCnt |= 0x3F00;
        Sound_ReplayTrackedWithVolume();
        gEwramData->startPauseEnabled = 1;
        gEwramData->paused = 0;
        gEwramData->bgControlMinWriteLevel = gEwramData->unusedWriteLevel = 0;
        ret = 1;
        break;
    }
    if ((u8)(gEwramData->pauseMenuPage - 1) <= 1)
    {
        MenuItemIcon_Draw();
        MenuSlotIcons_Draw();
    }
    return ret;
}

/**
 * @brief 47B44 | Leave the menu: delete the menu entity, restore the room
 * graphics and BG layers and refresh soul effects whose equipped blue /
 * yellow soul changed while the menu was open.
 */
void PauseMenu_Exit(void)
{
    struct RoomDefinition *room;
    struct RoomBgLayerDefinition *layer;
    s32 i;

    {
        struct EwramData_EntityData *ent = &gEwramData->entityData[18];
        struct GameState *g = &gEwramData->gameState;
        room = g->currentRoomDefinition;
        EntityDelete(ent);
    }
    MenuItemIcon_Delete();
    MenuItemIcon_Delete();
    BgMap_Setup(1, gEwramData->savedBgMetadata, gEwramData->savedBgScrollX, gEwramData->savedBgScrollY);
    Hud_Init();
    SharedObjPalette_RestoreAll();
    Palette_RestoreState();
    ItemIconCache_Restore();
    ObjGfx_RestoreState();
    ObjAffine_RestoreState();
    gUnk_03002CB0.dispCnt = 0;
    Room_LoadGraphics(room);
    Room_LoadPalettes(room);
    Display_RestoreState();
    layer = gEwramData->gameState.currentRoomDefinition->bgLayers;
    for (i = 1; i <= 3; i++)
    {
        Obj_SetBgLayer(layer->layer, i);
        layer++;
    }
    Bg_SyncPriorities();
    if (gEwramData->sharedScratchBuffer[0x14] != gEwramData->playerStatus.equippedBlueSoul)
        BlueSoul_StopAll();
    if (gEwramData->sharedScratchBuffer[0x15] != gEwramData->playerStatus.equippedYellowSoul)
        YellowSoul_Stop();
}

/* Menu helpers 0x08047C1C..0x08048107 (graphics state, page list, cursor
 * and scroll arrows). */

void MenuCursor_DrawScrollArrows(struct EwramData_EntityData *entity);
void Cursor_DrawWithOptionalBob(s32 bob);

/**
 * @brief 47C1C | ObjAffine_SaveState, ObjGfx_SaveState, ItemIconCache_Save, Palette_SaveState.
 */
void Menu_SaveGraphicsState(void)
{
    ObjAffine_SaveState();
    ObjGfx_SaveState();
    ItemIconCache_Save();
    Palette_SaveState();
}

/**
 * @brief 47C34 | Palette_RestoreState, ItemIconCache_Restore, ObjGfx_RestoreState, ObjAffine_RestoreState
 * (counterpart of Menu_SaveGraphicsState).
 */
void Menu_RestoreGraphicsState(void)
{
    Palette_RestoreState();
    ItemIconCache_Restore();
    ObjGfx_RestoreState();
    ObjAffine_RestoreState();
}

/**
 * @brief 47C4C | Draw the seven 6-tile rows of a list at (2, 12..18) from
 * tile 0x340 on, palette 3 for the row pauseMenuPage and 7 for the others, then
 * MenuText_ClearAndPrint(0x362 + pauseMenuPage).
 */
void PauseMenu_DrawPageList(void)
{
    s32 x = 2;
    s32 y = 12;
    s32 sel = gEwramData->pauseMenuPage;
    s32 i;
    s32 tile;
    s32 t;

    for (i = 0, tile = 0x40; i <= 6; i++)
    {
        t = tile + 0x300;
        if (i == sel)
            t |= 0x3000;
        else
            t |= 0x7000;
        HudMap_WriteTiles(x, y + i, t, 6);
        tile += 0x10;
    }
    MenuText_ClearAndPrint(gEwramData->pauseMenuPage + 0x362);
}

/**
 * @brief 47CC8 | Draw the list scroll arrows of entity at x 8 (frame 0x18):
 * the up arrow at y 0x54 when unk_4FA (presumably the first shown row) is
 * not 0, the down arrow (flipped) at y 0x6C when unk_4F8 (row count?)
 * > unk_4FA + 8.
 */
void MenuCursor_DrawScrollArrows(struct EwramData_EntityData *entity)
{
    entity->posX.part16.integer = 8;
    entity->animFrame = 0x18;
    if (entity->ext.menuPageList.bytes[2] != 0)
    {
        entity->posY.part16.integer = 0x54;
        Sprite_Draw(entity);
    }
    if (entity->ext.menuPageList.bytes[0] > entity->ext.menuPageList.bytes[2] + 8)
    {
        entity->posY.part16.integer = 0x6C;
        entity->flipY = 1;
        Sprite_Draw(entity);
        entity->flipY = 0;
    }
}

/**
 * @brief 47D24 | Draw the cursor of entity on the two-column grid at slot
 * unk_4F9 - unk_4FA, bobbing 3 px with frameCounter, then the scroll arrows
 * and at (0x80, 0x38) the frame chosen by bits 2/1/0 of statusEffects.
 */
void ItemMenu_DrawCursor(struct PlayerStatus *menu, struct EwramData_EntityData *entity)
{
    s32 index = entity->ext.menuPageList.bytes[1] - entity->ext.menuPageList.bytes[2];
    u32 t = gEwramData->frameCounter >> 1;
    s32 jitter;
    s32 flags;
    s32 kind;

    if (t & 8)
        jitter = 3 - (t & 7);
    else
        jitter = (t & 7) - 3;

    entity->posX.part16.integer = (index & 1) * 112 + 8 + jitter;
    entity->posY.part16.integer = (index & 6) * 4 + 0x54;
    Sprite_Draw(entity);
    MenuCursor_DrawScrollArrows(entity);

    flags = gEwramData->playerStatus.statusEffects;
    if (flags & 4)
        kind = 0x31;
    else if (flags & 2)
        kind = 2;
    else if (flags & 1)
        kind = 3;
    else
        kind = 1;
    entity->posX.part16.integer = 0x80;
    entity->posY.part16.integer = 0x38;
    entity->animFrame = kind;
    Sprite_Draw(entity);
}

/**
 * @brief 47DD4 | Draw entity as the cursor at (0x18 +- 3, 0x24 + 16 *
 * unk_4F9), then at x 0xAC the four frames sButtonConfigCursorFrames[unk_4FC..4FF]
 * at y 0x24 + 16 * i.
 */
void ButtonConfigMenu_DrawCursor(struct PlayerStatus *menu, struct EwramData_EntityData *entity)
{
    u32 t = gEwramData->frameCounter >> 1;
    s32 jitter;
    s32 i;
    s32 y;

    if (t & 8)
        jitter = 3 - (t & 7);
    else
        jitter = (t & 7) - 3;

    entity->posX.part16.integer = jitter + 0x18;
    entity->posY.part16.integer = entity->ext.menuPageList.bytes[1] * 16 + 0x24;
    Sprite_Draw(entity);
    entity->posX.part16.integer = 0xAC;
    for (i = 3; i >= 0; i--)
    {
        entity->posY.part16.integer = 0x24 + i * 0x10;
        entity->animFrame = sButtonConfigCursorFrames[entity->ext.menu.pageButtons[i]];
        Sprite_Draw(entity);
    }
}

/**
 * @brief 47E6C | Draw entity as the cursor for mode subState: 1 = two
 * columns 128 px apart by bit 0 of unk_4FA, row unk_4F9; 2 = as
 * ItemMenu_DrawCursor with the scroll arrows.
 */
void BestiaryMenu_DrawCursor(struct PlayerStatus *menu, struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 1:
        {
            u32 t = gEwramData->frameCounter >> 1;
            s32 jitter;

            if (t & 8)
                jitter = 3 - (t & 7);
            else
                jitter = (t & 7) - 3;
            entity->posX.part16.integer = (entity->ext.menuPage.top & 1) * 128 + 8 + jitter;
            entity->posY.part16.integer = entity->ext.menuPageList.bytes[1] * 16 + 0x34;
            Sprite_Draw(entity);
            break;
        }
        case 2:
        {
            s32 index = entity->ext.menuPageList.bytes[1] - entity->ext.menuPageList.bytes[2];
            u32 t = gEwramData->frameCounter >> 1;
            s32 jitter;

            if (t & 8)
                jitter = 3 - (t & 7);
            else
                jitter = (t & 7) - 3;
            entity->posX.part16.integer = (index & 1) * 112 + 8 + jitter;
            entity->posY.part16.integer = (index & 6) * 4 + 0x54;
            Sprite_Draw(entity);
            MenuCursor_DrawScrollArrows(entity);
            break;
        }
        case 0:
            break;
    }
}

/**
 * @brief 47F34 | Load the language dependent graphics set into unk_4F5 and
 * set up five ObjPalette_LoadRange entries (1, k) = 10..14 of sMenuCursorPalette.
 */
void MenuCursor_LoadGraphics(struct EwramData_EntityData *entity)
{
    u8 *set;

    switch (gEwramData->gameState.language)
    {
        case 2:
            set = sMenuCursorGfxLanguage2;
            break;
        case 3:
            set = sMenuCursorGfxLanguage3;
            break;
        default:
            set = sMenuCursorGfx;
            break;
    }
    entity->ext.menu.gfxSlot = ObjGfx_LoadResource(set, 0);
    ObjPalette_LoadRange(sMenuCursorPalette, 2, 1, 10);
    ObjPalette_LoadRange(sMenuCursorPalette, 3, 1, 11);
    ObjPalette_LoadRange(sMenuCursorPalette, 4, 1, 12);
    ObjPalette_LoadRange(sMenuCursorPalette, 1, 1, 13);
    ObjPalette_LoadRange(sMenuCursorPalette, 0, 1, 14);
}

/**
 * @brief Place and draw the menu cursor (entity 18) at the selected row, or
 * the bobbing yes/no cursor plus the row marker during confirmation.
 */
void Shop_DrawListCursor(struct EwramData_EntityData *entity)
{
    if (entity->stage == 0)
    {
        MENU_CURSOR_X(entity) = 12;
        MENU_CURSOR_Y(entity) = entity->ext.menuPageList.bytes[1] * 8 + 0x5C;
        Cursor_Draw();
    }
    else
    {
        MENU_CURSOR_X(entity) = entity->timer ? 0x88 : 0x3C;
        MENU_CURSOR_Y(entity) = 0x88;
        Cursor_DrawWithOptionalBob(1);
        if (gEwramData->frameCounter & 2)
        {
            MENU_CURSOR_X(entity) = 0x10;
            MENU_CURSOR_Y(entity) = entity->ext.menuPageList.bytes[1] * 8 + 0x5C;
            Cursor_DrawWithOptionalBob(0);
        }
    }
}

/**
 * @brief 48018 | Draw the cursor entityData[18] (frame 0x1E) at its stored
 * position MENU_CURSOR_X / MENU_CURSOR_Y, bobbing horizontally.
 */
void Cursor_Draw(void)
{
    struct EwramData_EntityData *cursor = &gEwramData->entityData[18];
    u32 t;
    s32 jitter;

    cursor->animFrame = 0x1E;
    cursor->palette = 13;
    t = gEwramData->frameCounter >> 1;
    if (t & 8)
        jitter = 3 - (t & 7);
    else
        jitter = (t & 7) - 3;
    cursor->posX.part16.integer = MENU_CURSOR_X(cursor) + jitter;
    cursor->posY.part16.integer = MENU_CURSOR_Y(cursor);
    Sprite_Draw(cursor);
}

/**
 * @brief 48084 | As Cursor_Draw; the bobbing only when bob != 0.
 */
void Cursor_DrawWithOptionalBob(s32 bob)
{
    struct EwramData_EntityData *cursor = &gEwramData->entityData[18];

    cursor->animFrame = 0x1E;
    cursor->palette = 13;
    cursor->posX.part16.integer = MENU_CURSOR_X(cursor);
    cursor->posY.part16.integer = MENU_CURSOR_Y(cursor);
    if (bob)
    {
        u32 t = gEwramData->frameCounter >> 1;
        s32 jitter;

        if (t & 8)
            jitter = 3 - (t & 7);
        else
            jitter = (t & 7) - 3;
        cursor->posX.part16.integer += jitter;
    }
    Sprite_Draw(cursor);
}

/* Soul type selector entity. */
/* Overlay of the entity fields used by the soul type selector. */
struct SoulTypeIcons
{
    u8 pad_0[0xA];
    u8 state;          /* 0x0A */
    u8 pad_B[2];
    u8 step;           /* 0x0D */
    u8 pad_E[2];
    s8 sel;            /* 0x10 */
    u8 pad_11[3];
    s32 acc[3];        /* 0x14 */
    s32 dir;           /* 0x20 */
    u8 pad_24[4];
    u16 matrix[3];     /* 0x28 */
    s16 scale;         /* 0x2E */
    u8 pad_30[8];
    u8 keepSelection;  /* 0x38: the next turn keeps sel (then cleared) */
    u8 unk_39;         /* 0x39 */
    u8 pad_3A[6];
    s32 posX;          /* 0x40 */
    s32 posY;          /* 0x44 */
    u8 pad_48[0x10];
    /* Match the canonical entity slot type without a separate store overlay. */
    u16 slot:5;        /* 0x58 */
    u8 slotRest:3;
    u8 flags59_0:3;    /* 0x59 */
    u8 hidden:1;
    u8 affine:1;
    u8 flags59_5:3;
    u8 flags5A_0:3;    /* 0x5A */
    u8 palette:4;
    u8 flags5A_7:1;
    u8 pad_5B[0x65 - 0x5B];
    u8 owned;          /* 0x65 */
};

/**
 * @brief 48108 | Soul type selector (three rotating icons for the red,
 * blue and yellow soul): set up, rotate by one position per 16 frames in
 * state 3 (direction from dir), mark the icon of an equipped type.
 */
void SoulTypeSelector_Update(struct SoulTypeIcons *e)
{
    struct EwramData *ew = gEwramData;
    struct PlayerStatus *stats = &ew->playerStatus;
    s32 found;
    s32 found2;

    switch (e->state)
    {
    case 0:
        Sprite_Attach((struct EwramData_EntityData *)e, sMenuSpriteSheet, ({ struct EwramData_EntityData *s_ = &ew->entityData[18]; s_; })->ext.menu.gfxSlot, 0xD);
        Anim_StartIndexed((struct EwramData_EntityData *)e, 0, 0, 1);
        e->matrix[0] = ObjAffine_Alloc(2);
        e->matrix[1] = ObjAffine_Alloc(2);
        e->matrix[2] = ObjAffine_Alloc(2);
        e->owned = 0;
        e->sel = 0;
    case 1:
        e->state = 2;
        e->step = 0;
        e->acc[0] = 0;
        e->acc[1] = 0;
        e->acc[2] = 0;
        e->dir = 0x8000;
        e->unk_39 = 0;
    case 2:
    case 3:
        if (e->state == 3)
        {
            if (e->dir >= 0)
            {
                e->scale += 0x400;
                if (e->scale > 0x3FFF)
                    e->scale = 0x4000;
            }
            else
                e->scale -= 0x400;
        }

        switch (e->sel)
        {
        case 0:
            e->palette = 0xA;
            break;
        case 1:
            e->palette = 0xB;
            break;
        case 2:
            e->palette = 0xC;
            break;
        }
        if (e->state == 3)
        {
            e->slot = e->matrix[0];
            e->affine = 1;
            if (e->dir >= 0)
                ObjAffine_Set(0, 0x10000 - e->scale * 2, 0x10000 - e->scale * 2, 0, 0, e->matrix[0]);
            else
                ObjAffine_Set(0, e->scale + 0x10000, e->scale + 0x10000, 0, 0, e->matrix[0]);
            e->acc[0] += e->dir;
        }
        else
        {
            e->affine = 0;
            e->acc[0] = 0;
        }
        e->posX = 0x150000;
        e->posY = 0x200000;
        if (e->dir >= 0)
        {
            s32 kx = 0xA000;
            s32 ky = 0x1C000;

            e->posX = e->step * kx + 0x150000;
            e->posY = e->step * ky + 0x200000;
        }
        else
        {
            s32 kx = 0xA000;

            e->posX = 0x150000 - e->step * kx;
            e->posY = (e->step << 16) + 0x200000;
        }
        found = 0;
        switch (e->sel)
        {
        case 0:
            if (stats->equippedRedSoul)
                found = 1;
            break;
        case 1:
            if (stats->equippedBlueSoul)
                found = 1;
            break;
        case 2:
            if (stats->equippedYellowSoul)
                found = 1;
            break;
        }
        if (found)
        {
            e->owned = 1;
            Anim_Update((struct EwramData_EntityData *)e);
            Sprite_Draw((struct EwramData_EntityData *)e);
        }
        if (!found)
        {
            e->owned = 0;
            Sprite_Draw((struct EwramData_EntityData *)e);
        }

        switch (e->sel)
        {
        case 2:
            e->palette = 0xA;
            break;
        case 0:
            e->palette = 0xB;
            break;
        case 1:
            e->palette = 0xC;
            break;
        }
        e->slot = e->matrix[1];
        e->affine = 1;
        if (e->state == 3)
        {
            ObjAffine_Set(0, e->scale + 0xC000, e->scale + 0xC000, 0, 0, e->matrix[1]);
            e->acc[1] += e->dir;
        }
        else
        {
            ObjAffine_Set(0, 0xC000, 0xC000, 0, 0, e->matrix[1]);
            e->acc[1] = 0;
        }
        e->posX = 0xB0000;
        e->posY = 0x300000;
        if (e->dir >= 0)
        {
            s32 kx = 0xA000;

            e->posX = e->step * kx + 0xB0000;
            e->posY = 0x300000 - (e->step << 16);
        }
        else
        {
            s32 kx = 0x14000;
            s32 ky = 0xC000;

            e->posX = e->step * kx + 0xB0000;
            e->posY = e->step * ky + 0x300000;
        }
        found2 = 0;
        switch (e->sel)
        {
        case 2:
            if (stats->equippedRedSoul)
                found2 = 1;
            break;
        case 0:
            if (stats->equippedBlueSoul)
                found2 = 1;
            break;
        case 1:
            if (stats->equippedYellowSoul)
                found2 = 1;
            break;
        }
        if (found2)
        {
            e->owned = 1;
            Sprite_Draw((struct EwramData_EntityData *)e);
        }
        if (!found2)
        {
            e->owned = 0;
            Sprite_Draw((struct EwramData_EntityData *)e);
        }

        switch (e->sel)
        {
        case 1:
            e->palette = 0xA;
            break;
        case 2:
            e->palette = 0xB;
            break;
        case 0:
            e->palette = 0xC;
            break;
        }
        e->slot = e->matrix[2];
        e->affine = 1;
        if (e->state == 3)
        {
            if (e->dir >= 0)
                ObjAffine_Set(0, e->scale + 0x8000, e->scale + 0x8000, 0, 0, e->matrix[2]);
            else
                ObjAffine_Set(0, 0x8000 - e->scale * 2, 0x8000 - e->scale * 2, 0, 0, e->matrix[2]);
            e->acc[2] += e->dir;
        }
        else
        {
            ObjAffine_Set(0, 0x8000, 0x8000, 0, 0, e->matrix[2]);
            e->acc[2] = 0;
        }
        {
            s32 x0, y0;
            s32 kx, ky;

            x0 = 0x1F0000;
            e->posX = x0;
            y0 = 0x3C0000;
            e->posY = y0;
            if (e->dir >= 0)
            {
                kx = 0x14000;
                ky = 0xC000;
            }
            else
            {
                kx = 0xA000;
                ky = 0x1C000;
            }
            e->posX = x0 - e->step * kx;
            e->posY = y0 - e->step * ky;
        }
        found2 = 0;
        switch (e->sel)
        {
        case 1:
            if (stats->equippedRedSoul)
                found2 = 1;
            break;
        case 2:
            if (stats->equippedBlueSoul)
                found2 = 1;
            break;
        case 0:
            if (stats->equippedYellowSoul)
                found2 = 1;
            break;
        }
        if (found2)
        {
            e->owned = 1;
            Sprite_Draw((struct EwramData_EntityData *)e);
        }
        if (!found2)
        {
            e->owned = 0;
            Sprite_Draw((struct EwramData_EntityData *)e);
        }

        if (e->state == 3)
        {
            if (++e->step > 15)
            {
                e->scale = 0;
                e->state--;
                e->step = 0;
                if (e->keepSelection == 0)
                {
                    if (e->dir >= 0)
                    {
                        if (++e->sel > 2)
                            e->sel = 0;
                    }
                    else
                    {
                        if (--e->sel < 0)
                            e->sel = 2;
                    }
                }
                else
                    e->keepSelection = 0;
                e->acc[0] = 0;
                e->acc[1] = 0;
                e->acc[2] = 0;
            }
        }
        break;
    case 4:
        if (e->keepSelection == 0)
        {
            e->state = 3;
            if (e->dir >= 0)
            {
                if (++e->sel > 2)
                    e->sel = 0;
            }
            else
            {
                if (--e->sel < 0)
                    e->sel = 2;
            }
        }
        else
        {
            e->state -= 2; /* 4 -> 2 */
            e->keepSelection = 0;
        }
        e->step = 0;
        e->scale = 0;
        e->acc[0] = 0;
        e->acc[1] = 0;
        e->acc[2] = 0;
        break;
    case 0xFF:
        ObjAffine_Free((s16)e->matrix[0]);
        ObjAffine_Free((s16)e->matrix[1]);
        ObjAffine_Free((s16)e->matrix[2]);
        e->hidden = 1;
        break;
    }
}

/* SoulTypeSelector_SetDirection; only
 * matches without -f2003-patch (NO_2003_PATCH_SRCS in ../Makefile). The
 * caller passes entity slot 21 (gEwramData + 0xFB8). */

#define ABS(x) ((x) < 0 ? -(x) : (x))
/* Bytes 0x51C (turned around) and 0x51D (direction) of this entity. */
#define ENTITY_TURNED(e) (((s8 *)&(e)->enemyStatus)[0])
#define ENTITY_DIR(e) (((s8 *)&(e)->enemyStatus)[1])
#define ENTITY_DIR_BYTE(e) ((u8 *)&e->enemyStatus)[1]

/**
 * @brief 486E0 | Set the direction byte 0x51D to dir (-1 or 1) and
 * point unk_4F6 and the rotation step that way (negative for -1); in state 3 a
 * change of a set direction marks byte 0x51C. Next state.
 */
void SoulTypeSelector_SetDirection(struct EwramData_EntityData *entity, s8 dir)
{
    if (ENTITY_DIR(entity) != 0 && entity->state == 3 && ENTITY_DIR(entity) != dir)
        ENTITY_TURNED(entity) = 1;
    if (dir == -1)
    {
        ENTITY_DIR_BYTE(entity) = 0xFF; // -1, stored as an unsigned byte
        entity->ext.soulSelector.unusedSpin = -ABS(entity->ext.soulSelector.unusedSpin);
        entity->ext.soulSelector.spin = -ABS(entity->ext.soulSelector.spin);
    }
    else
    {
        ENTITY_DIR(entity) = 1;
        entity->ext.soulSelector.unusedSpin = ABS(entity->ext.soulSelector.unusedSpin);
        entity->ext.soulSelector.spin = ABS(entity->ext.soulSelector.spin);
    }
    entity->state++;
}
