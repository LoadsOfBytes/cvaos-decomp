#include "code_08013960.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void TextWindow_Begin(s32);
extern s32 TextWindow_Update(void);
extern void Sound_Play(u16);
extern void Interaction_End(struct EwramData_EntityData *, s32);
extern void GameFlag_Set(s32 flag);

/**
 * @brief Update of event scene variant 5 (sEventSceneUpdateFuncs[5]); returns the player
 * input mask. Cutscene object for conversation 0x10: waits, keeps the
 * player at x <= 0x50 (returns 0x10 meanwhile), then walks off to the
 * right and sets unk_414 to 5; on the end of the conversation sets event
 * 0x22.
 */
s32 EventScene05_Update(struct EwramData_EntityData *entity)
{
    s32 ret;
    struct EwramData **ewp;
    struct EwramData *ew;
    struct GameState *g;
    struct EwramData_EntityData *player;
    u8 state;
    u8 step;

    ret = 0;
    ewp = &gEwramData;
    ew = *ewp;
    g = &ew->gameState;
    switch (state = entity->state)
    {
    case 0:
        if (entity->subState == 0)
        {
            Sound_Play(0x1040);
            TextWindow_Begin(0x10);
            ew->gameState.textWindowPause = 1;
            entity->timer = 0x20;
            entity->subState++;
        }
        if (--entity->timer == 0xFF)
        {
            entity->state++;
            entity->stage = ret;
            entity->subState = ret;
        }
        break;
    case 1:
        if (TextWindow_Update())
            entity->state = 0x80;
        switch (step = entity->subState)
        {
        case 0:
            switch (entity->stage)
            {
            case 0:
                entity->stage = state;
                entity->timer = 0x30;
                /* fall through */
            case 1:
                if (--entity->timer != 0xFF)
                    break;
                goto next;
            case 2:
                player = (*ewp)->playerState.player;
                {
                    u16 *px = &player->posX.part16.integer;
                    if ((s16)*px > 0x4F)
                    {
                        *px = 0x50;
                        goto next;
                    }
                }
                ret = 0x10;
                break;
            case 3:
                Music_SetCurrent(0x16);
                ew->gameState.textWindowPause = ret;
                break;
            }
            break;
        case 1:
            switch (entity->stage)
            {
            case 0:
                Sound_Play(0x1040);
                ew->gameState.textWindowPause = step;
                entity->velX.whole = 0x8000;
                entity->flipX = 1 - (*ewp)->playerState.player->flipX;
                if (entity->animScriptIndex != 1)
                    Anim_StartIndexed(entity, 1, 0, 1);
                entity->stage++;
                /* fall through */
            case 1:
                if ((s16)entity->posX.part16.integer > 0xFF)
                    goto next;
                entity->posX.whole += entity->velX.whole;
                break;
            case 2:
                ew->gameState.textWindowPause = ret;
                ew->gameState.textWindowState = 5;
            next:
                entity->stage++;
                break;
            }
            break;
        }
        break;
    case 0x80:
        Music_UpdateRoom(g);
        GameFlag_Set(0x22);
        Interaction_End(entity, 1);
        break;
    }
    if (entity->state != 0x80)
        Anim_Update(entity);
    return ret;
}
