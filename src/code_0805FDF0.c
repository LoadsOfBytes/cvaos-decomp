#include "code_08013960.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void TextWindow_Begin(s32);
extern s32 TextWindow_Update(void);
extern void Sound_Play(u16);
extern void Interaction_End(struct EwramData_EntityData *, s32);
extern void GameFlag_Set(s32 flag);

/**
 * @brief Update of event scene variant 8 (sEventSceneUpdateFuncs[8]); returns the player
 * input mask. Cutscene object for conversation 0x13 (sibling of
 * EventScene05_Update): keeps the player at x <= 0xA0 (returns 0x20
 * meanwhile), plays song 0x1B7, walks off to the left; on the end of the
 * conversation sets event 0x25 (and returns without a value, as the
 * original).
 */
s32 EventScene08_Update(struct EwramData_EntityData *entity)
{
    s32 ret;
    struct EwramData **ewp;
    struct EwramData *ew;
    struct GameState *g;
    u8 state;

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
            TextWindow_Begin(0x13);
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
        switch (entity->subState)
        {
        case 0:
            switch (entity->stage)
            {
            case 0:
                ew->gameState.textWindowPause = state;
                entity->stage++;
                entity->timer = 0x30;
                /* fall through */
            case 1:
                if (--entity->timer != 0xFF)
                    break;
                goto next;
            case 2:
                if ((s16)(*ewp)->playerState.player->posX.part16.integer <= 0xA0)
                {
                    Music_SetCurrent(0x16);
                    ew->gameState.textWindowPause = ret;
                    (*ewp)->playerState.player->posX.part16.integer = 0xA0;
                    goto next;
                }
                ret = 0x20;
                break;
            }
            break;
        case 1:
            if (entity->stage == 0)
            {
                PlaySong(0x1B7);
                goto next;
            }
            break;
        case 2:
            switch (entity->stage)
            {
            case 0:
                Sound_Play(0x1040);
                ew->gameState.textWindowPause = state;
                entity->velX.whole = -0x8000;
                entity->flipX = 0;
                if (entity->animScriptIndex != 1)
                    Anim_StartIndexed(entity, 1, 0, 1);
                entity->stage++;
                /* fall through */
            case 1:
                if ((s16)entity->posX.part16.integer < -8)
                {
                    g->textWindowPause = 0;
                    g->textWindowState = 5;
                next:
                    entity->stage++;
                }
                else
                {
                    entity->posX.whole += entity->velX.whole;
                }
                break;
            }
            break;
        }
        break;
    case 0x80:
        Music_UpdateRoom(g);
        GameFlag_Set(0x25);
        Interaction_End(entity, 1);
        goto end;
    }
    Anim_Update(entity);
    return ret;
end:
}
