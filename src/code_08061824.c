/* Event script steps 0x08061824..0x080619FF. */
#include "structs/ewram.h"

#define PLAYER_ENTITY (gEwramData->playerState.player)

extern void TextWindow_Begin(s32 textId);
extern s32 TextWindow_Update(void);
extern void TextWindow_Close(void);
extern void Interaction_End(struct EwramData_EntityData *entity, s32 deleteEntity);

/**
 * @brief 61824 | Where the player is relative to x centerX: 1 when outside
 * the window of width outerWidth centred on centerX, -1 when inside the
 * (narrower) window of width innerWidth, else 0.
 */
s32 Player_GetZoneAroundXCopy(u16 centerX, u16 innerWidth, u16 outerWidth)
{
    struct EwramData_EntityData *player = PLAYER_ENTITY;

    if ((u16)(player->posX.part16.integer - (centerX - outerWidth / 2)) < outerWidth)
    {
        if ((u16)(player->posX.part16.integer - (centerX - innerWidth / 2)) < innerWidth)
            return -1;
        return 0;
    }
    return 1;
}

/**
 * @brief Init of event scene variant 13 (Object 0x20 unk_514 = 13,
 * sEventSceneCreateFuncs[13]). Set stateFlags bit 0.
 */
void EventScene13_Init(void)
{
    gEwramData->gameState.stateFlags |= 1;
}

/**
 * @brief Update of event scene variant 13 (sEventSceneUpdateFuncs[13]); returns the
 * player input mask. Event step: start script 0x1A, then once it
 * finished (TextWindow_Update) call Interaction_End(1).
 */
s32 EventScene13_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
        case 0:
            TextWindow_Begin(0x1A);
            entity->state++;
            break;
        case 1:
            if (TextWindow_Update())
                Interaction_End(entity, 1);
            break;
    }
    return 0;
}

/**
 * @brief Init of event scene variant 14 (Object 0x20 unk_514 = 14,
 * sEventSceneCreateFuncs[14]). Same as EventScene13_Init.
 */
void EventScene14_Init(void)
{
    gEwramData->gameState.stateFlags |= 1;
}

/**
 * @brief Update of event scene variant 14 (sEventSceneUpdateFuncs[14]); returns the
 * player input mask. Event step: start script 0x1B, then once it
 * finished call TextWindow_Close and end (0x53D bit 3).
 */
s32 EventScene14_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
        case 0:
            TextWindow_Begin(0x1B);
            entity->state++;
            break;
        case 1:
            if (TextWindow_Update())
            {
                TextWindow_Close();
                entity->deleteRequested = 1;
            }
            break;
    }
    return 0;
}

/**
 * @brief Init of event scene variant 17 (Object 0x20 unk_514 = 17,
 * sEventSceneCreateFuncs[17]). Nothing.
 */
void EventScene17_Init(void)
{
}

/**
 * @brief Update of event scene variant 17 (sEventSceneUpdateFuncs[17]); returns the
 * player input mask. Event step: set stateFlags bit 0 and start script
 * 0x17, then once it finished call Interaction_End(1).
 */
s32 EventScene17_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
        case 0:
            gEwramData->gameState.stateFlags |= 1;
            TextWindow_Begin(0x17);
            entity->state++;
            break;
        case 1:
            if (TextWindow_Update())
                Interaction_End(entity, 1);
            break;
    }
    return 0;
}

/**
 * @brief Init of event scene variant 18 (Object 0x20 unk_514 = 18,
 * sEventSceneCreateFuncs[18]). Same as EventScene13_Init.
 */
void EventScene18_Init(void)
{
    gEwramData->gameState.stateFlags |= 1;
}

/**
 * @brief Update of event scene variant 18 (sEventSceneUpdateFuncs[18]); returns the
 * player input mask. Event step: set stateFlags bit 0 and start script
 * 0x18; when it finished call TextWindow_Close and end; state 0x80 calls
 * Interaction_End(1).
 */
s32 EventScene18_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
        case 0:
            gEwramData->gameState.stateFlags |= 1;
            TextWindow_Begin(0x18);
            entity->state++;
            break;
        case 1:
            if (TextWindow_Update())
            {
                TextWindow_Close();
                entity->deleteRequested = 1;
            }
            break;
        case 0x80:
            Interaction_End(entity, 1);
            break;
    }
    return 0;
}
