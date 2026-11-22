/* Elevator movement and floor input. The r0 timer locals retain the original
 * load/copy sequence; case 1 keeps its zero copy across the sound call. */
#include "code_08009A0.h"
#include "room_object_entity.h"
#include "code/code_08039340.h"
#include "code_08011DD0.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"
extern s32 Entity_IsInRect(struct EwramData_EntityData *, s16, s16, u16, u16);
extern s32 Player_IsBusy(void);
extern void Player_ClearSolidObject(void);
extern s32 Sound_PlayTracked(s32 song);
extern s32 Sound_StopTracked(s32 song);

static inline s32 AddBgY(s32 v)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    return v + bg->yPos.whole;
}

extern const s16 sUnk_08118590[];
void Object17Update(struct EwramData_EntityData *entity)
{
    s32 y;
    u16 yi;
    u8 t;
    u8 t3;
    u8 x;
    struct BgLayerState *bg;
    s16 *tbl;
    struct EwramData_EntityData *player;

    if (!entity->roomActive)
    {
        entity->deleteRequested = 1;
        Player_ClearSolidObject();
        return;
    }
    switch (entity->state)
    {
    case 0:
        if (Player_IsBusy())
        {
            OBJECT17_HOLD_FRAMES(entity) = 0;
            break;
        }
        player = gEwramData->playerState.player;
        if (!Entity_IsInRect(entity, player->posX.part16.integer - 0x20, player->posY.part16.integer - 0x20, 0x40, 0x10))
        {
            OBJECT17_HOLD_FRAMES(entity) = 0;
            break;
        }
        if (gEwramData->inputData.playerHeldInput & 0xC0)
            OBJECT17_HOLD_FRAMES(entity)++;
        else
            OBJECT17_HOLD_FRAMES(entity) = 0;
        if ((u8)OBJECT17_HOLD_FRAMES(entity) <= 2)
            break;
        if ((gEwramData->inputData.playerHeldInput & 0x80) && entity->subState <= 1)
        {
            entity->subState++;
            entity->stage = 0;
            entity->state = 1;
            entity->timer = 0x20;
            SetPlayerInput(1, 0);
        }
        if ((gEwramData->inputData.playerHeldInput & 0x40) && entity->subState != 0)
        {
            entity->subState--;
            entity->stage = 1;
            entity->state = 1;
            entity->timer = 0x40;
            SetPlayerInput(1, 0);
        }
        entity->velY.whole = 0;
        break;
    case 1: {
        s16 timer;
        u32 copy;
        SetPlayerInput(1, 0);
        timer = entity->timer;
        copy = timer;
        if (copy) { entity->timer = timer - 1; break; }
        Sound_PlayTracked(0x116);
        entity->state = 2;
        entity->velY.whole = copy;
        break;
    }
    case 2:
        SetPlayerInput(1, 0);
        entity->posY.whole += entity->velY.whole;
        if (entity->stage == 0)
        {
            if (entity->velY.whole <= 0x2FFFF)
                entity->velY.whole += 0x1000;
            y = AddBgY(entity->posY.whole + entity->velY.whole);
            yi = y < 0 ? -(-y >> 16) : y >> 16;
            if ((s16)yi < sUnk_08118590[entity->subState])
                break;
        }
        else
        {
            if (entity->velY.whole > -0x30000)
                entity->velY.whole -= 0x1000;
            y = AddBgY(entity->posY.whole + entity->velY.whole);
            yi = y < 0 ? -(-y >> 16) : y >> 16;
            if ((s16)yi > sUnk_08118590[entity->subState])
                break;
        }
        Sound_StopTracked(0x116);
        PlaySong(0x117);
        ScreenShake_Request(0, 0x10, 0, 7);
        {
            const s16 *tbl = sUnk_08118590;
            struct BgLayerState *bg2;
            s32 fy = tbl[entity->subState];
            bg2 = &gEwramData->bgInfo[1];
            entity->posY.whole = (fy - (s16)bg2->yPos.part16.integer) << 16;
        }
        entity->velY.whole = 0;
        entity->state = 3;
        entity->timer = 0x40;
        break;
    case 3: {
        s16 timer = entity->timer;
        u32 copy = timer;
        if (copy) { entity->timer = timer - 1; break; }
        entity->state = copy;
        break;
    }
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
}
