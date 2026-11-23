#include "structs/ewram.h"
#include "code_08009A0.h"
#include "code_08011DD0.h"

struct ConversationState {
    u16 script;
    u16 step;
    u16 group;
    u16 entry;
};

struct TalkTable { u8 entry[7][8]; };

extern u32 GameFlag_Is3ASet(void);
extern void Conversation_Setup(s32 script, s32 group, s32 entry);
extern struct TalkTable sEventScene27TalkTable;

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
 * 0x45: group 0..6 by progress, entry from the 7 x 8 table sEventScene27TalkTable
 * cycling through the low nibble of pad_133E8[0] (restarted when the group
 * changes), skipping entries 7 / 9 while flags 0x39 / 0x3D are set.
 */
void EventScene27_PickConversation(struct EwramData_EntityData *entity)
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
        struct TalkTable table = sEventScene27TalkTable;
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
