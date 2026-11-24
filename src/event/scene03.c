/*
 * Event scene 03: mosaic, camera, init, update, the scripted sequence and its parts, debris and orbs.
 */
#include "core.h"
#include "event_scene03.h"
#include "structs/ewram.h"
#include "structs/main.h"
#include "bg_effects.h"
#include "gba.h"
#include "music.h"
#include "soul_inventory.h"
#include "sound.h"
#include "graphics.h"
#include "entity_util.h"
#include "gameplay.h"
#include "effects.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "map_flags.h"
#include "math_anim.h"
#include "obj_gfx.h"
#include "shop_update.h"
#include "text_window.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"
#include "data/data_0E5364.h"
#include "math_inline.h"
#include "player_core.h"
#include "hitbox.h"
#include "player_soma.h"
#include "red_souls_b.h"

#define SET_MOSAIC(t)                                                        \
    {                                                                        \
        v = (u32)((t) << 24) >> 26;                                              \
        gDisplayRegisters.mosaic = v | (v << 4) | (v << 8) | (v << 12);      \
    }

extern u32 Entity_AngleToPoint(struct EwramData_EntityData *, s32, s32);

s32 EventScene03_Start(struct EwramData_EntityData *entity);
s32 EventScene03_RunSequence(struct EwramData_EntityData *);
s32 EventScene03_EndParts(struct EwramData_EntityData *);
void EventScene03MainPart_Update(struct EwramData_EntityData *entity);
struct EwramData_EntityData *EventScene03_SpawnRisingPart(struct EwramData_EntityData *parent, s32 x, s32 y, s32 flags);
struct EwramData_EntityData *EventScene03_SpawnFlyingPart(struct EwramData_EntityData *parent, s32 x, s32 y);
struct EwramData_EntityData *EventScene03_SpawnSoulOrb(s32 x, s32 y, s32 variant, s32 index, s32 mode);
void EventScene03_SpawnPartDebris(struct EwramData_EntityData *owner);
void EventScene03UnusedPart_Update(struct EwramData_EntityData *);
void EventScene03AttachedEffect_Update(struct EwramData_EntityData *);
struct EwramData_EntityData *EventScene03_SpawnTargetOrb(s32 x, s32 y, s32 variant, s32 index, struct EwramData_EntityData *target);
void EventScene03AttachedHelper_Update(struct EwramData_EntityData *);
void EventScene03_SpawnUnusedPartDebris(struct EwramData_EntityData *);
void EventScene03FlyingPart_Update(struct EwramData_EntityData *entity);
void EventScene03RisingPart_Update(struct EwramData_EntityData *entity);
void EventScene03Debris_Update(struct EwramData_EntityData *);
void EventScene03SoulOrb_Update(struct EwramData_EntityData *);
void EventScene03SoulOrb_OnPlayerContact(struct EwramData_EntityData *);
void EventScene03TargetOrb_Update(struct EwramData_EntityData *);
void EventScene03TargetOrb_OnContact(struct EwramData_EntityData *);

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
            DMA_FILL_32(3, z, VRAM_BASE + 0x4000, 0x2000);
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
        if (Entity_IsInRect(gEwramData->playerState.player, (s16)(entity->ext.event03.actor1->posX.part16.integer - 0x20), (s16)(entity->ext.event03.actor1->posY.part16.integer - 0x20), 0x40, (u16)(sz = 0x40)))
        {
            found = 1;
            convo = 0xE;
        }
        else if (Entity_IsInRect(gEwramData->playerState.player, (s16)(entity->ext.event03.actor0->posX.part16.integer - 0x20), (s16)(entity->ext.event03.actor0->posY.part16.integer - 0x20), 0x40, (u16)sz))
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

/* Scene start: place the player and the three helper entities at fixed room
 * positions (relative to the BG1 camera) and give each helper its animation. */

#define PLAYER (gEwramData->playerState.player)

/* The camera X reads use block-local u16 copies; the 0x16F position is
 * derived from the 0x119 literal register by CSE. The result of the last
 * animation call stays live in r0, so it is returned. */
s32 EventScene03_Start(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg1, *bg2, *bg3;
    s32 result;

    Player_StartScriptedAnimation(0x51, 0, 0, 0);
    PLAYER->posX.part16.integer = 0xA8 - (bg1 = &gEwramData->bgInfo[1])->xPos.part16.integer;
    PLAYER->posY.part16.integer = 0x2C0 - bg1->yPos.part16.integer;
    PLAYER->velX.whole = 0;
    PLAYER->mosaic = 1;

    entity->ext.event03.actor0->posX.part16.integer = PLAYER->posX.part16.integer - 0x18;
    entity->ext.event03.actor0->posY.part16.integer = PLAYER->posY.part16.integer;
    if (entity->ext.event03.actor0->animScriptIndex != 4)
        Anim_StartIndexed(entity->ext.event03.actor0, 4, 0, 0);
    entity->ext.event03.actor0->mosaic = 1;
    ENTITY_PARENT_INDEX(entity->ext.event03.actor0) = ENTITY_INDEX(entity);

    {
        struct EwramData_EntityData *child = entity->ext.event03.actor1;
        u16 camX = (bg2 = &gEwramData->bgInfo[1])->xPos.part16.integer;

        child->posX.part16.integer = 0x119 - camX;
    }
    entity->ext.event03.actor1->posY.part16.integer = 0x2C0 - bg2->yPos.part16.integer;
    Anim_StartIndexed(entity->ext.event03.actor1, 1, 0, 1);
    entity->ext.event03.actor1->velX.whole = -0x6000;
    ENTITY_PARENT_INDEX(entity->ext.event03.actor1) = ENTITY_INDEX(entity);

    {
        struct EwramData_EntityData *child = entity->ext.event03.actor8;
        u16 camX = (bg3 = &gEwramData->bgInfo[1])->xPos.part16.integer;

        child->posX.part16.integer = 0x16F - camX;
    }
    entity->ext.event03.actor8->posY.part16.integer = 0x268 - bg3->yPos.part16.integer;
    result = Anim_StartIndexed(entity->ext.event03.actor8, 4, 0, 1);
    ENTITY_PARENT_INDEX(entity->ext.event03.actor8) = ENTITY_INDEX(entity);
    entity->ext.event03.actor8->updateFunc = EventScene03MainPart_Update;
    return result;
}

/* EventScene03_SpawnPartsA, EventScene03_SpawnPartsB. */

/**
 * @brief 5D2C8 | Spawn the five effects of sEventScene03SpotsA (EventScene03_SpawnRisingPart at
 * room (x + 0x20, 0x29F), attack = index) and one EventScene03_SpawnFlyingPart effect at
 * room (0x14F, 0x288), all relative to the camera bgInfo[1].
 */
void EventScene03_SpawnPartsA(struct EwramData_EntityData *entity)
{
    struct Unk_081187AC spots[5];
    s32 i;

    memcpy(spots, sEventScene03SpotsA, sizeof(spots));
    for (i = 0; i <= 4; i++)
    {
        struct BgLayerState *bg = &gEwramData->bgInfo[1];

        EventScene03_SpawnRisingPart(entity, spots[i].x - (s16)bg->xPos.part16.integer + 0x20,
                     0x29F - (s16)bg->yPos.part16.integer, spots[i].flags)->attack = i;
    }
    {
        struct BgLayerState *bg = &gEwramData->bgInfo[1];

        EventScene03_SpawnFlyingPart(entity, 0x14F - (s16)bg->xPos.part16.integer, 0x288 - (s16)bg->yPos.part16.integer);
    }
}

/**
 * @brief 5D35C | As EventScene03_SpawnPartsA with the two spots of sEventScene03SpotsB
 * relative to the player's x (attack = 5 + index) and two EventScene03_SpawnFlyingPart
 * effects at room (0x7C, 0x290) and (0xC4, 0x280).
 */
void EventScene03_SpawnPartsB(struct EwramData_EntityData *entity)
{
    struct Unk_081187AC spots[2];
    s32 i;

    memcpy(spots, sEventScene03SpotsB, sizeof(spots));
    for (i = 0; i <= 1; i++)
    {
        struct BgLayerState *bg;
        s32 x = (s16)gEwramData->playerState.player->posX.part16.integer + spots[i].x;

        bg = &gEwramData->bgInfo[1];
        EventScene03_SpawnRisingPart(entity, x, 0x29F - (s16)bg->yPos.part16.integer, spots[i].flags)->attack = i + 5;
    }
    {
        struct BgLayerState *bg = &gEwramData->bgInfo[1];

        EventScene03_SpawnFlyingPart(entity, 0x7C - (s16)bg->xPos.part16.integer, 0x290 - (s16)bg->yPos.part16.integer);
    }
    {
        struct BgLayerState *bg = &gEwramData->bgInfo[1];

        EventScene03_SpawnFlyingPart(entity, 0xC4 - (s16)bg->xPos.part16.integer, 0x280 - (s16)bg->yPos.part16.integer);
    }
}

/* EventScene03_RunSequence, decompiled from the original assembly: event script with
 * nine steps (subState) and sub-steps (stage); returns flags 2 / 0x80.
 * Each step advance is written out (no shared label) so only the final
 * stage++ is cross-jumped; the decrement in step 5/3 goes through an s32
 * temporary; step 8 has an empty case 2. */

#define STEP (*(u8 *)&entity->enemyStatus)

/**
 * @brief Main sequence of event scene 3 (EventScene03_Update): camera steps,
 * part spawns, the soul orbs and the finishing debris; returns the
 * player input mask.
 */
s32 EventScene03_RunSequence(struct EwramData_EntityData *entity)
{
    s32 result = 0;
    struct GameState *save = &gEwramData->gameState;
    u8 state = entity->subState;

    switch (state)
    {
    case 0:
        switch (entity->stage)
        {
        case 0:
            save->textWindowPause = 1;
            entity->timer = 0x40;
            entity->stage++;
            break;
        case 1:
            if ((u8)--entity->timer == 0xFF)
                entity->stage++;
            break;
        case 2:
            save->textWindowPause = 0;
            break;
        }
        Player_StartScriptedAnimation(0x51, 0, 0, 0);
        break;
    case 1:
        switch (entity->stage)
        {
        case 0:
        {
            struct EwramData_EntityData *p = entity->ext.event03.actor0;
            if (p->animScriptIndex != 1)
                Anim_StartIndexed(p, 1, 0, 0);
            gEwramData->gameState.cameraScrollMinPriority = 1;
            save->textWindowPause = 1;
            entity->stage++;
            STEP = 1;
        }
        case 1:
            if (EventScene03_ScrollCamera(entity))
            {
                entity->stage++;
                entity->timer = 0x10;
            }
            break;
        case 2:
            if ((u8)--entity->timer == 0xFF)
            {
                entity->stage++;
                save->textWindowPause = 0;
            }
            break;
        }
        break;
    case 2:
        if (entity->stage == 0)
        {
            entity->ext.event03.actor0->flipX = 0;
            entity->ext.event03.actor1->flipX = 1;
            Music_SetCurrent(0x10);
            entity->stage++;
            break;
        }
        break;
    case 3:
        switch (entity->stage)
        {
        case 0:
            EventScene03_SpawnPartsA(entity);
            entity->ext.event03.progress |= 8;
            save->textWindowPause = 1;
            entity->stage++;
            STEP = 2;
            break;
        case 1:
            if (EventScene03_ScrollCamera(entity))
            {
                entity->timer = 0x20;
                entity->stage++;
                break;
            }
            break;
        case 2:
            if ((u8)--entity->timer == 0xFF)
                entity->stage++;
            break;
        case 3:
        {
            struct EwramData_EntityData *p;
            struct EwramData_EntityData *q;
            EventScene03_SpawnPartsB(entity);
            p = entity->ext.event03.actor0;
            if (p->animScriptIndex != 5)
                Anim_StartIndexed(p, 5, 0, 0);
            PlaySong(0x1B4);
            q = entity->ext.event03.actor1;
            q->flipX = 1 - q->flipX;
            entity->timer = 0x20;
            entity->stage++;
            break;
        }
        case 4:
            if ((u8)--entity->timer == 0xFF)
            {
                entity->subState++;
                entity->stage = 0;
            }
            break;
        }
        break;
    case 4:
        switch (entity->stage)
        {
        case 0:
            save->textWindowPause = 1;
            entity->stage++;
            STEP = 3;
            break;
        case 1:
            if (EventScene03_ScrollCamera(entity))
            {
                entity->timer = 0;
                entity->stage++;
                break;
            }
            break;
        case 2:
            if ((u8)--entity->timer == 0xFF)
            {
                entity->ext.event03.progress |= 0x10;
                PlaySong(0x1DF);
                Anim_StartIndexed(entity->ext.event03.actor1, 2, 0, 0);
                entity->stage++;
                break;
            }
            break;
        case 3:
            if (entity->ext.event03.actor1->animDone)
            {
                entity->ext.event03.progress |= 2;
                entity->stage++;
                entity->timer = 0x10;
            }
            break;
        case 4:
            if ((u8)--entity->timer == 0xFF)
            {
                struct EwramData_EntityData *q;
                entity->stage++;
                PlaySong(0x1E0);
                q = entity->ext.event03.actor1;
                if (q->animScriptIndex != 3)
                    Anim_StartIndexed(q, 3, 0, 0);
            }
            break;
        case 5:
            if (entity->ext.event03.actor1->animDone)
            {
                entity->timer = 0x58;
                entity->stage++;
                break;
            }
            break;
        case 6:
        {
            if (entity->timer == 0)
                save->textWindowPause = 0;
            else
                entity->timer--;
            break;
        }
        }
        break;
    case 5:
        switch (entity->stage)
        {
        case 0:
            save->textWindowPause = 1;
            entity->timer = 0x10;
            entity->stage++;
            break;
        case 1:
            if ((u8)--entity->timer == 0xFF)
            {
                entity->timer = 0xD2;
                result |= 2;
                entity->stage++;
                break;
            }
            break;
        case 2:
            if (entity->timer == 0xD0)
            {
                struct EwramData_EntityData *o;
                EventScene03_SpawnSoulOrb((s16)entity->ext.event03.actor8->posX.part16.integer, (s16)entity->ext.event03.actor8->posY.part16.integer, 0, 0, 0);
                entity->ext.event03.actor8->hidden = 1;
                o = entity->ext.event03.actor8;
                Effect_SpawnAt(o->posX.whole, o->posY.whole, o->spriteLayer - 1);
                EventScene03_SpawnPartDebris(entity->ext.event03.actor8);
                PlaySong(0x6B);
                gUnk_03002CB0.dispCnt &= 0xFEFF;
            }
            if (entity->timer == 0x8F)
            {
                Hud_EndEventTextLayer();
                gEwramData->gameState.stateFlags &= ~0x200;
                Hud_ShowSoulAcquiredMessage(0, 0, 1);
                gUnk_03002CB0.dispCnt |= 0x100;
            }
            if (entity->timer <= 0x8F)
            {
                if (entity->timer == 0x8F)
                {
                    PlaySong(0x197);
                    Sound_Play(0x1040);
                    entity->stage++;
                }
                if (entity->timer & 1)
                    gEwramData->playerState.player->palette = 4;
                else
                    gEwramData->playerState.player->palette = 0;
                Player_StartScriptedAnimation(0x52, 0, 0, 0);
            }
            entity->timer--;
            break;
        case 3:
        {
            struct EwramData **ew2 = &gEwramData;
            if ((*ew2)->inputData.newInput & 1)
            {
                PlaySong(0x132);
                gUnk_03002CB0.dispCnt &= 0xFEFF;
                entity->timer = 0;
                entity->stage++;
                break;
            }
            Hud_DrawWindowPrompt();
            if (({ s32 t = entity->timer - 1; entity->timer = t; t; }) & 1)
                (*ew2)->playerState.player->palette = 4;
            else
                gEwramData->playerState.player->palette = 0;
            Player_StartScriptedAnimation(0x52, 0, 0, 0);
            break;
        }
        case 4:
            Hud_BeginEventTextLayer();
            entity->stage++;
        case 5:
            Player_StartScriptedAnimation(0x52, 0, 0, 0);
            if ((u8)--entity->timer == 0xFF)
            {
                gUnk_03002CB0.dispCnt |= 0x100;
                gEwramData->playerState.player->palette = 0;
                entity->timer = 0;
                entity->stage = 6;
                save->textWindowPause = 0;
            }
            break;
        case 6:
            result |= 0x80;
            break;
        }
        break;
    case 6:
        switch (entity->stage)
        {
        case 0:
            Music_SetCurrent(0x17);
            save->textWindowPause = 1;
            entity->timer = 0x20;
            entity->stage++;
            break;
        case 1:
            if ((u8)--entity->timer == 0xFF)
            {
                save->textWindowPause = 0;
                entity->stage++;
                break;
            }
            break;
        }
        break;
    case 7:
        if (entity->stage == 0)
        {
            Sound_Play(0x1041);
            entity->stage++;
            break;
        }
        break;
    case 8:
        switch (entity->stage)
        {
        case 0:
            save->textWindowPause = 1;
            STEP = 4;
            entity->stage++;
            break;
        case 1:
            if (EventScene03_ScrollCamera(entity))
            {
                save->textWindowPause = 0;
                save->textWindowState = 5;
                entity->stage++;
                break;
            }
            break;
        case 2:
            break;
        }
        break;
    }
    return result;
}

/**
 * @brief 5DAA0 | Event part like EventScene03RisingPart_Update: bob up and down after the
 * owner sets flag 8, then (flag 0x10) wait 0x60 frames and walk left until
 * room x 0xCA; owner flag 4 makes it vanish (state 4 bursts with sound 0x6B
 * and debris via EventScene03_SpawnPartDebris).
 */
void EventScene03MainPart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    s32 wave;

    if ((u16)owner->ext.event03.progress & 4)
    {
        entity->state = 5;
        entity->stage = 0;
        entity->subState = 0;
    }
    switch (entity->state)
    {
    case 0:
        if ((u16)owner->ext.event03.progress & 8)
        {
            entity->state = 1;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    case 1:
        wave = Sin(entity->ext.event03Part.wave + 0x4000) / 4;
        entity->ext.event03Part.wave += 0x200;
        entity->posY.whole += wave;
        entity->velY.whole = wave + entity->accelY.whole;
        if ((u16)owner->ext.event03.progress & 0x10)
        {
            entity->state = 2;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    case 2:
        switch (entity->subState)
        {
        case 0:
            switch (entity->stage)
            {
            case 0:
                if (entity->animScriptIndex != 5)
                    Anim_StartIndexed(entity, 5, 0, 1);
                entity->timer = 0x60;
                entity->stage++;
                break;
            case 1:
                if (--entity->timer == 0xFF)
                {
                    entity->subState++;
                    entity->stage = 0;
                }
                break;
            }
            break;
        case 1:
            if ((s16)entity->posX.part16.integer > 0xCA - (s16)(gEwramData->bgInfo + 1)->xPos.part16.integer)
            {
                if (!(Entity_TryWalk(entity, 0x30000, 0x200000, 0x14000, 1) & 4))
                    entity->posY.whole += entity->velY.whole;
                entity->posX.whole += entity->velX.whole;
            }
            else
            {
                if (entity->animScriptIndex != 2)
                    Anim_StartIndexed(entity, 2, 0, 1);
                entity->state = 3;
                entity->stage = 0;
                entity->subState = 0;
            }
            break;
        }
        break;
    case 4:
        if (entity->subState == 0)
        {
            if (Entity_IsInRect(entity, -0x80, -0x40, 0x1F0, 0x120))
            {
                PlaySong(0x6B);
                Effect_SpawnAt(entity->posX.whole, entity->posY.whole, entity->spriteLayer - 1);
                EventScene03_SpawnPartDebris(entity);
            }
            entity->state = 5;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    case 5:
        entity->deleteRequested = 1;
        break;
    }
    Anim_Update(entity);
}

/* Spawn an entity and an attached animated part in event slot group 9. */
struct EwramData_EntityData *EventScene03_SpawnUnusedPart(struct EwramData_EntityData *owner, s32 x, s32 y, s32 flags)
{
    struct EwramData_EntityData *entity = Entity_CreateFromObjectTable(9);
    struct EwramData_EntityData *part;
    s32 i, facing;
    entity->updateFunc = EventScene03UnusedPart_Update;
    entity->posX.part16.integer = x;
    entity->posY.part16.integer = y;
    ENTITY_PARENT_INDEX(entity) = ENTITY_INDEX(owner);
    entity->ext.event03Part.flags = flags;
    entity->enemyId = 0xFF;
    Entity_SnapToGround(entity);
    Anim_StartIndexed(entity, 0, 0, 1);
    if (entity->ext.event03Part.flags & 4)
    {
        for (i = 16; i != 0; i--)
            entity->posY.whole += 0x8000;
        entity->spriteLayer = 0x17;
    }
    facing = 0;
    if (entity->posX.whole < gEwramData->playerState.player->posX.whole)
        facing = 1;
    entity->flipX = facing;
    part = Entity_CreateFromObjectTable(9);
    part->updateFunc = EventScene03AttachedEffect_Update;
    ENTITY_PARENT_INDEX(part) = ENTITY_INDEX(entity);
    part->posX.part16.integer = entity->posX.part16.integer;
    part->posY.part16.integer = entity->posY.part16.integer;
    Anim_StartIndexed(part, 16, 0, 1);
    part->spriteLayer = entity->spriteLayer;
    entity->ext.event03Part.linkIndex = ENTITY_INDEX(part);
    part->enemyId = entity->enemyId;
    return entity;
}

/* Event part (EventScene03UnusedPart_Update, cf. EventScene03RisingPart_Update): wait for its owner, rise,
 * shake (optionally spawning a EventScene03AttachedHelper_Update helper), blink, then break apart. */
void EventScene03UnusedPart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    struct EwramData_EntityData *other = &gEwramData->entityData[entity->ext.event03Part.linkIndex];
    struct EwramData_EntityData *child;
    if ((u16)owner->ext.event03.progress & 4)
    {
        entity->state = 4;
        entity->stage = 0;
        entity->subState = 0;
    }
    switch (entity->state)
    {
    case 0:
        if ((u16)owner->ext.event03.progress & 8)
        {
            if (RandomNumberGenerator() % 7 == entity->attack)
            {
                if ((u8)(entity->ext.event03Part.flags & 2))
                {
                    entity->state = 2;
                    entity->stage = 0;
                    entity->subState = 0;
                }
                else
                {
                    entity->state = 1;
                    entity->stage = 0;
                    entity->subState = 0;
                }
            }
        }
        break;
    case 1:
        switch (entity->subState)
        {
        case 0:
            if (RandomNumberGenerator() % 7 == entity->attack)
            {
                entity->timer = 16;
                entity->subState++;
                entity->stage = 0;
            }
            break;
        case 1:
            if (RandomNumberGenerator() % 3 == 0)
            {
                if (--entity->timer == 255)
                {
                    entity->state = 2;
                    entity->stage = 0;
                    entity->subState = 0;
                }
                else entity->posY.whole -= 0x8000;
            }
            break;
        case 2: break;
        }
        break;
    case 2:
        if (entity->ext.event03Part.flags & 8)
        {
            switch (entity->subState)
            {
            case 0:
                if (entity->ext.event03Part.flags & 16)
                {
                    child = Entity_CreateFromObjectTable(9);
                    child->updateFunc = EventScene03AttachedHelper_Update;
                    ENTITY_PARENT_INDEX(child) = ENTITY_INDEX(entity);
                    child->posX.part16.integer = entity->posX.part16.integer;
                    child->posY.part16.integer = entity->posY.part16.integer;
                    Anim_StartIndexed(child, 8, 0, 1);
                    child->spriteLayer = entity->spriteLayer;
                    child->enemyId = entity->enemyId;
                    if (entity->animScriptIndex != 2) Anim_StartIndexed(entity, 2, 0, 1);
                }
                else
                {
                    if (entity->animScriptIndex != 1) Anim_StartIndexed(entity, 1, 0, 1);
                }
                entity->timer = 160;
                entity->subState++;
                entity->stage = 0;
                /* fall through */
            case 1:
                if (entity->timer && !(RandomNumberGenerator() & 1))
                {
                    if (!(Entity_TryWalk(entity, 0x4000, 0x200000, 0x20000, 1) & 4))
                        entity->posY.whole += entity->velY.whole;
                    entity->timer--;
                }
                break;
            }
        }
        if ((u16)owner->ext.event03.progress & 2)
        {
            entity->state = 3;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    case 3:
        switch (entity->subState)
        {
        case 0:
            entity->timer = 32;
            entity->subState++;
            entity->stage = 0;
            break;
        case 1:
            if (entity->timer & 1) entity->palette = 4;
            else entity->palette = entity->basePalette;
            if (--entity->timer == 255)
            {
                entity->subState++;
                entity->stage = 0;
                entity->palette = entity->basePalette;
            }
            break;
        case 2:
            if (RandomNumberGenerator() % 7 == entity->attack)
            {
                entity->subState++;
                entity->stage = 0;
            }
            break;
        case 3:
            if (RandomNumberGenerator() % 7 == entity->attack)
            {
                if (Entity_IsInRect(entity, -128, -64, 496, 288))
                {
                    PlaySong(0x6B);
                    Effect_SpawnAt(entity->posX.whole, entity->posY.whole, entity->spriteLayer - 1);
                    EventScene03_SpawnUnusedPartDebris(entity);
                }
                EventScene03_SpawnTargetOrb((s16)entity->posX.part16.integer, (s16)entity->posY.part16.integer - 32, 0, 0, (struct EwramData_EntityData *)owner->ext.raw.unk_4FC.whole);
                entity->state = 4;
                entity->stage = 0;
                entity->subState = 0;
            }
            break;
        }
        break;
    case 4:
        entity->deleteRequested = 1;
        other->deleteRequested = 1;
        break;
    }
    Anim_Update(entity);
}

/* EventScene03AttachedEffect_Update. */

/**
 * @brief 5E0D0 | Keep an attached effect on body part 0 of its parent
 * entityData[unk_4F5] (Sprite_GetPartCornerOffset), with the parent's facing and
 * palette; deleted (deleteRequested) once the parent slot holds another object
 * (enemyId differs).
 */
void EventScene03AttachedEffect_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    struct { s16 x; s16 y; } point;

    if (parent == NULL || parent->enemyId != entity->enemyId)
    {
        entity->deleteRequested = 1;
        return;
    }
    Sprite_GetPartCornerOffset(parent, 0, &point.x);
    entity->posX.part16.integer = parent->posX.part16.integer + point.x;
    entity->posY.part16.integer = parent->posY.part16.integer + point.y;
    entity->flipX = parent->flipX;
    Anim_Update(entity);
    entity->palette = parent->palette;
}

/* Child entity that follows its parent with an offset (EventScene03AttachedHelper_Update).
 * The do/while (0) adds weight to entity so global-alloc gives it r5. */
/* Like EventScene03AttachedEffect_Update, but with an extra (+-8, -8) offset mirrored by the
 * parent's facing. */

/**
 * @brief Keep the helper on body part 0 of its parent (offset 8/-8 by facing)
 * with the parent palette; delete it when the parent slot holds another
 * object.
 */
void EventScene03AttachedHelper_Update(struct EwramData_EntityData *entity)
{
    struct { s16 x; s16 y; } point;
    s32 dx, dy;
    struct EwramData_EntityData *parent = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];

    dx = 8;
    dy = -8;

    if (parent == NULL || parent->enemyId != entity->enemyId)
    {
        entity->deleteRequested = 1;
        return;
    }
    entity->flipX = parent->flipX;
    Sprite_GetPartCornerOffset(parent, 0, (s16 *)&point);
    if (entity->flipX)
        dx = dy;
    entity->posX.part16.integer = parent->posX.part16.integer + point.x + dx;
    entity->posY.part16.integer = parent->posY.part16.integer + point.y + dy;
    do { Anim_Update(entity); } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
    entity->palette = parent->palette;
}

/* EventScene03_SpawnFlyingPart. */

/**
 * @brief 5E260 | Spawn an effect entity (Entity_CreateFromObjectTable(8), update
 * EventScene03FlyingPart_Update) at (x, y) with parent's unk_4F4 in unk_4F5, a random phase
 * in unk_50A, animation 4, facing the player.
 */
struct EwramData_EntityData *EventScene03_SpawnFlyingPart(struct EwramData_EntityData *parent, s32 x, s32 y)
{
    struct EwramData_EntityData *entity = Entity_CreateFromObjectTable(8);
    s32 flip;

    entity->updateFunc = EventScene03FlyingPart_Update;
    entity->posX.part16.integer = x;
    entity->posY.part16.integer = y;
    ENTITY_PARENT_INDEX(entity) = ENTITY_INDEX(parent);
    entity->ext.event03Part.wave = (RandomNumberGenerator() % 128) << 9;
    Anim_StartIndexed(entity, 4, 0, 1);
    flip = 0;
    if (entity->posX.whole < gEwramData->playerState.player->posX.whole)
        flip = 1;
    entity->flipX = flip;
    return entity;
}

/**
 * @brief 5E2D8 | Event part like EventScene03MainPart_Update: bobs (sine on unk_50A) while
 * in states 0-2, advances on owner flags 8 / 2, in state 3 flies along a
 * sine path (velX doubled, velY / 3) facing its direction; state 4 blinks
 * the palette (palette) and bursts at random (Rng % 7 == attack) with
 * sound 0x6B and debris via EventScene03_SpawnPartDebris. Owner flag 4 makes it vanish.
 */
void EventScene03FlyingPart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    s32 wave;

    if ((u16)owner->ext.event03.progress & 4)
    {
        entity->state = 5;
        entity->stage = 0;
        entity->subState = 0;
    }
    if (entity->state <= 2)
    {
        wave = Sin(entity->ext.event03Part.wave + 0x4000) / 4;
        entity->ext.event03Part.wave += 0x200;
        entity->posY.whole += wave;
        entity->velY.whole = wave + entity->accelY.whole;
    }
    switch (entity->state)
    {
    case 0:
        if ((u16)owner->ext.event03.progress & 8)
        {
            entity->state = 1;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    case 1:
        entity->state = 2;
        entity->stage = 0;
        entity->subState = 0;
        break;
    case 2:
        if ((u16)owner->ext.event03.progress & 2)
        {
            entity->state = 4;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    case 3:
        if ((u16)owner->ext.event03.progress & 2)
        {
            entity->state = 4;
            entity->stage = 0;
            entity->subState = 0;
        }
        else
        {
            u16 angle = (entity->ext.event03Part.wave << 23) >> 16;
            s32 vx;
            s32 vy;
            entity->velX.whole = Sin(angle + 0x4000) >> 8 << 8;
            entity->velY.whole = Sin(angle) >> 8 << 8;
            vx = entity->velX.whole << 1;
            entity->velX.whole = vx;
            vy = entity->velY.whole / 3;
            entity->velY.whole = vy;
            entity->posX.whole += vx;
            entity->posY.whole += vy;
            entity->ext.event03Part.wave++;
            entity->flipX = vx >= 0;
        }
        break;
    case 4:
        switch (entity->subState)
        {
        case 0:
            entity->timer = 0x20;
            entity->subState++;
            entity->stage = 0;
            break;
        case 1:
            if (entity->timer & 1)
                entity->palette = 4;
            else
                entity->palette = entity->basePalette;
            if (--entity->timer == 0xFF)
            {
                entity->subState++;
                entity->stage = 0;
                entity->palette = entity->basePalette;
                entity->timer = 0x10;
            }
            break;
        case 2:
            if (--entity->timer == 0xFF)
            {
                entity->subState++;
                entity->stage = 0;
            }
            break;
        case 3:
            if (RandomNumberGenerator() % 7 == entity->attack)
            {
                if (Entity_IsInRect(entity, -0x80, -0x40, 0x1F0, 0x120))
                {
                    PlaySong(0x6B);
                    Effect_SpawnAt(entity->posX.whole, entity->posY.whole, entity->spriteLayer - 1);
                    EventScene03_SpawnPartDebris(entity);
                }
                entity->state = 5;
                entity->stage = 0;
                entity->subState = 0;
            }
            break;
        }
        break;
    case 5:
        entity->deleteRequested = 1;
        break;
    }
    Anim_Update(entity);
}

/* EventScene03_SpawnRisingPart. */

/**
 * @brief 5E528 | Spawn an effect entity (Entity_CreateFromObjectTable(10), update
 * EventScene03RisingPart_Update) at (x, y) with parent's unk_4F4 in unk_4F5 and flags in
 * unk_4F7; flag 4 moves it 8 px down (16 steps of 0x8000) into layer
 * 0x17. Animation 0, facing the player.
 */
struct EwramData_EntityData *EventScene03_SpawnRisingPart(struct EwramData_EntityData *parent, s32 x, s32 y, s32 flags)
{
    struct EwramData_EntityData *entity = Entity_CreateFromObjectTable(10);
    s32 flip;

    entity->updateFunc = EventScene03RisingPart_Update;
    entity->posX.part16.integer = x;
    entity->posY.part16.integer = y;
    Entity_SnapToGround(entity);
    ENTITY_PARENT_INDEX(entity) = ENTITY_INDEX(parent);
    entity->ext.event03Part.flags = flags;
    entity->enemyId = 0xFF;
    Anim_StartIndexed(entity, 0, 0, 1);
    if (entity->ext.event03Part.flags & 4)
    {
        s32 i;

        for (i = 16; i != 0; i--)
            entity->posY.whole += 0x8000;
        entity->spriteLayer = 0x17;
    }
    flip = 0;
    if (entity->posX.whole < gEwramData->playerState.player->posX.whole)
        flip = 1;
    entity->flipX = flip;
    return entity;
}

/* Event part: wait for its owner, rise, blink, then emit debris. */
void EventScene03RisingPart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    if ((u16)owner->ext.event03.progress & 4)
    {
        entity->state = 4;
        entity->stage = 0;
        entity->subState = 0;
    }
    switch (entity->state)
    {
    case 0:
        if ((u16)owner->ext.event03.progress & 8)
        {
            if (RandomNumberGenerator() % 7 == entity->attack)
            {
                if ((u8)(entity->ext.event03Part.flags & 2))
                {
                    entity->state = 2;
                    entity->stage = 0;
                    entity->subState = 0;
                }
                else
                {
                    entity->state = 1;
                    entity->stage = 0;
                    entity->subState = 0;
                }
            }
        }
        break;
    case 1:
        switch (entity->subState)
        {
        case 0:
            if (RandomNumberGenerator() % 7 == entity->attack)
            {
                entity->timer = 16;
                entity->subState++;
                entity->stage = 0;
            }
            break;
        case 1:
            if (RandomNumberGenerator() % 3 == 0)
            {
                if (--entity->timer == 255)
                {
                    entity->state = 2;
                    entity->stage = 0;
                    entity->subState = 0;
                }
                else entity->posY.whole -= 0x8000;
            }
            break;
        case 2: break;
        }
        break;
    case 2:
        if (entity->ext.event03Part.flags & 8)
        {
            switch (entity->subState)
            {
            case 0:
                if (entity->animScriptIndex != 1) Anim_StartIndexed(entity, 1, 0, 1);
                entity->timer = 160;
                entity->subState++;
                entity->stage = 0;
                /* fall through */
            case 1:
                if (!(RandomNumberGenerator() & 1) && entity->timer)
                {
                    if (!(Entity_TryWalk(entity, 0x4000, 0x200000, 0x20000, 1) & 4))
                        entity->posY.whole += entity->velY.whole;
                    entity->timer--;
                }
                break;
            }
        }
        else if (entity->ext.event03Part.flags & 16)
        {
            if (entity->subState == 0)
            {
                if (entity->animScriptIndex != 2) Anim_StartIndexed(entity, 2, 0, 0);
                entity->subState++;
            }
        }
        if ((u16)owner->ext.event03.progress & 2)
        {
            entity->state = 3;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    case 3:
        switch (entity->subState)
        {
        case 0:
            entity->timer = 32;
            entity->subState++;
            entity->stage = 0;
            break;
        case 1:
            if (entity->timer & 1) entity->palette = 4;
            else entity->palette = entity->basePalette;
            if (--entity->timer == 255)
            {
                entity->subState++;
                entity->stage = 0;
                entity->palette = entity->basePalette;
            }
            break;
        case 2:
            if (RandomNumberGenerator() % 7 == entity->attack)
            {
                entity->subState++;
                entity->stage = 0;
            }
            break;
        case 3:
            if (RandomNumberGenerator() % 7 == entity->attack)
            {
                if (Entity_IsInRect(entity, -128, -64, 496, 288))
                {
                    PlaySong(0x6B);
                    Effect_SpawnAt(entity->posX.whole, entity->posY.whole, entity->spriteLayer - 1);
                }
                EventScene03_SpawnTargetOrb((s16)entity->posX.part16.integer, entity->posY.part16.integer - 32, 0, 0, (struct EwramData_EntityData *)owner->ext.raw.unk_4FC.whole);
                entity->state = 4;
                entity->stage = 0;
                entity->subState = 0;
            }
            break;
        }
        break;
    case 4:
        entity->deleteRequested = 1;
        break;
    }
    Anim_Update(entity);
}

/* Seven debris pieces using the owner's graphics and palette. */

void EventScene03_SpawnUnusedPartDebris(struct EwramData_EntityData *owner)
{
    s32 offset = 0;
    s32 i, x, graphics;
    struct EwramData_EntityData *part;
    const s8 *info;
    for (i = 6; i >= 0; offset += 6, i--)
    {
        part = EntityCreateInRange(0x49, 0xCF, EventScene03Debris_Update);
        if (part != NULL)
        {
            info = sEventScene03UnusedDebris + offset;
            ENTITY_PARENT_INDEX(part) = ENTITY_INDEX(owner);
            part->spriteLayer = owner->spriteLayer;
            part->basePalette = owner->basePalette;
            part->flipX = owner->flipX;
            graphics = Sprite_GetTileSlot(owner);
            Sprite_Attach(part, sSkeletonSpriteSheet, graphics, part->basePalette);
            part->scrollWithBg = 1;
            part->drawFunc = Sprite_DrawEntity;
            part->posX.whole = owner->posX.whole;
            part->posY.whole = owner->posY.whole;
            x = info[0];
            part->posY.part16.integer += info[1];
            part->animFrame = info[2];
            {
                s32 r = RandomNumberGenerator();
                s32 v = info[3] + 16;
                part->velX.whole = (v - (r & 31)) << 12;
            }
            {
                s32 r = RandomNumberGenerator();
                s32 v = info[4] + 16;
                part->velY.whole = -((v - (r & 31)) << 12);
            }
            part->timer = info[5] / 2;
            if (part->flipX)
            {
                part->velX.whole = -part->velX.whole;
                x = -x;
            }
            part->posX.part16.integer += x;
        }
    }
}

/* Seven debris pieces using the owner's graphics and palette. */
void EventScene03_SpawnPartDebris(struct EwramData_EntityData *owner)
{
    s32 offset = 0;
    s32 i, x, vx, graphics;
    struct EwramData_EntityData *part;
    const s8 *info;
    for (i = 6; i >= 0; offset += 6, i--)
    {
        part = EntityCreateInRange(0x49, 0xCF, EventScene03Debris_Update);
        if (part != NULL)
        {
            info = sEventScene03Debris + offset;
            ENTITY_PARENT_INDEX(part) = ENTITY_INDEX(owner);
            part->spriteLayer = owner->spriteLayer;
            part->basePalette = owner->basePalette;
            part->flipX = owner->flipX;
            graphics = Sprite_GetTileSlot(owner);
            Sprite_Attach(part, sWingedSkeletonSpriteSheet, graphics, part->basePalette);
            part->scrollWithBg = 1;
            part->drawFunc = Sprite_DrawEntity;
            part->posX.whole = owner->posX.whole;
            part->posY.whole = owner->posY.whole;
            x = info[0];
            part->posY.part16.integer += info[1];
            part->animFrame = info[2];
            vx = info[3] << 12;
            part->velX.whole = vx;
            part->velY.whole = -(info[4] << 12);
            part->timer = info[5] / 2;
            if (part->flipX)
            {
                part->velX.whole = -vx;
                x = -x;
            }
            part->posX.part16.integer += x;
        }
    }
}

/**
 * @brief Spawn a soul orb (soul pickup palette of the variant) that homes in on
 * the player, or waits (mode != 0); its contact callback ends it.
 */
struct EwramData_EntityData *EventScene03_SpawnSoulOrb(s32 x, s32 y, s32 variant, s32 index, s32 mode)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, EventScene03SoulOrb_Update);
    if (entity != NULL)
    {
        s32 graphics;
        entity->spriteLayer = 0x13;
        graphics = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, Soul_GetPickupPalette(variant));
        Anim_StartIndexed(entity, 6, 0, 1);
        Anim_Update(entity);
        entity->stage = 1;
        entity->drawFunc = Sprite_DrawEntity;
        Hit_Enable(entity, 0xB, EventScene03SoulOrb_OnPlayerContact, 0);
        entity->hit.flags |= 4;
        HitBox_SetAttackRaw(entity, 0x0C0CFAFA);
        entity->posX.part16.integer = x;
        entity->posY.part16.integer = y;
        entity->scrollWithBg = 1;
        if (mode == 0)
        {
            struct EwramData_EntityData *player = gEwramData->playerState.player;
            entity->ext.event03Orb.heading = (Entity_AngleToPoint(entity, player->posX.whole, player->posY.whole - 0x80000) + 0x8000) & 0xFFFF;
            entity->ext.event03Orb.turnRate = mode;
            entity->ext.event03Orb.speed = 0x30000;
        }
        else entity->state = 99;
        entity->enemyId = variant + 5;
        entity->spawnParam1 = index;
        entity->timer = 200;
    }
    return entity;
}

/* Homing projectile, including impact and its particle trail. */
/**
 * @brief Steer the soul orb towards the player with growing turn rate and speed
 * and leave a particle trail.
 */
void EventScene03SoulOrb_Update(struct EwramData_EntityData *entity)
{
    u32 collision[4];
    if (entity->state <= 1)
    {
        struct EwramData_EntityData *target = gEwramData->playerState.player;
        entity->ext.event03Orb.heading = Entity_StepAngleTowardPoint(entity, target->posX.whole, target->posY.whole - 0x100000,
                                               entity->ext.event03Orb.heading, entity->ext.event03Orb.turnRate);
        entity->ext.event03Orb.turnRate += 42;
        if (entity->ext.event03Orb.turnRate > 0x4000) entity->ext.event03Orb.turnRate = 0x4000;
        entity->ext.event03Orb.speed += 0x1800;
        if (entity->ext.event03Orb.speed > 0x80000) entity->ext.event03Orb.speed = 0x80000;
        entity->velX.whole = FixedMulFast(Sin(entity->ext.event03Orb.heading + 0x4000), entity->ext.event03Orb.speed);
        entity->velY.whole = FixedMulFast(Sin(entity->ext.event03Orb.heading), entity->ext.event03Orb.speed);
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
    }
    Hit_CheckWithPlayer((struct HitResult *)collision, entity);
    if ((entity->timer & 3) == 0)
    {
        s32 palette = entity->palette;
        s32 x = entity->posX.whole + RandomSigned(0x200000);
        s32 y = entity->posY.whole + RandomSigned(0x200000);
        struct EwramData_EntityData *particle = Effect_CreateFromTable(x, y, entity->velX.whole / 64, entity->velY.whole / 64, 8, palette);
        if (particle != NULL)
        {
            particle->velX.whole = -entity->velX.whole / 4;
            particle->velY.whole = -entity->velY.whole / 4;
        }
    }
    switch (entity->state)
    {
    case 0:
        if (--entity->timer == 0)
        {
            entity->drawSkip = 1;
            entity->timer = 64;
            entity->state++;
        }
        break;
    case 1:
        if (--entity->timer == 0) entity->deleteRequested = 1;
        break;
    default: entity->timer++; break;
    }
    if (entity->stage != 0) Anim_Update(entity);
}

/**
 * @brief Spawn an orb that homes in on the given target entity (deleted on
 * arrival with sound 0xBC).
 */
struct EwramData_EntityData *EventScene03_SpawnTargetOrb(s32 x, s32 y, s32 variant, s32 index, struct EwramData_EntityData *target)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, EventScene03TargetOrb_Update);
    if (entity != NULL)
    {
        s32 graphics, targetX, offset;
        entity->ext.event03Orb.target = target;
        entity->spriteLayer = 0x13;
        graphics = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, 6);
        Anim_StartIndexed(entity, 0x16, 0, 1);
        Anim_Update(entity);
        entity->stage = 1;
        entity->drawFunc = Sprite_DrawEntity;
        Hit_Enable(entity, 0xB, EventScene03TargetOrb_OnContact, 0);
        entity->hit.flags |= 4;
        HitBox_SetAttackRaw(entity, 0x0C0CFAFA);
        entity->posX.part16.integer = x;
        entity->posY.part16.integer = y;
        entity->scrollWithBg = 1;
        targetX = target->posX.whole;
        offset = ((RandomNumberGenerator() & 0x7F) << 12) + 0x80000;
        entity->ext.event03Orb.heading = (Entity_AngleToPoint(entity, targetX, target->posY.whole - offset) + 0x8000) & 0xFFFF;
        entity->ext.event03Orb.turnRate = 0;
        entity->ext.event03Orb.speed = 0x30000;
        entity->enemyId = variant + 5;
        entity->spawnParam1 = index;
        entity->timer = 200;
    }
    return entity;
}

/* Homing projectile, including impact and its particle trail. */
/**
 * @brief Steer the orb towards its target with growing turn rate and speed;
 * delete it with sound 0xBC when it reaches the target.
 */
void EventScene03TargetOrb_Update(struct EwramData_EntityData *entity)
{
    u32 unused[4]; /* The original reserves 16 unused stack bytes. */
    struct EwramData_EntityData *target = entity->ext.event03Orb.target;
    if (entity->state <= 1)
    {
        entity->ext.event03Orb.heading = Entity_StepAngleTowardPoint(entity, target->posX.whole, target->posY.whole - 0x100000,
                                               entity->ext.event03Orb.heading, entity->ext.event03Orb.turnRate);
        entity->ext.event03Orb.turnRate += 42;
        if (entity->ext.event03Orb.turnRate > 0x4000) entity->ext.event03Orb.turnRate = 0x4000;
        entity->ext.event03Orb.speed += 0x1800;
        if (entity->ext.event03Orb.speed > 0x80000) entity->ext.event03Orb.speed = 0x80000;
        entity->velX.whole = FixedMulFast(Sin(entity->ext.event03Orb.heading + 0x4000), entity->ext.event03Orb.speed);
        entity->velY.whole = FixedMulFast(Sin(entity->ext.event03Orb.heading), entity->ext.event03Orb.speed);
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
    }
    if (Entity_IsInRect(target, (s16)(entity->posX.part16.integer - 16), (s16)(entity->posY.part16.integer - 16), 32, 32))
    {
        entity->deleteRequested = 1;
        PlaySong(0xBC);
        return;
    }
    if ((entity->timer & 1) == 0)
    {
        s32 palette = entity->palette;
        s32 x = entity->posX.whole + RandomSigned(0x200000);
        s32 y = entity->posY.whole + RandomSigned(0x200000);
        struct EwramData_EntityData *particle = Effect_CreateFromTable(x, y, entity->velX.whole / 64, entity->velY.whole / 64, 8, palette);
        if (particle != NULL)
        {
            particle->velX.whole = -entity->velX.whole / 4;
            particle->velY.whole = -entity->velY.whole / 4;
        }
    }
    switch (entity->state)
    {
    case 0:
        if (--entity->timer == 0)
        {
            entity->drawSkip = 1;
            entity->timer = 64;
            entity->state++;
        }
        break;
    case 1:
        if (--entity->timer == 0) entity->deleteRequested = 1;
        break;
    default: entity->timer++; break;
    }
    if (entity->stage != 0) Anim_Update(entity);
}

/* Related-entity animation and particle helpers, 0x0805F008..0x0805F11F. */

/**
 * @brief Finish event scene 3: animation 5 on helper unk_4F8, animation 1 on
 * helper unk_4FC and delete helper unk_500.
 */
s32 EventScene03_EndParts(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *actor = entity->ext.event03.actor0;
    s32 result;
    if (actor->animScriptIndex != 5)
        Anim_StartIndexed(actor, 5, 0, 0);
    result = Anim_StartIndexed(entity->ext.event03.actor1, 1, 0, 1);
    entity->ext.event03.actor8->deleteRequested = 1;
    return result;
}

/* Same ballistic particle update as MandragoraDeathPart_Update, without a parent count. */
void EventScene03Debris_Update(struct EwramData_EntityData *entity)
{
    if (!Entity_IsInRect(entity, -0x80, -0x40, 0x1F0, 0x120)
        || (u8)--entity->timer == 0xFF || entity->posY.part16.integer > 0xAA)
        entity->deleteRequested = 1;
    else
    {
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        entity->velY.whole += 0x3000;
    }
}

/**
 * @brief Delete the soul orb, show the absorb animation on the player and play
 * sound 0xBC.
 */
void EventScene03SoulOrb_OnPlayerContact(struct EwramData_EntityData *entity)
{
    entity->deleteRequested = 1;
    Effect_SpawnAttachedAnimation(gEwramData->playerState.player, 0xC, 6, 0, -0x100000);
    PlaySong(0xBC);
}

/**
 * @brief Delete the orb, show the absorb animation on its target and play sound
 * 0xBC.
 */
void EventScene03TargetOrb_OnContact(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *anchor = entity->ext.event03Orb.target;
    entity->deleteRequested = 1;
    Effect_SpawnAttachedAnimation(anchor, 0xC, 6, 0, -0x100000);
    PlaySong(0xBC);
}
