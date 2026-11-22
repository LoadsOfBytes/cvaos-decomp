/* Conversation text setup, decompiled from assembly.
 * The temporary state occupies 0x138F4 in the shared 0x133F4 buffer. */
#include "structs/ewram.h"
#include "code_08009A0.h"
#include "code_08011DD0.h"
#include "code/code_08040A38.h"

struct ConversationState {
    u16 script;
    u16 step;
    u16 group;
    u16 entry;
};

struct ConversationTextList {
    u16 unk_0;
    u16 closingText;
    u16 count;
    u16 entries[1];
};

struct ConversationTextTable {
    u32 unk_0;
    struct ConversationTextList *lists[1];
};

extern struct ConversationTextTable sConversationTextTable;

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

