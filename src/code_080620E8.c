/* EventScene22_Update: event step (scene with player walk-in); matched. */
#include "code_08009A0.h"
#include "code_0800CB00.h"
#include "code_08013960.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void TextWindow_Begin(s32);
extern s32 TextWindow_Update(void);
extern void Sound_Play(u16);
extern void Interaction_End(struct EwramData_EntityData *, s32);
extern void GameFlag_Set(s32 flag);
extern s32 GameFlag_GetMask(s32 flag);
extern void Music_StartOverride(s32 song);
extern s32 EventScene22_Wait(struct EwramData_EntityData *, s32);

/**
 * @brief Update of event scene variant 22 (sEventSceneUpdateFuncs[22]); returns the
 * player input mask.
 */
s32 EventScene22_Update(struct EwramData_EntityData *entity)
{
    s32 ret = 0;
    struct EwramData *ew = gEwramData;
    u8 state = entity->state;
    struct EwramData_EntityData *player;

    switch (state)
    {
    case 0:
        Music_StartOverride(0x2D);
        if (GameFlag_GetMask(0x2B))
        {
            entity->deleteRequested = 1;
            goto end;
        }
        {
            register u32 *p asm("r1") = &gEwramData->gameState.stateFlags; /* FAKEMATCH: pinned register */
            *p |= 1;
        }
        goto next;
    case 1:
        Display_SaveState();
        TextWindow_Begin(0x22);
        ew->gameState.textWindowPause = state;
    next:
        do { /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
            entity->state++;
            entity->stage = ret;
            entity->subState = ret;
        } while (0);
        break;
    case 2:
        if (TextWindow_Update())
        {
            entity->state = 0x80;
            GameFlag_Set(0x2B);
            Interaction_End(entity, 1);
            Display_RestoreState();
            Sound_Play(0x1041);
            gEwramData->musicOverrideSong = 0x2D;
            PlaySong(0x2D);
            goto end;
        }
        switch (entity->subState)
        {
        case 0:
            if (!EventScene22_Wait(entity, 0x40))
            {
                entity->subState++;
                entity->stage = ret;
            }
            break;
        case 1:
            player = gEwramData->playerState.player;
            if ((s16)player->posX.part16.integer > 0x9F)
            {
                ret = 0x20;
                break;
            }
            player->posX.part16.integer = 0x9F;
            entity->subState++;
            entity->stage = 0;
            break;
        case 2:
            if (EventScene22_Wait(entity, 0x40))
                break;
            goto clear;
        case 3:
            if (!EventScene22_Wait(entity, 0x40))
            {
                Sound_StopTracked(0x2D);
                Music_SetCurrent(0x24);
            clear:
                do { /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
                    ew->gameState.textWindowPause = ret;
                } while (0);
            }
            break;
        }
        break;
    }
    return ret;
end:
}
