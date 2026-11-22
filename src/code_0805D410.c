/* EventScene03_RunSequence, decompiled from the original assembly: event script with
 * nine steps (subState) and sub-steps (stage); returns flags 2 / 0x80.
 * Each step advance is written out (no shared label) so only the final
 * stage++ is cross-jumped; the decrement in step 5/3 goes through an s32
 * temporary; step 8 has an empty case 2. */
#include "code_08009A0.h"
#include "event_scene03.h"
#include "code_080D73B8.h"
#include "code_0800CB00.h"
#include "code_08013960.h"
#include "code/code_08039340.h"
#include "main.h"
#include "structs/ewram.h"

extern s32 EventScene03_ScrollCamera(struct EwramData_EntityData *entity);
extern void EventScene03_SpawnPartsA(struct EwramData_EntityData *entity);
extern void EventScene03_SpawnPartsB(struct EwramData_EntityData *entity);
extern struct EwramData_EntityData *EventScene03_SpawnSoulOrb(s32 x, s32 y, s32 variant, s32 index, s32 mode);
extern void Effect_SpawnAt(s32, s32, s32);
extern void EventScene03_SpawnPartDebris(struct EwramData_EntityData *owner);
extern void Sound_Play(u16);
extern void Player_StartScriptedAnimation(s32, s32, s32, s32);

#define STEP (*(u8 *)&entity->unk_51C)

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
            struct EwramData_EntityData *p = EVENT03_ACTOR0(entity);
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
            EVENT03_ACTOR0(entity)->flipX = 0;
            EVENT03_ACTOR1(entity)->flipX = 1;
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
            EVENT03_PROGRESS(entity) |= 8;
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
            p = EVENT03_ACTOR0(entity);
            if (p->animScriptIndex != 5)
                Anim_StartIndexed(p, 5, 0, 0);
            PlaySong(0x1B4);
            q = EVENT03_ACTOR1(entity);
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
                EVENT03_PROGRESS(entity) |= 0x10;
                PlaySong(0x1DF);
                Anim_StartIndexed(EVENT03_ACTOR1(entity), 2, 0, 0);
                entity->stage++;
                break;
            }
            break;
        case 3:
            if (EVENT03_ACTOR1(entity)->animDone)
            {
                EVENT03_PROGRESS(entity) |= 2;
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
                q = EVENT03_ACTOR1(entity);
                if (q->animScriptIndex != 3)
                    Anim_StartIndexed(q, 3, 0, 0);
            }
            break;
        case 5:
            if (EVENT03_ACTOR1(entity)->animDone)
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
                EventScene03_SpawnSoulOrb((s16)EVENT03_ACTOR8(entity)->posX.part16.integer, (s16)EVENT03_ACTOR8(entity)->posY.part16.integer, 0, 0, 0);
                EVENT03_ACTOR8(entity)->hidden = 1;
                o = EVENT03_ACTOR8(entity);
                Effect_SpawnAt(o->posX.whole, o->posY.whole, o->spriteLayer - 1);
                EventScene03_SpawnPartDebris(EVENT03_ACTOR8(entity));
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
