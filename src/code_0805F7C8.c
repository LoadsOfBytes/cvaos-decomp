#include "gba.h"
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
 * @brief Update of event scene variant 6 (sEventSceneUpdateFuncs[6]); returns the player
 * input mask. Cutscene object for conversation 0x11: clears BG VRAM at
 * 0x06004000, scrolls the camera back to 0 and keeps the player at x <=
 * 0xAE (returns 0x20 meanwhile), then walks off to the left and sets
 * unk_414 to 5; on the end of the conversation sets event 0x23.
 */
s32 EventScene06_Update(struct EwramData_EntityData *entity)
{
    s32 ret;
    struct EwramData **ewp;
    struct EwramData *ew;
    struct GameState *g;
    u8 step;

    ret = 0;
    ewp = &gEwramData;
    ew = *ewp;
    g = &ew->gameState;
    switch (entity->state)
    {
    case 0:
        DMA_FILL_32(3, ret, 0x06004000, 0x1000);
        Sound_Play(0x1040);
        TextWindow_Begin(0x11);
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
                gEwramData->gameState.cameraScrollMinPriority = 1;
                /* fall through */
            case 1:
                entity->stage++;
                /* fall through */
            case 2:
            {
                struct BgLayerState *cam = &gEwramData->bgInfo[1];
                s32 cx;
                if (cam->xPos.whole > 0)
                    Camera_ScrollWithPriority(1, -0x18000, 0);
                else
                    cam->xPos.whole = 0;
                {
                    u16 *px = &gEwramData->playerState.player->posX.part16.integer;
                    s32 x = (s16)*px;
                    struct BgLayerState *c = &gEwramData->bgInfo[1];
                    if (x <= 0xAE - (s16)c->xPos.part16.integer)
                        *px = 0xAE - c->xPos.part16.integer;
                    else
                        ret |= 0x20;
                }
                cam = &gEwramData->bgInfo[1];
                cx = cam->xPos.whole;
                if (cx == 0
                    && (s16)gEwramData->playerState.player->posX.part16.integer == 0xAE - (s16)cam->xPos.part16.integer)
                {
                    Music_SetCurrent(0x16);
                    g->textWindowPause = cx;
                    goto next;
                }
                break;
            }
            }
            break;
        case 1:
            switch (entity->stage)
            {
            case 0:
                Sound_Play(0x1040);
                g->textWindowPause = step;
                entity->flipX = 0;
                if (entity->animScriptIndex != 1)
                    Anim_StartIndexed(entity, 1, 0, 1);
                entity->stage++;
                /* fall through */
            case 1:
                if ((s16)entity->posX.part16.integer > -0x10
                    && !(Entity_TryWalk(entity, 0x10000, 0x200000, 0x20000, 1) & 4))
                    entity->posY.whole += entity->velY.whole;
                Camera_ScrollWithPriority(1, 0x10000, 0);
                if ((s16)gEwramData->playerState.player->posX.part16.integer == 0x78)
                {
                    g->textWindowPause = 0;
                    g->textWindowState = 5;
                next:
                    entity->stage++;
                }
                break;
            }
            break;
        }
        break;
    case 0x80:
        Music_UpdateRoom(g);
        (*ewp)->gameState.cameraScrollMinPriority = ret;
        GameFlag_Set(0x23);
        Interaction_End(entity, 1);
        break;
    }
    if (entity->state != 0x80)
        Anim_Update(entity);
    return ret;
}
