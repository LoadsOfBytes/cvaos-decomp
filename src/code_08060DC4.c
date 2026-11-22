#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void TextWindow_Begin(s32);
extern s32 TextWindow_Update(void);
extern void TextWindow_Close(void);

/**
 * @brief Update of event scene variant 12 (sEventSceneUpdateFuncs[12]); returns the
 * player input mask. Cutscene step object for conversation 0x19: walks
 * in until x > 7, turns the player around and toggles textWindowPause; returns
 * 0x40 from step 3 on and after the conversation ends.
 */
s32 EventScene12_Update(struct EwramData_EntityData *entity)
{
    s32 ret;
    u8 step;

    ret = 0;
    switch (entity->state)
    {
    case 0:
        gEwramData->gameState.stateFlags |= 1;
        TextWindow_Begin(0x19);
        entity->state++;
        break;
    case 1:
        if (TextWindow_Update())
        {
            entity->state = 0x80;
            break;
        }
        step = entity->subState;
        if (step == 1)
        {
            if (++entity->stage == 1)
            {
                if (entity->animScriptIndex != 0)
                    Anim_StartIndexed(entity, 0, 0, 1);
                gEwramData->gameState.textWindowPause = step;
            }
            if ((s16)entity->posX.part16.integer <= 7)
            {
                entity->posX.whole += entity->velX.whole;
                break;
            }
            if (entity->animScriptIndex != 4)
                Anim_StartIndexed(entity, 4, 0, 1);
            gEwramData->playerState.player->flipX = 1;
            gEwramData->gameState.textWindowPause = 0;
            entity->subState++;
            entity->stage = 0;
        }
        else if (step > 2)
        {
            ret = 0x40;
            if (step == 4)
            {
                if (++entity->stage == 1)
                {
                    entity->timer = 0x40;
                    gEwramData->gameState.textWindowPause = 1;
                }
                else if (entity->timer == 0)
                {
                    gEwramData->gameState.textWindowPause = 0;
                }
                else
                {
                    entity->timer--;
                }
            }
        }
        break;
    case 0x80:
        ret = 0x40;
        TextWindow_Close();
        gEwramData->gameState.stateFlags |= 1;
        break;
    }
    if (entity->state != 0x80)
        Anim_Update(entity);
    return ret;
}
