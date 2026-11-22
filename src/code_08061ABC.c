#include "code_08009A0.h"
#include "code_0800F1FC.h"
#include "code_08013960.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void TextWindow_Begin(s32);
extern s32 TextWindow_Update(void);
extern void Sound_Play(u16);
extern void GameFlag_Set(s32 flag);
extern void EventScene19_Finish(struct EwramData_EntityData *);

/**
 * @brief Update of event scene variant 19 (sEventSceneUpdateFuncs[19]); returns the
 * player input mask. Cutscene object for conversation 0x1F: waits until
 * the player is left of x = 0xC0 (returns 0x20 meanwhile), then walks in
 * while the camera scrolls; on the end of the conversation sets event
 * 0x29.
 */
s32 EventScene19_Update(struct EwramData_EntityData *entity)
{
    struct EwramData **ewp;
    struct EwramData *ew;
    s32 ret;
    s32 x;
    struct BgLayerState *c2;

    ewp = &gEwramData;
    ew = *ewp;
    ret = 0;
    switch (entity->state)
    {
    case 0:
        Sound_Play(0x1040);
        Sound_PlayTracked(0x2B);
        entity->state++;
        entity->stage = ret;
        entity->subState = ret;
        /* fall through */
    case 1:
    {
        struct EwramData *w = gEwramData;
        struct BgLayerState *cam;
        s32 px;
        w->gameState.stateFlags |= 1;
        px = (s16)w->playerState.player->posX.part16.integer;
        cam = &w->bgInfo[1];
        if (px + (s16)cam->xPos.part16.integer <= 0xBF)
        {
            entity->state++;
            entity->stage = 0;
            entity->subState = 0;
        }
        else
        {
            ret |= 0x20;
        }
        break;
    }
    case 2:
        TextWindow_Begin(0x1F);
        entity->state++;
        entity->stage = ret;
        entity->subState = ret;
        break;
    case 3:
        if (TextWindow_Update())
        {
            GameFlag_Set(0x29);
            Sound_StopTracked(0x2B);
            (*ewp)->gameState.stateFlags &= ~0x40000;
            Music_SetCurrent(0x14);
            entity->state = 0x80;
            break;
        }
        switch (entity->subState)
        {
        case 0:
            break;
        case 1:
            switch (entity->stage)
            {
            case 0:
                (*ewp)->playerState.player->flipX = 0;
                entity->hidden = 0;
                ew->gameState.textWindowPause = 1;
                (*ewp)->gameState.cameraScrollMinPriority = 1;
                entity->stage++;
                /* fall through */
            case 1:
                c2 = &gEwramData->bgInfo[1];
                if ((s16)c2->xPos.part16.integer > 0x70)
                    Camera_ScrollWithPriority(1, 0, 0);
                else
                    Camera_ScrollWithPriority(1, 0x10000, 0);
                x = (s16)entity->posX.part16.integer;
                c2 = &gEwramData->bgInfo[1];
                x += (s16)c2->xPos.part16.integer;
                if (x <= 0x11F)
                {
                    entity->timer = 0x28;
                    entity->stage++;
                    if (entity->animScriptIndex != 0)
                        Anim_StartIndexed(entity, 0, 1, 1);
                }
                else
                {
                    if (x <= 0x127)
                    {
                        if (entity->animScriptIndex != 2)
                            Anim_StartIndexed(entity, 2, 1, 0);
                        entity->velX.whole = -0x8000;
                    }
                    entity->posX.whole += entity->velX.whole;
                }
                break;
            case 2:
                Camera_ScrollWithPriority(1, 0, 0);
                if (--entity->timer == 0xFF)
                {
                    ew->gameState.textWindowPause = ret;
                    entity->subState++;
                    entity->stage = ret;
                }
                break;
            }
            break;
        case 2:
            Camera_ScrollWithPriority(1, 0, 0);
            break;
        }
        Anim_Update(entity);
        break;
    case 0x80:
        ew->gameState.cameraScrollMinPriority = ret;
        EventScene19_Finish(entity);
        break;
    }
    return ret;
}
