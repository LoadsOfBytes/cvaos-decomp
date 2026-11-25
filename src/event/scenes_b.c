/*
 * Event scenes 04 to 28 (except 24's init and 27 / 28, which follow scene 30).
 */
#include "core.h"
#include "sound.h"
#include "graphics.h"
#include "structs/ewram.h"
#include "music.h"
#include "gba.h"
#include "map_flags.h"
#include "bg_effects.h"
#include "player_core.h"
#include "gba/dma.h"
#include "macros.h"
#include "in_game.h"
#include "save_data.h"
#include "structs/main.h"
#include "soul_inventory.h"
#include "blue_souls.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "palette.h"
#include "shop_update.h"
#include "text_window.h"
#include "data/data_0E5364.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"
#include "player_soma.h"
#include "entity_motion.h"
#include "bg_and_collision.h"

s32 Player_GetZoneAroundXCopy(u16 centerX, u16 innerWidth, u16 outerWidth);
s32 EventSceneJuliusBattleStart_LoadGraphics(struct EwramData_EntityData *entity);
void EventSceneJuliusBattleStart_Finish(struct EwramData_EntityData *);

/* EventSceneMeetGraham_Init, EventSceneMeetGraham_RunMusicScript. */

/**
 * @brief Init of event scene variant 4 (Object 0x20 spawnParam0 = 4,
 * sEventSceneCreateFuncs[4]). Create step of a scene object: deleted when flag
 * 0x21 is set or the player is right of x 0x80; else set up animation 0
 * at room (0xA0, 0xC0) moving right (0xC000), facing right, and set
 * gameState.stateFlags bit 0.
 */
void EventSceneMeetGraham_Init(struct EwramData_EntityData *entity)
{
    struct EwramData *ewram;
    struct BgLayerState *bg;

    if (GameFlag_GetMask(0x21) != 0 || (s16)gEwramData->playerState.player->posX.part16.integer > 0x80)
    {
        entity->deleteRequested = 1;
        return;
    }
    if (Entity_SetupFromObjectTable(entity, 5) == 0)
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    ewram = gEwramData;
    bg = &ewram->bgInfo[1];
    entity->posX.part16.integer = 0xA0 - bg->xPos.part16.integer;
    entity->posY.part16.integer = 0xC0 - bg->yPos.part16.integer;
    entity->velX.whole = 0xC000;
    entity->flipX = 0;
    ewram->gameState.stateFlags |= 1;
}

#define STEP(e) ((e)->ext.event04.scriptStep)
#define TIMER(e) ((e)->ext.event04.scriptTimer)

/**
 * @brief 5F1CC | Timed script: song 0x1010 and wait 0xA0 frames, then
 * Music_SetCurrent(0x16); again song 0x1010 and wait, then Music_UpdateRoom;
 * returns 1 once finished.
 */
s32 EventSceneMeetGraham_RunMusicScript(struct EwramData_EntityData *entity)
{
    s32 done = 0;

    switch (STEP(entity))
    {
        case 0:
            PlaySong(0x1010);
            TIMER(entity) = 0xA0;
            STEP(entity)++;
            break;
        case 1:
            if (--TIMER(entity) == 0xFF)
            {
                Music_SetCurrent(0x16);
                STEP(entity)++;
            }
            break;
        case 2:
            break;
        case 3:
            PlaySong(0x1010);
            TIMER(entity) = 0xA0;
            STEP(entity)++;
            break;
        case 4:
            if (--TIMER(entity) == 0xFF)
            {
                Music_UpdateRoom(&gEwramData->gameState);
                STEP(entity)++;
            }
            break;
        case 5:
            done = 1;
            break;
    }
    return done;
}

/**
 * @brief Update of event scene variant 4 (sEventSceneUpdateFuncs[4]); returns the player
 * input mask. Cutscene object for conversation 0xF: keeps the player at
 * x <= 0x57 (returns 0x10 meanwhile), plays the animation list
 * sUnk_0852682C, then walks off to the right and sets unk_414 to 5; on
 * the end of the conversation sets event 0x21.
 */
s32 EventSceneMeetGraham_Update(struct EwramData_EntityData *entity)
{
    s32 ret;
    struct EwramData **ewp;
    struct EwramData *ew;
    struct GameState *g;
    struct EwramData_EntityData *player;
    u8 state;
    s32 v;

    ret = 0;
    ewp = &gEwramData;
    ew = *ewp;
    g = &ew->gameState;
    switch (state = entity->state)
    {
    case 0:
        if (entity->subState == 0)
        {
            Sound_Play(0x1040);
            TextWindow_Begin(0xF);
            ew->gameState.textWindowPause = 1;
            entity->timer = 0x20;
            entity->subState++;
        }
        if (--entity->timer == 0xFF)
        {
            entity->state++;
            entity->stage = ret;
            entity->subState = ret;
        }
        break;
    case 1:
        if (TextWindow_Update())
            entity->state = 0x80;
        switch (entity->subState)
        {
        case 0:
            switch (entity->stage)
            {
            case 0:
                entity->stage = state;
                entity->timer = 0x30;
                /* fall through */
            case 1:
                if (--entity->timer != 0xFF)
                    break;
                entity->stage++;
                break;
            case 2:
                player = (*ewp)->playerState.player;
                if ((s16)player->posX.part16.integer > 0x56)
                {
                    player->flipX = 0;
                    (*ewp)->playerState.player->posX.part16.integer = 0x57;
                    entity->stage++;
                    break;
                }
                ret = 0x10;
                break;
            case 3:
                Music_SetCurrent(0x16);
                ew->gameState.textWindowPause = ret;
                entity->subState = state;
                entity->stage = ret;
                break;
            }
            break;
        case 1:
            switch (entity->stage)
            {
            case 0:
                entity->ext.event04.animPos = ret;
                v = 0xFF;
                entity->timer = v;
                entity->stage++;
                break;
            case 1:
                if (--entity->timer == 0xFF)
                {
                    const u8 *tbl = sUnk_0852682C;
                    s32 anim = tbl[(u8)entity->ext.event04.animPos++];
                    if (anim == 0xFF)
                    {
                        anim = 0;
                        entity->ext.event04.animPos = ret;
                    }
                    Anim_StartIndexed(entity, anim, 0, 0);
                    entity->timer |= 0xFF;
                }
                break;
            }
            break;
        case 2:
            switch (entity->stage)
            {
            case 0:
                Sound_Play(0x1040);
                ew->gameState.textWindowPause = state;
                entity->flipX = 1;
                if (entity->animScriptIndex != 1)
                    Anim_StartIndexed(entity, 1, 0, 1);
                entity->stage++;
                /* fall through */
            case 1:
                if ((s16)entity->posX.part16.integer > 0xFF)
                {
                    v = 0x10;
                    entity->timer = v;
                    entity->stage++;
                }
                else
                {
                    entity->posX.whole += entity->velX.whole;
                }
                break;
            case 2:
                if (--entity->timer == 0xFF)
                {
                    ew->gameState.textWindowPause = ret;
                    ew->gameState.textWindowState = 5;
                    entity->stage++;
                    entity->ext.event04.scriptStep = 4;
                }
                break;
            }
            break;
        }
        break;
    case 0x80:
        Music_UpdateRoom(g);
        GameFlag_Set(0x21);
        Interaction_End(entity, 1);
        break;
    }
    if (entity->state != 0x80)
        Anim_Update(entity);
    return ret;
}

/* EventSceneMeetYoko_Init. */

/**
 * @brief Init of event scene variant 5 (Object 0x20 spawnParam0 = 5,
 * sEventSceneCreateFuncs[5]). Create step of a scene object: when flag 0x22 is
 * set Interaction_End(entity, 1); else animation 0 at room (0xA0, 0xA0)
 * facing right and gameState.stateFlags bit 0.
 */
void EventSceneMeetYoko_Init(struct EwramData_EntityData *entity)
{
    struct EwramData *ewram;
    struct BgLayerState *bg;

    if (GameFlag_GetMask(0x22) != 0)
    {
        Interaction_End(entity, 1);
        return;
    }
    if (Entity_SetupFromObjectTable(entity, 2) == 0)
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    ewram = gEwramData;
    bg = &ewram->bgInfo[1];
    entity->posX.part16.integer = 0xA0 - bg->xPos.part16.integer;
    entity->posY.part16.integer = 0xA0 - bg->yPos.part16.integer;
    entity->flipX = 0;
    ewram->gameState.stateFlags |= 1;
}

/**
 * @brief Update of event scene variant 5 (sEventSceneUpdateFuncs[5]); returns the player
 * input mask. Cutscene object for conversation 0x10: waits, keeps the
 * player at x <= 0x50 (returns 0x10 meanwhile), then walks off to the
 * right and sets unk_414 to 5; on the end of the conversation sets event
 * 0x22.
 */
s32 EventSceneMeetYoko_Update(struct EwramData_EntityData *entity)
{
    s32 ret;
    struct EwramData **ewp;
    struct EwramData *ew;
    struct GameState *g;
    struct EwramData_EntityData *player;
    u8 state;
    u8 step;

    ret = 0;
    ewp = &gEwramData;
    ew = *ewp;
    g = &ew->gameState;
    switch (state = entity->state)
    {
    case 0:
        if (entity->subState == 0)
        {
            Sound_Play(0x1040);
            TextWindow_Begin(0x10);
            ew->gameState.textWindowPause = 1;
            entity->timer = 0x20;
            entity->subState++;
        }
        if (--entity->timer == 0xFF)
        {
            entity->state++;
            entity->stage = ret;
            entity->subState = ret;
        }
        break;
    case 1:
        if (TextWindow_Update())
            entity->state = 0x80;
        switch (step = entity->subState)
        {
        case 0:
            switch (entity->stage)
            {
            case 0:
                entity->stage = state;
                entity->timer = 0x30;
                /* fall through */
            case 1:
                if (--entity->timer != 0xFF)
                    break;
                entity->stage++;
                break;
            case 2:
                player = (*ewp)->playerState.player;
                {
                    u16 *px = &player->posX.part16.integer;
                    if ((s16)*px > 0x4F)
                    {
                        *px = 0x50;
                        entity->stage++;
                        break;
                    }
                }
                ret = 0x10;
                break;
            case 3:
                Music_SetCurrent(0x16);
                ew->gameState.textWindowPause = ret;
                break;
            }
            break;
        case 1:
            switch (entity->stage)
            {
            case 0:
                Sound_Play(0x1040);
                ew->gameState.textWindowPause = step;
                entity->velX.whole = 0x8000;
                entity->flipX = 1 - (*ewp)->playerState.player->flipX;
                if (entity->animScriptIndex != 1)
                    Anim_StartIndexed(entity, 1, 0, 1);
                entity->stage++;
                /* fall through */
            case 1:
                if ((s16)entity->posX.part16.integer > 0xFF)
                {
                    entity->stage++;
                    break;
                }
                entity->posX.whole += entity->velX.whole;
                break;
            case 2:
                ew->gameState.textWindowPause = ret;
                ew->gameState.textWindowState = 5;
                entity->stage++;
                break;
            }
            break;
        }
        break;
    case 0x80:
        Music_UpdateRoom(g);
        GameFlag_Set(0x22);
        Interaction_End(entity, 1);
        break;
    }
    if (entity->state != 0x80)
        Anim_Update(entity);
    return ret;
}

/* EventSceneMeetJ_Init. */
/**
 * @brief Init of event scene variant 6 (Object 0x20 spawnParam0 = 6,
 * sEventSceneCreateFuncs[6]). Create step of a scene object: deleted when flag
 * 0x23 is set; else animation 0 at room (0x50, 0xC0) facing left,
 * gameState.stateFlags bit 0 and hBlankEffect.requestStop.
 */
void EventSceneMeetJ_Init(struct EwramData_EntityData *entity)
{
    struct EwramData *ewram;
    struct BgLayerState *bg;

    if (GameFlag_GetMask(0x23) != 0)
    {
        entity->deleteRequested = 1;
        return;
    }
    if (Entity_SetupFromObjectTable(entity, 6) == 0)
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    ewram = gEwramData;
    bg = &ewram->bgInfo[1];
    entity->posX.part16.integer = 0x50 - bg->xPos.part16.integer;
    entity->posY.part16.integer = 0xC0 - bg->yPos.part16.integer;
    entity->flipX = 1;
    ewram->gameState.stateFlags |= 1;
    ewram->hBlankEffect.requestStop = 1;
}

/**
 * @brief Update of event scene variant 6 (sEventSceneUpdateFuncs[6]); returns the player
 * input mask. Cutscene object for conversation 0x11: clears BG VRAM at
 * 0x06004000, scrolls the camera back to 0 and keeps the player at x <=
 * 0xAE (returns 0x20 meanwhile), then walks off to the left and sets
 * unk_414 to 5; on the end of the conversation sets event 0x23.
 */
s32 EventSceneMeetJ_Update(struct EwramData_EntityData *entity)
{
    s32 ret;
    struct EwramData **ewp;
    struct EwramData *ew;
    struct GameState *g;
    u8 step;

    ret = 0;
    ewp = &gEwramData;
    ew = *ewp;
    g = &ew->gameState;
    switch (entity->state)
    {
    case 0:
        DMA_FILL_32(3, ret, VRAM_BASE + 0x4000, 0x1000);
        Sound_Play(0x1040);
        TextWindow_Begin(0x11);
        ew->gameState.textWindowPause = 1;
        entity->state++;
        /* fall through */
    case 1:
        if (TextWindow_Update())
            entity->state = 0x80;
        switch (step = entity->subState)
        {
        case 0:
            switch (entity->stage)
            {
            case 0:
                g->textWindowPause = 1;
                entity->stage++;
                gEwramData->gameState.cameraScrollMinPriority = 1;
                /* fall through */
            case 1:
                entity->stage++;
                /* fall through */
            case 2:
            {
                struct BgLayerState *cam = &gEwramData->bgInfo[1];
                s32 cx;
                if (cam->xPos.whole > 0)
                    Camera_ScrollWithPriority(1, -0x18000, 0);
                else
                    cam->xPos.whole = 0;
                {
                    u16 *px = &gEwramData->playerState.player->posX.part16.integer;
                    s32 x = (s16)*px;
                    struct BgLayerState *c = &gEwramData->bgInfo[1];
                    if (x <= 0xAE - (s16)c->xPos.part16.integer)
                        *px = 0xAE - c->xPos.part16.integer;
                    else
                        ret |= 0x20;
                }
                cam = &gEwramData->bgInfo[1];
                cx = cam->xPos.whole;
                if (cx == 0
                    && (s16)gEwramData->playerState.player->posX.part16.integer == 0xAE - (s16)cam->xPos.part16.integer)
                {
                    Music_SetCurrent(0x16);
                    g->textWindowPause = cx;
                    entity->stage++;
                    break;
                }
                break;
            }
            }
            break;
        case 1:
            switch (entity->stage)
            {
            case 0:
                Sound_Play(0x1040);
                g->textWindowPause = step;
                entity->flipX = 0;
                if (entity->animScriptIndex != 1)
                    Anim_StartIndexed(entity, 1, 0, 1);
                entity->stage++;
                /* fall through */
            case 1:
                if ((s16)entity->posX.part16.integer > -0x10
                    && !(Entity_TryWalk(entity, 0x10000, 0x200000, 0x20000, 1) & 4))
                    entity->posY.whole += entity->velY.whole;
                Camera_ScrollWithPriority(1, 0x10000, 0);
                if ((s16)gEwramData->playerState.player->posX.part16.integer == 0x78)
                {
                    g->textWindowPause = 0;
                    g->textWindowState = 5;
                    entity->stage++;
                }
                break;
            }
            break;
        }
        break;
    case 0x80:
        Music_UpdateRoom(g);
        (*ewp)->gameState.cameraScrollMinPriority = ret;
        GameFlag_Set(0x23);
        Interaction_End(entity, 1);
        break;
    }
    if (entity->state != 0x80)
        Anim_Update(entity);
    return ret;
}

/* Scene setup, flag 0x24: stop HBlank effect and place actor at (190,160). */
/**
 * @brief Init of event scene variant 7 (Object 0x20 spawnParam0 = 7,
 * sEventSceneCreateFuncs[7]).
 */
void EventSceneGrahamClaimsDracula_Init(struct EwramData_EntityData *entity)
{
    s32 flag = GameFlag_GetMask(0x24);
    struct BgLayerState *bg;
    if (flag != 0 || (s16)gEwramData->playerState.player->posX.part16.integer <= 31)
    {
        entity->deleteRequested = 1;
        return;
    }
    gEwramData->hBlankEffect.requestStop = 1;
    DMA_FILL_32(3, flag, gEwramData->hBlankEffect.hBlankBuffer, 0xA00);
    if (Entity_SetupFromObjectTable(entity, 5) == 0)
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    bg = &gEwramData->bgInfo[1];
    entity->posX.part16.integer = 190 - bg->xPos.part16.integer;
    entity->posY.part16.integer = 160 - bg->yPos.part16.integer;
    entity->flipX = 1;
    Entity_SnapToGround(entity);
    gEwramData->playerState.player->flipX = 1;
    gEwramData->gameState.stateFlags |= 1;
}

/**
 * @brief Update of event scene variant 7 (sEventSceneUpdateFuncs[7]); returns the player
 * input mask. Cutscene object for conversation 0x12: stops the player,
 * scrolls the camera, walks off to the left and finally sets unk_414 to
 * 5; on the end of the conversation sets event 0x24.
 */
s32 EventSceneGrahamClaimsDracula_Update(struct EwramData_EntityData *entity)
{
    s32 ret;
    struct EwramData **ewp;
    struct EwramData *ew;
    struct GameState *g;
    struct EwramData_EntityData *player;
    u8 step;

    ret = 0;
    ewp = &gEwramData;
    ew = *ewp;
    g = &ew->gameState;
    switch (entity->state)
    {
    case 0:
        player = ew->playerState.player;
        player->velY.whole = ret;
        player->velX.whole = ret;
        Sound_Play(0x1040);
        TextWindow_Begin(0x12);
        ew->gameState.textWindowPause = 1;
        entity->state++;
        /* fall through */
    case 1:
        if (TextWindow_Update())
            entity->state = 0x80;
        switch (step = entity->subState)
        {
        case 0:
            switch (entity->stage)
            {
            case 0:
                g->textWindowPause = 1;
                entity->stage++;
                /* fall through */
            case 1:
                entity->stage++;
                /* fall through */
            case 2:
            {
                struct EwramData **p2 = &gEwramData;
                struct BgLayerState *cam;
                (*p2)->gameState.cameraScrollMinPriority = 1;
                cam = &(*p2)->bgInfo[1];
                if ((s16)cam->xPos.part16.integer > 0x94)
                {
                    Camera_ScrollWithPriority(1, -0x8000, 0);
                    break;
                }
                Music_SetCurrent(0x16);
                g->textWindowPause = 0;
                entity->stage++;
                break;
            }
            }
            break;
        case 1:
            switch (entity->stage)
            {
            case 0:
                g->textWindowPause = step;
                entity->flipX = 1 - entity->flipX;
                if (entity->animScriptIndex != 8)
                    Anim_StartIndexed(entity, 8, 0, 1);
                entity->stage++;
                /* fall through */
            case 1:
                if ((s16)entity->posX.part16.integer > -0x10)
                {
                    if (!(Entity_TryWalk(entity, 0x10000, 0x200000, 0x20000, 1) & 4))
                        entity->posY.whole += entity->velY.whole;
                }
                else
                {
                    entity->stage++;
                    g->textWindowPause = 0;
                }
                break;
            }
            break;
        case 2:
        {
            u8 *flag = &g->textWindowPause;
            {
                s32 one = 1;
                *flag = one;
                Camera_ScrollWithPriority(one, 0x8000, 0);
            }
            Sound_Play(0x1040);
            if ((s16)gEwramData->playerState.player->posX.part16.integer == 0x78)
            {
                *flag = 0;
                g->textWindowState = 5;
                entity->stage++;
            }
            break;
        }
        }
        break;
    case 0x80:
        ew->gameState.cameraScrollMinPriority = ret;
        Music_UpdateRoom(&(*ewp)->gameState);
        GameFlag_Set(0x24);
        Interaction_End(entity, 1);
        break;
    }
    if (entity->state != 0x80)
        Anim_Update(entity);
    return ret;
}

/* EventSceneYokoPlansTrap_Init. */
/**
 * @brief Init of event scene variant 8 (Object 0x20 spawnParam0 = 8,
 * sEventSceneCreateFuncs[8]). Create step of a scene object: deleted when flag
 * 0x25 is set; else animation 0 at room (0x50, 0xA0) facing left and
 * gameState.stateFlags bit 0.
 */
void EventSceneYokoPlansTrap_Init(struct EwramData_EntityData *entity)
{
    struct EwramData *ewram;
    struct BgLayerState *bg;

    if (GameFlag_GetMask(0x25) != 0)
    {
        entity->deleteRequested = 1;
        return;
    }
    if (Entity_SetupFromObjectTable(entity, 2) == 0)
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    ewram = gEwramData;
    bg = &ewram->bgInfo[1];
    entity->posX.part16.integer = 0x50 - bg->xPos.part16.integer;
    entity->posY.part16.integer = 0xA0 - bg->yPos.part16.integer;
    entity->flipX = 1;
    ewram->gameState.stateFlags |= 1;
}

/**
 * @brief Update of event scene variant 8 (sEventSceneUpdateFuncs[8]); returns the player
 * input mask. Cutscene object for conversation 0x13 (sibling of
 * EventSceneMeetYoko_Update): keeps the player at x <= 0xA0 (returns 0x20
 * meanwhile), plays song 0x1B7, walks off to the left; on the end of the
 * conversation sets event 0x25 (and returns without a value, as the
 * original).
 */
s32 EventSceneYokoPlansTrap_Update(struct EwramData_EntityData *entity)
{
    s32 ret;
    struct EwramData **ewp;
    struct EwramData *ew;
    struct GameState *g;
    u8 state;

    ret = 0;
    ewp = &gEwramData;
    ew = *ewp;
    g = &ew->gameState;
    switch (state = entity->state)
    {
    case 0:
        if (entity->subState == 0)
        {
            Sound_Play(0x1040);
            TextWindow_Begin(0x13);
            ew->gameState.textWindowPause = 1;
            entity->timer = 0x20;
            entity->subState++;
        }
        if (--entity->timer == 0xFF)
        {
            entity->state++;
            entity->stage = ret;
            entity->subState = ret;
        }
        break;
    case 1:
        if (TextWindow_Update())
            entity->state = 0x80;
        switch (entity->subState)
        {
        case 0:
            switch (entity->stage)
            {
            case 0:
                ew->gameState.textWindowPause = state;
                entity->stage++;
                entity->timer = 0x30;
                /* fall through */
            case 1:
                if (--entity->timer != 0xFF)
                    break;
                entity->stage++;
                break;
            case 2:
                if ((s16)(*ewp)->playerState.player->posX.part16.integer <= 0xA0)
                {
                    Music_SetCurrent(0x16);
                    ew->gameState.textWindowPause = ret;
                    (*ewp)->playerState.player->posX.part16.integer = 0xA0;
                    entity->stage++;
                    break;
                }
                ret = 0x20;
                break;
            }
            break;
        case 1:
            if (entity->stage == 0)
            {
                PlaySong(0x1B7);
                entity->stage++;
                break;
            }
            break;
        case 2:
            switch (entity->stage)
            {
            case 0:
                Sound_Play(0x1040);
                ew->gameState.textWindowPause = state;
                entity->velX.whole = -0x8000;
                entity->flipX = 0;
                if (entity->animScriptIndex != 1)
                    Anim_StartIndexed(entity, 1, 0, 1);
                entity->stage++;
                /* fall through */
            case 1:
                if ((s16)entity->posX.part16.integer < -8)
                {
                    g->textWindowPause = 0;
                    g->textWindowState = 5;
                    entity->stage++;
                }
                else
                {
                    entity->posX.whole += entity->velX.whole;
                }
                break;
            }
            break;
        }
        break;
    case 0x80:
        Music_UpdateRoom(g);
        GameFlag_Set(0x25);
        Interaction_End(entity, 1);
        goto end;
    }
    Anim_Update(entity);
    return ret;
end:
}

/* Cutscene setup with two helper entities (EventSceneRescueYoko_Init). The do/while (0)
 * around Anim_Update(entity) gives entity 32 refs so it gets r4. */

/**
 * @brief Init of event scene variant 9 (Object 0x20 spawnParam0 = 9,
 * sEventSceneCreateFuncs[9]).
 */
void EventSceneRescueYoko_Init(struct EwramData_EntityData *self)
{
    struct EwramData_EntityData *entity = self;
    s32 flag;
    struct EwramData_EntityData *player;
    struct BgLayerState *bg;
    s32 slot;

    flag = GameFlag_GetMask(0x26);
    if (flag || (player = gEwramData->playerState.player, (s16)player->posX.part16.integer <= 0x7F))
    {
        entity->deleteRequested = 1;
        return;
    }
    player->flipX = 1;
    if (!Entity_SetupFromObjectTable(entity, 2))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 3, 0, 1);
    do { Anim_Update(entity); } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
    bg = &gEwramData->bgInfo[1];
    entity->posX.part16.integer = 0x48 - bg->xPos.part16.integer;
    entity->posY.part16.integer = 0xA0 - bg->yPos.part16.integer;
    entity->flipX = 1;
    entity->spriteLayer = gEwramData->playerState.player->spriteLayer - 1;
    entity->ext.event09.actor5 = Entity_CreateFromObjectTable(5);
    if (!entity->ext.event09.actor5)
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity->ext.event09.actor5, 5, 0, 0);
    Anim_Update(entity->ext.event09.actor5);
    entity->ext.event09.actor5->posX.part16.integer = entity->posX.part16.integer + 0x13;
    entity->ext.event09.actor5->posY.part16.integer = entity->posY.part16.integer;
    entity->ext.event09.actor5->velX.whole = -0x18000;
    entity->ext.event09.actor5->flipX = 1 - entity->flipX;
    entity->ext.event09.actor5->spriteLayer = entity->spriteLayer + 1;
    slot = ObjAffine_Alloc(1);
    if (slot < 0)
    {
        EntityDelete(entity);
        return;
    }
    entity->ext.event09.scaleX = 0x10000;
    entity->ext.event09.scaleY = 0x10000;
    entity->ext.event09.actor5->affineIndex = slot;
    entity->ext.event09.actor5->affine = 1;
    ObjAffine_Set(0x10000, entity->ext.event09.scaleX, entity->ext.event09.scaleY, 0, flag, entity->ext.event09.actor5->affineIndex);
    entity->ext.event09.actor1 = Entity_CreateFromObjectTable(1);
    if (!entity->ext.event09.actor1)
    {
        EntityDelete(entity->ext.event09.actor5);
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity->ext.event09.actor1, 0, 0, 1);
    Anim_Update(entity->ext.event09.actor1);
    { struct EwramData_EntityData *c = entity->ext.event09.actor1; struct BgLayerState *b = &gEwramData->bgInfo[1]; c->posX.part16.integer = 0xFF - b->xPos.part16.integer; }
    entity->ext.event09.actor1->posY.part16.integer = entity->posY.part16.integer;
    entity->ext.event09.actor1->velX.whole = -0x6000;
    entity->ext.event09.actor1->flipX = 0;
    gEwramData->gameState.stateFlags |= 1;
}

/* EventSceneRescueYoko_PlaceHelpers. */
/**
 * @brief 601D4 | Scene step: face left one layer above the player with
 * frame 0x12, end helper unk_4F8 (deleteRequested) and put helper unk_4FC 0x60
 * px to the right with animation 1.
 */
s32 EventSceneRescueYoko_PlaceHelpers(struct EwramData_EntityData *entity)
{
    entity->flipX = 1;
    entity->spriteLayer = gEwramData->playerState.player->spriteLayer + 1;
    entity->animFrame = 0x12;
    if (entity->ext.event09.actor5->updateFunc != NULL)
        entity->ext.event09.actor5->deleteRequested = 1;
    if (entity->ext.event09.actor1->updateFunc != NULL)
        entity->ext.event09.actor1->posX.whole = entity->posX.whole + 0x600000;
    Anim_StartIndexed(entity->ext.event09.actor1, 1, 0, 1);
    entity->flipX = 1;
}

/* Event sequencing. Several original early-return paths leave r0 unspecified. */
/**
 * @brief Update of event scene variant 9 (sEventSceneUpdateFuncs[9]); returns the player
 * input mask.
 */
s32 EventSceneRescueYoko_Update(struct EwramData_EntityData *entity)
{
    s32 input = 0;
    struct GameState *context;
    /* The pinned flag pointer below preserves agbcc's scratch-address lifetime;
     * it reconstructs the binary, not the original source spelling. */
    struct EwramData *ew = gEwramData;
    context = &ew->gameState;
    switch (entity->state) {
    case 0:
        PlaySong(0x1B8);
        Music_StartOverride(40);
        TextWindow_Begin(20);
        { u8 *flag = &ew->gameState.textWindowPause; *flag = 0; }
        entity->state++;
    case 1:
        if (TextWindow_Update()) {
            EventSceneRescueYoko_PlaceHelpers(entity);
            entity->state = 0x80;
            Interaction_End(entity, 0);
            GameFlag_Set(0x26);
            return;
        }
        switch (*(vu8 *)&entity->subState) { /* volatile read (replaces a pinned register) */
        case 0:
        case 7:
            break;
        case 1: {
            switch (entity->stage) {
            case 0:
                context->textWindowPause = 1;
                entity->stage++;
            case 1: {
                struct EwramData_EntityData *other = entity->ext.event09.actor5;
                if (other->updateFunc) {
                    Anim_StartIndexed(other, 6, 0, 0);
                    { struct EwramData_EntityData *updated = entity->ext.event09.actor5;
                      updated->flipX = 1 - updated->flipX; }
                }
                Anim_StartIndexed(entity, 5, 0, 1);
                entity->stage++;
            }
            case 2: {
                struct EwramData_EntityData *other2 = entity->ext.event09.actor5;
                if (other2->animDone) {
                    if (other2->animScriptIndex != 7)
                        Anim_StartIndexed(other2, 7, 0, 1);
                    entity->stage++;
                    entity->timer = 8;
                }
                break;
            }
            case 3:
                if ((u8)--entity->timer == 0xFF)
                    entity->stage++;
                break;
            case 4:
                context->textWindowPause = 0;
                break;
            }
            break;
        }
        case 2:
            switch (entity->stage) {
            case 0:
                context->textWindowPause = 1;
                entity->stage++;
                entity->timer = 16;
                if (entity->animScriptIndex != 4)
                    Anim_StartIndexed(entity, 4, 0, 0);
            case 1:
                if ((u8)--entity->timer == 0xFF) {
                    entity->stage++;
                    entity->timer = 0;
                }
                break;
            case 2: {
                struct EwramData_EntityData *player;
                s32 x;
                if (entity->timer == 36)
                    PlaySong(0x156);
                else if (entity->timer == 0)
                    PlaySong(0x155);
                entity->timer++;
                player = gEwramData->playerState.player;
                x = entity->posX.whole + 0x200000;
                if (player->posX.whole <= x) {
                    player->posX.whole = x;
                    entity->stage++;
                } else
                    input |= 0x20;
                break;
            }
            case 3:
                input |= 0x80;
                context->textWindowPause = 0;
                break;
            }
            if (entity->ext.event09.scaleX != 0) {
                entity->ext.event09.scaleX -= 0x800;
                entity->ext.event09.scaleY += 0x400;
                ObjAffine_Set(0x10000, entity->ext.event09.scaleX, entity->ext.event09.scaleY,
                            0, 0, entity->ext.event09.actor5->affineIndex);
            } else
                entity->ext.event09.actor5->hidden = 1;
            break;
        case 3:
            switch (entity->stage) {
            case 0:
                context->textWindowPause = 1;
                entity->stage++;
                entity->timer = 48;
            case 1:
                if ((u8)--entity->timer == 0xFF) {
                    entity->stage++;
                    entity->timer = 96;
                    if (entity->animScriptIndex != 7)
                        Anim_StartIndexed(entity, 7, 0, 0);
                }
                break;
            case 2:
                if (entity->animDone)
                    context->textWindowPause = 0;
                break;
            }
            input |= 0x80;
            break;
        case 4:
            switch (entity->stage) {
            case 0:
                context->textWindowPause = 1;
                entity->timer = 48;
                entity->stage++;
                if (entity->animScriptIndex != 8)
                    Anim_StartIndexed(entity, 8, 0, 0);
            case 1:
                if ((u8)--entity->timer == 0xFF)
                    entity->stage++;
                break;
            case 2:
                context->textWindowPause = 0;
                break;
            }
            input |= 0x80;
            break;
        case 5:
            switch (entity->stage) {
            case 0:
                context->textWindowPause = 1;
                entity->stage++;
                entity->timer = 48;
            case 1:
                if (entity->timer == 0) {
                    context->textWindowPause = 0;
                    entity->ext.event09.actor1->state = 1;
                } else
                    entity->timer--;
                break;
            }
            input |= 0x80;
            break;
        case 6:
            gEwramData->playerState.player->flipX = 0;
            entity->spriteLayer = gEwramData->playerState.player->spriteLayer + 1;
            break;
        }
        { struct EwramData_EntityData *other = entity->ext.event09.actor1;
        if (other->state) {
            s32 x = entity->posX.whole + 0x600000;
            if (other->posX.whole < x) {
                other->posX.whole = x;
                other = entity->ext.event09.actor1;
                if (other->animScriptIndex != 1)
                    Anim_StartIndexed(other, 1, 0, 1);
                entity->ext.event09.actor1->state = 0;
            } else
                other->posX.whole += other->velX.whole;
        }
        }
        break;
    case 3:
        switch (entity->stage) {
        case 0:
            { struct EwramData_EntityData **player = &ew->playerState.player;
              (*player)->flipX = 1; }
            entity->stage++;
        case 1:
            if ((s16)gEwramData->playerState.player->posX.part16.integer <= 2)
                entity->stage++;
            else
                input = 0x20;
            break;
        case 2:
            break;
        }
        break;
    case 0x80:
        return;
    }
    Anim_Update(entity);
    return input;
}

/**
 * @brief Init of event scene variant 10 (Object 0x20 spawnParam0 = 10,
 * sEventSceneCreateFuncs[10]). Set up a cutscene object: only when event flag
 * 0x27 is not set, bossEventFlags bit 6 is set and the player is left of x =
 * 0x80; clears the HBlank buffer and faces the object towards the
 * player.
 */
void EventSceneJRegainsMemories_Init(struct EwramData_EntityData *entity)
{
    s32 r;
    struct BgLayerState *cam;

    r = GameFlag_GetMask(0x27);
    if (r != 0)
    {
        entity->deleteRequested = 1;
        return;
    }
    if (!(gEwramData->gameState.bossEventFlags & 0x40))
    {
        entity->deleteRequested = 1;
        return;
    }
    if ((s16)gEwramData->playerState.player->posX.part16.integer > 0x80)
    {
        entity->deleteRequested = 1;
        return;
    }
    gEwramData->hBlankEffect.requestStop = 1;
    DMA_FILL_32(3, r, gEwramData->hBlankEffect.hBlankBuffer, 0xA00);
    if (!Entity_SetupFromObjectTable(entity, 7))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 4, 1, 1);
    Anim_Update(entity);
    cam = &gEwramData->bgInfo[1];
    entity->posX.part16.integer = 0x118 - cam->xPos.part16.integer;
    entity->posY.part16.integer = 0xC0 - cam->yPos.part16.integer;
    entity->flipX = entity->posX.whole < gEwramData->playerState.player->posX.whole;
    gEwramData->playerState.player->flipX = 0;
}

/* A signed field view keeps the camera base and ldrsh offset used by agbcc. */
struct CameraXView { u8 pad[6]; s16 x; };
/* Scripted movement and camera sequence; state meanings remain tentative. */
/**
 * @brief Update of event scene variant 10 (sEventSceneUpdateFuncs[10]); returns the
 * player input mask.
 */
s32 EventSceneJRegainsMemories_Update(struct EwramData_EntityData *entity)
{
    s32 input = 0;
    struct EwramData *ew = gEwramData;
    struct GameState *context = &ew->gameState;
    switch (entity->state) {
    case 0:
        if ((s16)ew->playerState.player->posX.part16.integer > 96) {
            ew->hBlankEffect.requestStop = 1;
            DMA_FILL_32(3, 0, gEwramData->hBlankEffect.hBlankBuffer, 0xA00);
            gEwramData->gameState.stateFlags |= 0x200;
            Hud_CloseMessageWindow();
            RedSoul_CancelAll();
            BlueSoul_StopAll();
            BgPalette_StopColorFade();
            ObjPalette_StopColorFade();
            entity->state++;
            entity->stage = 0;
            entity->subState = 0;
        } else
            ew->gameState.stateFlags &= ~0x200;
        break;
    case 1:
        ew->gameState.stateFlags |= 1;
        TextWindow_Begin(21);
        ew->gameState.textWindowPause = 1;
        entity->state++;
        entity->stage = 0;
        entity->subState = 0;
    case 2:
        if (TextWindow_Update())
            entity->state = 0x80;
        switch (entity->subState) {
        case 0:
            switch (entity->stage) {
            case 0:
                context->textWindowPause = 1;
                entity->stage++;
                entity->timer = 48;
            case 1:
                if ((u8)--entity->timer == 0xFF) {
                    Sound_Play(0x1040);
                    entity->stage++;
                }
                break;
            case 2: {
                struct EwramData *room = gEwramData;
                struct EwramData_EntityData **player = &room->playerState.player;
                s32 x = (s16)(*player)->posX.part16.integer;
                struct BgLayerState *bg = &gEwramData->bgInfo[1];
                if (x > 224 - (s16)bg->xPos.part16.integer) {
                    (*player)->posX.part16.integer = 224 - bg->xPos.part16.integer;
                    (*player)->flipX = 0;
                    gEwramData->gameState.cameraScrollMinPriority = 1;
                    entity->stage++;
                } else
                    input |= 0x10;
                break;
            }
            case 3:
                if (((struct CameraXView *)&gEwramData->bgInfo[1])->x <= 137)
                    Camera_ScrollWithPriority(1, 0x8000, 0);
                else {
                    Music_SetCurrent(22);
                    context->textWindowPause = 0;
                    entity->stage++;
                }
                break;
            case 4:
                break;
            }
            break;
        case 1:
            switch (entity->stage) {
            case 0:
                context->textWindowPause = 1;
                if (entity->animScriptIndex != 15)
                    Anim_StartIndexed(entity, 15, 1, 0);
                entity->stage++;
                break;
            case 1:
                if (entity->animDone) {
                    entity->timer = 64;
                    if (entity->animScriptIndex != 0)
                        Anim_StartIndexed(entity, 0, 1, 1);
                    entity->stage++;
                }
                break;
            case 2:
                if ((u8)--entity->timer == 0xFF)
                    entity->stage++;
                break;
            case 3:
                context->textWindowPause = 0;
                break;
            }
            break;
        case 2:
            switch (entity->stage) {
            case 0:
                Sound_Play(0x1040);
                context->textWindowPause = 1;
                entity->velX.whole = 0x18000;
                entity->flipX = 1;
                if (entity->animScriptIndex != 1)
                    Anim_StartIndexed(entity, 1, 1, 1);
                entity->timer = 16;
                entity->stage++;
            case 1:
                if ((s16)entity->posX.part16.integer <= 511)
                    entity->posX.whole += entity->velX.whole;
                if (entity->timer == 0) {
                    Camera_ScrollWithPriority(1, -0x8000, 0);
                    if ((s16)gEwramData->playerState.player->posX.part16.integer == 120) {
                        Sound_Play(0x1040);
                        context->textWindowPause = 0;
                        context->textWindowState = 5;
                        entity->stage++;
                    }
                } else
                    entity->timer--;
                break;
            }
            break;
        }
        break;
    case 0x80:
        Music_UpdateRoom(context);
        gEwramData->gameState.cameraScrollMinPriority = 0;
        Interaction_End(entity, 1);
        GameFlag_Set(0x27);
        break;
    }
    if (entity->state != 0x80)
        Anim_Update(entity);
    return input;
}

/* EventSceneGrahamBattleStart_Init. */
/**
 * @brief Init of event scene variant 11 (Object 0x20 spawnParam0 = 11,
 * sEventSceneCreateFuncs[11]). Create step of a scene object: deleted once
 * stateFlags bit 16 is set; else animation 0 facing like the player,
 * stateFlags bit 0.
 */
void EventSceneGrahamBattleStart_Init(struct EwramData_EntityData *entity)
{
    struct EwramData *ewram;

    if (gEwramData->gameState.stateFlags & 0x10000)
    {
        entity->deleteRequested = 1;
        return;
    }
    if (Entity_SetupFromObjectTable(entity, 4) == 0)
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    ewram = gEwramData;
    entity->flipX = ewram->playerState.player->flipX;
    ewram->gameState.stateFlags |= 1;
}

/**
 * @brief Update of event scene variant 11 (sEventSceneUpdateFuncs[11]); returns the
 * player input mask. Cutscene step object: shows conversation 0x16,
 * toggles textWindowPause while it runs and returns 0x20 once the player walks
 * right of x = 0xB6; when the conversation ends (state 0x80) sets event
 * flag 0x28.
 */
s32 EventSceneGrahamBattleStart_Update(struct EwramData_EntityData *entity)
{
    s32 ret;
    struct EwramData *ew;
    struct GameState *g;

    ret = 0;
    ew = gEwramData;
    g = &ew->gameState;
    ew->gameState.stateFlags |= 1;
    switch (entity->state)
    {
    case 0:
        Sound_Play(0x1040);
        TextWindow_Begin(0x16);
        ew->gameState.textWindowPause = 1;
        entity->state++;
        entity->timer = 0x90;
        break;
    case 1:
        if (TextWindow_Update())
        {
            entity->state = 0x80;
            break;
        }
        switch (entity->subState)
        {
        case 0:
            if (--entity->timer == 0xFF)
            {
                entity->subState++;
                g->textWindowPause = 0;
            }
            break;
        case 2:
            g->textWindowPause = 1;
            if (GetEntityRoomXPositionIntegerWide(gEwramData->playerState.player) > 0xB6)
            {
                ret = 0x20;
                break;
            }
            entity->subState++;
            break;
        case 1:
            break;
        case 3:
            g->textWindowPause = 0;
            entity->subState++;
            break;
        case 4:
            break;
        }
        break;
    case 0x80:
        GameFlag_Set(0x28);
        Interaction_End(entity, 1);
        break;
    }
    if (entity->state != 0x80)
        Anim_Update(entity);
    return ret;
}

/* EventSceneOpenPortal_Init. */
/**
 * @brief Init of event scene variant 12 (Object 0x20 spawnParam0 = 12,
 * sEventSceneCreateFuncs[12]). Create step: animation 1 entering from x -16 at
 * room y 0xA0, moving right (0x4000), facing left; always sets stateFlags
 * bit 0.
 */
void EventSceneOpenPortal_Init(struct EwramData_EntityData *entity)
{
    if (Entity_SetupFromObjectTable(entity, 1) == 0)
    {
        EntityDelete(entity);
    }
    else
    {
        Anim_StartIndexed(entity, 1, 0, 1);
        Anim_Update(entity);
        entity->posX.part16.integer = -0x10;
        {
            struct BgLayerState *bg = &gEwramData->bgInfo[1];

            entity->posY.part16.integer = 0xA0 - bg->yPos.part16.integer;
        }
        entity->velX.whole = 0x4000;
        entity->flipX = 1;
    }
    gEwramData->gameState.stateFlags |= 1;
}

/**
 * @brief Update of event scene variant 12 (sEventSceneUpdateFuncs[12]); returns the
 * player input mask. Cutscene step object for conversation 0x19: walks
 * in until x > 7, turns the player around and toggles textWindowPause; returns
 * 0x40 from step 3 on and after the conversation ends.
 */
s32 EventSceneOpenPortal_Update(struct EwramData_EntityData *entity)
{
    s32 ret;
    u8 step;

    ret = 0;
    switch (entity->state)
    {
    case 0:
        gEwramData->gameState.stateFlags |= 1;
        TextWindow_Begin(0x19);
        entity->state++;
        break;
    case 1:
        if (TextWindow_Update())
        {
            entity->state = 0x80;
            break;
        }
        step = entity->subState;
        if (step == 1)
        {
            if (++entity->stage == 1)
            {
                if (entity->animScriptIndex != 0)
                    Anim_StartIndexed(entity, 0, 0, 1);
                gEwramData->gameState.textWindowPause = step;
            }
            if ((s16)entity->posX.part16.integer <= 7)
            {
                entity->posX.whole += entity->velX.whole;
                break;
            }
            if (entity->animScriptIndex != 4)
                Anim_StartIndexed(entity, 4, 0, 1);
            gEwramData->playerState.player->flipX = 1;
            gEwramData->gameState.textWindowPause = 0;
            entity->subState++;
            entity->stage = 0;
        }
        else if (step > 2)
        {
            ret = 0x40;
            if (step == 4)
            {
                if (++entity->stage == 1)
                {
                    entity->timer = 0x40;
                    gEwramData->gameState.textWindowPause = 1;
                }
                else if (entity->timer == 0)
                {
                    gEwramData->gameState.textWindowPause = 0;
                }
                else
                {
                    entity->timer--;
                }
            }
        }
        break;
    case 0x80:
        ret = 0x40;
        TextWindow_Close();
        gEwramData->gameState.stateFlags |= 1;
        break;
    }
    if (entity->state != 0x80)
        Anim_Update(entity);
    return ret;
}

/* EventSceneSomaAwakens_Init. */
/**
 * @brief Init of event scene variant 15 (Object 0x20 spawnParam0 = 15,
 * sEventSceneCreateFuncs[15]). Create step: animation 0 at room (0, 0xA0) with
 * hidden and facing left, stateFlags bit 0.
 */
void EventSceneSomaAwakens_Init(struct EwramData_EntityData *entity)
{
    struct EwramData *ewram;
    struct BgLayerState *bg;

    if (Entity_SetupFromObjectTable(entity, 1) == 0)
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    ewram = gEwramData;
    bg = &ewram->bgInfo[1];
    entity->posX.part16.integer = -bg->xPos.part16.integer;
    entity->posY.part16.integer = 0xA0 - bg->yPos.part16.integer;
    entity->hidden = 1;
    entity->flipX = 1;
    ewram->gameState.stateFlags |= 1;
}

/* EventSceneSomaAwakens_Update, decompiled from the original assembly: event scene with the
 * flashing player (palette fades, periods table at sEventSceneSomaAwakensPeriods copied to the
 * stack). The x mark is copied into a u16 local in sub-state 2 (keeps the
 * separate register copy), and the owner address is written as an integer
 * sum so the base comes first. */

struct Periods { s32 v[10]; };

#define PLAYER (gEwramData->playerState.player)
#define IDX (*(u8 *)&entity->ext.raw.unk_4F8.part8.unk_4F8)

/**
 * @brief Update of event scene variant 15 (sEventSceneUpdateFuncs[15]); returns the
 * player input mask.
 */
s32 EventSceneSomaAwakens_Update(struct EwramData_EntityData *entity)
{
    struct Periods periods = *(struct Periods *)sEventSceneSomaAwakensPeriods;
    s32 result = 0;
    struct EwramData *ew = gEwramData;
    struct GameState *save = &ew->gameState;
    s32 x = 0xAA;
    s32 y = 0x50;
    u32 full = 0xFF;

    switch (entity->state)
    {
    case 0:
        TextWindow_Begin(0x1C);
        ew->gameState.textWindowPause = 1;
        entity->state++;
        break;
    case 1:
        if (TextWindow_Update())
            entity->state = 0x80;
        switch (entity->subState)
        {
        case 0:
            if (Player_GetFocusedActionFlags() & 0x1E)
                break;
            {
                u8 i = ENTITY_PARENT_INDEX(entity);
                struct EwramData *w = gEwramData;
                struct EwramData_EntityData *e = (struct EwramData_EntityData *)(i * sizeof(struct EwramData_EntityData) + (u32)w + 0x4E4);
                u8 flip;
                flip = 0;
                if (e->posX.whole < w->playerState.player->posX.whole)
                    flip = 1;
                w->entityData[i].flipX = flip;
            }
            if ((s8)Player_GetZoneAroundXCopy(x, 0x3C, 0x21))
            {
                entity->subState = 2;
                break;
            }
            entity->subState = 3;
            break;
        case 2:
        {
            u16 mark = x;
            if ((s8)Player_GetZoneAroundXCopy(mark, 0x3C, 0x21) == 1)
            {
                s32 px = (s16)PLAYER->posX.part16.integer;
                result = 0x10;
                if (mark < px)
                    result = 0x20;
                break;
            }
            entity->subState = 3;
            break;
        }
        case 3:
            if (entity->stage == 0)
            {
                entity->timer = 0x60;
                entity->stage++;
                Music_StartOverride(0x16C);
            }
            if (entity->timer == 0)
            {
                save->textWindowPause = 0;
                entity->subState++;
                entity->stage = 0;
            }
            break;
        case 4:
            if (entity->stage == 0)
            {
                BgPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0, 0x10000, 0x20);
                ObjPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0x4000, 0x10000, 0x10);
                PlaySong(0x170);
                entity->stage++;
            }
            if (entity->timer-- & 3)
                PLAYER->palette = 4;
            else
                PLAYER->palette = 0;
            Player_StartScriptedAnimation(0x52, 0, 0, 0);
            break;
        case 5:
            if (entity->timer-- & 3)
                PLAYER->palette = 4;
            else
                PLAYER->palette = 0;
            if (entity->timer % (periods.v[IDX] * 3 / 2) == 0)
            {
                PlaySong(0x170);
                BgPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0, 0x10000, 0x20);
                ObjPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0x4000, 0x10000, 0x10);
                if (++IDX == 10)
                    IDX = 0;
            }
            Player_StartScriptedAnimation(0x52, 0, 0, 0);
            break;
        case 6:
            if (entity->timer-- & 2)
                PLAYER->palette = 4;
            else
                PLAYER->palette = 0;
            if (entity->timer % periods.v[IDX] == 0)
            {
                PlaySong(0x170);
                BgPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0, 0x10000, 0x20);
                ObjPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0x4000, 0x10000, 0x10);
                if (++IDX == 10)
                    IDX = 0;
            }
            Player_StartScriptedAnimation(0x52, 0, 0, 0);
            break;
        case 7:
            switch (entity->stage)
            {
            case 0:
            {
                s32 half;
                save->textWindowPause = 1;
                half = full >> 1;
                entity->timer = half + 8;
                entity->stage++;
                PlaySong(0x816C);
                PlaySong(0x197);
                PlaySong(0x169);
                BgPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0x10000, 0, half);
                ObjPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0x10000, 0, half);
            }
            case 1:
                if (entity->timer & 2)
                    PLAYER->palette = 4;
                else
                    PLAYER->palette = 0;
                if ((u8)--entity->timer == 0xFF)
                {
                    EntityDeleteGroup(0x40);
                    BgPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0, 0x10000, full);
                    ObjPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0, 0x10000, full);
                    entity->timer = full;
                    entity->stage++;
                    PLAYER->palette = 0;
                }
                break;
            case 2:
                if (entity->timer == 0x20)
                    Music_SetCurrent(0x21);
                if ((u8)--entity->timer == 0xFF)
                {
                    BgPalette_StopColorFade();
                    ObjPalette_StopColorFade();
                    entity->stage++;
                    entity->timer = 0x40;
                }
                break;
            case 3:
                if ((u8)--entity->timer == 0xFF)
                {
                    PLAYER->palette = 0;
                    entity->subState++;
                    entity->stage = 0;
                    save->textWindowPause = 0;
                }
                break;
            }
            result |= 0x80;
            break;
        case 8:
            if (entity->stage == 0)
                entity->stage++;
            result |= 0x80;
            break;
        case 9:
            switch (entity->stage)
            {
            case 0:
                PLAYER->flipX = 1;
                save->textWindowPause = 1;
                entity->hidden = 0;
                entity->stage++;
                entity->timer = 0xC0;
            case 1:
                if (!(Entity_TryWalk(entity, 0x6000, 0x200000, 0x10000, 1) & 4))
                {
                    entity->posX.whole += entity->velX.whole;
                    entity->posY.whole += entity->velY.whole;
                }
                else if (entity->animScriptIndex != 0)
                    Anim_StartIndexed(entity, 0, 0, 1);
                if ((s16)entity->posX.part16.integer >= y)
                {
                    entity->posX.part16.integer = y;
                    if (entity->animScriptIndex != 1)
                        Anim_StartIndexed(entity, 1, 0, 1);
                    entity->posY.part16.integer++;
                    save->textWindowPause = 0;
                    entity->subState++;
                    entity->stage = 0;
                }
                break;
            }
            break;
        case 11:
            if (entity->stage == 0)
            {
                entity->stage++;
                if (entity->animScriptIndex != 4)
                    Anim_StartIndexed(entity, 4, 0, 1);
            }
            break;
        }
        break;
    case 0x80:
        GameFlag_Set(0x3A);
        ObjPalette_StopColorFade();
        BgPalette_StopColorFade();
        Sound_Play(0x1040);
        save->currentSong = result;
        Interaction_End(entity, 0);
        PLAYER->palette = 0;
        entity->posX.part16.integer = y;
        { struct BgLayerState *bg = &gEwramData->bgInfo[1]; entity->posY.part16.integer = 0xC0 - bg->yPos.part16.integer; }
        entity->hidden = 0;
        if (entity->animScriptIndex != 4)
            Anim_StartIndexed(entity, 4, 0, 1);
        Anim_Update(entity);
        PlaySong(0x816C);
        EntityDeleteGroup(0x40);
        entity->state = 0x81;
        entity->stage = 0;
        entity->subState = 0;
        break;
    }
    if ((s8)entity->state >= 0)
        Anim_Update(entity);
    return result;
}

/* EventSceneArikadoChaosPath_Init. */
/**
 * @brief Init of event scene variant 16 (Object 0x20 spawnParam0 = 16,
 * sEventSceneCreateFuncs[16]). Create step: deleted once stateFlags bit 16 is set;
 * else animation 4 at room (0x50, 0xC0) facing left with enemyFlags bit 2.
 */
void EventSceneArikadoChaosPath_Init(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg;

    if (gEwramData->gameState.stateFlags & 0x10000)
    {
        entity->deleteRequested = 1;
        return;
    }
    if (Entity_SetupFromObjectTable(entity, 1) == 0)
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 4, 0, 1);
    Anim_Update(entity);
    bg = &gEwramData->bgInfo[1];
    entity->posX.part16.integer = 0x50 - bg->xPos.part16.integer;
    entity->posY.part16.integer = 0xC0 - bg->yPos.part16.integer;
    entity->flipX = 1;
    entity->enemyFlags |= 4;
}

/**
 * @brief Update of event scene variant 16 (sEventSceneUpdateFuncs[16]); returns the
 * player input mask. NPC object: when the player stands near it and
 * presses up / A while no conversation runs, stops the player and starts
 * conversation 0x1D (0x1E after the fourth time), facing the player.
 */
s32 EventSceneArikadoChaosPath_Update(struct EwramData_EntityData *entity)
{
    s32 ret;
    u16 new;
    struct EwramData_EntityData *player;

    ret = 0;
    new = gEwramData->inputData.newInput;
    switch (entity->state)
    {
    case 0:
        Interaction_End(entity, 0);
        entity->state++;
        /* fall through */
    case 1:
        if (!Entity_IsInRect(gEwramData->playerState.player, (s16)(entity->posX.part16.integer - 0x20), (s16)(entity->posY.part16.integer - 0x20), 0x40, 0x40))
            break;
        if (!(new & 0x42))
            break;
        if (gEwramData->gameState.timeStopTimer != 0)
            break;
        gEwramData->playerState.player->velX.whole = 0;
        gEwramData->playerState.player->accelX.whole = 0;
        player = gEwramData->playerState.player;
        if (player->velY.whole < 0)
            player->velY.whole = 0;
        if (entity->timer++ > 3)
            TextWindow_Begin(0x1E);
        else
            TextWindow_Begin(0x1D);
        entity->flipX = entity->posX.whole < gEwramData->playerState.player->posX.whole;
        gEwramData->gameState.stateFlags |= 1;
        entity->state++;
        break;
    case 2:
        if (TextWindow_Update())
        {
            entity->state = 0x80;
            Interaction_End(entity, 0);
            entity->state = ret;
        }
        break;
    }
    return ret;
}

/* Event script steps 0x08061824..0x080619FF. */
#define PLAYER_ENTITY (gEwramData->playerState.player)

/**
 * @brief 61824 | Where the player is relative to x centerX: 1 when outside
 * the window of width outerWidth centred on centerX, -1 when inside the
 * (narrower) window of width innerWidth, else 0.
 */
s32 Player_GetZoneAroundXCopy(u16 centerX, u16 innerWidth, u16 outerWidth)
{
    struct EwramData_EntityData *player = PLAYER_ENTITY;

    if ((u16)(player->posX.part16.integer - (centerX - outerWidth / 2)) < outerWidth)
    {
        if ((u16)(player->posX.part16.integer - (centerX - innerWidth / 2)) < innerWidth)
            return -1;
        return 0;
    }
    return 1;
}

/**
 * @brief Init of event scene variant 13 (Object 0x20 spawnParam0 = 13,
 * sEventSceneCreateFuncs[13]). Set stateFlags bit 0.
 */
void EventSceneGrahamSeesSomaPower_Init(void)
{
    gEwramData->gameState.stateFlags |= 1;
}

/**
 * @brief Update of event scene variant 13 (sEventSceneUpdateFuncs[13]); returns the
 * player input mask. Event step: start script 0x1A, then once it
 * finished (TextWindow_Update) call Interaction_End(1).
 */
s32 EventSceneGrahamSeesSomaPower_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
        case 0:
            TextWindow_Begin(0x1A);
            entity->state++;
            break;
        case 1:
            if (TextWindow_Update())
                Interaction_End(entity, 1);
            break;
    }
    return 0;
}

/**
 * @brief Init of event scene variant 14 (Object 0x20 spawnParam0 = 14,
 * sEventSceneCreateFuncs[14]). Same as EventSceneGrahamSeesSomaPower_Init.
 */
void EventSceneGrahamNotDracula_Init(void)
{
    gEwramData->gameState.stateFlags |= 1;
}

/**
 * @brief Update of event scene variant 14 (sEventSceneUpdateFuncs[14]); returns the
 * player input mask. Event step: start script 0x1B, then once it
 * finished call TextWindow_Close and end (0x53D bit 3).
 */
s32 EventSceneGrahamNotDracula_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
        case 0:
            TextWindow_Begin(0x1B);
            entity->state++;
            break;
        case 1:
            if (TextWindow_Update())
            {
                TextWindow_Close();
                entity->deleteRequested = 1;
            }
            break;
    }
    return 0;
}

/**
 * @brief Init of event scene variant 17 (Object 0x20 spawnParam0 = 17,
 * sEventSceneCreateFuncs[17]). Nothing.
 */
void EventSceneGrahamTransforms_Init(void)
{
}

/**
 * @brief Update of event scene variant 17 (sEventSceneUpdateFuncs[17]); returns the
 * player input mask. Event step: set stateFlags bit 0 and start script
 * 0x17, then once it finished call Interaction_End(1).
 */
s32 EventSceneGrahamTransforms_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
        case 0:
            gEwramData->gameState.stateFlags |= 1;
            TextWindow_Begin(0x17);
            entity->state++;
            break;
        case 1:
            if (TextWindow_Update())
                Interaction_End(entity, 1);
            break;
    }
    return 0;
}

/**
 * @brief Init of event scene variant 18 (Object 0x20 spawnParam0 = 18,
 * sEventSceneCreateFuncs[18]). Same as EventSceneGrahamSeesSomaPower_Init.
 */
void EventSceneGrahamLastWords_Init(void)
{
    gEwramData->gameState.stateFlags |= 1;
}

/**
 * @brief Update of event scene variant 18 (sEventSceneUpdateFuncs[18]); returns the
 * player input mask. Event step: set stateFlags bit 0 and start script
 * 0x18; when it finished call TextWindow_Close and end; state 0x80 calls
 * Interaction_End(1).
 */
s32 EventSceneGrahamLastWords_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
        case 0:
            gEwramData->gameState.stateFlags |= 1;
            TextWindow_Begin(0x18);
            entity->state++;
            break;
        case 1:
            if (TextWindow_Update())
            {
                TextWindow_Close();
                entity->deleteRequested = 1;
            }
            break;
        case 0x80:
            Interaction_End(entity, 1);
            break;
    }
    return 0;
}

/* EventSceneJuliusBattleStart_Init. */

/**
 * @brief Init of event scene variant 19 (Object 0x20 spawnParam0 = 19,
 * sEventSceneCreateFuncs[19]). Create step of a scene object: deleted when flag
 * 0x29 is set or the player is left of room x 0x80; if EventSceneJuliusBattleStart_LoadGraphics
 * fails mark the event and delete; else enter from room x 0x150 moving
 * left (-0x10000) with animation 1, stateFlags bit 0. The camera address is
 * added to the loaded gEwramData in place, hence the OFFSET_OF form.
 */
void EventSceneJuliusBattleStart_Init(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg;

    if (GameFlag_GetMask(0x29) != 0)
    {
        entity->deleteRequested = 1;
        return;
    }
    {
        s32 x;

        bg = (struct BgLayerState *)gEwramData;
        x = (s16)((struct EwramData *)bg)->playerState.player->posX.part16.integer;
        bg = (struct BgLayerState *)((u32)bg + OFFSET_OF(struct EwramData, bgInfo[1]));
        if (x + (s16)bg->xPos.part16.integer <= 0x7F)
        {
            entity->deleteRequested = 1;
            return;
        }
    }
    if (EventSceneJuliusBattleStart_LoadGraphics(entity) == 0)
    {
        Interaction_End(entity, 1);
        EntityDelete(entity);
        return;
    }
    {
        struct BgLayerState *cam = (struct BgLayerState *)gEwramData;

        cam = (struct BgLayerState *)((u32)cam + OFFSET_OF(struct EwramData, bgInfo[1]));
        entity->posX.part16.integer = 0x150 - cam->xPos.part16.integer;
    }
    entity->velX.whole = -0x10000;
    entity->flipX = 0;
    entity->hidden = 1;
    Anim_StartIndexed(entity, 1, 1, 1);
    Anim_Update(entity);
    gEwramData->gameState.stateFlags |= 1;
}

/**
 * @brief Update of event scene variant 19 (sEventSceneUpdateFuncs[19]); returns the
 * player input mask. Cutscene object for conversation 0x1F: waits until
 * the player is left of x = 0xC0 (returns 0x20 meanwhile), then walks in
 * while the camera scrolls; on the end of the conversation sets event
 * 0x29.
 */
s32 EventSceneJuliusBattleStart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData **ewp;
    struct EwramData *ew;
    s32 ret;
    s32 x;
    struct BgLayerState *c2;

    ewp = &gEwramData;
    ew = *ewp;
    ret = 0;
    switch (entity->state)
    {
    case 0:
        Sound_Play(0x1040);
        Sound_PlayTracked(0x2B);
        entity->state++;
        entity->stage = ret;
        entity->subState = ret;
        /* fall through */
    case 1:
    {
        struct EwramData *w = gEwramData;
        struct BgLayerState *cam;
        s32 px;
        w->gameState.stateFlags |= 1;
        px = (s16)w->playerState.player->posX.part16.integer;
        cam = &w->bgInfo[1];
        if (px + (s16)cam->xPos.part16.integer <= 0xBF)
        {
            entity->state++;
            entity->stage = 0;
            entity->subState = 0;
        }
        else
        {
            ret |= 0x20;
        }
        break;
    }
    case 2:
        TextWindow_Begin(0x1F);
        entity->state++;
        entity->stage = ret;
        entity->subState = ret;
        break;
    case 3:
        if (TextWindow_Update())
        {
            GameFlag_Set(0x29);
            Sound_StopTracked(0x2B);
            (*ewp)->gameState.stateFlags &= ~0x40000;
            Music_SetCurrent(0x14);
            entity->state = 0x80;
            break;
        }
        switch (entity->subState)
        {
        case 0:
            break;
        case 1:
            switch (entity->stage)
            {
            case 0:
                (*ewp)->playerState.player->flipX = 0;
                entity->hidden = 0;
                ew->gameState.textWindowPause = 1;
                (*ewp)->gameState.cameraScrollMinPriority = 1;
                entity->stage++;
                /* fall through */
            case 1:
                c2 = &gEwramData->bgInfo[1];
                if ((s16)c2->xPos.part16.integer > 0x70)
                    Camera_ScrollWithPriority(1, 0, 0);
                else
                    Camera_ScrollWithPriority(1, 0x10000, 0);
                x = (s16)entity->posX.part16.integer;
                c2 = &gEwramData->bgInfo[1];
                x += (s16)c2->xPos.part16.integer;
                if (x <= 0x11F)
                {
                    entity->timer = 0x28;
                    entity->stage++;
                    if (entity->animScriptIndex != 0)
                        Anim_StartIndexed(entity, 0, 1, 1);
                }
                else
                {
                    if (x <= 0x127)
                    {
                        if (entity->animScriptIndex != 2)
                            Anim_StartIndexed(entity, 2, 1, 0);
                        entity->velX.whole = -0x8000;
                    }
                    entity->posX.whole += entity->velX.whole;
                }
                break;
            case 2:
                Camera_ScrollWithPriority(1, 0, 0);
                if (--entity->timer == 0xFF)
                {
                    ew->gameState.textWindowPause = ret;
                    entity->subState++;
                    entity->stage = ret;
                }
                break;
            }
            break;
        case 2:
            Camera_ScrollWithPriority(1, 0, 0);
            break;
        }
        Anim_Update(entity);
        break;
    case 0x80:
        ew->gameState.cameraScrollMinPriority = ret;
        EventSceneJuliusBattleStart_Finish(entity);
        break;
    }
    return ret;
}

/* EventSceneJuliusBattleEnd_Update: event step (cutscene facing / approach checks); matched. */
/* Declared s16 in bg_and_collision.h; the caller here uses the full register. */
/* Declared with u16 parameters in object/shop_update.c; the caller passes s16 values unconverted. */

static inline s32 IsLeft(struct EwramData_EntityData *e)
{
    return e->posX.whole < gEwramData->playerState.player->posX.whole;
}

#define FACE() { u8 i_ = entity->ext.raw.unk_4F4.part8.unk_4F5; struct EwramData *w_ = gEwramData; struct EwramData *b_ = (struct EwramData *)(i_ * 0x84 + (u32)w_); b_->entityData[0].flipX = IsLeft(&b_->entityData[0]); }

/**
 * @brief Update of event scene variant 20 (sEventSceneUpdateFuncs[20]); returns the
 * player input mask.
 */
s32 EventSceneJuliusBattleEnd_Update(struct EwramData_EntityData *entity)
{
    struct EwramData **ewp;
    struct EwramData *ew;
    s32 ret;
    struct GameState *g;
    struct EwramData_EntityData *other;
    struct EwramData_EntityData *player;
    u16 w;
    s32 m1;

    ewp = &gEwramData;
    ew = *ewp;
    ret = 0;
    m1 = -1;
    g = &ew->gameState;
    switch (entity->state)
    {
    case 0:
        TextWindow_Begin(0x20);
        ew->gameState.textWindowPause = 1;
        entity->state++;
        /* fall through */
    case 1:
        if (TextWindow_Update())
        {
            entity->timer = 0x40;
            entity->state = 0x7F;
        }
        switch (entity->subState)
        {
        case 0:
            if (Player_GetFocusedActionFlags() & 0x1E)
                break;
            FACE();
            switch ((s8)Player_GetZoneAroundXWide((s16)entity->posX.part16.integer, 0x3C, 0x64))
            {
            case 0:
                entity->subState = 4;
                break;
            case -1:
                entity->subState = 3;
                break;
            case 1:
                entity->subState = 2;
                break;
            }
            break;
        case 2:
        {
            s16 *px = (s16 *)&entity->posX.part16.integer;
            if ((s8)Player_GetZoneAroundXWide(*px, 0x3C, 0x64) != 1)
            {
                entity->subState = 4;
                break;
            }
            if (*px < (s16)gEwramData->playerState.player->posX.part16.integer)
                ret = 0x20;
            else
                ret = 0x10;
            break;
        }
        case 3:
        {
            s32 x;
            FACE();
            if ((s8)Player_GetZoneAroundXWide((s16)entity->posX.part16.integer, 0x3C, 0x64) == m1)
            {
                if (gEwramData->bgInfo[1].pBgMetadata->widthCells != 1)
                    w = gEwramData->bgInfo[1].pBgMetadata->widthCells << 5;
                else
                    w = 0x1E;
                x = GetEntityRoomXPositionIntegerWide(entity) / 8;
                if (x > (s32)(w >> 1))
                    ret = 0x20;
                else
                    ret = 0x10;
                break;
            }
            entity->subState = 4;
            break;
        }
        case 4:
        {
            u8 z = entity->stage;
            if (z != 0)
                break;
            {
                struct EwramData **e2 = &gEwramData;
                other = &(*e2)->entityData[ENTITY_PARENT_INDEX(entity)];
                other->flipX = other->posX.whole < (*e2)->playerState.player->posX.whole;
                player = (*e2)->playerState.player;
            }
            if (player->flipX != other->flipX)
                player->flipX = 1 - player->flipX;
            g->textWindowPause = z;
            entity->stage++;
            break;
        }
        }
        break;
    case 0x7F:
        GameFlag_Set(0x2A);
        (*ewp)->gameState.stateFlags |= 0x400000;
        if (--entity->timer == 0xFF)
            entity->state++;
        break;
    case 0x80:
        Interaction_End(entity, 1);
        break;
    }
    return ret;
}

/* Event object helpers 0x08061FDC..0x080620E7. */

/**
 * @brief 61FDC | Load the graphics (layer 0x11); delete the entity and
 * return 0 when that fails.
 */
s32 EventSceneJuliusBattleStart_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 result;
    u8 *palette = sJuliusEntityPalette;
    u8 *sheet = sJuliusSpriteSheet;

    if (Entity_LoadGraphics(entity, sEventSceneJuliusBattleStartGfx, palette, 0, sheet))
    {
        entity->spriteLayer = 0x11;
        result = 1;
    }
    else
    {
        EntityDelete(entity);
        result = 0;
    }
    return result;
}

/**
 * @brief 62020 | Release the graphics slot of sEventSceneJuliusBattleStartGfx and continue
 * with Interaction_End(1).
 */
void EventSceneJuliusBattleStart_Finish(struct EwramData_EntityData *entity)
{
    ObjGfx_Unload(ObjGfx_Find(sEventSceneJuliusBattleStartGfx));
    Interaction_End(entity, 1);
}

/**
 * @brief Init of event scene variant 20 (Object 0x20 spawnParam0 = 20,
 * sEventSceneCreateFuncs[20]). Set stateFlags bit 0, BgPalette_StopColorFade and
 * ObjPalette_StopColorFade.
 */
void EventSceneJuliusBattleEnd_Init(void)
{
    gEwramData->gameState.stateFlags |= 1;
    BgPalette_StopColorFade();
    ObjPalette_StopColorFade();
}

/**
 * @brief Init of event scene variant 21 (Object 0x20 spawnParam0 = 21,
 * sEventSceneCreateFuncs[21]). End the entity (0x53D bit 3).
 */
void EventScene21_Init(struct EwramData_EntityData *entity)
{
    entity->deleteRequested = 1;
}

/**
 * @brief Update of event scene variant 21 (sEventSceneUpdateFuncs[21]); returns the
 * player input mask. Sound_Play(0x1040), Music_StartOverride(0x25), then
 * continue with Interaction_End(1).
 */
s32 EventScene21_Update(struct EwramData_EntityData *entity)
{
    Sound_Play(0x1040);
    Music_StartOverride(0x25);
    Interaction_End(entity, 1);
    return 0;
}

/**
 * @brief 62098 | Event wait step: first time set textWindowPause and start a
 * duration frame timer; returns 1 while waiting, 0 once done (stage 2).
 */
s32 EventSceneChaoticRealmMessages_Wait(struct EwramData_EntityData *entity, s32 duration)
{
    struct EwramData *ewram = gEwramData;
    s32 waiting = 1;

    if (entity->stage == 0)
    {
        ewram->gameState.textWindowPause = waiting;
        entity->timer = duration;
        entity->stage++;
    }
    else if (entity->stage == 1)
    {
        if ((u8)--entity->timer == 0xFF)
            entity->stage++;
    }
    else
    {
        waiting = 0;
    }
    return waiting;
}

/* EventSceneChaoticRealmMessages_Update: event step (scene with player walk-in); matched. */
/**
 * @brief Update of event scene variant 22 (sEventSceneUpdateFuncs[22]); returns the
 * player input mask.
 */
s32 EventSceneChaoticRealmMessages_Update(struct EwramData_EntityData *entity)
{
    s32 ret = 0;
    struct EwramData *ew = gEwramData;
    u8 state = entity->state;
    struct EwramData_EntityData *player;

    switch (state)
    {
    case 0:
        Music_StartOverride(0x2D);
        if (GameFlag_GetMask(0x2B))
        {
            entity->deleteRequested = 1;
            goto end;
        }
        {
            register u32 *p asm("r1") = &gEwramData->gameState.stateFlags; /* FAKEMATCH: pinned register */
            *p |= 1;
        }
        entity->state++;
        entity->stage = ret;
        entity->subState = ret;
        break;
    case 1:
        Display_SaveState();
        TextWindow_Begin(0x22);
        ew->gameState.textWindowPause = state;
        entity->state++;
        entity->stage = ret;
        entity->subState = ret;
        break;
    case 2:
        if (TextWindow_Update())
        {
            entity->state = 0x80;
            GameFlag_Set(0x2B);
            Interaction_End(entity, 1);
            Display_RestoreState();
            Sound_Play(0x1041);
            gEwramData->musicOverrideSong = 0x2D;
            PlaySong(0x2D);
            goto end;
        }
        switch (entity->subState)
        {
        case 0:
            if (!EventSceneChaoticRealmMessages_Wait(entity, 0x40))
            {
                entity->subState++;
                entity->stage = ret;
            }
            break;
        case 1:
            player = gEwramData->playerState.player;
            if ((s16)player->posX.part16.integer > 0x9F)
            {
                ret = 0x20;
                break;
            }
            player->posX.part16.integer = 0x9F;
            entity->subState++;
            entity->stage = 0;
            break;
        case 2:
            if (EventSceneChaoticRealmMessages_Wait(entity, 0x40))
                break;
            do { /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
                ew->gameState.textWindowPause = ret;
            } while (0);
            break;
        case 3:
            if (!EventSceneChaoticRealmMessages_Wait(entity, 0x40))
            {
                Sound_StopTracked(0x2D);
                Music_SetCurrent(0x24);
                ew->gameState.textWindowPause = ret;
            }
            break;
        }
        break;
    }
    return ret;
end:
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   EventSceneChaoticRealmMessages_Init = EventSceneGrahamSeesSomaPower_Init */

/**
 * @brief Init of event scene variant 22 (Object 0x20 spawnParam0 = 22,
 * sEventSceneCreateFuncs[22]). Same as EventSceneGrahamSeesSomaPower_Init.
 */
void EventSceneChaoticRealmMessages_Init(void)
{
    gEwramData->gameState.stateFlags |= 1;
}

/* Prepare the event actor, player and companion for the scene. */
/**
 * @brief Init of event scene variant 23 (Object 0x20 spawnParam0 = 23,
 * sEventSceneCreateFuncs[23]).
 */
void EventSceneReturnHome_Init(struct EwramData_EntityData *entity)
{
    struct EwramData *ram;
    struct BgLayerState *background;
    if(!(gEwramData->gameState.stateFlags&0x2000)){entity->deleteRequested=1;return;}
    if(!Entity_SetupFromObjectTable(entity,0))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity,0,0,1);Anim_Update(entity);
    ram=gEwramData;background=&ram->bgInfo[1];
    entity->posX.part16.integer=64-background->xPos.part16.integer;
    entity->posY.part16.integer=192-background->yPos.part16.integer;
    entity->flipX=0;
    ram->playerState.player->posX.part16.integer=entity->posX.part16.integer+32;
    ram->playerState.player->posY.part16.integer=entity->posY.part16.integer;
    ram->playerState.player->flipX=0;
    entity->ext.event23.actor=Entity_CreateFromObjectTable(1);
    if(!entity->ext.event23.actor)
    {
        EntityDelete(entity);return;
    }
    entity->ext.event23.actor->posX.part16.integer=entity->posX.part16.integer+96;
    entity->ext.event23.actor->posY.part16.integer=entity->posY.part16.integer+1;
    Anim_StartIndexed(entity->ext.event23.actor,1,0,1);
    entity->ext.event23.actor->flipX=0;
    gEwramData->gameState.stateFlags|=1;
    BgPalette_StartColorFade(31,31,31,0,0x10000,128);
    ObjPalette_StartColorFade(31,31,31,0,0x10000,128);
    DMA_FILL_32(3,0,(void *)(VRAM_BASE + 0x4000),0x2000);
    DMA_FILL_16(3,0,(void *)(VRAM_BASE + 0xE000),0x800);
    gEwramData->gameState.stateFlags|=32;
    gEwramData->gameState.stateFlags|=0x10000;
    Music_SetCurrent(25);
}

/* Finish the event script, fade the scene and resume normal game state. */
/**
 * @brief Update of event scene variant 23 (sEventSceneUpdateFuncs[23]); returns the
 * player input mask.
 */
s32 EventSceneReturnHome_Update(struct EwramData_EntityData *entity)
{
    s32 zero=0;
    gEwramData->gameState.stateFlags|=1;
    Game_SetMenusEnabled(0);
    switch(entity->state)
    {
    case 0:
        if(entity->subState==0)
        {
            DMA_FILL_32(3,zero,(void *)(VRAM_BASE + 0x4000),0x2000);
            DMA_FILL_16(3,zero,(void *)(VRAM_BASE + 0xE000),0x800);
            gEwramData->gameState.stateFlags|=32;
            gEwramData->gameState.stateFlags|=0x10000;
            entity->timer=128;entity->subState++;
        }
        if(--entity->timer==255)
        {TextWindow_Begin(35);entity->state++;entity->stage=zero;entity->subState=zero;}
        break;
    case 1:
        if(TextWindow_Update())
        {
            entity->state++;entity->stage=zero;entity->subState=zero;
            BgPalette_StartColorFade(0,0,0,0x10000,zero,255);
            ObjPalette_StartColorFade(0,0,0,0x10000,zero,255);
            gUnk_03002CB0.dispCnt&=0xFEFF;
            GameFlag_Set(44);Sound_Play(0x1040);
        }
        break;
    case 2:
        if(!BgPalette_GetPendingFades()){entity->state=128;entity->stage=zero;entity->subState=zero;}
        break;
    case 128:
        gEwramData->gameState.systemFlags|=1;
        SaveData_SaveSystemFlags();SaveData_ResaveSlotAfterEnding(gEwramData->gameState.currentSaveSlot,0);
        TextWindow_Close();gEwramData->gameState.subMode=10;
        break;
    }
    Player_StartScriptedAnimation(0,0,0,0);
    gEwramData->gameState.stateFlags|=32;
    return zero;
}

/* Place the event actor and player relative to the scrolling background. */
/**
 * @brief Set up event scene 24: place the object at room (64, 192) facing right
 * with the player 24 px to its right, set stateFlags bits 0 and 0x20, clear
 * the HUD graphics and map, play song 24.
 */
void EventSceneEndingWakeUp_Setup(struct EwramData_EntityData *entity)
{
    if(!Entity_SetupFromObjectTable(entity,0)){EntityDelete(entity);return;}
    Anim_StartIndexed(entity,4,0,1);Anim_Update(entity);
    {
    struct EwramData *ram=gEwramData;
    struct BgLayerState *background=&ram->bgInfo[1];
    entity->posX.part16.integer=64-background->xPos.part16.integer;
    entity->posY.part16.integer=192-background->yPos.part16.integer;
    entity->flipX=0;
    ram->playerState.player->posX.part16.integer=entity->posX.part16.integer+24;
    ram->playerState.player->posY.part16.integer=entity->posY.part16.integer;
    ram->playerState.player->flipX=0;
    }
    gEwramData->gameState.stateFlags|=1;
    gEwramData->gameState.stateFlags|=32;
    DMA_FILL_32(3,0,(void *)(VRAM_BASE + 0x4000),0x2000);
    DMA_FILL_16(3,0,(void *)(VRAM_BASE + 0xE000),0x800);
    Music_SetCurrent(24);
}

/* Event actor: dialogue selection, fade and optional walk to the player. */
/**
 * @brief Update of event scene variant 24 (sEventSceneUpdateFuncs[24]); returns the
 * player input mask.
 */
s32 EventSceneEndingWakeUp_Update(struct EwramData_EntityData *entity)
{
    s32 zero=0;
    s32 stage=entity->state;
    switch(stage)
    {
    case 0:
        if(entity->subState==0)
        {
            if(entity->stage==0)
            {
                BgPalette_StartColorFade(31,31,31,0,0x10000,128);
                ObjPalette_StartColorFade(31,31,31,0,0x10000,128);
                entity->stage++;
            }
            else if(BgPalette_GetPendingFades()){entity->subState++;entity->stage=zero;}
        }
        else
        {
            EventSceneEndingWakeUp_Setup(entity);entity->state++;entity->stage=zero;entity->subState=zero;entity->timer=255;
        }
        Player_StartScriptedAnimation(81,0,0,0);
        break;
    case 1:
        if(--entity->timer==255)
        {
            s32 selected=0;
            if(SoulInventory_GetCompletionTenths()>799)selected=1;
            if(selected){TextWindow_Begin(38);entity->ext.event24.answer=stage;}
            else {TextWindow_Begin(39);entity->ext.event24.answer=zero;}
            entity->state++;entity->stage=0;entity->subState=0;
        }
        Player_StartScriptedAnimation(81,0,0,0);
        break;
    case 2:
        if(TextWindow_Update())
        {
            entity->state++;entity->stage=zero;entity->subState=zero;
            BgPalette_StartColorFade(0,0,0,0x10000,zero,255);
            ObjPalette_StartColorFade(0,0,0,0x10000,zero,255);
            gUnk_03002CB0.dispCnt&=0xFEFF;
            SaveData_RecordFullClear();Sound_Play(0x1040);
        }
        else if(entity->subState==0)
        {
            Player_StartScriptedAnimation(81,0,0,0);
            break;
        }
        else if(entity->subState==1)
        {
            gEwramData->playerState.player->flipX=1;
            if(entity->animScriptIndex!=0)Anim_StartIndexed(entity,0,0,0);
        }
        else gEwramData->playerState.player->flipX=0;
        break;
    case 3:
        if(!BgPalette_GetPendingFades())
        {
            TextWindow_Close();GameFlag_Set(45);gEwramData->gameState.subMode=10;
        }
        else if(entity->ext.event24.answer)
        {
            if((s16)gEwramData->playerState.player->posX.part16.integer-12>(s16)entity->posX.part16.integer)
            {
                if(entity->animScriptIndex!=6)Anim_StartIndexed(entity,6,0,1);
                entity->posX.whole+=0x2000;
            }
            else if(entity->animScriptIndex!=1)Anim_StartIndexed(entity,1,0,0);
        }
        break;
    }
    if(entity->state)Anim_Update(entity);
    gEwramData->gameState.stateFlags|=32;
    return zero;
}
