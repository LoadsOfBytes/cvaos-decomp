#include "code_0800F1FC.h"
#include "code_08013960.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void TextWindow_Begin(s32);
extern s32 TextWindow_Update(void);
extern void Sound_Play(u16);
extern void Interaction_End(struct EwramData_EntityData *, s32);
extern void GameFlag_Set(s32 flag);
extern s32 Entity_TryWalk(struct EwramData_EntityData *, s32, s32, s32, s32);

/**
 * @brief Update of event scene variant 7 (sEventSceneUpdateFuncs[7]); returns the player
 * input mask. Cutscene object for conversation 0x12: stops the player,
 * scrolls the camera, walks off to the left and finally sets unk_414 to
 * 5; on the end of the conversation sets event 0x24.
 */
s32 EventScene07_Update(struct EwramData_EntityData *entity)
{
    s32 ret;
    struct EwramData **ewp;
    struct EwramData *ew;
    struct GameState *g;
    struct EwramData_EntityData *player;
    u8 step;

    ret = 0;
    ewp = &gEwramData;
    ew = *ewp;
    g = &ew->gameState;
    switch (entity->state)
    {
    case 0:
        player = ew->playerState.player;
        player->velY.whole = ret;
        player->velX.whole = ret;
        Sound_Play(0x1040);
        TextWindow_Begin(0x12);
        ew->gameState.textWindowPause = 1;
        entity->state++;
        /* fall through */
    case 1:
        if (TextWindow_Update())
            entity->state = 0x80;
        switch (step = entity->subState)
        {
        case 0:
            switch (entity->stage)
            {
            case 0:
                g->textWindowPause = 1;
                entity->stage++;
                /* fall through */
            case 1:
                entity->stage++;
                /* fall through */
            case 2:
            {
                struct EwramData **p2 = &gEwramData;
                struct BgLayerState *cam;
                (*p2)->gameState.cameraScrollMinPriority = 1;
                cam = &(*p2)->bgInfo[1];
                if ((s16)cam->xPos.part16.integer > 0x94)
                {
                    Camera_ScrollWithPriority(1, -0x8000, 0);
                    break;
                }
                Music_SetCurrent(0x16);
                g->textWindowPause = 0;
                goto next;
            }
            }
            break;
        case 1:
            switch (entity->stage)
            {
            case 0:
                g->textWindowPause = step;
                entity->flipX = 1 - entity->flipX;
                if (entity->animScriptIndex != 8)
                    Anim_StartIndexed(entity, 8, 0, 1);
                entity->stage++;
                /* fall through */
            case 1:
                if ((s16)entity->posX.part16.integer > -0x10)
                {
                    if (!(Entity_TryWalk(entity, 0x10000, 0x200000, 0x20000, 1) & 4))
                        entity->posY.whole += entity->velY.whole;
                }
                else
                {
                    entity->stage++;
                    g->textWindowPause = 0;
                }
                break;
            }
            break;
        case 2:
        {
            u8 *flag = &g->textWindowPause;
            {
                s32 one = 1;
                *flag = one;
                Camera_ScrollWithPriority(one, 0x8000, 0);
            }
            Sound_Play(0x1040);
            if ((s16)gEwramData->playerState.player->posX.part16.integer == 0x78)
            {
                *flag = 0;
                g->textWindowState = 5;
            next:
                entity->stage++;
            }
            break;
        }
        }
        break;
    case 0x80:
        ew->gameState.cameraScrollMinPriority = ret;
        Music_UpdateRoom(&(*ewp)->gameState);
        GameFlag_Set(0x24);
        Interaction_End(entity, 1);
        break;
    }
    if (entity->state != 0x80)
        Anim_Update(entity);
    return ret;
}
