/* EventScene15_Update, decompiled from the original assembly: event scene with the
 * flashing player (palette fades, periods table at 0x081188E4 copied to the
 * stack). The x mark is copied into a u16 local in sub-state 2 (keeps the
 * separate register copy), and the owner address is written as an integer
 * sum so the base comes first. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code_08011DD0.h"
#include "code_08013960.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void TextWindow_Begin(s32);
extern s32 TextWindow_Update(void);
extern s8 Player_GetZoneAroundXCopy(u16 centerX, u16 innerWidth, u16 outerWidth);
extern void Player_StartScriptedAnimation(s32, s32, s32, s32);
extern s32 Entity_TryWalk(struct EwramData_EntityData *, s32, s32, s32, s32);
extern void Sound_Play(u16);
extern void Interaction_End(struct EwramData_EntityData *, s32);

struct Periods { s32 v[10]; };

#define PLAYER (gEwramData->playerState.player)
#define IDX (*(u8 *)&entity->unk_4F8.part8.unk_4F8)

/**
 * @brief Update of event scene variant 15 (sEventSceneUpdateFuncs[15]); returns the
 * player input mask.
 */
s32 EventScene15_Update(struct EwramData_EntityData *entity)
{
    struct Periods periods = *(struct Periods *)0x081188E4;
    s32 result = 0;
    struct EwramData *ew = gEwramData;
    struct GameState *save = &ew->gameState;
    s32 x = 0xAA;
    s32 y = 0x50;
    u32 full = 0xFF;

    switch (entity->state)
    {
    case 0:
        TextWindow_Begin(0x1C);
        ew->gameState.textWindowPause = 1;
        entity->state++;
        break;
    case 1:
        if (TextWindow_Update())
            entity->state = 0x80;
        switch (entity->subState)
        {
        case 0:
            if (Player_GetFocusedActionFlags() & 0x1E)
                break;
            {
                u8 i = entity->unk_4F4.part8.unk_4F5;
                struct EwramData *w = gEwramData;
                struct EwramData_EntityData *e = (struct EwramData_EntityData *)(i * sizeof(struct EwramData_EntityData) + (u32)w + 0x4E4);
                u8 flip;
                flip = 0;
                if (e->posX.whole < w->playerState.player->posX.whole)
                    flip = 1;
                w->entityData[i].flipX = flip;
            }
            if (Player_GetZoneAroundXCopy(x, 0x3C, 0x21))
            {
                entity->subState = 2;
                break;
            }
            goto three;
        case 2:
        {
            u16 mark = x;
            if ((s8)Player_GetZoneAroundXCopy(mark, 0x3C, 0x21) != 1)
                goto three;
            {
                s32 px = (s16)PLAYER->posX.part16.integer;
                result = 0x10;
                if (mark < px)
                    result = 0x20;
            }
        }
            break;
        three:
            entity->subState = 3;
            break;
        case 3:
            if (entity->stage == 0)
            {
                entity->timer = 0x60;
                entity->stage++;
                Music_StartOverride(0x16C);
            }
            if (entity->timer == 0)
            {
                save->textWindowPause = 0;
                entity->subState++;
                entity->stage = 0;
            }
            break;
        case 4:
            if (entity->stage == 0)
            {
                BgPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0, 0x10000, 0x20);
                ObjPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0x4000, 0x10000, 0x10);
                PlaySong(0x170);
                entity->stage++;
            }
            if (entity->timer-- & 3)
                PLAYER->palette = 4;
            else
                PLAYER->palette = 0;
            Player_StartScriptedAnimation(0x52, 0, 0, 0);
            break;
        case 5:
            if (entity->timer-- & 3)
                PLAYER->palette = 4;
            else
                PLAYER->palette = 0;
            if (entity->timer % (periods.v[IDX] * 3 / 2) == 0)
            {
                PlaySong(0x170);
                BgPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0, 0x10000, 0x20);
                ObjPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0x4000, 0x10000, 0x10);
                if (++IDX == 10)
                    IDX = 0;
            }
            Player_StartScriptedAnimation(0x52, 0, 0, 0);
            break;
        case 6:
            if (entity->timer-- & 2)
                PLAYER->palette = 4;
            else
                PLAYER->palette = 0;
            if (entity->timer % periods.v[IDX] == 0)
            {
                PlaySong(0x170);
                BgPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0, 0x10000, 0x20);
                ObjPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0x4000, 0x10000, 0x10);
                if (++IDX == 10)
                    IDX = 0;
            }
            Player_StartScriptedAnimation(0x52, 0, 0, 0);
            break;
        case 7:
            switch (entity->stage)
            {
            case 0:
            {
                s32 half;
                save->textWindowPause = 1;
                half = full >> 1;
                entity->timer = half + 8;
                entity->stage++;
                PlaySong(0x816C);
                PlaySong(0x197);
                PlaySong(0x169);
                BgPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0x10000, 0, half);
                ObjPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0x10000, 0, half);
            }
            case 1:
                if (entity->timer & 2)
                    PLAYER->palette = 4;
                else
                    PLAYER->palette = 0;
                if ((u8)--entity->timer == 0xFF)
                {
                    EntityDeleteGroup(0x40);
                    BgPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0, 0x10000, full);
                    ObjPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0, 0x10000, full);
                    entity->timer = full;
                    entity->stage++;
                    PLAYER->palette = 0;
                }
                break;
            case 2:
                if (entity->timer == 0x20)
                    Music_SetCurrent(0x21);
                if ((u8)--entity->timer == 0xFF)
                {
                    BgPalette_StopColorFade();
                    ObjPalette_StopColorFade();
                    entity->stage++;
                    entity->timer = 0x40;
                }
                break;
            case 3:
                if ((u8)--entity->timer == 0xFF)
                {
                    PLAYER->palette = 0;
                    entity->subState++;
                    entity->stage = 0;
                    save->textWindowPause = 0;
                }
                break;
            }
            result |= 0x80;
            break;
        case 8:
            if (entity->stage == 0)
                entity->stage++;
            result |= 0x80;
            break;
        case 9:
            switch (entity->stage)
            {
            case 0:
                PLAYER->flipX = 1;
                save->textWindowPause = 1;
                entity->hidden = 0;
                entity->stage++;
                entity->timer = 0xC0;
            case 1:
                if (!(Entity_TryWalk(entity, 0x6000, 0x200000, 0x10000, 1) & 4))
                {
                    entity->posX.whole += entity->velX.whole;
                    entity->posY.whole += entity->velY.whole;
                }
                else if (entity->animScriptIndex != 0)
                    Anim_StartIndexed(entity, 0, 0, 1);
                if ((s16)entity->posX.part16.integer >= y)
                {
                    entity->posX.part16.integer = y;
                    if (entity->animScriptIndex != 1)
                        Anim_StartIndexed(entity, 1, 0, 1);
                    entity->posY.part16.integer++;
                    save->textWindowPause = 0;
                    entity->subState++;
                    entity->stage = 0;
                }
                break;
            }
            break;
        case 11:
            if (entity->stage == 0)
            {
                entity->stage++;
                if (entity->animScriptIndex != 4)
                    Anim_StartIndexed(entity, 4, 0, 1);
            }
            break;
        }
        break;
    case 0x80:
        GameFlag_Set(0x3A);
        ObjPalette_StopColorFade();
        BgPalette_StopColorFade();
        Sound_Play(0x1040);
        save->currentSong = result;
        Interaction_End(entity, 0);
        PLAYER->palette = 0;
        entity->posX.part16.integer = y;
        { struct BgLayerState *bg = &gEwramData->bgInfo[1]; entity->posY.part16.integer = 0xC0 - bg->yPos.part16.integer; }
        entity->hidden = 0;
        if (entity->animScriptIndex != 4)
            Anim_StartIndexed(entity, 4, 0, 1);
        Anim_Update(entity);
        PlaySong(0x816C);
        EntityDeleteGroup(0x40);
        entity->state = 0x81;
        entity->stage = 0;
        entity->subState = 0;
        break;
    }
    if ((s8)entity->state >= 0)
        Anim_Update(entity);
    return result;
}
