/* EventScene03_Update: event script step (room event state machine); matched. */
#include "gba.h"
#include "event_scene03.h"
#include "code_08009A0.h"
#include "code_08013960.h"
#include "code_08032444.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void TextWindow_Begin(s32);
extern s32 TextWindow_Update(void);
extern void Interaction_End(struct EwramData_EntityData *, s32);
extern void GameFlag_Set(s32 flag);
extern void Music_StartOverride(s32 song);
extern s32 Entity_IsInRect(struct EwramData_EntityData *, s16, s16, u16, u16);
extern void Player_StartScriptedAnimation(s32, s32, s32, s32);
extern s32 EventScene03_ScrollCamera(struct EwramData_EntityData *);
extern s32 EventScene03_UpdateMosaic(struct EwramData_EntityData *);
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
                u16 f = EVENT03_PROGRESS(entity);
                s32 k = 4;
                z = 0;
                k |= f;
                EVENT03_PROGRESS(entity) = k;
            }
            Music_StartOverride(0x26);
            goto set10;
        }
        ret = EventScene03_RunSequence(entity);
        break;
    case 10:
        if (Entity_IsInRect(gEwramData->playerState.player, EVENT03_ACTOR1(entity)->posX.part16.integer - 0x20, EVENT03_ACTOR1(entity)->posY.part16.integer - 0x20, 0x40, sz = 0x40))
        {
            found = 1;
            convo = 0xE;
        }
        else if (Entity_IsInRect(gEwramData->playerState.player, EVENT03_ACTOR0(entity)->posX.part16.integer - 0x20, EVENT03_ACTOR0(entity)->posY.part16.integer - 0x20, 0x40, sz))
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
        other = EVENT03_ACTOR1(entity);
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
