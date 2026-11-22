/* Object16Update: affine zoom-out cutscene object with rising dust particles; matched. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code_080109F4.h"
#include "code_08011DD0.h"
#include "code_080D73B8.h"
#include "syscalls.h"
#include "structs/ewram.h"
extern s32 Game_SetMenusEnabled(s32);
extern s32 RandomSigned(s32);
extern s32 Entity_IsInRect(struct EwramData_EntityData *, s16, s16, u16, u16);
extern void RedSoul_CancelAll(void);
extern void BlueSoul_StopAll(void);
extern void Object16Effect_Update(struct EwramData_EntityData *);

static inline struct EwramData_EntityData *Player(void) { return gEwramData->playerState.player; }

void Object16Update(struct EwramData_EntityData *entity)
{
    struct BgAffineSrcData src;
    struct EwramData_EntityData *child;
    struct EwramData_EntityData *player;
    s32 input;
    s32 t;
    u32 r;

    src.texX = 0x8000;
    src.texY = 0x8000;
    src.scrX = 0x80;
    src.scrY = 0x50;
    src.alpha = entity->unk_504.whole;
    entity->unk_504.whole += entity->unk_508.whole;
    switch (entity->state)
    {
    case 0:
        if (gEwramData->gameState.stateFlags & 1)
        {
            gEwramData->gameState.stateFlags |= 1;
            Game_SetMenusEnabled(0);
            SetPlayerInput(1, 0);
            entity->state = 1;
        }
        break;
    case 1:
        {
            s32 px = gEwramData->playerState.player->posX.whole;
            SetPlayerInput(1, entity->posX.whole > px ? 0x10 : 0x20);
        }
        Game_SetMenusEnabled(0);
        if (Entity_IsInRect(entity, Player()->posX.part16.integer - 8, Player()->posY.part16.integer, 0x10, 2))
            entity->state = 2;
        break;
    case 2:
        SetPlayerInput(1, 0);
        RedSoul_CancelAll();
        BlueSoul_StopAll();
        entity->state = 3;
        PlaySong(0x157);
        break;
    case 3:
        SetPlayerInput(1, 0);
        if (entity->unk_4F8.whole > 0)
        {
            entity->unk_500.whole = 0x1000 - Sin((entity->unk_4F8.whole << 6) + 0x4000) / 16;
            entity->unk_4F8.whole--;
            entity->unk_508.whole += 2;
            if (entity->unk_4F8.whole == 0x40)
            {
                PlaySong(0x1AE);
                t = entity->unk_4F8.whole;
                if (t == 0x40)
                {
                    BgPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0x10000, 0, t);
                    ObjPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0x10000, 0, t);
                }
            }
        }
        else
        {
            entity->state = 4;
        }
        break;
    case 4:
        SetPlayerInput(1, 0);
        gEwramData->gameState.stateFlags &= ~1;
        RoomJump_SetDestination((void *)0x08522ABC, 0, 0, 0x78, 0x6F);
        RoomJump_Begin(&gEwramData->gameState);
        gEwramData->gameState.stateFlags |= 0x800000;
        break;
    }
    src.sx = entity->unk_500.whole;
    src.sy = entity->unk_500.whole;
    BgAffineSet(&src, (struct BgAffineDstData *)0x03002C78, 1);
    entity->timer++;
    if (entity->timer & 1)
    {
        child = EntityCreateInRange(0x49, 0xCF, (u32 *)Object16Effect_Update);
        if (child != NULL)
        {
            child->posX.whole = entity->posX.whole;
            child->posY.whole = entity->posY.whole + ((RandomSigned(8) - 4) << 16);
            child->velY.whole = RandomSigned(0x1000) - 0x8000;
            if ((RandomNumberGenerator() & 0x3F) <= 0x27)
            {
                if ((RandomNumberGenerator() & 0x3F) <= 0xF)
                {
                    child->subState = 1;
                    child->velY.whole += -0x1C000;
                }
                child->posX.whole += RandomSigned(4) << 16;
            }
            else
            {
                r = RandomNumberGenerator();
                child->posX.whole += (((r & 7) >> 1) + 2) * ((RandomNumberGenerator() & 1) * 2 - 1) << 16;
            }
            child->accelY.whole = -entity->velY.whole / 128;
        }
    }
}
