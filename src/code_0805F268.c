#include "code_08013960.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void TextWindow_Begin(s32);
/*
 * Event scene 04 actor: position in the animation list sUnk_0852682C (0xFF
 * ends it; also read through the byte view), and a flag set to 4 when its
 * text starts (never read). Token-identical expansion.
 */
#define EVENT04_ANIM_POS(e) e->unk_4F8.part8.unk_4F8
#define EVENT04_ANIM_POS_BYTE(e) e->unk_4F8.bytes[0]
#define EVENT04_UNUSED_FLAG(e) e->unk_4FC.bytes[0]
extern s32 TextWindow_Update(void);
extern void Sound_Play(u16);
extern void Interaction_End(struct EwramData_EntityData *, s32);
extern void GameFlag_Set(s32 flag);
extern const u8 sUnk_0852682C[];

/**
 * @brief Update of event scene variant 4 (sEventSceneUpdateFuncs[4]); returns the player
 * input mask. Cutscene object for conversation 0xF: keeps the player at
 * x <= 0x57 (returns 0x10 meanwhile), plays the animation list
 * sUnk_0852682C, then walks off to the right and sets unk_414 to 5; on
 * the end of the conversation sets event 0x21.
 */
s32 EventScene04_Update(struct EwramData_EntityData *entity)
{
    s32 ret;
    struct EwramData **ewp;
    struct EwramData *ew;
    struct GameState *g;
    struct EwramData_EntityData *player;
    u8 state;
    s32 v;

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
            TextWindow_Begin(0xF);
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
                entity->stage = state;
                entity->timer = 0x30;
                /* fall through */
            case 1:
                if (--entity->timer != 0xFF)
                    break;
                goto next;
            case 2:
                player = (*ewp)->playerState.player;
                if ((s16)player->posX.part16.integer > 0x56)
                {
                    player->flipX = 0;
                    (*ewp)->playerState.player->posX.part16.integer = 0x57;
                    goto next;
                }
                ret = 0x10;
                break;
            case 3:
                Music_SetCurrent(0x16);
                ew->gameState.textWindowPause = ret;
                entity->subState = state;
                entity->stage = ret;
                break;
            }
            break;
        case 1:
            switch (entity->stage)
            {
            case 0:
                EVENT04_ANIM_POS(entity) = ret;
                v = 0xFF;
                goto set;
            case 1:
                if (--entity->timer == 0xFF)
                {
                    const u8 *tbl = sUnk_0852682C;
                    s32 anim = tbl[(u8)EVENT04_ANIM_POS_BYTE(entity)++];
                    if (anim == 0xFF)
                    {
                        anim = 0;
                        EVENT04_ANIM_POS(entity) = ret;
                    }
                    Anim_StartIndexed(entity, anim, 0, 0);
                    entity->timer |= 0xFF;
                }
                break;
            }
            break;
        case 2:
            switch (entity->stage)
            {
            case 0:
                Sound_Play(0x1040);
                ew->gameState.textWindowPause = state;
                entity->flipX = 1;
                if (entity->animScriptIndex != 1)
                    Anim_StartIndexed(entity, 1, 0, 1);
                entity->stage++;
                /* fall through */
            case 1:
                if ((s16)entity->posX.part16.integer > 0xFF)
                {
                    v = 0x10;
                set:
                    entity->timer = v;
                next:
                    entity->stage++;
                }
                else
                {
                    entity->posX.whole += entity->velX.whole;
                }
                break;
            case 2:
                if (--entity->timer == 0xFF)
                {
                    ew->gameState.textWindowPause = ret;
                    ew->gameState.textWindowState = 5;
                    entity->stage++;
                    EVENT04_UNUSED_FLAG(entity) = 4;
                }
                break;
            }
            break;
        }
        break;
    case 0x80:
        Music_UpdateRoom(g);
        GameFlag_Set(0x21);
        Interaction_End(entity, 1);
        break;
    }
    if (entity->state != 0x80)
        Anim_Update(entity);
    return ret;
}
