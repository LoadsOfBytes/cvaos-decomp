#include "code_08008750.h"
#include "code_08009A0.h"
#include "code_08001004.h"
#include "code_08001194.h"
#include "code_08002454.h"
#include "code_08005894.h"
#include "code_080096AC.h"
#include "code_0800B700.h"
#include "code_0800CB00.h"
#include "code_0800F1FC.h"
#include "code/code_08014548.h"
#include "code/code_080211F0.h"
#include "code_08032444.h"
#include "code_08032E4C.h"
#include "code_08033CAC.h"
#include "code_08035930.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "code_080D73B8.h"
#include "gba.h"
#include "macros.h"
#include "agb_sram.h"
#include "syscalls.h"

/*
 * Intro cutscene entities (token-identical expansion):
 * - drifting sprite: fade timer (0x120 frames, sets the blend);
 * - shrinking / floating sprite: affine scale; floating sprite: x / y sway
 *   phases;
 * - text (IntroCutsceneText_PrintNextLine): next text row and the position in
 *   the script.
 */
#define INTRO_DRIFT_FADE(e) e->unk_4F8.whole
#define INTRO_SPRITE_SCALE(e) e->unk_500.whole
#define INTRO_SWAY_X(e) e->unk_4F8.whole
#define INTRO_SWAY_Y(e) e->unk_4FC.whole
#define INTRO_TEXT_ROW(e) e->unk_4F8.part8.unk_4F9
#define INTRO_TEXT_POS(e) e->unk_4FC.part16.unk_4FC
/* Initial BG0 vertical scroll of the text, stored with the register and never read. */
#define INTRO_TEXT_START_SCROLL(e) e->unk_4FC.part16.unk_4FE[0]

#include "constants/main.h"
#include "constants/songs.h"

#include "data/data_0E0334.h"
#include "data/data_0E3464.h"

#include "structs/agb_sram.h"
#include "structs/ewram.h"
#include "structs/main.h"

const u8 sBossRushStartItems[] = {
    0x3, 0x4, 0x5, 0x6, 0x17, 0x18, 0xFF,
};

/**
 * @brief 8750 | Handle boss rush menu game mode
 * 
 * @return s32 Game mode, -1 is no change, -2 is next game mode, else the specified game mode
 */
s32 GameModeBossRushMenuUpdate(void)
{
    struct EwramData_EntityData *entity;
    s32 gameMode;
    s32 pendingFades;
    s32 choice;
    s32 i;

    entity = &gEwramData->entityData[4];
    gameMode = GAME_MODE_SAME_MODE;

    switch (gEwramData->gameModeUpdateStage)
    {
        case 0:
            SaveSelect_Open(entity);
            BgPalette_StartColorFade(0, 0, 0, 0, 0x10000, 0x10);
            ObjPalette_StartColorFade(0, 0, 0, 0, 0x10000, 0x10);
            gEwramData->gameModeUpdateStage = 1;
            gEwramData->gameModeSubStage = 0;
            break;
    
        case 1:
            Entity_UpdateAll();
            pendingFades = BgPalette_GetPendingFades();
            if (pendingFades == 0)
            {
                ObjPalette_StopColorFade();
                BgPalette_StopColorFade();
                gEwramData->gameModeUpdateStage = 3;
                gEwramData->gameModeSubStage = pendingFades;
            }
            break;
    
        case 3:
            choice = SaveSelect_Update(entity);
            if (choice == -1)
            {
                PlaySong(SE_240);
                gameMode = GAME_MODE_TITLE_SCREEN;
            }
            else if (choice != 0)
            {
                PlaySong(SE_244);
                gEwramData->gameModeUpdateStage = 4;
                gEwramData->gameModeSubStage = 0;
            }
            break;
    
        case 4:
            EntityDelete(&gEwramData->entityData[4]);
            PlaySong(AUDIO_STOP);
            EntityDeleteAll();
            ObjTiles_Init();
            Obj_Init();
            Sprite_Init();
            Palette_Init();
            Text_Initialize();
            HitBox_ClearAll();
            gEwramData->bgControlMinWriteLevel = gEwramData->unusedWriteLevel = 3;
            gUnk_03002CB0.dispCnt = DCNT_BG0;
            gDisplayRegisters.bldCnt = BLDCNT_SCREEN_SECOND_TARGET | BLDCNT_BRIGHTNESS_DECREASE_EFFECT | BLDCNT_SCREEN_FIRST_TARGET;
            gDisplayRegisters.bldY = BLDY_MAX_VALUE;
            gEwramData->gameModeUpdateStage = 5;
            gEwramData->gameModeSubStage = 0;
            gEwramData->gameState.difficultyMode = 1;
            gEwramData->gameState.shownAreaNameFlags = -1;
            gEwramData->gameState.gameFlagsTail[1] &= ~0x40000;
            DMA_FILL_32(3, 0, gEwramData->playerStatus.itemInventory, 0x20);
            if (gEwramData->playerStatus.currentCharacter == 0)
            {
                for (i = 0; sBossRushStartItems[i] != 0xFF; i++)
                {
                    gEwramData->playerStatus.itemInventory[sBossRushStartItems[i]] = 1;
                }
                Player_SetLevel(0x28);
            }
            else
            {
                Player_InitBaseLevelStats();
                Player_ApplyJuliusStatGrowth();
                Player_ApplyJuliusStatGrowth();
                Player_ApplyJuliusStatGrowth();
                Player_ApplyJuliusStatGrowth();
                Player_ApplyJuliusStatGrowth();
                Player_ApplyJuliusStatGrowth();
                Player_ApplyJuliusStatGrowth();
                Player_ApplyJuliusStatGrowth();
            }
            gEwramData->playerStatus.currentHP = gEwramData->playerStatus.maxHP;
            gEwramData->playerStatus.currentMP = gEwramData->playerStatus.maxMP;
            Player_RefreshEquipmentStats();
            break;
    
        case 5:
            gEwramData->gameState.bossRushMode = 1;
            gEwramData->gameState.currentRoomDefinition = (struct RoomDefinition *)0x085247B4;
            gameMode = GAME_MODE_IN_GAME;
            break;
    }

    return gameMode;
}

/**
 * @brief 89D0 | Handle empty game mode
 * 
 * @return s32 Game mode, -1 is no change, -2 is next game mode, else the specified game mode
 */
s32 GameModeUnk9Update(void)
{
    return GAME_MODE_SAME_MODE;
}

// TODO: No inline?
static inline void GameModeIntroCutsceneUpdate_inline(s32 alpha)
{
    gDisplayRegisters.bldAlpha = C_16_2_8(alpha, BLDALPHA_MAX_VALUE - alpha);
}

/**
 * @brief 89D8 | Handle intro cutscene game mode
 * 
 * @return s32 Game mode, -1 is no change, -2 is next game mode, else the specified game mode
 */
s32 GameModeIntroCutsceneUpdate(void)
{
    s16 *tilemap;
    s32 alpha;
    s32 i;
    s32 gameMode;
    struct EwramData *ewram;
    struct EwramData *ewram2;
    struct EwramData_EntityData *textEntity;
    s32 step;
    struct SavedDisplayState *saved;
    struct BgLayerState *bg;

    saved = &gEwramData->gameState.savedDisplay;
    textEntity = gEwramData->entityData;
    gameMode = GAME_MODE_SAME_MODE;

    switch (gEwramData->gameModeUpdateStage)
    {
        case 0:
            gEwramData->hBlankEffect.requestStop = 1;
            EntityDeleteAll();
            ObjTiles_Init();
            Obj_Init();
            Sprite_Init();
            Palette_Init();
            Text_Initialize();
            HitBox_ClearAll();
            DMA_COPY_32(3, &gDisplayRegisters.bgCnt[1], saved->bgControlBytes, sizeof(saved->bgControlBytes));
            gUnk_03002CB0.dispCnt = DCNT_OBJ | DCNT_BG3 | DCNT_BG2 | DCNT_BG1 | DCNT_BG0;
            BgPalette_Load((u8 *)0x080E6634);
            Gfx_CopyResource((u8 *)0x080E5BF0, VRAM_BASE + 0x8000, 0);
            Gfx_CopyResource((u8 *)0x080E5BF8, VRAM_BASE + 0xA000, 0);
            BgMap_Setup(1, (struct BgMapMetadata *)0x080E9464, 0, 0x160);
            BgMap_Setup(2, (struct BgMapMetadata *)0x080E9244, 0, 0x260);
            BgMap_Setup(3, (struct BgMapMetadata *)0x080E9354, 0, 0xC9);
            gDisplayRegisters.bgCnt[0] = CREATE_BGCNT(0, 28, BGCNT_HIGH_PRIORITY, BGCNT_SIZE_256x256);
            gDisplayRegisters.bgCnt[1] = CREATE_BGCNT(2, 29, BGCNT_HIGH_MID_PRIORITY, BGCNT_SIZE_256x256);
            gDisplayRegisters.bgCnt[2] = CREATE_BGCNT(2, 30, BGCNT_LOW_MID_PRIORITY, BGCNT_SIZE_256x256);
            gDisplayRegisters.bgCnt[3] = CREATE_BGCNT(2, 31, BGCNT_LOW_PRIORITY, BGCNT_SIZE_256x256);
            tilemap = (s16 *)0x0600E000;
            i = 0;
            for ( ; i < 0x400; )
            {
                *tilemap = i - 0x1000;
                tilemap += 1;
                i += 1;
            }
            Text_SetTileBuffer(VRAM_BASE);
            Text12_SetRegion(0, 0, 0x29, 0x15);
            DMA_FILL_32(3, 0, 0x06000000, 0x8000);
            BgPalette_LoadRange((u8 *)0x081183F4, 0, 1, 0xF);
            Text_SetForegroundColor(8);
            Text_SetBackgroundColor(0);
            Obj_SetBgLayers(0, 0x18, 0x14, 0x1F);
            Bg_SyncPriorities();
            IntroCutscene_CreateEntities(textEntity);
            gDisplayRegisters.bldCnt = BLDCNT_SCREEN_FIRST_TARGET | BLDCNT_BRIGHTNESS_DECREASE_EFFECT;
            gDisplayRegisters.bldY = BLDY_MAX_VALUE;
            gEwramData->gameModeTimer = 0;
            gEwramData->gameModeUpdateStage += 1;
            gEwramData->gameModeSubStage = 0;
            break;
    
        case 1:
            gDisplayRegisters.bldY = BLDY_MAX_VALUE - (gEwramData->gameModeTimer >> 3);
            if (gDisplayRegisters.bldY != 0)
            {
                if (gEwramData->gameModeTimer == 0)
                {
                    PlaySong(MUS_21);
                }
                gEwramData->gameModeTimer += 1;
            }
            else
            {
                gDisplayRegisters.bldCnt = BLDCNT_SCREEN_SECOND_TARGET | BLDCNT_BRIGHTNESS_DECREASE_EFFECT;
                gEwramData->gameModeTimer = 0;
                gEwramData->gameModeUpdateStage += 1;
                gEwramData->gameModeSubStage = 0;
            }
            break;
    
        case 2:
            gEwramData->gameModeUpdateStage += 1;
            gEwramData->gameModeSubStage = 0;
            break;
    
        case 3:
            bg = &gEwramData->bgInfo[1];
            gDisplayRegisters.bgOfs[1].hOfs = 0;
            BgMap_Scroll(1, 0, -0x1C00);
            BgMap_Scroll(2, 0, -0x3200);
            BgMap_Scroll(3, 0, -0x1000);
            if (bg->yVel.whole == 0)
            {
                gDisplayRegisters.bldCnt = BLDCNT_SCREEN_SECOND_TARGET | BLDCNT_ALPHA_BLENDING_EFFECT;
                gEwramData->gameModeTimer = 0;
                textEntity->hidden = 0;
                gEwramData->gameModeUpdateStage += 1;
                gEwramData->gameModeSubStage = 0;
            }
            break;
    
        case 4:
            gEwramData->gameModeTimer += 1;
            if (!(gEwramData->gameModeTimer & 7))
            {
                step = gEwramData->gameModeTimer >> 3;
                alpha = (gEwramData->gameModeTimer & 2) + (0x10 - (gEwramData->gameModeTimer >> 0x4));
                gDisplayRegisters.bldAlpha = C_16_2_8(alpha, BLDALPHA_MAX_VALUE - alpha);
                if (step > 0xA)
                {
                    gEwramData->gameModeTimer = 0;
                    gEwramData->gameModeUpdateStage += 1;
                    gEwramData->gameModeSubStage = 0;
                }
            }
            break;
    
        case 5:
            gEwramData->gameModeTimer += 1;
            GameModeIntroCutsceneUpdate_inline((gEwramData->gameModeTimer & 1) + ((RandomNumberGenerator() & 1) * 2) + 0xD);
            if ((gEwramData->gameModeTimer > 0x77) || (gEwramData->inputData.newInput & (KEY_A | KEY_START)))
            {
                BgPalette_StartColorFade(0, 0, 0, 0x10000, 0, 0x78);
                ObjPalette_StartColorFade(0, 0, 0, 0x10000, 0, 0x78);
                PlaySong(SONG_MUTE);
                gEwramData->gameModeUpdateStage += 1;
                gEwramData->gameModeSubStage = 0;
            }
            break;
    
        case 6:
            if (BgPalette_GetPendingFades() == 0)
            {
                ewram2 = gEwramData;
                BgPalette_StopColorFade();
                ObjPalette_StopColorFade();
                gDisplayRegisters.bldCnt = BLDCNT_SCREEN_FIRST_TARGET | BLDCNT_BRIGHTNESS_DECREASE_EFFECT;
                gDisplayRegisters.bldY = BLDY_MAX_VALUE;
                gEwramData->hBlankEffect.requestStop = 1;
                EntityDeleteAll();
                ObjTiles_Init();
                Obj_Init();
                Sprite_Init();
                Palette_Init();
                Text_Initialize();
                HitBox_ClearAll();
                DMA_COPY_32(3, &ewram2->gameState.savedDisplay.bgControlBytes, &gDisplayRegisters.bgCnt[1], sizeof(ewram2->gameState.savedDisplay.bgControlBytes));
                PlaySong(SONG_MUTE);
                gEwramData->gameModeUpdateStage += 1;
                gEwramData->gameModeSubStage = 0;
                if (gEwramData->gameModeCounter == 0)
                {
                    gameMode = GAME_MODE_IN_GAME;
                }
                else
                {
                    gameMode = GAME_MODE_KONAMI_LOGO;
                }
            }
            break;
    }

    if ((gEwramData->inputData.newInput & (KEY_A | KEY_START)) && (gEwramData->gameModeUpdateStage < 6))
    {
        gEwramData->gameModeUpdateStage = 0x63;
        gEwramData->gameModeSubStage = 0;
        ewram = gEwramData;
        BgPalette_StopColorFade();
        ObjPalette_StopColorFade();
        gDisplayRegisters.bldCnt = BLDCNT_SCREEN_FIRST_TARGET | BLDCNT_BRIGHTNESS_DECREASE_EFFECT;
        gDisplayRegisters.bldY = BLDY_MAX_VALUE;
        gEwramData->hBlankEffect.requestStop = 1;
        EntityDeleteAll();
        ObjTiles_Init();
        Obj_Init();
        Sprite_Init();
        Palette_Init();
        Text_Initialize();
        HitBox_ClearAll();
        DMA_COPY_32(3, &ewram->gameState.savedDisplay.bgControlBytes, &gDisplayRegisters.bgCnt[1], sizeof(ewram->gameState.savedDisplay.bgControlBytes));
        PlaySong(SONG_MUTE);
        if (gEwramData->gameModeCounter == 0)
        {
            gameMode = GAME_MODE_IN_GAME;
        }
        else
        {
            gDisplayRegisters.bldCnt = BLDCNT_SCREEN_FIRST_TARGET | BLDCNT_BRIGHTNESS_DECREASE_EFFECT;
            gDisplayRegisters.bldY = BLDY_MAX_VALUE;
            gameMode = GAME_MODE_TITLE_SCREEN;
        }
    }
    Entity_UpdateAll();
    if (gameMode == GAME_MODE_IN_GAME)
    {
        Game_SetStartPosition(0);
    }
    return gameMode;
}

struct GameStartPosition {
    u8 area;
    u8 room;
    u16 cameraX;
    u16 cameraY;
    u16 playerX;
    u16 playerY;
    u8 pad_A[0xC - 0xA];
};

// Second entry seems to be unused
struct GameStartPosition sGameStartPositions[2] = {
    [0] = {
        .area = 0x00,
        .room = 0x00,
        .cameraX = 0x0020,
        .cameraY = 0x0200,
        .playerX = 0x0078,
        .playerY = 0x008D
    },
    [1] = {
        .area = 0x00,
        .room = 0x00,
        .cameraX = 0x0000,
        .cameraY = 0x0200,
        .playerX = 0x0078,
        .playerY = 0x008D
    },
};

// Unused data? it could be part of the above struct, but unlikely
u8 sUnk_084F0DA4[] = {
    0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x1, 0x7, 0x8, 0x1, 0x0
};

/**
 * @brief Init a new game if needed and set the start area, room and position
 * from entry n of sGameStartPositions.
 */
void Game_SetStartPosition(s32 arg0)
{
    Player_InitNewGameIfNeeded();
    gEwramData->gameState.currentArea =  sGameStartPositions[arg0].area;
    gEwramData->gameState.currentRoom =  sGameStartPositions[arg0].room;
    gEwramData->gameState.savedCameraX = sGameStartPositions[arg0].cameraX;
    gEwramData->gameState.savedCameraY = sGameStartPositions[arg0].cameraY;
    gEwramData->gameState.savedPlayerX = sGameStartPositions[arg0].playerX;
    gEwramData->gameState.savedPlayerY = sGameStartPositions[arg0].playerY;
    gEwramData->gameState.roomData = GetRoomPointer(gEwramData->gameState.currentArea, gEwramData->gameState.currentRoom);
}

/**
 * @brief Intro sprite at (0x78, 0x9E) that rises in steps with its animation
 * and the BG1 scroll; deleted when the scroll passes 0x20.
 */
void IntroCutscene_UpdateRisingSprite(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg;

    bg = &gEwramData->bgInfo[1];
    switch (entity->state)
    {
        case 0:
            entity->drawFunc = (u32 *) Sprite_DrawEntity;
            entity->spriteLayer = 0x16;
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->posX.part16.integer = 0x78;
            entity->posY.part16.integer = 0x9E;
            entity->state += 1;
            /* fallthrough */
        case 1:
            if ((u32)bg->yPos.whole < 0x200000)
            {
                entity->deleteRequested = 1;
            }
            switch (entity->animStep)
            {
                case 2:
                case 0:
                    entity->velY.whole = 0;
                    break;
                case 1:
                case 3:
                    entity->velY.whole = -0x2E00;
                    break;
            }
            entity->posY.whole += (entity->velY.whole - bg->yVel.whole);
            Anim_Update(entity);
            break;
    }
}

/**
 * @brief Draw the sprite and a mirrored copy 0x80 px to the side.
 */
void IntroCutscene_DrawMirrored(struct EwramData_EntityData *entity)
{
    s32 oldX;
    u32 flipX;

    oldX = entity->posX.whole;
    flipX = entity->flipX;
    Sprite_Draw(entity);
    entity->flipX ^= 1;
    if (entity->subState == 0)
    {
        entity->posX.part16.integer -= 0x80;
    }
    else
    {
        entity->posX.part16.integer += 0x80;
    }
    Sprite_Draw(entity);
    entity->posX.whole = oldX;
    entity->flipX = flipX;
}

/**
 * @brief Semi-transparent mirrored sprite drifting sideways; the first one
 * fades the blend alpha after the scroll passes 0x20.
 */
void IntroCutscene_UpdateDriftingSprite(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg;
    s32 alpha;
    s32 alpha2;

    bg = &gEwramData->bgInfo[1];

    switch (entity->state)
    {
        case 0:
            entity->animFrame = entity->subState + 8;
            entity->drawFunc = (u32 *) IntroCutscene_DrawMirrored;
            entity->objMode = 1;
            entity->spriteLayer = 0x19;
            entity->velY.whole = 0x1000;
            entity->velX.whole = -0x1000;
            if (entity->subState == 0)
            {
                entity->posX.part16.integer = 0xF8;
                alpha = 10;
                gDisplayRegisters.bldAlpha = C_16_2_8(alpha,  BLDALPHA_MAX_VALUE - alpha);
            }
            else
            {
                entity->posX.part16.integer = 0xFFF8;
                entity->velX.whole = -entity->velX.whole;
            }
            entity->state += 1;
            /* fallthrough */
        case 1:
            entity->posX.whole += entity->velX.whole;
            if ((u32)bg->yPos.whole < 0x200000)
            {
                INTRO_DRIFT_FADE(entity) = 0x120;
                entity->state += 1;
            }
            break;

        case 2:
            entity->posX.whole += entity->velX.whole;
            if (INTRO_DRIFT_FADE(entity) != 0)
            {
                INTRO_DRIFT_FADE(entity) -= 1;
                if (entity->subState == 0)
                {
                    alpha2 = 0x10 - (INTRO_DRIFT_FADE(entity) / 48);
                    gDisplayRegisters.bldAlpha = C_16_2_8(alpha2,  BLDALPHA_MAX_VALUE - alpha2);
                }
            }
            else
            {
                entity->deleteRequested = 1;
            }
            break;
    }

    bg = &gEwramData->bgInfo[3];
    entity->posY.whole = 0x5A0000 - bg->yPos.whole;
}

/**
 * @brief Affine sprite that appears with sound 318 when the scroll passes
 * 0x1E, rises with damping and shrinks to nothing.
 */
void IntroCutscene_UpdateShrinkingSprite(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg;
    s16 affineIndex;

    bg = &gEwramData->bgInfo[1];

    switch (entity->state)
    {
        case 0:
            affineIndex = ObjAffine_Alloc(1);
            if (affineIndex < 0)
            {
                EntityDelete(entity);
                break;
            }
            entity->affineIndex = affineIndex;
            entity->doubleSize = entity->affine = 1;
            entity->spriteLayer = 0x1A;
            Anim_StartIndexed(entity, 1, 0, 1);
            Anim_Update(entity);
            INTRO_SPRITE_SCALE(entity) = 0x20000;
            entity->posX.part16.integer = 0x78;
            entity->posY.part16.integer = 0xB4;
            entity->velY.whole = -0x16000;
            entity->state += 1;
            /* fallthrough */
        case 1:
            if ((u32)bg->yPos.whole > 0x1E0000)
                break;

            PlaySong(SE_318);
            entity->drawFunc = (u32 *) Sprite_DrawEntity;
            entity->state += 1;
            /* fallthrough */
        case 2:
            EntityDampVelocityY(entity, 0x50);
            entity->posY.whole += entity->velY.whole;
            Anim_Update(entity);
            ObjAffine_Set(0, INTRO_SPRITE_SCALE(entity), INTRO_SPRITE_SCALE(entity), 0, 0, entity->affineIndex);
            if (INTRO_SPRITE_SCALE(entity) != 0)
            {
                INTRO_SPRITE_SCALE(entity) -= 0x200;
            }
            else
            {
                ObjAffine_Free(entity->affineIndex);
                entity->deleteRequested = 1;
            }
            break;
    }
}

/**
 * @brief Affine sprite that floats upwards on a Lissajous path and is freed
 * below y 0x12C.
 */
void IntroCutscene_UpdateFloatingSprite(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg;
    s32 swayX;
    s32 swayY;
    s16 affineIndex;

    bg = &gEwramData->bgInfo[1];

    switch (entity->state)
    {
        case 0:
            affineIndex = ObjAffine_Alloc(1);
            if (affineIndex < 0)
            {
                EntityDelete(entity);
                break;
            }
            entity->affineIndex = affineIndex;
            entity->doubleSize = entity->affine = 1;
            entity->posX.part16.integer = 0xD2;
            entity->posY.part16.integer = 0x87;
            INTRO_SPRITE_SCALE(entity) = 0x10000;
            entity->drawFunc = (u32 *) Sprite_DrawEntity;
            entity->spriteLayer = 0x16;
            Anim_StartIndexed(entity, 1, 0, 1);
            entity->state += 1;
            /* fallthrough */
        case 1:
            INTRO_SWAY_X(entity) += 0x600;
            INTRO_SWAY_Y(entity) += 0x800;
            swayX = Sin(INTRO_SWAY_X(entity) + 0x4000) * 0xA0;
            if (swayX < 0)
            {
                swayX += 0xFF;
            }
            entity->velX.whole = swayX >> 8;
            swayY = Sin(INTRO_SWAY_Y(entity)) * 0xA0;
            if (swayY < 0)
            {
                swayY += 0xFF;
            }
            entity->velY.whole = swayY >> 8;
            entity->posY.whole -= 0x1400 - (entity->velY.whole - bg->yVel.whole);
            entity->posX.whole += entity->velX.whole;
            Anim_Update(entity);
            ObjAffine_Set(0, INTRO_SPRITE_SCALE(entity), INTRO_SPRITE_SCALE(entity), 0, 0, entity->affineIndex);
            if (entity->posY.part16.integer > 0x12C)
            {
                ObjAffine_Free(entity->affineIndex);
                entity->deleteRequested = 1;
            }
            return;
    }
}

/**
 * @brief Print the next line of the intro story text (text 0xB) into the
 * scrolling text window.
 */
s32 IntroCutsceneText_PrintNextLine(struct EwramData_EntityData *entity)
{
    s32 pos;
    u8 *script;
    u8 *text;
    s32 index;
    s32 more;
    s32 argPos;

    text = Text_GetLocalizedEntry(0xB);
    more = 1;
    Text12_SetRegion(4, Mod(INTRO_TEXT_ROW(entity), 0x15), 0x25, 1);
    Text12_ClearRegion();
    pos = INTRO_TEXT_POS(entity);
    if (pos < 0)
    {
        INTRO_TEXT_ROW(entity) += 1;
    }
    else
    {
        script = text + 2;
        while (more != 0)
        {
            switch (script[pos] - 1)
            {
                case 0:
                    argPos = pos + 1;
                    index = script[argPos];
                    pos += 2;
                    Text12_Print(Text_GetLocalizedEntry(index), 0);
                    break;
                case 4:
                    pos += 1;
                    break;
                case 5:
                case 8:
                    pos += 1;
                    more = 0;
                    break;
                case 9:
                    pos = -1;
                    more = 0;
                    break;
                default:
                    pos = Text12_Print(text, pos);
                    break;
            }
        }

        INTRO_TEXT_ROW(entity) += 1;
        INTRO_TEXT_POS(entity) = pos;
    }
    return pos;
}

/**
 * @brief Turn the entity into the story text scroller and create the five
 * intro sprites.
 */
void IntroCutscene_CreateEntities(struct EwramData_EntityData *entity)
{
    // TODO: cleanup
    s32 gfx;
    struct EwramData_EntityData *rising;
    struct EwramData_EntityData *shrinking;
    struct EwramData_EntityData *floating;
    struct EwramData_EntityData *drifting;
    struct EwramData_EntityData *drifting2;
    s32 spriteGfx;

    EntityDelete(entity);
    gfx = ObjGfx_LoadResource((u8 *)0x08160490, 0);
    ObjPalette_LoadRange((u8 *)0x0820972C, 0, 4, 0);
    Sprite_Attach(entity, (u8 *)0x0820CB60, gfx, 0);
    entity->animFrame = 0;
    entity->drawFunc = (u32 *) Sprite_DrawEntity;
    entity->posX.part16.integer = 0x78;
    entity->posY.part16.integer = 0x50;
    entity->velY.whole = 0x1000;
    entity->spriteLayer = 0x1A;
    entity->objMode = 1;
    entity->updateFunc = (u32 *) IntroCutsceneText_Update;
    entity->hidden = 1;
    gDisplayRegisters.bgOfs[0].vOfs = INTRO_TEXT_START_SCROLL(entity) = -0xA4;

    rising = EntityCreateInRange(0x49, 0xCF, (u32*)IntroCutscene_UpdateRisingSprite);
    spriteGfx = ObjGfx_LoadResource((u8 *)0x08160498, 0);
    Sprite_Attach(rising, (u8 *)0x0820CD00, spriteGfx, 2);

    shrinking = EntityCreateInRange(0x49, 0xCF, (u32*)IntroCutscene_UpdateShrinkingSprite);
    spriteGfx = ObjGfx_LoadResource((u8 *)0x08160498, 0);
    Sprite_Attach(shrinking, (u8 *)0x0820CD00, spriteGfx, 3);

    floating = EntityCreateInRange(0x49, 0xCF, (u32*)IntroCutscene_UpdateFloatingSprite);
    spriteGfx = ObjGfx_LoadResource((u8 *)0x08160498, 0);
    Sprite_Attach(floating, (u8 *)0x0820CD00, spriteGfx, 3);

    drifting = EntityCreateInRange(0x49, 0xCF, (u32*)IntroCutscene_UpdateDriftingSprite);
    spriteGfx = ObjGfx_LoadResource((u8 *)0x08160498, 0);
    Sprite_Attach(drifting, (u8 *)0x0820CD00, spriteGfx, 1);

    drifting2 = EntityCreateInRange(0x49, 0xCF, (u32*)IntroCutscene_UpdateDriftingSprite);
    spriteGfx = ObjGfx_LoadResource((u8 *)0x08160498, 0);
    Sprite_Attach(drifting2, (u8 *)0x0820CD00, spriteGfx, 1);
    drifting2->subState = 1;
}

/**
 * @brief Unreferenced: create an intro sprite with the given update and
 * palette.
 */
struct EwramData_EntityData* IntroCutscene_CreateSprite(u32 *updateFunc, u8 palette)
{
    // TODO: inlined function for 94AC?
    struct EwramData_EntityData *child;
    s32 gfx;

    child = EntityCreateInRange(0x49, 0xCF, updateFunc);
    gfx = ObjGfx_LoadResource((u8 *)0x08160498, 0);
    Sprite_Attach(child, (u8 *)0x0820CD00, gfx, palette);
    return child;
}

/**
 * @brief Scroll the story text one line every 5 frames, printing a new line
 * every 12 pixels; unload when the cutscene ends.
 */
void IntroCutsceneText_Update(struct EwramData_EntityData *entity)
{
    s32 scroll;

    entity->timer = Mod(entity->timer + 1, 5);
    if (entity->timer == 0)
    {
        scroll = gDisplayRegisters.bgOfs[0].vOfs;
        scroll++;
        if (Mod(scroll + 0xF0, 0xC) == 0)
        {
            IntroCutsceneText_PrintNextLine(entity);
        }
        gDisplayRegisters.bgOfs[0].vOfs = scroll;
    }
    if (gEwramData->gameModeUpdateStage > 6)
    {
        ObjGfx_Unload(ObjGfx_Find((u8 *)0x08160490));
        entity->deleteRequested = 1;
    }
}
