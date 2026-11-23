/*
 * Event scene 03: mosaic, camera, init and update (kept apart from scene03_parts.c: the update calls Entity_IsInRect with s16 / u16 arguments, the parts with the header prototype).
 */
#include "code_08009A0.h"
#include "event_scene03.h"
#include "structs/ewram.h"
#include "structs/main.h"
#include "code_0800CB00.h"
#include "gba.h"
#include "code_08013960.h"
#include "code_08032444.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"

#define SET_MOSAIC(t)                                                        \
    {                                                                        \
        v = (u32)((t) << 24) >> 26;                                              \
        gDisplayRegisters.mosaic = v | (v << 4) | (v << 8) | (v << 12);      \
    }

/**
 * @brief Mosaic step of event scene 3: fade the mosaic in and out over its
 * sub-states; return 0 when finished.
 */
s32 EventScene03_UpdateMosaic(struct EwramData_EntityData *entity)
{
    s32 ret = 1;
    s32 t;
    u32 v;

    switch (entity->subState)
    {
    case 0:
        gDisplayRegisters.mosaic = 0xFFFF;
        entity->timer = 0x40;
        entity->subState++;
        break;
    case 1:
        t = entity->timer;
        SET_MOSAIC(t);
        if ((u32)(t << 24) >> 24 == 0x20)
            entity->subState++;
        else
            entity->timer = t - 1;
        break;
    case 2:
        t = entity->timer;
        SET_MOSAIC(t);
        if ((u32)(t << 24) >> 24 == 0x30)
            entity->subState++;
        else
            entity->timer = t + 1;
        break;
    case 3:
        t = entity->timer;
        SET_MOSAIC(t);
        if ((u32)(t << 24) >> 24 == 0x10)
            entity->subState++;
        else
            entity->timer = t - 1;
        break;
    case 4:
        t = entity->timer;
        SET_MOSAIC(t);
        if ((u32)(t << 24) >> 24 == 0x20)
            entity->subState++;
        else
            entity->timer = t + 2;
        break;
    case 5:
        t = entity->timer;
        SET_MOSAIC(t);
        if ((u32)(t << 24) >> 24 == 0)
            entity->subState++;
        else
            entity->timer = t - 1;
        if (--entity->timer == 0xFF)
        {
            gDisplayRegisters.mosaic = 0;
            gDisplayRegisters.bldCnt = 0;
            gEwramData->playerState.player->mosaic = 0;
            entity->ext.event03.actor0->mosaic = 0;
            ret = 0;
        }
        break;
    }
    return ret;
}

/**
 * @brief Scroll the camera for camera step EVENT_SCENE03_CAMERA_STEP of event scene 3 until its
 * target x is reached; return 1 then.
 */
s32 EventScene03_ScrollCamera(struct EwramData_EntityData *entity)
{
    s32 done = 0;
    struct BgLayerState *bg;

    gEwramData->gameState.cameraScrollMinPriority = 1;
    switch (EVENT_SCENE03_CAMERA_STEP(entity))
    {
    case 0:
        Camera_ScrollWithPriority(1, 0x80000, 0);
        bg = &gEwramData->bgInfo[1];
        if ((s16)bg->xPos.part16.integer > 0x1F)
            done = 1;
        break;
    case 1:
        Camera_ScrollWithPriority(1, 0x10000, 0);
        bg = &gEwramData->bgInfo[1];
        if ((s16)bg->xPos.part16.integer > 0x5F)
            done = 1;
        break;
    case 2:
        Camera_ScrollWithPriority(1, 0x38000, 0);
        bg = &gEwramData->bgInfo[1];
        if ((s16)bg->xPos.part16.integer > 0xDD)
            done = 1;
        break;
    case 3:
        Camera_ScrollWithPriority(1, -0x40000, 0);
        bg = &gEwramData->bgInfo[1];
        if ((s16)bg->xPos.part16.integer <= 0x60)
            done = 1;
        break;
    case 4:
        Camera_ScrollWithPriority(1, -0x10000, 0);
        if ((s16)gEwramData->playerState.player->posX.part16.integer == 0x78)
            done = 1;
        break;
    case 5:
        Camera_ScrollWithPriority(1, -0x80000, 0);
        bg = &gEwramData->bgInfo[1];
        if ((s16)bg->xPos.part16.integer <= 0x20)
            done = 1;
        break;
    }
    return done;
}

/* EventScene03_Init. */
extern s32 GameFlag_GetMask(s32 flag);
extern struct EwramData_EntityData *Entity_CreateFromObjectTable(s32 kind);
extern void EventScene03_Start(struct EwramData_EntityData *entity);

/**
 * @brief Init of event scene variant 3 (Object 0x20 spawnParam0 = 3,
 * sEventSceneCreateFuncs[3]). Create step of a scene object: unless flag 0x20 is
 * set (GameFlag_GetMask), create its three helper entities
 * (Entity_CreateFromObjectTable kinds 0, 1 and 8) into
 * unk_4F8/unk_4FC/unk_500; if all exist start it (EventScene03_Start,
 * gameState.stateFlags bit 0, byte 0x51C = 5 when the camera x is past 0x1F),
 * else delete the ones created and itself.
 */
void EventScene03_Init(struct EwramData_EntityData *entity)
{
    s32 done = GameFlag_GetMask(0x20);
    struct BgLayerState *bg;

    if (done == 0)
    {
        entity->ext.event03.actor0 = Entity_CreateFromObjectTable(0);
        entity->ext.event03.actor1 = Entity_CreateFromObjectTable(1);
        entity->ext.event03.actor8 = Entity_CreateFromObjectTable(8);
        if (entity->ext.event03.actor0 != NULL && entity->ext.event03.actor1 != NULL
            && entity->ext.event03.actor8 != NULL)
            goto created;
        if (entity->ext.event03.actor0 != NULL)
            EntityDelete(entity->ext.event03.actor0);
        if (entity->ext.event03.actor1 != NULL)
            EntityDelete(entity->ext.event03.actor1);
        if (entity->ext.event03.actor8 != NULL)
            EntityDelete(entity->ext.event03.actor8);
    }
    entity->deleteRequested = 1;
    return;

created:
    EventScene03_Start(entity);
    gEwramData->gameState.stateFlags |= 1;
    bg = &gEwramData->bgInfo[1];
    if ((s16)bg->xPos.part16.integer > 0x1F)
        EVENT_SCENE03_CAMERA_STEP(entity) = 5;
    else
        EVENT_SCENE03_CAMERA_STEP(entity) = done;
}

/* EventScene03_Update: event script step (room event state machine); matched. */
extern void TextWindow_Begin(s32);
extern s32 TextWindow_Update(void);
extern void Interaction_End(struct EwramData_EntityData *, s32);
extern void GameFlag_Set(s32 flag);
extern void Music_StartOverride(s32 song);
extern s32 Entity_IsInRect(struct EwramData_EntityData *, s16, s16, u16, u16);
extern void Player_StartScriptedAnimation(s32, s32, s32, s32);
extern s32 EventScene03_RunSequence(struct EwramData_EntityData *);
extern s32 EventScene03_EndParts(struct EwramData_EntityData *);

/**
 * @brief Update of event scene variant 3 (sEventSceneUpdateFuncs[3]); returns the player
 * input mask.
 */
s32 EventScene03_Update(struct EwramData_EntityData *entity)
{
    s32 ret;
    s32 found;
    s32 convo;
    s32 z;
    s32 v;
    s32 sz;
    struct EwramData *ew;
    struct EwramData_EntityData *other;
    struct EwramData_EntityData *player;

    ret = 0;
    z = gEwramData->inputData.newInput;
    found = 0;
    switch (entity->state)
    {
    case 0:
        if ((z = entity->stage) == 0)
        {
            PlaySong(0x1000);
            gEwramData->gameState.stateFlags |= 0x20;
            DMA_FILL_32(3, z, 0x06004000, 0x2000);
            entity->stage++;
        }
        if (!EventScene03_ScrollCamera(entity))
            break;
        Music_SetCurrent(0x1C);
        entity->state++;
        entity->stage = 0;
        entity->subState = 0;
        break;
    case 1:
        if (entity->stage == 0)
        {
            if (!EventScene03_UpdateMosaic(entity))
            {
                gUnk_03002CB0.dispCnt &= 0xDFFF;
                entity->stage++;
            }
        }
        else
        {
            TextWindow_Begin(0xC);
            entity->state++;
            entity->stage = 0;
            entity->subState = 0;
        }
        Player_StartScriptedAnimation(0x51, 1, 0, 0);
        break;
    case 2:
        if (TextWindow_Update())
        {
            struct PlayerStatus *stats;
            stats = &gEwramData->playerStatus;
            entity->state = 0x80;
            SoulInventory_AddAmountToSoulTotal(0, 0, 1);
            do { /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
                if (stats->equippedRedSoul == 0)
                    stats->equippedRedSoul = 1;
            } while (0);
            gEwramData->gameState.cameraScrollMinPriority = 0;
            gEwramData->playerState.player->palette = 0;
            EventScene03_EndParts(entity);
            Interaction_End(entity, 0);
            GameFlag_Set(0x20);
            {
                u16 f = entity->ext.event03.progress;
                s32 k = 4;
                z = 0;
                k |= f;
                entity->ext.event03.progress = k;
            }
            Music_StartOverride(0x26);
            goto set10;
        }
        ret = EventScene03_RunSequence(entity);
        break;
    case 10:
        if (Entity_IsInRect(gEwramData->playerState.player, entity->ext.event03.actor1->posX.part16.integer - 0x20, entity->ext.event03.actor1->posY.part16.integer - 0x20, 0x40, sz = 0x40))
        {
            found = 1;
            convo = 0xE;
        }
        else if (Entity_IsInRect(gEwramData->playerState.player, entity->ext.event03.actor0->posX.part16.integer - 0x20, entity->ext.event03.actor0->posY.part16.integer - 0x20, 0x40, sz))
        {
            found = 1;
            convo = 0xD;
        }
        if (found == 0)
            break;
        z &= 0x42;
        if (z == 0)
            break;
        ew = gEwramData;
        if ((z = ew->gameState.timeStopTimer) != 0)
            break;
        ew->playerState.player->velX.whole = z;
        ew->playerState.player->accelX.whole = z;
        player = ew->playerState.player;
        if (player->velY.whole < 0)
            player->velY.whole = z;
        gEwramData->gameState.stateFlags |= 1;
        TextWindow_Begin(convo);
        other = entity->ext.event03.actor1;
        other->flipX = other->posX.whole < gEwramData->playerState.player->posX.whole;
        v = entity->state + 1;
        goto set;
    case 11:
        if (TextWindow_Update())
        {
            z = 0;
            entity->state = 0x80;
            Interaction_End(entity, 0);
        set10:
            v = 0xA;
        set:
            entity->state = v;
            entity->stage = z;
            entity->subState = z;
        }
        break;
    }
    return ret;
}
