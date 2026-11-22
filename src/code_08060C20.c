#include "code/code_08039340.h"
#include "structs/ewram.h"
/* Declared s16 in code_08001194.h; the caller here uses the full register. */
extern s32 GetEntityRoomXPositionInteger(struct EwramData_EntityData *entity);
extern void TextWindow_Begin(s32);
extern s32 TextWindow_Update(void);
extern void Sound_Play(u16);
extern void Interaction_End(struct EwramData_EntityData *, s32);
extern void GameFlag_Set(s32 flag);

/**
 * @brief Update of event scene variant 11 (sEventSceneUpdateFuncs[11]); returns the
 * player input mask. Cutscene step object: shows conversation 0x16,
 * toggles textWindowPause while it runs and returns 0x20 once the player walks
 * right of x = 0xB6; when the conversation ends (state 0x80) sets event
 * flag 0x28.
 */
s32 EventScene11_Update(struct EwramData_EntityData *entity)
{
    s32 ret;
    struct EwramData *ew;
    struct GameState *g;

    ret = 0;
    ew = gEwramData;
    g = &ew->gameState;
    ew->gameState.stateFlags |= 1;
    switch (entity->state)
    {
    case 0:
        Sound_Play(0x1040);
        TextWindow_Begin(0x16);
        ew->gameState.textWindowPause = 1;
        entity->state++;
        entity->timer = 0x90;
        break;
    case 1:
        if (TextWindow_Update())
        {
            entity->state = 0x80;
            break;
        }
        switch (entity->subState)
        {
        case 0:
            if (--entity->timer == 0xFF)
            {
                entity->subState++;
                g->textWindowPause = 0;
            }
            break;
        case 2:
            g->textWindowPause = 1;
            if (GetEntityRoomXPositionInteger(gEwramData->playerState.player) > 0xB6)
            {
                ret = 0x20;
                break;
            }
            entity->subState++;
            break;
        case 1:
            break;
        case 3:
            g->textWindowPause = 0;
            entity->subState++;
            break;
        case 4:
            break;
        }
        break;
    case 0x80:
        GameFlag_Set(0x28);
        Interaction_End(entity, 1);
        break;
    }
    if (entity->state != 0x80)
        Anim_Update(entity);
    return ret;
}
