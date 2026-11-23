/* Conversation text setup, decompiled from assembly.
 * The temporary state occupies 0x138F4 in the shared 0x133F4 buffer. */
#include "structs/ewram.h"
#include "code_08009A0.h"
#include "code_08011DD0.h"
#include "code/code_08040A38.h"
#include "code/code_08039340.h"

struct ConversationState {
    u16 script;
    u16 step;
    u16 group;
    u16 entry;
};

extern u32 GameFlag_Is3ASet(void);
extern s32 Entity_SetupFromObjectTable(struct EwramData_EntityData *, s32);
extern void TextWindow_Begin(s32);
extern s32 TextWindow_Update(void);
extern void Interaction_End(struct EwramData_EntityData *, s32);
extern s32 Entity_IsInRect(struct EwramData_EntityData *, s16, s16, u16, u16);
void EventScene28_PickScript(struct EwramData_EntityData *entity);

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
void EventScene27_SelectScript36(struct EwramData_EntityData *entity)
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
s32 EventScene28_Update(struct EwramData_EntityData *entity)
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
                            entity->posX.part16.integer - 24,
                            entity->posY.part16.integer - 32, 36, 64)
                && (input & 0x42) && gEwramData->gameState.timeStopTimer == 0)
            {
                struct PlayerState *actors = &gEwramData->playerState;
                actors->player->velX.whole = 0;
                actors->player->accelX.whole = 0;
                if (actors->player->velY.whole < 0)
                    actors->player->velY.whole = 0;
                EventScene28_PickScript(entity);
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
void EventScene28_PickScript(struct EwramData_EntityData *entity)
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
 * @brief Init of event scene variant 28 (Object 0x20 unk_514 = 28,
 * sEventSceneCreateFuncs[28]).
 */
void EventScene28_Init(struct EwramData_EntityData *entity)
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
