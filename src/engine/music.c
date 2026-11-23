#include "music.h"
#include "core.h"
#include "sram.h"
#include "bg_and_collision.h"
#include "intro_cutscene.h"
#include "bg_effects.h"
#include "room_entities.h"
#include "player_core.h"
#include "soul_inventory.h"
#include "player_effects.h"
#include "player_level.h"
#include "debug_1967.h"
#include "graphics.h"
#include "gameplay.h"
#include "sound.h"
#include "gba.h"
#include "macros.h"
#include "agb_sram.h"
#include "syscalls.h"

#include "constants/main.h"
#include "constants/songs.h"

#include "data/data_0E0334.h"
#include "data/data_0E3464.h"

#include "structs/agb_sram.h"
#include "structs/ewram.h"
#include "structs/main.h"

u16 sAreaSongs[] = {
    2, 8, 7, 5, 6, 9, 1, 3, 10, 4, 11, 12, 12
};

u16 sJuliusAreaSongs[] = {
    20, 8, 7, 5, 6, 9, 2, 3, 10, 4, 11, 12, 12
};

static inline s32 Music_PlayIfChanged(s32 song)
{
    struct GameState *gameState;

    gameState = &gEwramData->gameState;
    if (gameState->currentSong == song)
    {
        return 1;
    }
    if (gameState->currentSong != 0)
    {
        PlaySong(gameState->currentSong | STOP_SONG);
    }
    gameState->currentSong = song;
    PlaySong(song);
    return 0;
}

/**
 * @brief Choose area music and room-specific overrides, apply quiet/transition rules and stop player 0 when required after a song change; return the current-song reuse result.
 */
s32 Music_UpdateRoom(struct GameState *gameState)
{
    struct BgLayerState *bg1Info;
    u8 xOffset;
    u8 yOffset;
    s32 area;
    s32 room;
    s32 alreadyPlaying;
    s32 song;

    bg1Info = &gEwramData->bgInfo[1];
    xOffset = bg1Info->xPos.part8.integer1;
    yOffset = bg1Info->yPos.part8.integer1;

    if (gEwramData->gameState.bossRushMode)
    {
        song = 0x14;
        return Music_PlayIfChanged(song);
    }

    if ((GetSaveRoomFlagFromMapPosition(xOffset, yOffset) != 0) || (GetWarpRoomFlagFromMapPosition(xOffset, yOffset) != 0))
    {
        PlaySong(SONG_QUIET);
        return 0;
    }

    if (CheckRoomTransition(xOffset, yOffset) != 0)
    {
        Music_SubmitCommandOnce(0x1040);
        gameState->currentSong = 0;
        return 0;
    }

    PlaySong(SONG_LOUD);
    area = GetAreaFromMapPosition(xOffset, yOffset);
    room = GetRoomFromMapPosition(xOffset, yOffset);

    if ((gEwramData->playerStatus.currentCharacter == 0) || !(gEwramData->gameState.systemFlags & 2))
    {
        song = sAreaSongs[area];
    }
    else
    {
        song = sJuliusAreaSongs[area];
    }

    gEwramData->specialRoomSong = 0;
    if ((area == 4) && (room == 0xA))
    {
        song = 0x2A;
        gEwramData->specialRoomSong = 0x2A;
    }

    if (gEwramData->specialRoomSong == 0)
    {
        if ((area == 7) && (room == 0x21))
        {
            song = 0x27;
            gEwramData->specialRoomSong = 0x27;
        }

        if (gEwramData->specialRoomSong == 0)
        {
            if (!(gEwramData->gameState.bossEventFlags & 0x20) && (area == 7) && (room == 0x22 || room == 0x23 || room == 0x24 || (room == 0x29) || (room == 0x10)))
            {
                song = 0x29;
                gEwramData->specialRoomSong = 0x29;
            }

            if ((gEwramData->specialRoomSong == 0) && (gEwramData->gameState.bossEventFlags & 4) && (area == 5) && ((room == 6) || (room == 0xE)))
            {
                song = 0x25;
                gEwramData->specialRoomSong = 0x25;
            }
        }
    }

    alreadyPlaying = Music_PlayIfChanged(song);
    if (alreadyPlaying == 0)
    {
        if (gEwramData->gameState.timeStopTimer != 0)
        {
            PlaySong(MUSIC_PLAYER_STOP(0));
        }
    }
    return alreadyPlaying;
}

/**
 * @brief Replay nonzero tracked sound IDs and issue SONG_LOUD when room and music-latch conditions allow it.
 */
void Sound_ReplayTrackedWithVolume(void)
{
    s32 i;
    u16 *tracked;
    u8 yOffset;
    u8 xOffset;
    struct BgLayerState *bg;

    bg = &gEwramData->bgInfo[1];
    xOffset = bg->xPos.part8.integer1;
    yOffset = bg->yPos.part8.integer1;

    tracked = &gEwramData->trackedSoundIds[0];
    i = 3;
    for (; i >= 0;)
    {
        if (tracked[i] != 0)
        {
            PlaySong(tracked[i]);
        }
        i -= 1;
    }

    if ((gEwramData->gameState.bossRushMode != 0) || ((GetSaveRoomFlagFromMapPosition(xOffset, yOffset) == 0) && (GetWarpRoomFlagFromMapPosition(xOffset, yOffset) == 0)))
    {
        if (!(gEwramData->gameState.stateFlags & 0x40000))
        {
            PlaySong(SONG_LOUD);
        }
    }
}

/**
 * @brief Stop nonzero tracked sound IDs without clearing the list and issue SONG_QUIET when room and music-latch conditions allow it.
 */
void Sound_StopTrackedWithVolume(void)
{
    s32 i;
    u16 *tracked;
    u8 yOffset;
    u8 xOffset;
    struct BgLayerState *bg;

    bg = &gEwramData->bgInfo[1];
    xOffset = bg->xPos.part8.integer1;
    yOffset = bg->yPos.part8.integer1;

    i = 3;
    tracked = &gEwramData->trackedSoundIds[0];
    for (; i >= 0;)
    {
        if (tracked[i] != 0)
        {
            PlaySong(tracked[i] | STOP_SONG);
        }
        i -= 1;
    }

    if ((gEwramData->gameState.bossRushMode != 0) || ((GetSaveRoomFlagFromMapPosition(xOffset, yOffset) == 0) && (GetWarpRoomFlagFromMapPosition(xOffset, yOffset) == 0)))
    {
        if (!(gEwramData->gameState.stateFlags & 0x40000))
        {
            PlaySong(SONG_QUIET);
        }
    }
}

/**
 * @brief Select the area table song for the current character/mode, switch current music if needed and issue SONG_QUIET; return 1 if already current, otherwise 0.
 */
s32 Music_PlayAreaQuiet(void)
{
    struct BgLayerState *bg;
    s32 area;
    s32 song;
    s32 alreadyPlaying;

    bg = &gEwramData->bgInfo[1];
    area = GetAreaFromMapPosition((u8) bg->xPos.part8.integer1, (u8) bg->yPos.part8.integer1);

    if ((gEwramData->playerStatus.currentCharacter == 0) || !(gEwramData->gameState.systemFlags & 2))
    {
        song = sAreaSongs[area];
    }
    else
    {
        song = sJuliusAreaSongs[area];
    }

    alreadyPlaying = Music_PlayIfChanged(song);
    PlaySong(SONG_QUIET);
    return alreadyPlaying;
}

/**
 * @brief Return 1 if the song is already current; otherwise stop the old nonzero song, store and play the new ID, then return 0.
 */
s32 Music_SetCurrent(s32 song)
{
    // todo: Music_SetCurrent is the same as Music_PlayIfChanged
    struct GameState *gameState;

    gameState = &gEwramData->gameState;
    if (gameState->currentSong == song)
    {
        return 1;
    }
    if (gameState->currentSong != 0)
    {
        PlaySong(gameState->currentSong | STOP_SONG);
    }
    gameState->currentSong = song;
    PlaySong(song);
    return 0;
}

/**
 * @brief Stop each nonzero tracked sound and clear all four tracking slots.
 */
void Sound_ClearTracked(void)
{
    s32 i;
    u16 *tracked;

    tracked = &gEwramData->trackedSoundIds[0];
    i = 3;
    for (; i >= 0;)
    {
        if (tracked[i] != 0)
        {
            Sound_StopTracked(tracked[i]);
        }
        tracked[i] = 0;
        i -= 1;
    }
}

/**
 * @brief Scan tracking slots from 3 to 0 until matching ID or first empty slot, register where possible and always play; return 1 only on the matching-ID path.
 */
s32 Sound_PlayTracked(s32 song)
{
    u16 *tracked;
    s32 found;
    s32 i;
    
    tracked = &gEwramData->trackedSoundIds[0];
    found = 0;
    i = 3;
    for (; i >= 0; )
    {
        if (tracked[i] == song)
        {
            found = 1;
            break;
        }
        else if (tracked[i] == 0)
        {
            tracked[i] = song;
            break;
        }
        i -= 1;
    }

    PlaySong(song);
    return found;
}

/**
 * @brief Search all four slots first; if absent, register where possible and play. The existing s32 function has no explicit return; its result is unspecified.
 */
s32 Sound_PlayTrackedIfAbsent(s32 song)
{
    u16 *tracked;
    u16 *tracked2;
    s32 found;
    s32 i;

    tracked = &gEwramData->trackedSoundIds[0];
    found = 0;
    i = 3;

    for (; i >= 0; )
    {
        if (tracked[i] == song)
        {
            found = 1;
            break;
        }
        i -= 1;
    }

    if (found == 0)
    {
        // This looks suspicously like Sound_PlayTracked
        tracked2 = &gEwramData->trackedSoundIds[0];
        found = 0;
        i = 3;
        for (; i >= 0; )
        {
            if (tracked2[i] == song)
            {
                found = 1;
                break;
            }
            if (tracked2[i] == 0)
            {
                tracked2[i] = song;
                break;
            }
            i -= 1;
        }
        PlaySong(song);
    }

    // BUG: No return
}

/**
 * @brief Clear the first matching tracking slot and always issue the stop-song command; return 1 when a slot was cleared, otherwise 0.
 */
s32 Sound_StopTracked(s32 song)
{
    u16 *tracked;
    s32 found;
    s32 i;

    tracked = &gEwramData->trackedSoundIds[0];
    found = 0;

    for (i = 3; i >= 0; i -= 1)
    {
        if (tracked[i] == song)
        {
            tracked[i] = 0;
            found = 1;
            break;
        }
    }

    PlaySong(song | STOP_SONG);
    return found;
}

/**
 * @brief Issue AUDIO_RESUME and replay all nonzero tracked sound IDs.
 */
void Sound_ResumeAndReplayTracked(void)
{
    struct EwramData *ewram;
    s32 i;
    u16 *tracked;

    ewram = gEwramData; // TODO: Fake?
    PlaySong(AUDIO_RESUME);

    tracked = &ewram->trackedSoundIds[0];
    for (i = 3; i >= 0; i--)
    {
        if (tracked[i] != 0)
        {
            PlaySong(tracked[i]);
        }
    } 
}

/**
 * @brief Issue AUDIO_PAUSE and stop all nonzero tracked sound IDs without clearing their slots.
 */
void Sound_PauseAndStopTracked(void)
{
    struct EwramData *ewram;
    s32 i;
    u16 *tracked;

    ewram = gEwramData; // TODO: Fake?
    PlaySong(AUDIO_PAUSE);

    tracked = &ewram->trackedSoundIds[0];
    for (i = 3; i >= 0; i--)
    {
        if (tracked[i] != 0)
        {
            PlaySong(tracked[i] | STOP_SONG);
        }
    } 
}

/**
 * @brief Set current music to ID 0, store the override ID separately and play it.
 */
void Music_StartOverride(s32 song)
{
    // struct GameState *temp_r4;
    // s32 var_0;

    // temp_r4 = &gEwramData->gameState;
    // var_0 = temp_r4->currentSong;
    // if (var_0 != 0)
    // {
    //     if (temp_r4->currentSong != 0)
    //     {
    //         PlaySong(temp_r4->currentSong | STOP_SONG); // stop song
    //     }
    //     temp_r4->currentSong = 0;
    //     PlaySong(MUS_DUMMY);
    // }
    Music_PlayIfChanged(0);
    gEwramData->musicOverrideSong = song;
    PlaySong(song);
}

/**
 * @brief Stop the separately stored override song when nonzero and clear its stored ID.
 */
void Music_StopOverride(void)
{
    if (gEwramData->musicOverrideSong != 0)
    {
        PlaySong(gEwramData->musicOverrideSong | STOP_SONG);
        gEwramData->musicOverrideSong = 0;
    }
}

/**
 * @brief Set flag 0x40000 and submit the supplied audio command only when that flag was clear; current callers supply player-0 mute command 0x1040.
 */
void Music_SubmitCommandOnce(s32 song)
{
    if (!(gEwramData->gameState.stateFlags & 0x40000))
    {
        gEwramData->gameState.stateFlags |= 0x40000;
        PlaySong(song);
    }
}

/**
 * @brief 13FA0 | Handle game over game mode
 * 
 * @return s32 Game mode, -1 is no change, -2 is next game mode, else the specified game mode
 */
s32 GameModeGameOverUpdate(void)
{
    struct EwramData_EntityData *textEntity;
    s32 gameMode;
    s32 gfx;

    textEntity = gEwramData->entityData;
    gameMode = GAME_MODE_SAME_MODE;

    switch (gEwramData->gameModeUpdateStage)
    {
        case 0:
            DMA_FILL_32(3, 0, VRAM_BASE + 0xE000, 0x800);
            gUnk_03002CB0.dispCnt = DCNT_OBJ | DCNT_BG1;
            gDisplayRegisters.bgCnt[1] = CREATE_BGCNT(0, 29, BGCNT_HIGH_MID_PRIORITY, BGCNT_SIZE_256x256) | BGCNT_256_COLOR;
            Gfx_CopyResource((u8 *)0x080E5C08, VRAM_BASE, 0);
            Gfx_CopyResource((u8 *)0x080E5C10, VRAM_BASE + 0x4000, 0);
            BgPalette_Load((u8 *)0x080E6A3C);
            BgMap_Setup(1, (struct BgMapMetadata *)0x080E9584, 0, 0);
            BgPalette_StartColorFade(0, 0, 0, 0, 0x10000, 0x20);
            gDisplayRegisters.bldCnt = BLDCNT_BG1_SECOND_TARGET_PIXEL | BLDCNT_ALPHA_BLENDING_EFFECT | BLDCNT_OBJ_FIRST_TARGET_PIXEL;
            gDisplayRegisters.bldAlpha = C_16_2_8(BLDALPHA_MAX_VALUE, 0);
            gEwramData->gameModeTimer = 0x20;
            gEwramData->gameModeUpdateStage += 1;
            gEwramData->gameModeSubStage = 0;
            PlaySong(MUS_15);
            break;

        case 1:
            if (BgPalette_GetPendingFades() == 0)
            {
                BgPalette_StopColorFade();
                gEwramData->gameModeUpdateStage += 1;
                gEwramData->gameModeSubStage = 0;
                gEwramData->gameModeSubStage = 0x80;
            }
            break;

        case 2:
            if (gEwramData->gameModeSubStage == 0)
            {
                if (gEwramData->gameModeTimer != 0)
                {
                    gEwramData->gameModeTimer -= 1;
                    gDisplayRegisters.bldAlpha = C_16_2_8(gEwramData->gameModeTimer >> 1, BLDALPHA_MAX_VALUE - (gEwramData->gameModeTimer >> 1));
                }
                else
                {
                    gEwramData->gameModeUpdateStage += 1;
                    gEwramData->gameModeSubStage = 0;
                }
            }
            else
            {
                if (gEwramData->gameModeSubStage == 1)
                {
                    EntityDelete(textEntity);

                    switch (gEwramData->gameState.language)
                    {
                        case 2:
                            gfx = ObjGfx_LoadResource((u8 *)0x082799A0, 0);
                            ObjPalette_LoadRange((u8 *)0x0825C368, 0, 1, 8);
                            Sprite_Attach(textEntity, (u8 *)0x0827A9FC, gfx, 8);
                            break;
                        case 3:
                            gfx = ObjGfx_LoadResource((u8 *)0x0826E85C, 0);
                            ObjPalette_LoadRange((u8 *)0x0825C368, 0, 1, 8);
                            Sprite_Attach(textEntity, (u8 *)0x0826F894, gfx, 8);
                            break;
                        default:
                            gfx = ObjGfx_LoadResource((u8 *)0x0825B220, 0);
                            ObjPalette_LoadRange((u8 *)0x0825C368, 0, 1, 8);
                            Sprite_Attach(textEntity, (u8 *)0x0825E06C, gfx, 8);
                            break;
                    }

                    textEntity->drawFunc = (u32 *) GameOverText_Draw;
                    textEntity->updateFunc = (u32 *) EntityUpdateNothing;
                }
                gEwramData->gameModeSubStage -= 1;
            }
            break;

        case 3:
            if (gEwramData->inputData.newInput != 0)
            {
                BgPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0x10000, 0, 0x20);
                ObjPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0x10000, 0, 0x20);
                gEwramData->gameModeUpdateStage += 1;
                gEwramData->gameModeSubStage = 0;
                PlaySong(SONG_MUTE);
            }
            break;

        case 4:
            if (BgPalette_GetPendingFades() == 0)
            {
                BgPalette_StopColorFade();
                ObjPalette_StopColorFade();
                gDisplayRegisters.bldCnt = BLDCNT_SCREEN_FIRST_TARGET | BLDCNT_BRIGHTNESS_INCREASE_EFFECT;
                gDisplayRegisters.bldY = BLDY_MAX_VALUE;

                switch (gEwramData->gameState.language)
                {
                    case 2:
                        ObjGfx_Unload(ObjGfx_Find((u8 *)0x082799A0));
                        break;
                    case 3:
                        ObjGfx_Unload(ObjGfx_Find((u8 *)0x0826E85C));
                        break;
                    default:
                        ObjGfx_Unload(ObjGfx_Find((u8 *)0x0825B220));
                        break;
                }

                textEntity->deleteRequested = 1;
                gDisplayRegisters.bldCnt = 0;
                gDisplayRegisters.bldAlpha = 0;
                gDisplayRegisters.bldY = 0;
                gEwramData->bgControlMinWriteLevel = gEwramData->unusedWriteLevel = 0;
                gUnk_03002CB0.dispCnt = DCNT_BLANK;
                gameMode = GAME_MODE_KONAMI_LOGO;
                DMA_FILL_32(3, 0, 0x06000000, 0x18000);
            }
            break;
    }

    Entity_UpdateAll();
    return gameMode;
}

/**
 * @brief Draw the game over sprite frame 0 at (0x80, 0xE).
 */
void GameOverText_Draw(struct EwramData_EntityData *entity)
{
    entity->posX.part16.integer = 0x80;
    entity->posY.part16.integer = 0xE;
    entity->animFrame = 0;
    Sprite_Draw(entity);
}

/**
 * @brief 1430C | Handle credits game mode
 * 
 * @return s32 Game mode, -1 is no change, -2 is next game mode, else the specified game mode
 */
s32 GameModeCreditsUpdate(void)
{
    struct BgLayerState *bg;
    s32 gameMode;

    bg = &gEwramData->bgInfo[1];
    gameMode = GAME_MODE_SAME_MODE;

    switch (gEwramData->gameModeUpdateStage)
    {
        case 0:
            if (gEwramData->gameState.stateFlags & 0x4000)
            {
                gUnk_03002CB0.dispCnt = DCNT_BG3 | DCNT_BG2 | DCNT_BG1;
            }
            else
            {
                gUnk_03002CB0.dispCnt = DCNT_BG2 | DCNT_BG1;
            }

            gDisplayRegisters.bgCnt[1] = CREATE_BGCNT(0, 29, BGCNT_LOW_PRIORITY, BGCNT_SIZE_256x256);
            gDisplayRegisters.bgCnt[2] = CREATE_BGCNT(0, 30, BGCNT_LOW_MID_PRIORITY, BGCNT_SIZE_256x256);
            gDisplayRegisters.bgCnt[3] = CREATE_BGCNT(0, 31, BGCNT_HIGH_MID_PRIORITY, BGCNT_SIZE_256x256);
            gDisplayRegisters.bldCnt = BLDCNT_SCREEN_FIRST_TARGET | BLDCNT_BRIGHTNESS_DECREASE_EFFECT;
            gDisplayRegisters.bldY = BLDY_MAX_VALUE;
            gEwramData->gameModeUpdateStage += 1;
            gEwramData->gameModeSubStage = 0;
            break;

        case 1:
            PlaySong(MUS_26);
            Gfx_CopyResource((u8 *)0x080E5BC8, VRAM_BASE, 0);
            Gfx_CopyResource((u8 *)0x080E5BD0, VRAM_BASE + 0x2000, 0);
            Gfx_CopyResource((u8 *)0x080E5BD8, VRAM_BASE + 0x4000, 0);
            BgPalette_LoadRange((u8 *)0x080E6028, 0, 8, 0);
            BgMap_Setup(1, (struct BgMapMetadata *)0x080E8F04, 0, 0x50);

            if (gEwramData->gameState.stateFlags & 0x4000)
            {
                BgMap_Setup(2, (struct BgMapMetadata *)0x080E7B54, 0, 0x50);
                BgMap_Setup(3, (struct BgMapMetadata *)0x080E81E4, 0, 0x50);
            }
            else
            {
                BgMap_Setup(2, (struct BgMapMetadata *)0x080E8874, 0, 0x50);
            }

            gEwramData->gameModeUpdateStage += 1;
            gEwramData->gameModeSubStage = 0;
            break;

        case 2:
            gDisplayRegisters.bldY = 0;
            gEwramData->gameModeUpdateStage += 1;
            gEwramData->gameModeSubStage = 0;
            break;

        case 3:
            BgMap_Scroll(1, 0, 0x8400);
            BgMap_Scroll(2, 0, 0x8400);

            if (gEwramData->gameState.stateFlags & 0x4000)
            {
                BgMap_Scroll(3, 0, 0x8400);
            }

            if ((bg->yVel.whole == 0) && (gEwramData->inputData.newInput & (KEY_A | KEY_START)))
            {
                PlaySong(SONG_MUTE);
                gDisplayRegisters.bldCnt = BLDCNT_SCREEN_FIRST_TARGET | BLDCNT_BRIGHTNESS_INCREASE_EFFECT;
                gEwramData->gameModeTimer = 0;
                gEwramData->gameModeUpdateStage += 1;
                gEwramData->gameModeSubStage = 0;
            }
            break;

        case 4:
            gDisplayRegisters.bldY = gEwramData->gameModeTimer >> 3;
            if (gDisplayRegisters.bldY < BLDY_MAX_VALUE)
            {
                gEwramData->gameModeTimer += 1;
            }
            else
            {
                Game_ResetSystems(&gEwramData->gameState);
                gDisplayRegisters.bldCnt = BLDCNT_SCREEN_FIRST_TARGET | BLDCNT_BRIGHTNESS_INCREASE_EFFECT;
                gDisplayRegisters.bldY = BLDY_MAX_VALUE;
                gameMode = GAME_MODE_KONAMI_LOGO;
            }
            break;
    }

    return gameMode;
}
