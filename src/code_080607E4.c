#include "code_08009A0.h"
#include "code_0800CB00.h"
#include "code_08013960.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "gba/dma.h"
#include "structs/ewram.h"
extern void TextWindow_Begin(s32);
extern s32 TextWindow_Update(void);
extern void Sound_Play(s32);
extern void Interaction_End(struct EwramData_EntityData *, s32);
extern void GameFlag_Set(s32);

/* A signed field view keeps the camera base and ldrsh offset used by agbcc. */
struct CameraXView { u8 pad[6]; s16 x; };
/* Scripted movement and camera sequence; state meanings remain tentative. */
/**
 * @brief Update of event scene variant 10 (sEventSceneUpdateFuncs[10]); returns the
 * player input mask.
 */
s32 EventScene10_Update(struct EwramData_EntityData *entity)
{
    s32 input = 0;
    struct EwramData *ew = gEwramData;
    struct GameState *context = &ew->gameState;
    switch (entity->state) {
    case 0:
        if ((s16)ew->playerState.player->posX.part16.integer > 96) {
            ew->hBlankEffect.requestStop = 1;
            DMA_FILL_32(3, 0, gEwramData->hBlankEffect.hBlankBuffer, 0xA00);
            gEwramData->gameState.stateFlags |= 0x200;
            Hud_CloseMessageWindow();
            RedSoul_CancelAll();
            BlueSoul_StopAll();
            BgPalette_StopColorFade();
            ObjPalette_StopColorFade();
            entity->state++;
            entity->stage = 0;
            entity->subState = 0;
        } else
            ew->gameState.stateFlags &= ~0x200;
        break;
    case 1:
        ew->gameState.stateFlags |= 1;
        TextWindow_Begin(21);
        ew->gameState.textWindowPause = 1;
        entity->state++;
        entity->stage = 0;
        entity->subState = 0;
    case 2:
        if (TextWindow_Update())
            entity->state = 0x80;
        switch (entity->subState) {
        case 0:
            switch (entity->stage) {
            case 0:
                context->textWindowPause = 1;
                entity->stage++;
                entity->timer = 48;
            case 1:
                if ((u8)--entity->timer == 0xFF) {
                    Sound_Play(0x1040);
                    entity->stage++;
                }
                break;
            case 2: {
                struct EwramData *room = gEwramData;
                struct EwramData_EntityData **player = &room->playerState.player;
                s32 x = (s16)(*player)->posX.part16.integer;
                struct BgLayerState *bg = &gEwramData->bgInfo[1];
                if (x > 224 - (s16)bg->xPos.part16.integer) {
                    (*player)->posX.part16.integer = 224 - bg->xPos.part16.integer;
                    (*player)->flipX = 0;
                    gEwramData->gameState.cameraScrollMinPriority = 1;
                    entity->stage++;
                } else
                    input |= 0x10;
                break;
            }
            case 3:
                if (((struct CameraXView *)&gEwramData->bgInfo[1])->x <= 137)
                    Camera_ScrollWithPriority(1, 0x8000, 0);
                else {
                    Music_SetCurrent(22);
                    context->textWindowPause = 0;
                    entity->stage++;
                }
                break;
            case 4:
                break;
            }
            break;
        case 1:
            switch (entity->stage) {
            case 0:
                context->textWindowPause = 1;
                if (entity->animScriptIndex != 15)
                    Anim_StartIndexed(entity, 15, 1, 0);
                entity->stage++;
                break;
            case 1:
                if (entity->animDone) {
                    entity->timer = 64;
                    if (entity->animScriptIndex != 0)
                        Anim_StartIndexed(entity, 0, 1, 1);
                    entity->stage++;
                }
                break;
            case 2:
                if ((u8)--entity->timer == 0xFF)
                    entity->stage++;
                break;
            case 3:
                context->textWindowPause = 0;
                break;
            }
            break;
        case 2:
            switch (entity->stage) {
            case 0:
                Sound_Play(0x1040);
                context->textWindowPause = 1;
                entity->velX.whole = 0x18000;
                entity->flipX = 1;
                if (entity->animScriptIndex != 1)
                    Anim_StartIndexed(entity, 1, 1, 1);
                entity->timer = 16;
                entity->stage++;
            case 1:
                if ((s16)entity->posX.part16.integer <= 511)
                    entity->posX.whole += entity->velX.whole;
                if (entity->timer == 0) {
                    Camera_ScrollWithPriority(1, -0x8000, 0);
                    if ((s16)gEwramData->playerState.player->posX.part16.integer == 120) {
                        Sound_Play(0x1040);
                        context->textWindowPause = 0;
                        context->textWindowState = 5;
                        entity->stage++;
                    }
                } else
                    entity->timer--;
                break;
            }
            break;
        }
        break;
    case 0x80:
        Music_UpdateRoom(context);
        gEwramData->gameState.cameraScrollMinPriority = 0;
        Interaction_End(entity, 1);
        GameFlag_Set(0x27);
        break;
    }
    if (entity->state != 0x80)
        Anim_Update(entity);
    return input;
}
