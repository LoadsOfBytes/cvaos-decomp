/*
 * Event scenes 00, 01, 02, 29 and 31 (and the update of scene 30).
 */
#include "core.h"
#include "map_screen.h"
#include "map_flags.h"
#include "music.h"
#include "graphics.h"
#include "gba/dma.h"
#include "structs/ewram.h"
#include "structs/main.h"
#include "gba/display.h"
#include "sound.h"
#include "save_data.h"
#include "effects.h"
#include "entity_common.h"
#include "math_anim.h"
#include "palette.h"
#include "scene30.h"
#include "shop_update.h"
#include "text_window.h"
#include "data/data_4F1444.h"
#include "data/data_118B48.h"
#include "player_soma.h"
#include "bg_and_collision.h"

extern struct RoomDefinition sEndingRoomDefinition;

s32 EventScene31Debris_Update(struct EwramData_EntityData *entity);

/* EventScene30_Update, decompiled from the original assembly. The second random
 * number has its own block variable and its offset is summed in a statement
 * expression before the player Y is added, as in the target. */

/* Unresolved original return value: the assembly returns the incoming r9. */
/**
 * @brief Update of event scene variant 30 (sEventSceneUpdateFuncs[30]); returns the
 * player input mask.
 */
s32 EventScene30_Update(struct EwramData_EntityData *entity)
{
    s32 result;
    switch (entity->state) {
    case 0:
        if (entity->stage == 0) {
            entity->timer = 8;
            entity->stage++;
        }
        if ((u8)--entity->timer == 255) {
            entity->state++;
            entity->stage = 0;
            entity->subState = 0;
        }
        entity->state++;
        entity->stage = 0;
        entity->subState = 0;
        break;
    case 1:
        if (entity->stage == 0)
            entity->stage++;
        if (EventScene30_PlayTransformation(entity)) {
            entity->ext.event30Driver.floatPhase = 0;
            gEwramData->playerState.player->hidden = 0;
            Music_SetCurrent(32);
            entity->state++;
            entity->stage = 0;
            entity->subState = 0;
            ObjPalette_LoadRange((void *)sSavePointPalette, 5, 1, 14);
        }
        break;
    case 2:
        if (entity->subState == 0) {
            gUnk_03002CB0.dispCnt &= 0xF1FF;
            entity->subState++;
            entity->stage = 0;
        } else if (entity->subState == 1) {
            DMA_FILL_16(3, 0xFFFF, 0x05000000, 32);
            /* The original DMA source is the literal address 0xFFFF. */
            DMA_COPY_16(3, 0xFFFF, &gEwramData->bgPalette, 32);
            TextWindow_Begin(37);
            entity->subState++;
            entity->stage = 0;
            entity->timer = 0;
        } else if (entity->subState == 2) {
            ObjPalette_StartColorFade(31, 31, 31, 0, 0x10000, 128);
            entity->subState++;
            entity->stage = 0;
        } else if (ObjPalette_GetPendingFades() == 0) {
            gUnk_03002CB0.dispCnt &= 0xF1FF;
            entity->state++;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    case 3:
        if (TextWindow_Update()) {
            entity->state++;
            entity->stage = 0;
            entity->subState = 0;
        }
        if (entity->subState == 1) {
            if (entity->stage == 0) {
                gEwramData->gameState.textWindowPause = 1;
                entity->timer = 64;
                entity->stage++;
            }
            if ((u8)--entity->timer == 255) {
                gEwramData->gameState.textWindowPause = 0;
                entity->subState++;
                entity->stage = 0;
            }
        }
        break;
    case 4:
        if (entity->subState == 0) {
            ObjPalette_StartColorFade(31, 31, 31, 0x10000, 0, 128);
            entity->subState++;
            entity->stage = 0;
            entity->timer = 32;
            Music_SubmitCommandOnce(0x1040);
        } else if (entity->subState == 1 && ObjPalette_GetPendingFades() == 0) {
            ObjPalette_StopBlend(14);
            BgPalette_StopBlend(0);
            gEwramData->startPauseEnabled = 0;
            gEwramData->gameState.stateFlags |= 0x800000;
            RoomJump_SetDestination(&sEndingRoomDefinition, 0, 0, 0, 0);
            RoomJump_Begin(&gEwramData->gameState);
            gDisplayRegisters.bldCnt = 0;
            entity->subState++;
            entity->stage = 0;
            gEwramData->gameState.stateFlags |= 0x4000;
        } else if (*(volatile u8 *)&entity->timer == 0 && entity->stage == 0) { /* FAKEMATCH: volatile read forces the reload of the original */
            Music_SubmitCommandOnce(0x1040);
            entity->stage++;
        }
        break;
    }
    if (entity->state > 1) {
        s32 sine = Sin((u16)entity->ext.event30Driver.floatPhase + 0x4000);
        Player_StartScriptedAnimation(51, 0, 0, sine / 4);
        entity->ext.event30Driver.floatPhase += 0x200;
        { u8 previous = entity->timer--;
        if ((previous % 16) == 0) {
            u32 random = RandomNumberGenerator();
            s32 x = gEwramData->playerState.player->posX.whole - 0x100000;
            s32 y;
            x += (random & 0x7FFFF) * 4;
            {
                u32 r2 = RandomNumberGenerator();
                y = gEwramData->playerState.player->posY.whole + ({ s32 t = (r2 & 0x7FFFF) * 2 - 0xC0000; t; });
            }
            Effect_CreateFromTable(x, y, 0, -0x2000, 7, 9);
        }
        }
    } else {
        struct EwramData_EntityData **player = &gEwramData->playerState.player;
        (*player)->posX.part16.integer = 120;
        (*player)->posY.part16.integer = 80;
        (*player)->velX.whole = 0;
        (*player)->velY.whole = 0;
        Player_StartScriptedAnimation(51, 0, 0, 0);
    }
    gEwramData->gameState.stateFlags |= 32;
    return result;
}

/**
 * @brief Update of event scene variant 0 (sEventSceneUpdateFuncs[0]); returns the player
 * input mask. Cutscene object for conversation 0xC: places the player at
 * x <= 0x4F, then runs its own animation script in subState (jump away
 * to the right, returns 0x200 / 0x10 while moving). The two volatile
 * reads of subState reproduce the original's reloads.
 */
s32 EventScene00_Update(struct EwramData_EntityData *entity)
{
    struct EwramData **ewp;
    struct EwramData_EntityData *player;
    s32 ret;
    u8 step;

    ewp = &gEwramData;
    player = (*ewp)->playerState.player;
    ret = 0;
    switch (entity->state)
    {
    case 0:
    {
        u16 *px = &player->posX.part16.integer;
        if ((s16)*px > 0x4F)
            *px = 0x4F;
        player->flipX = 0;
        entity->flipX = 1;
        TextWindow_Begin(0xC);
        entity->state++;
        break;
    }
    case 1:
        if (entity->subState != 3 && TextWindow_Update())
        {
            (*ewp)->gameState.stateFlags &= ~1;
            entity->state++;
        }
        step = *(vu8 *)&entity->subState;
        if (step == 1)
            entity->state = 0x80;
        switch (*(vu8 *)&entity->subState)
        {
        case 1:
            entity->flipX = 0;
            break;
        case 2:
            Anim_StartIndexed(entity, 2, 2, 0);
            goto next;
        case 3:
            if (entity->animDone)
            {
                Anim_StartIndexed(entity, 3, 2, 0);
                entity->subState++;
                entity->velX.whole = -0x20000;
                entity->accelX.whole = 0x800;
            }
            break;
        case 4:
            if (entity->animStep == 1)
                entity->animFlags |= 2;
            entity->posX.whole += entity->velX.whole;
            entity->velX.whole += entity->accelX.whole;
            if (entity->velX.whole > -0x18000)
                ret = 0x200;
            if (entity->velX.whole > -0x1000)
            {
            next:
                entity->subState++;
            }
            break;
        case 5:
            Anim_StartIndexed(entity, 2, 2, 0);
            break;
        case 6:
            entity->posX.whole += 0x40000;
            entity->posY.whole += -0x8000;
            if ((s16)entity->posX.part16.integer > 0x100)
                entity->subState = step + 1;
            ret = 0x10;
            break;
        case 7:
            break;
        }
        break;
    case 0x80:
        Interaction_End(entity, 1);
        break;
    }
    return ret;
}

/* EventScene01_Update. */
/* Declared with an int result as in object/objects_00_07.c: the original compares
 * the returned value without sign extension. */

/**
 * @brief Update of event scene variant 1 (sEventSceneUpdateFuncs[1]); returns the player
 * input mask. Event step (state state): once the player stands at room X
 * 0x81..0x11F set stateFlags bit 0; then TextWindow_Begin(0xC); then wait
 * for TextWindow_Update and finish via Interaction_End(entity, 1).
 * States 0x80/0x81 hide BG0 and show it again on finishing.
 */
s32 EventScene01_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
    case 0:
        if (GetEntityRoomXPositionIntegerWide(gEwramData->playerState.player) > 0x80
            && GetEntityRoomXPositionIntegerWide(gEwramData->playerState.player) <= 0x11F)
        {
            gEwramData->gameState.stateFlags |= 1;
            entity->state++;
        }
        break;
    case 1:
        TextWindow_Begin(0xC);
        entity->state++;
        break;
    case 2:
        if (TextWindow_Update())
            Interaction_End(entity, 1);
        break;
    case 0x80:
        gUnk_03002CB0.dispCnt &= ~DCNT_BG0;
        entity->state++;
        break;
    case 0x81:
        Interaction_End(entity, 1);
        gUnk_03002CB0.dispCnt |= DCNT_BG0;
        break;
    }
    return 0;
}

/**
 * @brief Init of event scene variant 0 (Object 0x20 spawnParam0 = 0,
 * sEventSceneCreateFuncs[0]).
 */
void EventScene00_Init(void)
{
    gEwramData->gameState.stateFlags |= 1;
    TextWindow_Begin(0xC);
}

/**
 * @brief Init of event scene variant 1 (Object 0x20 spawnParam0 = 1,
 * sEventSceneCreateFuncs[1]).
 */
void EventScene01_Init(void)
{
}

/**
 * @brief Init of event scene variant 29 (Object 0x20 spawnParam0 = 29,
 * sEventSceneCreateFuncs[29]). Scene object setup (only while stateFlags bit 15 is
 * set): place it at room (0x40, 0xBF) facing right, put the player 0x18
 * px to its right, set stateFlags bit 0, fade palettes and clear BG VRAM,
 * hide BG0.
 */
void EventScene29_Init(struct EwramData_EntityData *entity)
{
    struct EwramData *ewram;
    struct BgLayerState *bg;

    if (!(gEwramData->gameState.stateFlags & 0x8000) || Entity_SetupFromObjectTable(entity, 7) == 0)
    {
        entity->deleteRequested = 1;
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    ewram = gEwramData;
    bg = &ewram->bgInfo[1];
    entity->posX.part16.integer = 0x40 - bg->xPos.part16.integer;
    entity->posY.part16.integer = 0xBF - bg->yPos.part16.integer;
    entity->flipX = 1;
    ewram->playerState.player->hidden = 1;
    gEwramData->playerState.player->posX.part16.integer = entity->posX.part16.integer + 0x18;
    gEwramData->playerState.player->posY.part16.integer = entity->posY.part16.integer;
    gEwramData->gameState.stateFlags |= 1;
    BgPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0, 0x10000, 0x80);
    ObjPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0, 0x10000, 0x80);
    DMA_FILL_32(3, 0, VRAM_BASE + 0x4000, 0x2000);
    DMA_FILL_16(3, 0, VRAM_BASE + 0xE000, 0x800);
    gUnk_03002CB0.dispCnt &= ~0x100;
}

/**
 * @brief Update of event scene variant 29 (sEventSceneUpdateFuncs[29]); returns the
 * player input mask. Scene state machine: wait for the fade, clear BG
 * VRAM and show BG0, count down 0xFF frames, fade to black, then save
 * (slot currentSaveSlot) and switch to game mode 0xA. Always returns 0.
 */
s32 EventScene29_Update(struct EwramData_EntityData *entity)
{
    s32 ret = 0;

    switch (entity->state)
    {
    case 0:
        switch (entity->subState)
        {
        case 0:
            if (entity->stage == 0)
            {
                Music_StartOverride(0x25);
                entity->stage++;
            }
            if (BgPalette_GetPendingFades() == 0)
            {
                entity->subState++;
                entity->stage = ret;
            }
            else
            {
                gUnk_03002CB0.dispCnt &= ~0x100;
            }
            break;
        case 1:
            DMA_FILL_32(3, ret, VRAM_BASE + 0x4000, 0x2000);
            DMA_FILL_16(3, ret, VRAM_BASE + 0xE000, 0x800);
            gUnk_03002CB0.dispCnt |= 0x100;
            entity->timer = 0xFF;
            entity->state++;
            entity->stage = 0;
            entity->subState = 0;
            break;
        }
        break;
    case 1:
        if (--entity->timer == 0xFF)
        {
            BgPalette_StartColorFade(0, 0, 0, 0x10000, ret, 0x80);
            ObjPalette_StartColorFade(0, 0, 0, 0x10000, ret, 0x80);
            gUnk_03002CB0.dispCnt &= ~0x100;
            GameFlag_Set(0x2D);
            entity->state++;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    case 2:
        if (BgPalette_GetPendingFades() == 0)
        {
            gEwramData->gameState.bossEventFlags &= ~1;
            GameFlag_Clear(0x28);
            SaveData_ResaveSlotAfterEnding(gEwramData->gameState.currentSaveSlot, 1);
            entity->state = 0x80;
            TextWindow_Close();
            gEwramData->gameState.subMode = 0xA;
        }
        break;
    }
    gEwramData->gameState.stateFlags |= 0x20;
    Anim_Update(entity);
    return ret;
}

/* EventScene31_SpawnBlockDebris..sub_0805C49B. */

/* Enter state s of entity (state) with both sub-states cleared. */
#define SET_STATE(e, s)                     \
    {                                       \
        (e)->state = (s);                 \
        (e)->stage = 0;                   \
        (e)->subState = 0;                   \
    }

/**
 * @brief 5C218 | Spawn the 12 debris pieces of sEventScene31DebrisDefinitions around
 * (x, y): offset, velocity (y up), frame and gravity 0x2000, updated by
 * EventScene31Debris_Update.
 */
s32 EventScene31_SpawnBlockDebris(s16 x, s16 y)
{
    const struct EventScene31DebrisDefinition *info = sEventScene31DebrisDefinitions;
    s32 i;

    for (i = 0; i < 12; i++, info++)
    {
        struct EwramData_EntityData *entity = Entity_CreateFromObjectTable(11);

        if (entity != NULL)
        {
            entity->posX.part16.integer = x + info->dx;
            entity->posY.part16.integer = y + info->dy;
            entity->velX.whole = info->vx << 12;
            entity->velY.whole = -(info->vy << 12);
            entity->animFrame = info->frame;
            entity->timer = info->initialTimer;
            entity->accelY.whole = 0x2000;
            entity->updateFunc = (EntityFunc)EventScene31Debris_Update;
        }
    }
}

/**
 * @brief 5C284 | Update of a falling block: move with acceleration until
 * it hits the terrain (Collision_GetGroundOffset at its map position), then burst into
 * debris (EventScene31_SpawnBlockDebris, song 0xAA) and switch to frame 8.
 */
s32 EventScene31FallingBlock_Update(struct EwramData_EntityData *entity)
{
    u8 state = entity->state;

    if (state == 0)
    {
        struct BgLayerState *bg;
        s32 x = (s16)entity->posX.part16.integer;
        s32 hit;

        bg = &gEwramData->bgInfo[1];
        hit = Collision_GetGroundOffsetWide(x + (s16)bg->xPos.part16.integer,
                           (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer);

        if (hit)
        {
            EventScene31_SpawnBlockDebris(entity->posX.part16.integer, entity->posY.part16.integer - 2);
            PlaySong(0xAA);
            entity->state++;
            entity->stage = state;
            entity->subState = state;
        }
        else
        {
            entity->posX.whole += entity->velX.whole;
            entity->posY.whole += entity->velY.whole;
            entity->velX.whole += entity->accelX.whole;
            entity->velY.whole += entity->accelY.whole;
            if (entity->velX.whole > 0)
                entity->velX.whole = hit;
        }
    }
    else if (entity->stage == 0)
    {
        entity->animFrame = 8;
        entity->stage++;
    }
}

/**
 * @brief 5C328 | Step the four-step palette animation sEventScene31PaletteSteps of
 * entity (ext.event31.paletteTick / _STEP): returns 0 after the last step,
 * else 1.
 */
s32 EventScene31_StepPaletteAnim(struct EwramData_EntityData *entity)
{
    const struct EventScene31PaletteStep *anim = &sEventScene31PaletteSteps[entity->ext.event31.paletteStep];

    if (entity->ext.event31.paletteTick++ > anim->duration)
    {
        entity->ext.event31.paletteTick = 0;
        ObjPalette_LoadRange(anim->palette, anim->sourcePalette, 1, entity->basePalette);
        if (++entity->ext.event31.paletteStep > 3)
        {
            entity->ext.event31.paletteStep = 0;
            return 0;
        }
        anim = &sEventScene31PaletteSteps[entity->ext.event31.paletteStep];
        ObjPalette_StartBlend(0, (u16 *)anim->palette, anim->sourcePalette, 0x10000, 0, anim->duration);
    }
    return 1;
}

/**
 * @brief 5C39C | Update of the block holder: state 1 waits for animDone,
 * state 2 drops the block (EventScene31FallingBlock_Update) and waits 0x20 frames, state 3
 * runs the palette animation (EventScene31_StepPaletteAnim), then state 0x80. States 0
 * and 1 run Anim_Update while idle.
 */
s32 EventScene31BlockHolder_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
        case 0:
            Anim_Update(entity);
            break;
        case 1:
            if (entity->stage == 0)
            {
                entity->animDone = 0;
                entity->stage++;
            }
            if (entity->animDone)
                SET_STATE(entity, entity->state + 1)
            else
                Anim_Update(entity);
            break;
        case 2:
            if (entity->stage == 0)
            {
                s32 x = (s16)(entity->posX.part16.integer - 0xB);
                s32 y = (s16)(entity->posY.part16.integer - 0x10);
                struct EwramData_EntityData *block = Entity_CreateFromObjectTable(11);

                if (block != NULL)
                {
                    block->posX.part16.integer = x;
                    block->posY.part16.integer = y;
                    block->velX.whole = -0xA000;
                    block->velY.whole = 0x2000;
                    block->accelX.whole = 0x100;
                    block->accelY.whole = 0x1000;
                    block->animFrame = 3;
                    block->updateFunc = (EntityFunc)EventScene31FallingBlock_Update;
                }
                entity->animFrame = 2;
                entity->stage++;
                entity->timer = 0x20;
            }
            if (--entity->timer == 0xFF)
                SET_STATE(entity, entity->state + 1)
            break;
        case 3:
            if (entity->stage == 0)
            {
                entity->ext.event31.paletteTick = 0;
                entity->ext.event31.paletteStep = 0;
                entity->stage++;
            }
            if (EventScene31_StepPaletteAnim(entity) == 0)
                SET_STATE(entity, 0x80)
            break;
    }
}

/**
 * @brief 5C49C | Scene setup (needs Entity_SetupFromObjectTable(entity, 7)): place the
 * object at room (0xFF, 0xBF) facing left, create helper 0xB at (0x78, 0xB0)
 * with update EventScene31BlockHolder_Update, move the player to (0xA0, 0xBF), set stateFlags
 * bits 0 and 16, fade palettes, clear BG VRAM and hide BG0.
 */
void EventScene31_Init(struct EwramData_EntityData *entity)
{
    struct EwramData *ewram;
    struct BgLayerState *bg;
    struct EwramData_EntityData *child;
    s32 zero;
    s32 fill;

    if (Entity_SetupFromObjectTable(entity, 7) == 0)
        goto fail;
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    {
    struct BgLayerState *bg1 = &gEwramData->bgInfo[1];
    entity->posX.part16.integer = 0xFF - bg1->xPos.part16.integer;
    zero = 0;
    fill = 0;
    entity->posY.part16.integer = 0xBF - bg1->yPos.part16.integer;
    }
    entity->flipX = 0;
    child = Entity_CreateFromObjectTable(0xB);
    entity->ext.event31.blockHolder = child;
    if (child == NULL)
    {
    fail:
        entity->deleteRequested = 1;
        return;
    }
    {
    struct BgLayerState *bg2 = &gEwramData->bgInfo[1];
    child->posX.part16.integer = 0x78 - bg2->xPos.part16.integer;
    entity->ext.event31.blockHolder->posY.part16.integer = 0xB0 - bg2->yPos.part16.integer;
    }
    Anim_StartIndexed(entity->ext.event31.blockHolder, 0, 0, 1);
    Anim_Update(entity);
    entity->ext.event31.blockHolder->updateFunc = (EntityFunc)EventScene31BlockHolder_Update;
    gEwramData->playerState.player->hidden = 1;
    ewram = gEwramData;
    ewram->playerState.player->posX.part16.integer = 0xA0 - (bg = &ewram->bgInfo[1])->xPos.part16.integer;
    ewram->playerState.player->posY.part16.integer = 0xBF - bg->yPos.part16.integer;

    ewram->gameState.stateFlags |= 1;
    ewram->gameState.stateFlags |= 0x10000;
    BgPalette_StartColorFade(0, 0, 0, 0, 0x10000, 0x80);
    ObjPalette_StartColorFade(0, 0, 0, 0, 0x10000, 0x80);
    entity->state = 1;
    entity->stage = zero;
    entity->subState = zero;
    DMA_FILL_32(3, fill, VRAM_BASE + 0x4000, 0x2000);
    DMA_FILL_16(3, fill, VRAM_BASE + 0xE000, 0x800);
    gUnk_03002CB0.dispCnt &= ~0x100;
}

struct SignedCamera { void *metadata; u16 xFraction; s16 x; u16 yFraction; s16 y; };

/* Scene sequence; animation and transition semantics are not fully identified. */
/**
 * @brief Update of event scene variant 31 (sEventSceneUpdateFuncs[31], the last table
 * entry); returns the player input mask. Scene
 * sequence; animation and transition semantics are not fully identified.
 */
s32 EventScene31_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state) {
    case 0:
        if (entity->stage == 0) {
            entity->timer = 128;
            entity->stage++;
        }
        if ((u8)--entity->timer == 0xFF) {
            BgPalette_StartColorFade(0, 0, 0, 0, 0x10000, 128);
            ObjPalette_StartColorFade(0, 0, 0, 0, 0x10000, 128);
            entity->timer = 0;
            entity->state++;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    case 1:
        if (entity->stage == 0) {
            Music_SetCurrent(33);
            entity->stage++;
        }
        if (BgPalette_GetPendingFades() == 0) {
            BgPalette_StopColorFade();
            ObjPalette_StopColorFade();
            DMA_FILL_32(3, 0, VRAM_BASE + 0x4000, 0x2000);
            DMA_FILL_16(3, 0, VRAM_BASE + 0xE000, 0x800);
            gUnk_03002CB0.dispCnt |= 0x100;
            entity->timer = 255;
            entity->state++;
            entity->stage = 0;
            entity->subState = 0;
        } else
            gUnk_03002CB0.dispCnt &= 0xFEFF;
        break;
    case 2:
        switch (entity->subState) {
        case 0:
            if (entity->stage == 0) {
                entity->timer = 128;
                entity->stage++;
            }
            if ((u8)--entity->timer == 0xFF) {
                struct EwramData *ew = gEwramData;
                struct EwramData_EntityData **player = &ew->playerState.player;
                struct EwramData_EntityData *initial = *player;
                struct BgLayerState *bg = &ew->bgInfo[1];
                initial->posX.part16.integer = 208 - bg->xPos.part16.integer;
                (*player)->posY.part16.integer = 191 - bg->yPos.part16.integer;
                entity->subState++;
                entity->stage = 0;
            }
            break;
        case 1:
            if (entity->stage == 0) {
                entity->timer = 64;
                entity->stage++;
            }
            if ((u8)--entity->timer == 0xFF) {
                entity->subState++;
                entity->stage = 0;
            }
            break;
        case 2: {
            s32 x;
            struct EwramData *ew;
            struct SignedCamera *bg;
            if (entity->stage == 0) {
                if (entity->animScriptIndex != 1)
                    Anim_StartIndexed(entity, 1, 1, 1);
                entity->stage++;
            }
            x = (s16)entity->posX.part16.integer;
            ew = gEwramData;
            bg = (struct SignedCamera *)&ew->bgInfo[1];
            if (x + bg->x > 184) {
                if (!(Entity_TryWalk(entity, 0x10000, 0x200000, 0x20000, 1) & 4))
                    entity->posY.whole += entity->velY.whole;
            } else {
                struct EwramData_EntityData **player = &ew->playerState.player;
                (*player)->posX.part16.integer = 160 - bg->x;
                (*player)->posY.part16.integer = 191 - bg->y;
                entity->subState++;
                entity->stage = 0;
            }
            break;
        }
        case 3:
            if (entity->stage == 0) {
                if (entity->animScriptIndex != 2)
                    Anim_StartIndexed(entity, 2, 1, 0);
                entity->stage++;
            }
            if ((s16)entity->posX.part16.integer + ((struct SignedCamera *)&gEwramData->bgInfo[1])->x > 176) {
                if (!(Entity_TryWalk(entity, 0x8000, 0x200000, 0x20000, 1) & 4))
                    entity->posY.whole += entity->velY.whole;
            }
            if (entity->animDone) {
                if (entity->animScriptIndex != 0)
                    Anim_StartIndexed(entity, 0, 1, 1);
                entity->subState += 2;
                entity->stage = 0;
            }
            break;
        case 4:
            if (entity->stage == 0) {
                if (entity->animScriptIndex != 5)
                    Anim_StartIndexed(entity, 5, 1, 0);
                entity->stage++;
            }
            if ((s16)entity->posX.part16.integer + ((struct SignedCamera *)&gEwramData->bgInfo[1])->x > 176) {
                if (!(Entity_TryWalk(entity, 0x8000, 0x200000, 0x20000, 1) & 4))
                    entity->posY.whole += entity->velY.whole;
            }
            if (entity->animStep == 4)
                PlaySong(0x1DF);
            if (entity->animStep == 8)
                PlaySong(0xFC);
            if (entity->animDone) {
                if (entity->animScriptIndex != 0)
                    Anim_StartIndexed(entity, 0, 1, 1);
                entity->subState++;
                entity->stage = 0;
            }
            break;
        case 5:
            if (entity->stage == 0) {
                TextWindow_Begin(90);
                entity->timer = 68;
                entity->stage++;
            }
            if ((u8)--entity->timer == 0xFF) {
                entity->state++;
                entity->stage = 0;
                entity->subState = 0;
            }
            break;
        }
        break;
    case 3:
        if (TextWindow_Update()) {
            entity->state++;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    case 4:
        if (entity->stage == 0) {
            entity->timer = 104;
            entity->stage++;
        }
        if ((u8)--entity->timer == 0xFF) {
            entity->ext.event31.blockHolder->state++;
            { struct EwramData_EntityData *other = entity->ext.event31.blockHolder;
              other->stage = 0; other->subState = 0; }
            entity->state++;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    case 5:
        if (entity->ext.event31.blockHolder->state == 0x80) {
            Music_SubmitCommandOnce(0x1040);
            entity->state++;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    case 6:
        if (entity->stage == 0) {
            BgPalette_StartColorFade(0, 0, 0, 0x10000, 0, 128);
            ObjPalette_StartColorFade(0, 0, 0, 0x10000, 0, 128);
            gUnk_03002CB0.dispCnt &= 0xFEFF;
            GameFlag_Set(45);
            entity->stage++;
            entity->timer = 128;
        }
        if (BgPalette_GetPendingFades() == 0) {
            entity->state = 0x80;
            TextWindow_Close();
            gEwramData->gameState.subMode = 10;
        }
        break;
    }
    Anim_Update(entity);
    gEwramData->gameState.stateFlags |= 0x20;
    return 0;
}

/* EventScene31Debris_Update. */
/**
 * @brief 5CB08 | Update of a debris piece: fall with gravity accelY until
 * it touches the terrain, then set deleteRequested.
 */
s32 EventScene31Debris_Update(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg;
    s32 x = (s16)entity->posX.part16.integer;

    bg = &gEwramData->bgInfo[1];
    if (Collision_GetGroundOffsetWide(x + (s16)bg->xPos.part16.integer,
                     (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer))
    {
        entity->deleteRequested = 1;
    }
    else
    {
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        entity->velY.whole += entity->accelY.whole;
    }
}

/**
 * @brief Init of event scene variant 2 (Object 0x20 spawnParam0 = 2,
 * sEventSceneCreateFuncs[2]).
 */
void EventScene02_Init(struct EwramData_EntityData *entity)
{
}

/**
 * @brief Update of event scene variant 2 (sEventSceneUpdateFuncs[2]); returns the player
 * input mask.
 */
void EventScene02_Update(struct EwramData_EntityData *entity)
{
}
