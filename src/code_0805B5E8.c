/* Small helpers 0x0805B5E8..0x0805B8F3 (player-relative checks, sound,
 * entity setup from the table sObjectTableGraphics). */
#include "code_08009A0.h"
#include "code_0800B700.h"
#include "code_0800CB00.h"
#include "code_08011DD0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void TextWindow_Begin(s32 textId);
extern void TextWindow_Close(void);
extern void Conversation_Reset(void);
extern struct EwramData_EntityData *Entity_CreateObject(u32 *updateFunc);
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *entity, u8 *resource, void *palette, u8 variant, void *sheet);

/* Twelve resource tuples in palette, sprite-sheet, graphics order. */
struct ObjectTableGraphics {
    void *palette;
    void *spriteSheet;
    u8 *graphics;
};

extern struct ObjectTableGraphics sObjectTableGraphics[12];

void ObjectTable_UpdateAnimation(struct EwramData_EntityData *entity);

/**
 * @brief 5B5E8 | Create an entity running ObjectTable_UpdateAnimation, set up from entry
 * kind of sObjectTableGraphics (layer 0x11, 0x53D bit 1); NULL on failure (the
 * entity is then deleted twice, as in the original).
 */
struct EwramData_EntityData *Entity_CreateFromObjectTable(s32 kind)
{
    struct ObjectTableGraphics table[12];
    struct EwramData_EntityData *child = Entity_CreateObject((u32 *)ObjectTable_UpdateAnimation);

    if (child == NULL)
        return NULL;
    memcpy(table, sObjectTableGraphics, sizeof(table));
    if (Entity_LoadGraphics(child, table[kind].graphics, table[kind].palette, 0, table[kind].spriteSheet))
    {
        child->spriteLayer = 0x11;
        child->scrollWithBg = 1;
        return child;
    }
    EntityDelete(child);
    EntityDelete(child);
    return NULL;
}

/**
 * @brief Clear interaction flags, restore the existing input gate, store the deletion request, close text for a negative signed state and reset rendering/conversation state.
 */
void Interaction_End(struct EwramData_EntityData *entity, s32 deleteEntity)
{
    struct EwramData *ewram;

    gEwramData->gameState.stateFlags &= ~1;
    gEwramData->gameState.stateFlags &= ~0x200;
    Game_SetMenusEnabled(1);
    entity->deleteRequested = deleteEntity;
    if ((s8)entity->state < 0)
        TextWindow_Close();
    ewram = gEwramData;
    ewram->unk_A074_4 = 0;
    ewram->bgControlMinWriteLevel = 0;
    Display_SetDefaultBlend();
    Conversation_Reset();
}

/**
 * @brief Reset interaction progress, set interaction flag 1, begin the text window and clear the existing input gate; return one.
 */
s32 Interaction_BeginText(s32 textId)
{
    struct GameState *gameState = &gEwramData->gameState;

    gameState->subModeStep = 0;
    gEwramData->gameState.stateFlags |= 1;
    TextWindow_Begin(textId);
    Game_SetMenusEnabled(0);
    return 1;
}

/**
 * @brief Clear interaction flag 1, set the existing input gate and mark caller record byte 4 as one; return the gate setter result.
 */
s32 Interaction_EndAndMarkRecord(u8 *record)
{
    s32 result;

    gEwramData->gameState.stateFlags &= ~1;
    result = Game_SetMenusEnabled(1);
    record[4] = 1;
    return result;
}

/**
 * @brief Advance animation only; used as the update callback by Entity_CreateFromObjectTable.
 */
void ObjectTable_UpdateAnimation(struct EwramData_EntityData *entity)
{
    Anim_Update(entity);
}

/**
 * @brief 5B754 | Set up entity from entry kind of sObjectTableGraphics
 * (layer 0x11, 0x53D bit 1); delete it and return 0 on failure.
 */
s32 Entity_SetupFromObjectTable(struct EwramData_EntityData *entity, s32 kind)
{
    struct ObjectTableGraphics table[12];

    memcpy(table, sObjectTableGraphics, sizeof(table));
    if (!Entity_LoadGraphics(entity, table[kind].graphics, table[kind].palette, 0, table[kind].spriteSheet))
    {
        EntityDelete(entity);
        return 0;
    }
    entity->spriteLayer = 0x11;
    entity->scrollWithBg = 1;
    return 1;
}

/**
 * @brief 5B7BC | 1 when the player (playerState.player) faces target: the player is
 * left of it and not flipped, or right of/level with it and flipped.
 */
u8 Player_IsFacingEntity(struct EwramData_EntityData *target)
{
    u8 facing = 0;
    struct EwramData_EntityData *player = gEwramData->playerState.player;

    if ((s16)player->posX.part16.integer < (s16)target->posX.part16.integer)
    {
        if (!player->flipX)
            facing = 1;
    }
    else
    {
        /* flipX != 0, read as the whole byte 0x53C (entity offset 0x58);
         * written out because only this form gives the original
         * negs/lsrs #31 sequence. */
        facing = (u32)-(u8)(((u8 *)player)[0x58] & 0x40) >> 31;
    }
    return facing;
}

/**
 * @brief 5B814 | Horizontal position of the player relative to the window
 * [centerX - outerWidth / 2, +outerWidth): 1 outside it; inside it -1 when within
 * innerWidth / 2 of centerX's left side, else 0. (Hypothesis from the
 * arithmetic.)
 */
s32 Player_GetZoneAroundX(u16 centerX, u16 innerWidth, u16 outerWidth)
{
    if ((u16)(gEwramData->playerState.player->posX.part16.integer - (centerX - (outerWidth >> 1))) < outerWidth)
    {
        if ((u16)(gEwramData->playerState.player->posX.part16.integer - (centerX - (innerWidth >> 1))) < innerWidth)
            return -1;
        return 0;
    }
    return 1;
}

/**
 * @brief Return 1 when event flag 0x3A is set.
 */
u32 GameFlag_Is3ASet(void)
{
    s32 var_0 = GameFlag_GetMask(0x3A);

    /* var_0 != 0, written out for the original negs/orrs/lsrs #31. */
    return (u32)(var_0 | -var_0) >> 31;
}

/**
 * @brief 5B878 | PlaySong(song).
 */
void Sound_Play(u16 song)
{
    PlaySong(song);
}

/**
 * @brief Init of event scene variant 30 (Object 0x20 unk_514 = 30,
 * sEventSceneCreateFuncs[30]). Set bits 0x80000, 0x20 and 1 of stateFlags, call
 * Hud_ClearGraphics, play song 0x1000 and place the player at (0x78,
 * 0x50) with zero velocity (integer parts) and 0x53C bit 5 set.
 */
void EventScene30_Init(void)
{
    gEwramData->gameState.stateFlags |= 0x80000;
    gEwramData->gameState.stateFlags |= 0x20;
    gEwramData->gameState.stateFlags |= 1;
    Hud_ClearGraphics();
    PlaySong(0x1000);
    gEwramData->playerState.player->posX.part16.integer = 0x78;
    gEwramData->playerState.player->posY.part16.integer = 0x50;
    gEwramData->playerState.player->velX.part16.integer = 0;
    gEwramData->playerState.player->velY.part16.integer = 0;
    gEwramData->playerState.player->hidden = 1;
}
