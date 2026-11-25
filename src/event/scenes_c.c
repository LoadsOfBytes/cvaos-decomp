/*
 * Event scene 30 extras (unused zoom, frame upload, effects), the ending room background, BG2 effects, scene 24 init, scenes 27 and 28 and the conversation helpers.
 */
#include "core.h"
#include "event_scene30.h"
#include "bg_and_collision.h"
#include "graphics.h"
#include "structs/ewram.h"
#include "gba.h"
#include "syscalls.h"
#include "bg_effects.h"
#include "room_entities.h"
#include "structs/main.h"
#include "map_flags.h"
#include "player_core.h"
#include "gameplay.h"
#include "entity_util.h"
#include "sound.h"
#include "constants/main.h"
#include "constants/songs.h"
#include "data/data_0E0334.h"
#include "data/data_0E3464.h"
#include "structs/agb_sram.h"
#include "music.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "objects_35_37.h"
#include "palette.h"
#include "scene30.h"
#include "scenes_c.h"
#include "shop_update.h"
#include "text12.h"
#include "text_window.h"
#include "data/data_0E5364.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"

/* Per-scanline affine transition driven by the owner's event phase. */
#define PHASE(e) (e->ext.event30.phase)
#define CENTER_X(e) (e->ext.event30.centerX)
#define CENTER_Y(e) (e->ext.event30.centerY)
#define SCALE_X(e) (e->ext.event30.scale)
#define SCALE_Y(e) (e->ext.event30.scaleY)
#define SWEEP(e) (e->ext.event30.sweep)

extern struct RoomDefinition sEndingRoomDefinition;

void EventSceneMinaTalk_PickConversation(struct EwramData_EntityData *entity);
void EventSceneMinaTalk_SelectScript36(struct EwramData_EntityData *entity);
void Conversation_Setup(s32 script, s32 group, s32 entry);
void EventSceneUnconsciousYoko_PickScript(struct EwramData_EntityData *entity);

/**
 * @brief Unreferenced: H-blank BG2 affine zoom sequence in phases that also
 * spawns EventSceneEnding floating fragments.
 */
void EventSceneEnding_UpdateUnusedZoom(struct EwramData_EntityData *entity)
{
    struct BgAffineDstData *out=(struct BgAffineDstData *)&gEwramData->hBlankEffect.hBlankBuffer[1-gEwramData->hBlankEffect.currentBuffer];
    s32 dx=0;
    s32 dy;
    s32 line;
    struct BgAffineSrcData src;
    HBlankEffectSetup(0,160,16,REG_BG2PA);
    switch(PHASE(entity))
    {
    case 0:
        PHASE(entity)++;
        CENTER_X(entity)=152;
        CENTER_Y(entity)=96;
        SCALE_X(entity)=208;
        SCALE_Y(entity)=208;
        entity->ext.event30.zoomTexX=0;
        entity->ext.event30.zoomTexY=128;
        break;
    case 1:
        entity->timer=32;
        PHASE(entity)++;
        SWEEP(entity)=0;
        break;
    case 2:
        for(line=0;line<160;line++)
        {
            src.texX=0x8000;src.texY=0x8000;
            src.scrX=CENTER_X(entity);src.scrY=CENTER_Y(entity)-line;
            src.sx=208;src.sy=208;src.alpha=0;
            BgAffineSet(&src,out,1);
            out++;
        }
        if(--entity->timer==255)
        {SWEEP(entity)=0;PHASE(entity)++;entity->timer=0;}
        break;
    case 3:
        BgPalette_LoadRange(sEventSceneEndingTransformationPalette,0,1,0);
        if((u8)(entity->timer%24)==0)
        {
            s32 x=(s16)((u16)(CENTER_X(entity)+24)-RandomNumberGenerator()%48-16);
            u32 randY=RandomNumberGenerator(); s32 y=(u16)CENTER_Y(entity); y=(s16)(y-(randY%128)+48);
            EventSceneEnding_SpawnFloatingFragment(entity,x,y,0);
        }
        dx=2-(RandomNumberGenerator()&3);
        dy=2-(RandomNumberGenerator()&3);
        for(line=0;line<160;line++)
        {
            src.texX=0x8000;src.texY=0x8000;
            src.scrX=CENTER_X(entity);src.scrY=CENTER_Y(entity)-line;
            if(entity->timer&1){src.scrX+=dx;src.scrY+=dy;}
            src.sx=208;src.sy=208;src.alpha=0;
            BgAffineSet(&src,out,1);
            out++;
        }
        if(++entity->timer==129)
        {SWEEP(entity)=0;entity->timer=0;PHASE(entity)++;}
        break;
    case 4:
        for(line=0;line<160;line++)
        {
            if(line<80)
            {
                src.texX=0;src.texY=(s16)entity->ext.event30.zoomTexY<<8;
                src.scrX=0;src.scrY=CENTER_Y(entity)-line;
                dx=0;
                if(line<(u16)SWEEP(entity))dx=(u16)SWEEP(entity)-line;
                src.sx=SCALE_X(entity)-dx;src.sy=SCALE_Y(entity);src.alpha=0;
            }
            else
            {
                src.texX=0xA000;src.texY=(s16)entity->ext.event30.zoomTexY<<8;
                src.scrX=200;src.scrY=CENTER_Y(entity)-line;
                dx=0;
                if(160-line<(u16)SWEEP(entity)){ s32 amount=(u16)SWEEP(entity)-160; dx=amount+line; }
                src.sx=SCALE_X(entity)-dx;src.sy=SCALE_Y(entity);src.alpha=0;
            }
            BgAffineSet(&src,out,1);
            out++;
        }
        SWEEP(entity)++;
        if((entity->timer%16)==0)SCALE_Y(entity)++;
        entity->timer++;
        entity->ext.event30.timer++;
        break;
    case 5:
        BgPalette_LoadRange(sEventSceneEndingTransformationPalette,8,1,0);
        for(line=0;line<160;line++)
        {
            src.texX=0x8000;src.texY=0x8000;
            src.scrX=CENTER_X(entity);src.scrY=CENTER_Y(entity)-line;
            src.sx=SCALE_X(entity);src.sy=SCALE_Y(entity);
            src.alpha=(u8)entity->ext.event30.angle<<8;
            BgAffineSet(&src,out,1);
            out++;
        }
        if(entity->timer++>128)
        {SWEEP(entity)=0;entity->timer=0;PHASE(entity)=8;}
        break;
    case 8:
        BgPalette_LoadRange(sEventSceneEndingTransformationPalette,9,1,0);
        ObjPalette_LoadRange(sEventSceneEndingPalette,1,1,entity->basePalette);
        for(line=0;line<160;line++)
        {
            src.texX=0x8000;src.texY=0x8000;
            src.scrX=CENTER_X(entity)+dx;src.scrY=CENTER_Y(entity)-line;
            src.sx=SCALE_X(entity);src.sy=SCALE_Y(entity);
            src.alpha=(u8)entity->ext.event30.angle<<8;
            BgAffineSet(&src,out,1);
            out++;
        }
        if((u16)SCALE_Y(entity)<=65534)SCALE_Y(entity)+=64;
        break;
    }
    gEwramData->hBlankEffect.requestStart=1;
}

/* Load the transition's room metadata, backgrounds and initial palette. */
/**
 * @brief Unreferenced: load the graphics, palettes and BG layers (with their
 * initial scroll) of room sEndingRoomDefinition, the room the debug menu uses for
 * the ending events, without its entities.
 */
void Room_LoadEndingRoomBackground(void)
{
    struct RoomDefinition *room=&sEndingRoomDefinition;
    struct GameState *world=&gEwramData->gameState;
    struct RoomBgLayerDefinition *layer;
    s32 index;
    u16 *bgControl;
    gUnk_03002CB0.dispCnt=room->displayControl;
    Room_LoadGraphics(room);Room_LoadPalettes(room);
    layer=room->bgLayers;
    bgControl=gDisplayRegisters.bgCnt;
    for(index=1;index<4;index++)
    {
        struct RoomBgDimensions *metadata;
        u16 x,y;
        bgControl[index]=layer->control;
        gEwramData->bgInfo[index].scrollMode=layer->scrollMode;
        gEwramData->bgInfo[index].pixelWidth=layer->pixelWidth;
        gEwramData->bgInfo[index].pixelHeight=layer->pixelHeight;
        Obj_SetBgLayer(layer->layer,index);
        metadata=layer->dimensions;
        x=BgLayer_GetInitialScrollX(0,layer->scrollMode,metadata->widthCells,layer->pixelWidth);
        y=BgLayer_GetInitialScrollY(0,layer->scrollMode,layer->dimensions->heightCells,layer->pixelHeight);
        BgMap_Setup(index,(struct BgMapMetadata *)metadata,x,y);
        layer++;
    }
    Obj_SetBgLayer(0,0);Bg_SyncPriorities();
    world->roomMapXPos=room->mapX;
    world->roomMapYPos=room->mapY;
    world->unk_8E_0=room->unk_23_0;
    gEwramData->collisionBlocksActive=room->collisionBlocksEnabled;
    Collision_ClearAllBlocks();
    BgPalette_LoadRange(sTextBgPalette,0,1,14);
    Text_SetTileBuffer((void *)(VRAM_BASE + 0x4000));
    Text_SetForegroundColor(8);Text_SetBackgroundColor(0);
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   BgEffect_Bg2InterlacedWaveCopy = BgEffect_Bg2InterlacedWave
 *   BgEffect_Bg2PerspectiveCopy = BgEffect_Bg2Perspective */

/**
 * @brief 65600 | Same as BgEffect_Bg2InterlacedWave.
 */
void BgEffect_Bg2InterlacedWaveCopy(void)
{
    s32 angle;
    s16 *hBlankBuf;
    s32 y;
    s32 scanline;
    s32 zeroIndex;
    u16 vOfs;

    hBlankBuf = gEwramData->hBlankEffect.hBlankBuffer[1 - gEwramData->hBlankEffect.currentBuffer];
    angle = gEwramData->frameCounter;
    angle <<= 0xA;
    vOfs = gDisplayRegisters.bgOfs[2].vOfs;
    HBlankEffectSetup(0, SCREEN_SIZE_Y, 2, REG_BG2HOFS);

    for (scanline = 0, zeroIndex = 2; scanline < SCREEN_SIZE_Y; scanline++, zeroIndex += 3)
    {
        y = scanline + vOfs;
        if (y > 0x120 || (y & 1) == (gEwramData->frameCounter & 0x10) / 0x10)
            hBlankBuf[zeroIndex] = 0;
        else
            *hBlankBuf = Sin(angle) >> 0xF;

        hBlankBuf++;
        angle -= 0x2000;
    }

    gEwramData->hBlankEffect.requestStart = 1;
}

/**
 * @brief 656BC | Same as BgEffect_Bg2Perspective.
 */
void BgEffect_Bg2PerspectiveCopy(void)
{
    s32 sx;
    s32 dist;
    s32 scanline;
    s32 texY;
    struct BgAffineDstData *hBlankBuf;
    u32 scroll;
    struct BgAffineSrcData src;

    scroll = gEwramData->gameState.bgEffectFrameCounter;
    hBlankBuf = (struct BgAffineDstData *)&gEwramData->hBlankEffect.hBlankBuffer[1 - gEwramData->hBlankEffect.currentBuffer];
    texY = 0;
    HBlankEffectSetup(0, SCREEN_SIZE_Y, 0x10, REG_BG2PA);

    for (scanline = 0; scanline < SCREEN_SIZE_Y; scanline++)
    {
        src.texX = 0x8000 - (0x100 * scroll);
        src.scrX = 0x78;
        src.scrY = 0;
        if (scanline < 0x80)
        {
            dist = (0x80 - scanline) << 5;
            sx = Div(0x80000, dist + 0x400);
            src.sx = sx;
            texY -= sx;
            src.texY = texY - (scroll << 7);
        }
        else
        {
            dist = scanline << 7;
            sx = Div(0x80000, dist - 0x3C00);
            src.sx = sx;
            texY -= sx * 2;
            src.texY = texY + (scroll << 7);
        }
        src.sy = 0x100;
        src.alpha = 0;
        BgAffineSet(&src, hBlankBuf, 1);
        hBlankBuf++;
    }

    gEwramData->hBlankEffect.requestStart = 1;
}

/* Event and particle helpers, decompiled from the original assembly. */

/* End this entity unless event flag 0x4000 is set; otherwise advance the
 * event flags and play song 0x1000. */
/**
 * @brief Init of event scene variant 24 (Object 0x20 spawnParam0 = 24,
 * sEventSceneCreateFuncs[24]).
 */
void EventSceneEndingWakeUp_Init(struct EwramData_EntityData *entity)
{
    if (!(gEwramData->gameState.stateFlags & 0x4000))
        entity->deleteRequested = 1;
    else
    {
        gEwramData->gameState.stateFlags |= 0x20;
        gEwramData->gameState.stateFlags |= 0x10000;
        Hud_ClearGraphics();
        PlaySong(0x1000);
    }
}

/* Upload a frame selected by unk_6, advancing once every two counts. */
void EventSceneEnding_UploadFrame(void)
{
    struct ParticleFrames *frames = &sEventSceneEndingParticleFrames;
    s32 index = (gEwramData->gameModeCounter >> 1) % frames->count;
    struct ParticleFrame *frame = &frames->frames[index];
    BgPalette_LoadRange(frame->data, frame->size,
                1, frames->destination);
}

/**
 * @brief Delete the flash at the end of its animation.
 */
void EventSceneEndingFlash_Update(struct EwramData_EntityData *entity)
{
    if (entity->animDone)
        entity->deleteRequested = 1;
    else
        Anim_Update(entity);
}

/* Integrate velocity and acceleration, then discard outside the box. */
/**
 * @brief Integrate velocity and acceleration, then discard outside the box.
 */
void EventSceneEndingEffect_UpdateBallistic(struct EwramData_EntityData *entity)
{
    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
    entity->velX.whole += entity->accelX.whole;
    entity->velY.whole += entity->accelY.whole;
    if (!Entity_IsInRect(entity, -128, -64, 496, 288))
        entity->deleteRequested = 1;
}

/* The affine version also updates its transform and releases the slot
 * when it leaves the box. unk_500 supplies both transform scale arguments. */
/**
 * @brief Integrate velocity and acceleration, spin the affine transform and
 * free the affine slot when leaving the box.
 */
void EventSceneEndingEffect_UpdateSpinning(struct EwramData_EntityData *entity)
{
    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
    entity->velX.whole += entity->accelX.whole;
    entity->velY.whole += entity->accelY.whole;
    EVENT30_EFFECT_ANGLE(entity) += entity->ext.event30Effect.spin;
    ObjAffine_Set(EVENT30_EFFECT_ANGLE(entity), entity->ext.event30Effect.scale,
                entity->ext.event30Effect.scale, 0, 0, entity->affineIndex);
    if (!Entity_IsInRect(entity, -128, -64, 496, 288))
    {
        ObjAffine_Free(entity->affineIndex);
        entity->deleteRequested = 1;
    }
}

/* Same transform update, with the affine slot retained on exit. */
/**
 * @brief Same transform update, with the affine slot retained on exit.
 */
void EventSceneEndingEffect_UpdateSpinningKeepSlot(struct EwramData_EntityData *entity)
{
    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
    entity->velX.whole += entity->accelX.whole;
    entity->velY.whole += entity->accelY.whole;
    EVENT30_EFFECT_ANGLE(entity) += entity->ext.event30Effect.spin;
    ObjAffine_Set(EVENT30_EFFECT_ANGLE(entity), entity->ext.event30Effect.scale,
                entity->ext.event30Effect.scale, 0, 0, entity->affineIndex);
    if (!Entity_IsInRect(entity, -128, -64, 496, 288))
        entity->deleteRequested = 1;
}

/* Event actor: skip when flag 0x2E is already set. Otherwise load its
 * graphics and position it relative to the background and player. */
/**
 * @brief Init of event scene variant 25 (Object 0x20 spawnParam0 = 25,
 * sEventSceneCreateFuncs[25]).
 */
void EventSceneMeetHammer_Init(struct EwramData_EntityData *entity)
{
    struct EwramData *ewram;
    struct BgLayerState *background;
    struct PlayerState *actors;
    s32 facing;

    if (GameFlag_GetMask(0x2E))
        entity->deleteRequested = 1;
    else if (!Entity_SetupFromObjectTable(entity, 3))
        EntityDelete(entity);
    else
    {
        Anim_StartIndexed(entity, 1, 0, 1);
        Anim_Update(entity);
        ewram = gEwramData;
        background = &ewram->bgInfo[1];
        entity->posX.part16.integer = 96 - background->xPos.part16.integer;
        entity->posY.part16.integer = 160 - background->yPos.part16.integer;
        facing = 0;
        actors = &ewram->playerState;
        if (entity->posX.whole < actors->player->posX.whole)
            facing = 1;
        entity->flipX = facing;
        entity->spriteLayer = actors->player->spriteLayer - 1;
        ewram->gameState.stateFlags |= 1;
    }
}

/* Event 0x28: pause, move the player to x=160, then send this actor off
 * the right edge. Return simulated input while moving the player. */
/**
 * @brief Update of event scene variant 25 (sEventSceneUpdateFuncs[25]); returns the
 * player input mask.
 */
s32 EventSceneMeetHammer_Update(struct EwramData_EntityData *entity)
{
    s32 input = 0;
    struct EwramData *ewram = gEwramData;
    struct GameState *event = &ewram->gameState;

    switch (entity->state)
    {
        case 0:
            Music_SetCurrent(0xD);
            TextWindow_Begin(0x28);
            ewram->gameState.textWindowPause = 1;
            entity->state++;
            /* fall through */
        case 1:
            if (TextWindow_Update())
                entity->state = 0x80;
            switch (entity->subState)
            {
                case 0:
                    switch (entity->stage)
                    {
                        case 0:
                            event->textWindowPause = 1;
                            entity->stage++;
                            entity->timer = 0x30;
                            /* fall through */
                        case 1:
                            if (--entity->timer == 0xFF)
                                entity->stage++;
                            break;
                        case 2:
                        {
                            struct EwramData_EntityData *player = gEwramData->playerState.player;
                            if ((s16)player->posX.part16.integer <= 160)
                            {
                                player->posX.part16.integer = 160;
                                entity->stage++;
                            }
                            else
                                input |= 0x20;
                            break;
                        }
                        case 3:
                            event->textWindowPause = 0;
                            break;
                    }
                    break;
                case 1:
                    switch (entity->stage)
                    {
                        case 0:
                            event->textWindowPause = 1;
                            entity->velX.whole = 0x8000;
                            entity->flipX = gEwramData->playerState.player->flipX;
                            if (entity->animScriptIndex != 0)
                                Anim_StartIndexed(entity, 0, 0, 1);
                            entity->stage++;
                            /* fall through */
                        case 1:
                            if ((s16)entity->posX.part16.integer > 255)
                            {
                                Sound_Play(0x1040);
                                event->textWindowPause = 0;
                                entity->stage++;
                            }
                            else
                            {
                                struct EwramData_EntityData *player;
                                entity->posX.whole += entity->velX.whole;
                                player = gEwramData->playerState.player;
                                if (entity->posX.whole > player->posX.whole)
                                    player->flipX = 0;
                                else
                                    player->flipX = 1;
                            }
                            break;
                        case 2:
                            break;
                    }
                    break;
            }
            break;
        case 0x80:
            Music_UpdateRoom(event);
            GameFlag_Set(0x2E);
            Interaction_End(entity, 1);
            break;
    }
    if (entity->state != 0x80)
        Anim_Update(entity);
    return input;
}

/* Draw horizontally offset copies, restoring the original position. */
void EventSceneHammerTalk_DrawOffsetCopies(struct EwramData_EntityData *entity)
{
    s16 x = entity->posX.part16.integer;
    s32 count = 1;
    s32 i;

    if (gEwramData->gameState.bossEventFlags & 0x10)
        count = 2;
    if (gEwramData->gameState.bossEventFlags & 2)
        count++;
    for (i = 0; i < count; i++)
    {
        if (i == 0)
            entity->posX.part16.integer = (s16)x - 16;
        else if (i == 1)
            entity->posX.part16.integer = (s16)x + 16;
        else if (i == 2)
            entity->posX.part16.integer = (s16)x - 8;
        else
            entity->posX.part16.integer = (s16)x + 8;
        Sprite_Draw(entity);
    }
    entity->posX.part16.integer = x;
}

/* Create the actor and its companion effect once both event flags exist. */
/**
 * @brief Init of event scene variant 26 (Object 0x20 spawnParam0 = 26,
 * sEventSceneCreateFuncs[26]).
 */
void EventSceneHammerTalk_Init(struct EwramData_EntityData *entity)
{
    if (!GameFlag_GetMask(0x20) || !GameFlag_GetMask(0x2E) || !Entity_SetupFromObjectTable(entity, 3))
        entity->deleteRequested = 1;
    else
    {
        if ((RandomNumberGenerator() & 1) == 0)
            Anim_StartIndexed(entity, 1, 0, 1);
        else
            Anim_StartIndexed(entity, 2, 0, 1);
        Anim_Update(entity);
        entity->flipX = 1;
        entity->spriteLayer = gEwramData->playerState.player->spriteLayer + 1;
        entity->ext.event26.actor = Entity_CreateFromObjectTable(3);
        if (entity->ext.event26.actor != NULL)
        {
            Anim_StartIndexed(entity->ext.event26.actor, 4, 0, 0);
            Anim_Update(entity->ext.event26.actor);
            entity->ext.event26.actor->posX.part16.integer = entity->posX.part16.integer;
            entity->ext.event26.actor->posY.part16.integer = entity->posY.part16.integer;
            entity->ext.event26.actor->spriteLayer = entity->spriteLayer + 1;
            entity->ext.event26.actor->drawFunc = EventSceneHammerTalk_DrawOffsetCopies;
            entity->ext.event26.actor->updateFunc = EntityUpdateNothing;
        }
    }
}

/* Select the next conversation from event flags when the player presses
 * an interaction button inside the actor's 64 x 64 box. */
/**
 * @brief Update of event scene variant 26 (sEventSceneUpdateFuncs[26]); returns the
 * player input mask.
 */
s32 EventSceneHammerTalk_Update(struct EwramData_EntityData *entity)
{
    s32 result = 0;
    struct EwramData *ewram = gEwramData;
    u16 input = ewram->inputData.newInput;

    switch (entity->state)
    {
        case 0:
            ewram->gameState.stateFlags &= ~0x200;
            entity->state++;
            entity->stage = 0;
            entity->subState = 0;
            /* fall through */
        case 1:
            if (Entity_IsInRect(gEwramData->playerState.player,
                            (s16)(entity->posX.part16.integer - 32),
                            (s16)(entity->posY.part16.integer - 32), 64, 64)
                && (input & 0x42) && gEwramData->gameState.timeStopTimer == 0)
            {
                s32 facing;
                struct PlayerState *actors = &gEwramData->playerState;
                actors->player->velX.whole = 0;
                actors->player->accelX.whole = 0;
                if (actors->player->velY.whole < 0)
                    actors->player->velY.whole = 0;
                entity->ext.event26.textId = 0x2D;
                if (GameFlag_Is3ASet())
                {
                    if (!GameFlag_GetMask(0x32))
                    {
                        entity->ext.event26.textId = 0x2C;
                        GameFlag_Set(0x32);
                    }
                }
                else if (GameFlag_GetMask(0x26))
                {
                    if (!GameFlag_GetMask(0x31))
                    {
                        entity->ext.event26.textId = 0x2B;
                        GameFlag_Set(0x31);
                    }
                }
                else if (GameFlag_GetMask(0x25))
                {
                    if (!GameFlag_GetMask(0x30))
                    {
                        entity->ext.event26.textId = 0x2A;
                        GameFlag_Set(0x30);
                    }
                }
                else if (GameFlag_GetMask(0x2E))
                {
                    if (!GameFlag_GetMask(0x2F))
                    {
                        entity->ext.event26.textId = 0x29;
                        GameFlag_Set(0x2F);
                    }
                }
                gEwramData->gameState.stateFlags |= 1;
                TextWindow_Begin(entity->ext.event26.textId);
                facing = 0;
                if (entity->posX.whole < gEwramData->playerState.player->posX.whole)
                    facing = 1;
                entity->flipX = facing;
                entity->state++;
                entity->stage = 0;
                entity->subState = 0;
                Sound_StopTracked(0x26);
                Music_SetCurrent(0x1E);
                if (entity->animScriptIndex == 2)
                    Anim_StartIndexed(entity, 3, 0, 1);
            }
            break;
        case 2:
            if (TextWindow_Update())
            {
                if (entity->animScriptIndex == 3)
                    Anim_StartIndexed(entity, 2, 0, 1);
                TextWindow_Close();
                entity->state = 3;
                Shop_Open();
            }
            break;
        case 3:
        {
            u16 *song;
            struct EwramData *data;
            Interaction_End(entity, 0);
            Sound_Play(0x1041);
            data = gEwramData;
            song = &data->musicOverrideSong;
            *song = 0x26;
            data->gameState.currentSong = 0;
            PlaySong(0x26);
            entity->ext.event26.textId = 0x2D;
            entity->state = 1;
            break;
        }
    }
    Anim_Update(entity);
    return result;
}

/* Nonzero while the player is in a state that prevents interaction. */
s32 Player_IsInteractionBlocked(void)
{
    struct EwramData_EntityData *player = gEwramData->playerState.player;
    if (player->state == 0)
    {
        s32 speed = player->velX.whole;
        if (speed < 0)
            speed = -speed;
        if (speed <= 0x8000)
            return player->ext.player.actionFlags & 0x30010000;
    }
    return 1;
}

/* Load actor entry 0 after event flag 0x20, one layer above the player. */
/**
 * @brief Init of event scene variant 27 (Object 0x20 spawnParam0 = 27,
 * sEventSceneCreateFuncs[27]).
 */
void EventSceneMinaTalk_Init(struct EwramData_EntityData *entity)
{
    if (!GameFlag_GetMask(0x20) || !Entity_SetupFromObjectTable(entity, 0))
        entity->deleteRequested = 1;
    else
    {
        Anim_StartIndexed(entity, 0, 0, 1);
        Anim_Update(entity);
        entity->flipX = 0;
        entity->spriteLayer = gEwramData->playerState.player->spriteLayer + 1;
        entity->ext.event27.heal = 1;
    }
}

/**
 * @brief Update of event scene variant 27 (sEventSceneUpdateFuncs[27]); returns the
 * player input mask. Town character update (returns 0x10 while it walks
 * away): waits for the player to press up/talk inside its 0x30 x 0x40
 * box, stops the player, picks the conversation (EventSceneMinaTalk_PickConversation, the
 * Hammer branch with unk_4FC set, or 0x44 / EventSceneMinaTalk_SelectScript36), faces away
 * from the player and runs it (TextWindow_Begin); after the text
 * (TextWindow_Update) restores the area music 0x26 and either returns to
 * waiting or (state 3) walks off until it is 0x30 px past the player.
 */
s32 EventSceneMinaTalk_Update(struct EwramData_EntityData *entity)
{
    s32 result = 0;
    struct EwramData *ewram = gEwramData;
    u16 input = ewram->inputData.newInput;

    switch (entity->state)
    {
    case 0:
        ewram->gameState.stateFlags &= ~0x200;
        Music_StartOverride(0x26);
        entity->state++;
        entity->stage = result;
        entity->subState = result;
    case 1:
        if (Entity_IsInRect(gEwramData->playerState.player, (s16)(entity->posX.part16.integer - 0x10),
                         (s16)(entity->posY.part16.integer - 0x20), 0x30, 0x40)
            && (input & 0x42) && gEwramData->gameState.timeStopTimer == 0)
        {
            s32 facing;
            struct EwramData *data;
            struct PlayerState *actors;
            gEwramData->gameState.stateFlags |= 0x200;
            actors = &gEwramData->playerState;
            actors->player->velX.whole = 0;
            actors->player->accelX.whole = 0;
            if (actors->player->velY.whole < 0)
                actors->player->velY.whole = 0;
            if (entity->ext.event27.heal)
            {
                Player_RestoreHPAndMP();
                PlaySong(0x1B6);
                EventSceneMinaTalk_PickConversation(entity);
            }
            else if (GameFlag_Is3ASet())
            {
                Conversation_Reset();
                entity->ext.event27.textId = 0x44;
            }
            else
                EventSceneMinaTalk_SelectScript36(entity);
            facing = 0;
            data = gEwramData;
            if (entity->posX.whole < data->playerState.player->posX.whole)
                facing = 1;
            entity->flipX = facing;
            entity->flipX = 1 - entity->flipX;
            data->gameState.stateFlags |= 1;
            TextWindow_Begin(entity->ext.event27.textId);
            entity->state++;
            entity->stage = 0;
            entity->subState = 0;
            Music_SetCurrent(0x1D);
            Sound_Play(0x1040);
        }
        break;
    case 2:
        if (TextWindow_Update())
        {
            struct EwramData *data;
            u16 *song;
            s32 zero;
            if (entity->ext.event27.heal)
                entity->ext.event27.heal = result;
            PlaySong(0x1B5);
            data = gEwramData;
            song = &data->musicOverrideSong;
            zero = 0;
            *song = 0x26;
            data->gameState.currentSong = result;
            PlaySong(0x26);
            Sound_Play(0x1041);
            if (GameFlag_Is3ASet())
                entity->state++;
            else
            {
                Interaction_End(entity, 0);
                entity->state = 1;
            }
            entity->stage = zero;
            entity->subState = zero;
        }
        break;
    case 3:
    {
        u32 step = entity->subState;
        if ((u8)step == 0)
        {
            entity->timer = 0x10;
            entity->subState = step + 1;
            entity->stage = result;
        }
        else if ((u8)step == 1)
        {
            s32 facing = 0;
            struct PlayerState *actors = &ewram->playerState;
            if (entity->posX.whole < actors->player->posX.whole)
                facing = 1;
            entity->flipX = facing;
            entity->flipX = 1 - entity->flipX;
            result = 0x10;
            if (entity->posX.whole + 0x300000 < actors->player->posX.whole)
            {
                entity->timer = 0;
                entity->subState++;
                entity->stage = 0;
            }
        }
        else
        {
            Interaction_End(entity, 0);
            entity->state = 1;
            entity->stage = result;
            entity->subState = result;
        }
        break;
    }
    }
    return result;
}

struct ConversationState {
    u16 script;
    u16 step;
    u16 group;
    u16 entry;
};

static inline void NextEntry(struct ConversationState *state, u8 *index, struct TalkTable *table, s32 group)
{
    state->entry = table->entry[group][*index & 0xF];
    if ((++*index & 0xF) == 8)
        *index = group << 4;
}

/**
 * @brief 66308 | Pick the conversation of a town character into unk_4F8:
 * story scripts first (flags 0x36/0x3B with three inventory items, 0x26,
 * 0x2E, 0x21, 0x20 via GameFlag_GetMask / GameFlag_TestAndSet). Otherwise small talk
 * 0x45: group 0..6 by progress, entry from the 7 x 8 table sEventSceneMinaTalkTalkTable
 * cycling through the low nibble of pad_133E8[0] (restarted when the group
 * changes), skipping entries 7 / 9 while flags 0x39 / 0x3D are set.
 */
void EventSceneMinaTalk_PickConversation(struct EwramData_EntityData *entity)
{
    entity->ext.event27.textId = 0;
    if (GameFlag_Is3ASet())
    {
        if (!GameFlag_TestAndSet(0x36))
            entity->ext.event27.textId = 0x43;
    }
    else
    {
        struct EwramData *ew = gEwramData;
        s32 done = 0;

        if (!GameFlag_GetMask(0x3B) && ew->playerStatus.itemInventory[0x1A] && ew->playerStatus.itemInventory[0x1B]
            && ew->playerStatus.itemInventory[0x1C])
        {
            GameFlag_Set(0x3B);
            done = 1;
        }
        if (done)
        {
            struct ConversationState *state = (struct ConversationState *)&gEwramData->sharedScratchBuffer[0x500];
            entity->ext.event27.textId = 0x45;
            state->script = 0x45;
            state->step = 0;
            state->group = 4;
            state->entry = 8;
        }
        else if (GameFlag_GetMask(0x26))
        {
            if (!GameFlag_TestAndSet(0x35))
                entity->ext.event27.textId = 0x30;
            else if (!(RandomNumberGenerator() & 1))
            {
                entity->ext.event27.textId = 0x31;
                Conversation_Setup(0x31, 0, 0);
            }
        }
        else if (GameFlag_GetMask(0x2E))
        {
            if (!GameFlag_TestAndSet(0x34))
                entity->ext.event27.textId = 0x2F;
            else if (!GameFlag_TestAndSet(0x37))
                entity->ext.event27.textId = 0x53;
        }
        else if (GameFlag_GetMask(0x21))
        {
            if (!GameFlag_TestAndSet(0x37))
                entity->ext.event27.textId = 0x53;
        }
        else if (GameFlag_GetMask(0x20))
        {
            if (!GameFlag_TestAndSet(0x33))
                entity->ext.event27.textId = 0x2E;
        }
    }
    if (entity->ext.event27.textId == 0)
    {
        struct PlayerStatus *save = &gEwramData->playerStatus;
        s32 group = 0;
        struct TalkTable table = sEventSceneMinaTalkTalkTable;
        s32 last = save->pad_133E8[0] >> 4;
        struct ConversationState *state = (struct ConversationState *)&gEwramData->sharedScratchBuffer[0x500];

        entity->ext.event27.textId = 0x45;
        state->script = 0x45;
        state->step = group;
        state->group = 4;
        state->entry = RandomNumberGenerator() % 9;
        if (GameFlag_Is3ASet())
        {
            group = 5;
            state->group = 6;
        }
        else if (GameFlag_GetMask(0x21))
        {
            if (!(gEwramData->gameState.bossEventFlags & 0x10))
                group = 1;
            else if (!GameFlag_GetMask(0x24))
            {
                if (GameFlag_GetMask(0x3C))
                {
                    s32 r = GameFlag_GetMask(0x3D);
                    group = 3;
                    if (!r)
                        group = 6;
                }
                else
                    group = 2;
            }
            else
            {
                u16 flags = gEwramData->gameState.bossEventFlags;
                if (flags & 2)
                {
                    s32 m = 0x40;
                    m &= flags;
                    group = 4;
                    if (m)
                        goto chosen;
                }
                group = 3;
            }
        }
    chosen:
        if (group != last)
            save->pad_133E8[0] = group << 4;
        NextEntry(state, &save->pad_133E8[0], &table, group);
        if (GameFlag_GetMask(0x39) && state->entry == 7)
        {
            do
            {
                NextEntry(state, &save->pad_133E8[0], &table, group);
            } while (state->entry == 7);
        }
        if (GameFlag_GetMask(0x3D) && state->entry == 9)
        {
            do
            {
                NextEntry(state, &save->pad_133E8[0], &table, group);
            } while (state->entry == 9);
        }
    }
}

/* Conversation text setup, decompiled from assembly.
 * The temporary state occupies 0x138F4 in the shared 0x133F4 buffer. */

/* Load the chosen text on step 0, then the closing text on step 1. */
/**
 * @brief For steps 0/1 load a selected entry or closing text and advance the step; groups 4/6 use the supplied entry, others randomize; return one when loaded.
 */
s32 Conversation_LoadNextText(void)
{
    s32 result = 0;
    struct EwramData *ewram = gEwramData;
    struct GameState *event = &ewram->gameState;
    struct ConversationState *state = (struct ConversationState *)&ewram->sharedScratchBuffer[0x500];
    u16 *step = &state->step;
    u16 group = state->group;
    u16 entry = state->entry;

    if (*step <= 1)
    {
        struct ConversationTextTable *table = &sConversationTextTable;
        /* Separate base and index bindings preserve the original table addressing. */
        u32 address = (u32)table;
        u32 offset = group * 4;
        struct ConversationTextList *list;
        address += 4;
        offset += address;
        list = *(struct ConversationTextList **)offset;
        if (*step == 0)
        {
            u16 index;
            u16 *entries;
            u32 offset;
            if (group == 4 || group == 6)
                index = entry;
            else
                index = RandomNumberGenerator() % list->count;
            offset = index * 2;
            entries = list->entries;
            event->conversationText = Text_GetLocalizedEntry(*(u16 *)((u32)entries + offset));
        }
        else
            ewram->gameState.conversationText = Text_GetLocalizedEntry(list->closingText);
        (*step)++;
        result = 1;
    }
    return result;
}

/* Conversation text setup, decompiled from assembly.
 * The temporary state occupies 0x138F4 in the shared 0x133F4 buffer. */

/**
 * @brief Store script, zero step, group and entry in the four-halfword conversation state in the shared buffer.
 */
void Conversation_Setup(s32 script, s32 group, s32 entry)
{
    u16 *state = (u16 *)&gEwramData->sharedScratchBuffer[0x500];
    state[0] = script;
    state[1] = 0;
    state[2] = group;
    state[3] = entry;
}

/**
 * @brief Clear conversation script/group/entry and set step to 0xFF.
 */
void Conversation_Reset(void)
{
    struct ConversationState *state = (struct ConversationState *)&gEwramData->sharedScratchBuffer[0x500];
    state->script = 0;
    state->step = 0xFF;
    state->group = 0;
    state->entry = 0;
}

/**
 * @brief Select conversation script 0x36 with group 1, 2 or 3 depending on
 * event flags 0x2E and 0x26.
 */
void EventSceneMinaTalk_SelectScript36(struct EwramData_EntityData *entity)
{
    struct ConversationState *state = (struct ConversationState *)&gEwramData->sharedScratchBuffer[0x500];
    s32 zero = 0;
    entity->ext.event27.textId = 0x36;
    state->script = 0x36;
    state->step = zero;
    state->entry = zero;
    if (GameFlag_GetMask(0x2E))
    {
        if (GameFlag_GetMask(0x26))
            state->group = 3;
        else
            state->group = 2;
    }
    else
        state->group = 1;
}

/* Interaction handler for the actor whose conversation uses group 5. */
/**
 * @brief Update of event scene variant 28 (sEventSceneUpdateFuncs[28]); returns the
 * player input mask.
 */
s32 EventSceneUnconsciousYoko_Update(struct EwramData_EntityData *entity)
{
    s32 result = 0;
    struct EwramData *ewram = gEwramData;
    u16 input = ewram->inputData.newInput;

    switch (entity->state)
    {
        case 0:
            ewram->gameState.stateFlags &= ~0x200;
            entity->state++;
            entity->stage = 0;
            entity->subState = 0;
            /* fall through */
        case 1:
            if (Entity_IsInRect(gEwramData->playerState.player,
                            (s16)(entity->posX.part16.integer - 24),
                            (s16)(entity->posY.part16.integer - 32), 36, 64)
                && (input & 0x42) && gEwramData->gameState.timeStopTimer == 0)
            {
                struct PlayerState *actors = &gEwramData->playerState;
                actors->player->velX.whole = 0;
                actors->player->accelX.whole = 0;
                if (actors->player->velY.whole < 0)
                    actors->player->velY.whole = 0;
                EventSceneUnconsciousYoko_PickScript(entity);
                gEwramData->gameState.stateFlags |= 1;
                TextWindow_Begin(entity->ext.event28.textId);
                entity->state++;
                entity->stage = 0;
                entity->subState = 0;
            }
            break;
        case 2:
            if (TextWindow_Update())
            {
                GameFlag_Set(0x38);
                Interaction_End(entity, 0);
                entity->state = 0;
                entity->stage = 0;
                entity->subState = 0;
            }
            break;
    }
    return result;
}

/* Occasionally select script 0x59, otherwise initialize script 0x54. */
void EventSceneUnconsciousYoko_PickScript(struct EwramData_EntityData *entity)
{
    entity->ext.event28.textId = 0;
    if (GameFlag_Is3ASet() && (RandomNumberGenerator() & 15) <= 3)
        entity->ext.event28.textId = 0x59;
    if (entity->ext.event28.textId == 0)
    {
        struct ConversationState *state = (struct ConversationState *)&gEwramData->sharedScratchBuffer[0x500];
        entity->ext.event28.textId = 0x54;
        state->script = 0x54;
        state->step = 0;
        state->group = 5;
        state->entry = 0;
    }
}

/**
 * @brief Init of event scene variant 28 (Object 0x20 spawnParam0 = 28,
 * sEventSceneCreateFuncs[28]).
 */
void EventSceneUnconsciousYoko_Init(struct EwramData_EntityData *entity)
{
    if (!GameFlag_GetMask(0x20))
        entity->deleteRequested = 1;
    else if (!GameFlag_GetMask(0x26))
        entity->deleteRequested = 1;
    else if (!Entity_SetupFromObjectTable(entity, 2))
        EntityDelete(entity);
    else
    {
        Anim_StartIndexed(entity, 6, 0, 1);
        Anim_Update(entity);
        entity->flipX = 1;
    }
}
