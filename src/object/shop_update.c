/*
 * Shop update, room object 0x20 and the object table helpers (creation from the object table, interactions, animation).
 */
#include "shop_update.h"
#include "core.h"
#include "in_game.h"
#include "bg_effects.h"
#include "room_entities.h"
#include "map_flags.h"
#include "sound.h"
#include "gba/dma.h"
#include "structs/ewram.h"
#include "structs/main.h"
#include "player_core.h"
#include "blue_souls.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "switch_doors.h"
#include "palette_state.h"
#include "pause_menu.h"
#include "scenes_talk_ending.h"
#include "soul_menu.h"
#include "text12.h"
#include "text_window.h"
#include "entity_common.h"
#include "shop.h"
#include "data/data_4F1444.h"
#include "data/data_0E5364.h"
#include "data/data_118B48.h"
#include "syscalls.h"
#include "menu_common.h"
#include "constants/game_flags.h"
#include "constants/songs.h"

extern void (*const sEventSceneCreateFuncs[])(struct EwramData_EntityData *entity);
extern u32 (*const sEventSceneUpdateFuncs[])(struct EwramData_EntityData *entity);

void ObjectTable_UpdateAnimation(struct EwramData_EntityData *entity);

/* Shop screen entry, category selection and return to the room. */
/**
 * @brief In-game sub-mode 9: shop entry, category selection (buy/sell), the
 * item list and the return to the room. Return nonzero when done.
 */
s32 Shop_Update(struct GameState *context)
{
    struct EwramData_EntityData *menu = &gEwramData->entityData[18];
    s32 repeated = gEwramData->inputData.repeatedInput;
    s32 pressed = gEwramData->inputData.newInput;
    s32 result = 0;
    switch (context->subModeStep) {
    case 0:
        Entity_UpdateAll();
        if (++gDisplayRegisters.bldY == 16) {
            s32 i;
            void *bg;
            ItemIconCache_Save();
            ObjGfx_SaveState();
            ObjAffine_SaveState();
            Shop_SetupBackgrounds();
            MenuCursor_LoadGraphics(menu);
            Cursor_StartAnimation();
            menu->state = 0;
            menu->subState = 0;
            DMA_FILL_32(3, 0, VRAM_BASE + 0xE000, 0x800);
            Text12_SetRegion(0, 2, 37, 3);
            Text12_ClearRegion();
            for (i = 0; i < 4; i++)
                HudMap_WriteTiles(2, i + 16, 0xF060 + i * 32, 29);
            Text12_PrintTwoLineEntryAt(menu->subState + 948, 2);
            switch (gEwramData->gameState.language) {
            case 2: bg = (void *)sShopBgMap3; break;
            case 3: bg = (void *)sShopBgMap2; break;
            default: bg = (void *)sShopBgMap; break;
            }
            BgMap_Setup(1, bg, 0, 0);
            context->subModeStep++;
            PlaySong(48);
        }
        break;
    case 1:
        if (--gDisplayRegisters.bldY == 0)
            context->subModeStep++;
        break;
    case 2:
        if (pressed & 3) {
            if ((pressed & 2) || menu->subState == 2) {
                PlaySong(SE_MENU_BACK);
                context->subModeStep++;
            } else {
                menu->ext.menuPage.cursor = 0;
                PlaySong(SE_MENU_CONFIRM);
                context->subModeStep = 10;
            }
        } else if (repeated & 0xC0) {
            s32 choice = menu->subState;
            s32 direction = repeated & 0x40;
            s32 next = choice + 1;
            if (direction)
                next = choice + 2;
            menu->subState = Mod(next, 3);
            Text12_ClearRegion();
            Text12_PrintTwoLineEntryAt(menu->subState + 948, 2);
            PlaySong(SE_MENU_CURSOR);
        }
        break;
    case 3:
        if (++gDisplayRegisters.bldY == 16) {
            u8 *gfx;
            struct RoomDefinition *room;
            EntityDelete(menu);
            MenuItemIcon_Delete();
            switch (gEwramData->gameState.language) {
            case 2: gfx = (void *)sMenuCursorGfxLanguage2; break;
            case 3: gfx = (void *)sMenuCursorGfxLanguage3; break;
            default: gfx = (void *)sMenuCursorGfx; break;
            }
            ObjGfx_Unload(ObjGfx_Find(gfx));
            ItemIconCache_Restore();
            room = context->currentRoomDefinition;
            Room_LoadGraphics(room);
            ObjGfx_RestoreState();
            ObjAffine_RestoreState();
            Room_LoadPalettes(room);
            Palette_RestoreState();
            Hud_Init();
            BgMap_Setup(1, gEwramData->savedBgMetadata, gEwramData->savedBgScrollX, gEwramData->savedBgScrollY);
            context->subModeStep++;
        }
        break;
    case 4:
        Entity_UpdateAll();
        Display_RestoreState();
        if (--gDisplayRegisters.bldY == 0) {
            gDisplayRegisters.bldCnt |= 0x3F00;
            gEwramData->paused = 0;
            { struct EwramData *updated = gEwramData;
              updated->unusedWriteLevel = 0; updated->bgControlMinWriteLevel = 0; }
            result = 1;
        }
        break;
    case 10:
        if (++gDisplayRegisters.bldY == 16) {
            /* agbcc otherwise swaps the table/index registers. This binding
             * preserves ROM identity; it does not establish original spelling. */
            register struct BgMapMetadata **table asm("r0"); /* FAKEMATCH: pinned register */
            s32 index;
            menu->state = menu->subState + 1;
            switch (gEwramData->gameState.language) {
            case 2: table = sShopBgMaps; index = menu->subState + 2; break;
            case 3: table = sShopBgMaps; index = menu->subState + 4; break;
            default: table = sShopBgMaps; index = menu->subState; break;
            }
            BgMap_Setup(1, table[index], 0, 0);
            gDisplayRegisters.bgOfs[0].hOfs = 0;
            menu->subState = 0;
            menu->stage = 0;
            context->subModeStep++;
        }
        break;
    case 11:
        if (gDisplayRegisters.bldY == 16)
            Shop_OpenList(context, menu);
        if (--gDisplayRegisters.bldY == 0)
            context->subModeStep++;
        break;
    case 12:
        if (Shop_UpdateList(context, menu))
            context->subModeStep++;
        break;
    case 13:
        if (++gDisplayRegisters.bldY == 16) {
            s32 i;
            void *bg;
            menu->subState = menu->state - 1;
            menu->state = 0;
            DMA_FILL_32(3, 0, VRAM_BASE + 0xE000, 0x800);
            Text12_SetRegion(0, 2, 37, 3);
            Text12_ClearRegion();
            for (i = 0; i < 4; i++)
                HudMap_WriteTiles(2, i + 16, 0xF060 + i * 32, 29);
            Text12_PrintTwoLineEntryAt(menu->subState + 948, 2);
            switch (gEwramData->gameState.language) {
            case 2: bg = (void *)sShopBgMap3; break;
            case 3: bg = (void *)sShopBgMap2; break;
            default: bg = (void *)sShopBgMap; break;
            }
            BgMap_Setup(1, bg, 0, 0);
            context->subModeStep = 1;
        }
        break;
    }
    if (context->subModeStep != 0)
        Shop_DrawCursors(menu);
    UpdateInGameTimer();
    return result;
}

/* Object 0x20 0x0805B42C..0x0805B5E7. */

/* Create / update of the variants of object 0x20. */

/**
 * @brief 5B42C | Object 0x20 create. Deleted unless (HP > 0 or stateFlags bit
 * 0x200000), unk_8E_7 clear and (Soma or stateFlags bit 0x8000); runs the create
 * of its variant, then (unless 0x53D bit 3 is set) sets stateFlags bit 0x200,
 * stores itself in sceneController and releases the player input.
 */
void EventSceneControllerCreate(struct EwramData_EntityData *entity)
{
    if (gEwramData->playerStatus.currentHP <= 0 && !(gEwramData->gameState.stateFlags & STATE_CHAOS_FIGHT))
    {
        EntityDelete(entity);
        return;
    }
    if (gEwramData->gameState.unk_8E_7
        || (gEwramData->playerStatus.currentCharacter && !(gEwramData->gameState.stateFlags & STATE_ENDING_JULIUS)))
    {
        EntityDelete(entity);
        return;
    }
    sEventSceneCreateFuncs[entity->spawnParam0](entity);
    if (entity->deleteRequested)
    {
        EntityDelete(entity);
        return;
    }
    gEwramData->gameState.stateFlags |= STATE_HUD_MESSAGES_BLOCKED;
    Hud_CloseMessageWindow();
    RedSoul_CancelAll();
    BlueSoul_StopAll();
    Game_SetMenusEnabled(0);
    gEwramData->gameState.sceneController = entity;
    if (gEwramData->gameState.stateFlags & STATE_EVENT_RUNNING)
        SetPlayerInput(1, 0);
    Conversation_Reset();
}

/**
 * @brief 5B510 | Object 0x20 update: run the update of its variant and feed
 * its result to SetPlayerInput while stateFlags bit 0 is set.
 */
void EventSceneControllerUpdate(struct EwramData_EntityData *entity)
{
    u32 playerInputs;
    struct EwramData *ewram = gEwramData;

    if (ewram->gameState.stateFlags & STATE_AREA_NAME_SHOWN)
        ewram->hBlankEffect.requestStop = 1;
    if ((s8)entity->state < 0)
    {
        if (ewram->gameState.textWindowOpening)
            TextWindow_AdjustOpening(-3);
    }
    else if ((gEwramData->inputData.newInput & 8) && (ewram->gameState.systemFlags & 2)
             && !(gEwramData->gameState.stateFlags & STATE_ENDING_SEQUENCE))
    {
        ewram->gameState.textWindowState = 5;
    }
    playerInputs = sEventSceneUpdateFuncs[entity->spawnParam0](entity);
    if (gEwramData->gameState.stateFlags & STATE_EVENT_RUNNING)
    {
        SetPlayerInput(1, playerInputs);
        Game_SetMenusEnabled(0);
    }
    else
    {
        Game_SetMenusEnabled(1);
    }
}

/* Small helpers 0x0805B5E8..0x0805B8F3 (player-relative checks, sound,
 * entity setup from the table sObjectTableGraphics). */

/**
 * @brief 5B5E8 | Create an entity running ObjectTable_UpdateAnimation, set up from entry
 * kind of sObjectTableGraphics (layer 0x11, 0x53D bit 1); NULL on failure (the
 * entity is then deleted twice, as in the original).
 */
struct EwramData_EntityData *Entity_CreateFromObjectTable(s32 kind)
{
    struct ObjectTableGraphics table[12];
    struct EwramData_EntityData *child = Entity_CreateObject(ObjectTable_UpdateAnimation);

    if (child == NULL)
        return NULL;
    memcpy(table, sObjectTableGraphics, sizeof(table));
    if (Entity_LoadGraphics(child, table[kind].graphics, table[kind].palette, 0, table[kind].spriteSheet))
    {
        child->spriteLayer = 0x11;
        child->scrollWithBg = 1;
        return child;
    }
    EntityDelete(child);
    EntityDelete(child);
    return NULL;
}

/**
 * @brief Clear interaction flags, restore the existing input gate, store the deletion request, close text for a negative signed state and reset rendering/conversation state.
 */
void Interaction_End(struct EwramData_EntityData *entity, s32 deleteEntity)
{
    struct EwramData *ewram;

    gEwramData->gameState.stateFlags &= ~STATE_EVENT_RUNNING;
    gEwramData->gameState.stateFlags &= ~STATE_HUD_MESSAGES_BLOCKED;
    Game_SetMenusEnabled(1);
    entity->deleteRequested = deleteEntity;
    if ((s8)entity->state < 0)
        TextWindow_Close();
    ewram = gEwramData;
    ewram->unusedWriteLevel = 0;
    ewram->bgControlMinWriteLevel = 0;
    Display_SetDefaultBlend();
    Conversation_Reset();
}

/**
 * @brief Reset interaction progress, set interaction flag 1, begin the text window and clear the existing input gate; return one.
 */
s32 Interaction_BeginText(s32 textId)
{
    struct GameState *gameState = &gEwramData->gameState;

    gameState->subModeStep = 0;
    gEwramData->gameState.stateFlags |= STATE_EVENT_RUNNING;
    TextWindow_Begin(textId);
    Game_SetMenusEnabled(0);
    return 1;
}

/**
 * @brief Clear interaction flag 1, set the existing input gate and mark caller record byte 4 as one; return the gate setter result.
 */
s32 Interaction_EndAndMarkRecord(u8 *record)
{
    s32 result;

    gEwramData->gameState.stateFlags &= ~STATE_EVENT_RUNNING;
    result = Game_SetMenusEnabled(1);
    record[4] = 1;
    return result;
}

/**
 * @brief Advance animation only; used as the update callback by Entity_CreateFromObjectTable.
 */
void ObjectTable_UpdateAnimation(struct EwramData_EntityData *entity)
{
    Anim_Update(entity);
}

/**
 * @brief 5B754 | Set up entity from entry kind of sObjectTableGraphics
 * (layer 0x11, 0x53D bit 1); delete it and return 0 on failure.
 */
s32 Entity_SetupFromObjectTable(struct EwramData_EntityData *entity, s32 kind)
{
    struct ObjectTableGraphics table[12];

    memcpy(table, sObjectTableGraphics, sizeof(table));
    if (!Entity_LoadGraphics(entity, table[kind].graphics, table[kind].palette, 0, table[kind].spriteSheet))
    {
        EntityDelete(entity);
        return 0;
    }
    entity->spriteLayer = 0x11;
    entity->scrollWithBg = 1;
    return 1;
}

/**
 * @brief 5B7BC | 1 when the player (playerState.player) faces target: the player is
 * left of it and not flipped, or right of/level with it and flipped.
 */
u8 Player_IsFacingEntity(struct EwramData_EntityData *target)
{
    u8 facing = 0;
    struct EwramData_EntityData *player = gEwramData->playerState.player;

    if ((s16)player->posX.part16.integer < (s16)target->posX.part16.integer)
    {
        if (!player->flipX)
            facing = 1;
    }
    else
    {
        /* flipX != 0, read as the whole byte 0x53C (entity offset 0x58);
         * written out because only this form gives the original
         * negs/lsrs #31 sequence. */
        facing = (u32)-(u8)(((u8 *)player)[0x58] & 0x40) >> 31;
    }
    return facing;
}

/**
 * @brief 5B814 | Horizontal position of the player relative to the window
 * [centerX - outerWidth / 2, +outerWidth): 1 outside it; inside it -1 when within
 * innerWidth / 2 of centerX's left side, else 0. (Hypothesis from the
 * arithmetic.)
 */
s32 Player_GetZoneAroundX(u16 centerX, u16 innerWidth, u16 outerWidth)
{
    if ((u16)(gEwramData->playerState.player->posX.part16.integer - (centerX - (outerWidth >> 1))) < outerWidth)
    {
        if ((u16)(gEwramData->playerState.player->posX.part16.integer - (centerX - (innerWidth >> 1))) < innerWidth)
            return -1;
        return 0;
    }
    return 1;
}

/**
 * @brief Return 1 when FLAG_SOMA_AWAKENED is set.
 */
u32 GameFlag_IsSomaAwakened(void)
{
    s32 mask = GameFlag_GetMask(FLAG_SOMA_AWAKENED);

    /* mask != 0, written out for the original negs/orrs/lsrs #31. */
    return (u32)(mask | -mask) >> 31;
}

/**
 * @brief 5B878 | PlaySong(song).
 */
void Sound_Play(u16 song)
{
    PlaySong(song);
}

/**
 * @brief Init of event scene variant 30 (Object 0x20 spawnParam0 = 30,
 * sEventSceneCreateFuncs[30]). Set STATE_ENDING_SCENE, STATE_HUD_HIDDEN and STATE_EVENT_RUNNING, call
 * Hud_ClearGraphics, play song 0x1000 and place the player at (0x78,
 * 0x50) with zero velocity (integer parts) and 0x53C bit 5 set.
 */
void EventSceneEnding_Init(void)
{
    gEwramData->gameState.stateFlags |= STATE_ENDING_SCENE;
    gEwramData->gameState.stateFlags |= STATE_HUD_HIDDEN;
    gEwramData->gameState.stateFlags |= STATE_EVENT_RUNNING;
    Hud_ClearGraphics();
    PlaySong(AUDIO_STOP);
    gEwramData->playerState.player->posX.part16.integer = 0x78;
    gEwramData->playerState.player->posY.part16.integer = 0x50;
    gEwramData->playerState.player->velX.part16.integer = 0;
    gEwramData->playerState.player->velY.part16.integer = 0;
    gEwramData->playerState.player->hidden = 1;
}
