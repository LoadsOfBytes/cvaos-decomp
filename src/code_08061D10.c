/* EventScene20_Update: event step (cutscene facing / approach checks); matched. */
#include "code/code_08039340.h"
#include "code/code_080211F0.h"
#include "structs/ewram.h"
/* Declared s16 in code_08001194.h; the caller here uses the full register. */
extern s32 GetEntityRoomXPositionInteger(struct EwramData_EntityData *entity);
extern void TextWindow_Begin(s32);
extern s32 TextWindow_Update(void);
extern void Interaction_End(struct EwramData_EntityData *, s32);
extern void GameFlag_Set(s32 flag);
/* Declared with u16 parameters in code_0805B5E8.c; the caller passes s16 values unconverted. */
extern s32 Player_GetZoneAroundX(s32 centerX, s32 innerWidth, s32 outerWidth);


static inline s32 IsLeft(struct EwramData_EntityData *e)
{
    return e->posX.whole < gEwramData->playerState.player->posX.whole;
}

#define FACE() { u8 i_ = entity->unk_4F4.part8.unk_4F5; struct EwramData *w_ = gEwramData; struct EwramData *b_ = (struct EwramData *)(i_ * 0x84 + (u32)w_); b_->entityData[0].flipX = IsLeft(&b_->entityData[0]); }

/**
 * @brief Update of event scene variant 20 (sEventSceneUpdateFuncs[20]); returns the
 * player input mask.
 */
s32 EventScene20_Update(struct EwramData_EntityData *entity)
{
    struct EwramData **ewp;
    struct EwramData *ew;
    s32 ret;
    struct GameState *g;
    struct EwramData_EntityData *other;
    struct EwramData_EntityData *player;
    u16 w;
    s32 m1;

    ewp = &gEwramData;
    ew = *ewp;
    ret = 0;
    m1 = -1;
    g = &ew->gameState;
    switch (entity->state)
    {
    case 0:
        TextWindow_Begin(0x20);
        ew->gameState.textWindowPause = 1;
        entity->state++;
        /* fall through */
    case 1:
        if (TextWindow_Update())
        {
            entity->timer = 0x40;
            entity->state = 0x7F;
        }
        switch (entity->subState)
        {
        case 0:
            if (Player_GetFocusedActionFlags() & 0x1E)
                break;
            FACE();
            switch ((s8)Player_GetZoneAroundX((s16)entity->posX.part16.integer, 0x3C, 0x64))
            {
            case 0:
                goto set4;
            case -1:
                entity->subState = 3;
                break;
            case 1:
                entity->subState = 2;
                break;
            }
            break;
        case 2:
        {
            s16 *px = (s16 *)&entity->posX.part16.integer;
            if ((s8)Player_GetZoneAroundX(*px, 0x3C, 0x64) != 1)
                goto set4;
            if (*px < (s16)gEwramData->playerState.player->posX.part16.integer)
                ret = 0x20;
            else
                ret = 0x10;
            break;
        }
        case 3:
        {
            s32 x;
            FACE();
            if ((s8)Player_GetZoneAroundX((s16)entity->posX.part16.integer, 0x3C, 0x64) != m1)
                goto set4;
            if (gEwramData->bgInfo[1].pBgMetadata->widthCells != 1)
                w = gEwramData->bgInfo[1].pBgMetadata->widthCells << 5;
            else
                w = 0x1E;
            x = GetEntityRoomXPositionInteger(entity) / 8;
            if (x > (s32)(w >> 1))
                ret = 0x20;
            else
                ret = 0x10;
            break;
        }
        set4:
            entity->subState = 4;
            break;
        case 4:
        {
            u8 z = entity->stage;
            if (z != 0)
                break;
            {
                struct EwramData **e2 = &gEwramData;
                other = &(*e2)->entityData[ENTITY_PARENT_INDEX(entity)];
                other->flipX = other->posX.whole < (*e2)->playerState.player->posX.whole;
                player = (*e2)->playerState.player;
            }
            if (player->flipX != other->flipX)
                player->flipX = 1 - player->flipX;
            g->textWindowPause = z;
            entity->stage++;
            break;
        }
        }
        break;
    case 0x7F:
        GameFlag_Set(0x2A);
        (*ewp)->gameState.stateFlags |= 0x400000;
        if (--entity->timer == 0xFF)
            entity->state++;
        break;
    case 0x80:
        Interaction_End(entity, 1);
        break;
    }
    return ret;
}
