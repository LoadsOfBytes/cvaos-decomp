#include "structs/ewram.h"
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code_0800CB00.h"
#include "code_080D73B8.h"
#include "code_08011DD0.h"
#include "code_08013960.h"
extern s32 Entity_IsInRect(struct EwramData_EntityData *, s16, s16, u16, u16);
extern void TextWindow_Begin(s32);
extern s32 TextWindow_Update(void);
extern void Sound_Play(u16);
extern void Interaction_End(struct EwramData_EntityData *, s32);
extern u32 GameFlag_Is3ASet(void);
extern void Player_RestoreHPAndMP(void);
extern void EventScene27_PickConversation(struct EwramData_EntityData *entity);
extern void Conversation_Reset(void);
extern void EventScene27_SelectScript36(struct EwramData_EntityData *entity);

/**
 * @brief Update of event scene variant 27 (sEventSceneUpdateFuncs[27]); returns the
 * player input mask. Town character update (returns 0x10 while it walks
 * away): waits for the player to press up/talk inside its 0x30 x 0x40
 * box, stops the player, picks the conversation (EventScene27_PickConversation, the
 * Hammer branch with unk_4FC set, or 0x44 / EventScene27_SelectScript36), faces away
 * from the player and runs it (TextWindow_Begin); after the text
 * (TextWindow_Update) restores the area music 0x26 and either returns to
 * waiting or (state 3) walks off until it is 0x30 px past the player.
 */
s32 EventScene27_Update(struct EwramData_EntityData *entity)
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
        if (Entity_IsInRect(gEwramData->playerState.player, entity->posX.part16.integer - 0x10,
                         entity->posY.part16.integer - 0x20, 0x30, 0x40)
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
                EventScene27_PickConversation(entity);
            }
            else if (GameFlag_Is3ASet())
            {
                Conversation_Reset();
                entity->ext.event27.textId = 0x44;
            }
            else
                EventScene27_SelectScript36(entity);
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
