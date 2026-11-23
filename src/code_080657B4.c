/* Event and particle helpers, decompiled from the original assembly. */
#include "structs/ewram.h"
#include "event_scene30.h"
#include "event_scene_entity.h"
#include "player_entity.h"
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code_0800CB00.h"
#include "code_080D73B8.h"
#include "code_08011DD0.h"
#include "code_08013960.h"

struct ParticleFrame {
    u8 *data;
    u16 size;
    u16 pad;
};

struct ParticleFrames {
    u16 count;
    u16 destination;
    struct ParticleFrame *frames;
};

extern struct ParticleFrames sEventScene30ParticleFrames;
extern s32 Entity_IsInRect(struct EwramData_EntityData *, s16, s16, u16, u16);
extern s32 Entity_SetupFromObjectTable(struct EwramData_EntityData *, s32);
extern void TextWindow_Begin(s32);
extern s32 TextWindow_Update(void);
extern void Sound_Play(u16);
extern void Interaction_End(struct EwramData_EntityData *, s32);
extern struct EwramData_EntityData *Entity_CreateFromObjectTable(s32);
extern u32 GameFlag_Is3ASet(void);
extern void TextWindow_Close(void);
extern void Shop_Open(void);

/* End this entity unless event flag 0x4000 is set; otherwise advance the
 * event flags and play song 0x1000. */
/**
 * @brief Init of event scene variant 24 (Object 0x20 unk_514 = 24,
 * sEventSceneCreateFuncs[24]).
 */
void EventScene24_Init(struct EwramData_EntityData *entity)
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
void EventScene30_UploadFrame(void)
{
    struct ParticleFrames *frames = &sEventScene30ParticleFrames;
    s32 index = (gEwramData->gameModeCounter >> 1) % frames->count;
    struct ParticleFrame *frame = &frames->frames[index];
    BgPalette_LoadRange(frame->data, frame->size,
                1, frames->destination);
}

/**
 * @brief Delete the flash at the end of its animation.
 */
void EventScene30Flash_Update(struct EwramData_EntityData *entity)
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
void EventScene30Effect_UpdateBallistic(struct EwramData_EntityData *entity)
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
void EventScene30Effect_UpdateSpinning(struct EwramData_EntityData *entity)
{
    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
    entity->velX.whole += entity->accelX.whole;
    entity->velY.whole += entity->accelY.whole;
    EVENT30_EFFECT_ANGLE(entity) += EVENT30_EFFECT_SPIN(entity);
    ObjAffine_Set(EVENT30_EFFECT_ANGLE(entity), EVENT30_EFFECT_SCALE(entity),
                EVENT30_EFFECT_SCALE(entity), 0, 0, entity->affineIndex);
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
void EventScene30Effect_UpdateSpinningKeepSlot(struct EwramData_EntityData *entity)
{
    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
    entity->velX.whole += entity->accelX.whole;
    entity->velY.whole += entity->accelY.whole;
    EVENT30_EFFECT_ANGLE(entity) += EVENT30_EFFECT_SPIN(entity);
    ObjAffine_Set(EVENT30_EFFECT_ANGLE(entity), EVENT30_EFFECT_SCALE(entity),
                EVENT30_EFFECT_SCALE(entity), 0, 0, entity->affineIndex);
    if (!Entity_IsInRect(entity, -128, -64, 496, 288))
        entity->deleteRequested = 1;
}

/* Event actor: skip when flag 0x2E is already set. Otherwise load its
 * graphics and position it relative to the background and player. */
/**
 * @brief Init of event scene variant 25 (Object 0x20 unk_514 = 25,
 * sEventSceneCreateFuncs[25]).
 */
void EventScene25_Init(struct EwramData_EntityData *entity)
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
s32 EventScene25_Update(struct EwramData_EntityData *entity)
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
void EventScene26_DrawOffsetCopies(struct EwramData_EntityData *entity)
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
 * @brief Init of event scene variant 26 (Object 0x20 unk_514 = 26,
 * sEventSceneCreateFuncs[26]).
 */
void EventScene26_Init(struct EwramData_EntityData *entity)
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
        EVENT26_ACTOR(entity) = Entity_CreateFromObjectTable(3);
        if (EVENT26_ACTOR(entity) != NULL)
        {
            Anim_StartIndexed(EVENT26_ACTOR(entity), 4, 0, 0);
            Anim_Update(EVENT26_ACTOR(entity));
            EVENT26_ACTOR(entity)->posX.part16.integer = entity->posX.part16.integer;
            EVENT26_ACTOR(entity)->posY.part16.integer = entity->posY.part16.integer;
            EVENT26_ACTOR(entity)->spriteLayer = entity->spriteLayer + 1;
            EVENT26_ACTOR(entity)->drawFunc = (u32 *)EventScene26_DrawOffsetCopies;
            EVENT26_ACTOR(entity)->updateFunc = (u32 *)EntityUpdateNothing;
        }
    }
}

/* Select the next conversation from event flags when the player presses
 * an interaction button inside the actor's 64 x 64 box. */
/**
 * @brief Update of event scene variant 26 (sEventSceneUpdateFuncs[26]); returns the
 * player input mask.
 */
s32 EventScene26_Update(struct EwramData_EntityData *entity)
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
                            entity->posX.part16.integer - 32,
                            entity->posY.part16.integer - 32, 64, 64)
                && (input & 0x42) && gEwramData->gameState.timeStopTimer == 0)
            {
                s32 facing;
                struct PlayerState *actors = &gEwramData->playerState;
                actors->player->velX.whole = 0;
                actors->player->accelX.whole = 0;
                if (actors->player->velY.whole < 0)
                    actors->player->velY.whole = 0;
                EVENT26_TEXT_ID(entity) = 0x2D;
                if (GameFlag_Is3ASet())
                {
                    if (!GameFlag_GetMask(0x32))
                    {
                        EVENT26_TEXT_ID(entity) = 0x2C;
                        GameFlag_Set(0x32);
                    }
                }
                else if (GameFlag_GetMask(0x26))
                {
                    if (!GameFlag_GetMask(0x31))
                    {
                        EVENT26_TEXT_ID(entity) = 0x2B;
                        GameFlag_Set(0x31);
                    }
                }
                else if (GameFlag_GetMask(0x25))
                {
                    if (!GameFlag_GetMask(0x30))
                    {
                        EVENT26_TEXT_ID(entity) = 0x2A;
                        GameFlag_Set(0x30);
                    }
                }
                else if (GameFlag_GetMask(0x2E))
                {
                    if (!GameFlag_GetMask(0x2F))
                    {
                        EVENT26_TEXT_ID(entity) = 0x29;
                        GameFlag_Set(0x2F);
                    }
                }
                gEwramData->gameState.stateFlags |= 1;
                TextWindow_Begin(EVENT26_TEXT_ID(entity));
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
            EVENT26_TEXT_ID(entity) = 0x2D;
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
            return PLAYER_ACTION_FLAGS(player) & 0x30010000;
    }
    return 1;
}

/* Load actor entry 0 after event flag 0x20, one layer above the player. */
/**
 * @brief Init of event scene variant 27 (Object 0x20 unk_514 = 27,
 * sEventSceneCreateFuncs[27]).
 */
void EventScene27_Init(struct EwramData_EntityData *entity)
{
    if (!GameFlag_GetMask(0x20) || !Entity_SetupFromObjectTable(entity, 0))
        entity->deleteRequested = 1;
    else
    {
        Anim_StartIndexed(entity, 0, 0, 1);
        Anim_Update(entity);
        entity->flipX = 0;
        entity->spriteLayer = gEwramData->playerState.player->spriteLayer + 1;
        EVENT27_HEAL(entity) = 1;
    }
}

