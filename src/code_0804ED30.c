/* Door transition and effects. Explicit register locals retain agbcc output;
 * they do not establish the original source spelling. */
#include "code_08009A0.h"
#include "code_08001194.h"
#include "code/code_08039340.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"
extern s32 Entity_IsInRect(struct EwramData_EntityData *, s16, s16, u16, u16);

extern s32 RandomSigned(s32);
extern struct EwramData_EntityData *Effect_SpawnShared(s32, s32, s32, s32);
extern struct EwramData_EntityData *Effect_CreateFromTable(s32, s32, s32, s32, s32, u8);
extern s32 BgPalette_GetPendingFades(void);
extern void BgPalette_StopColorFade(void);
extern void ObjPalette_StopColorFade(void);
extern void Player_SetKnockbackVelocity(s32 vx, s32 vy);

void Object06Update(struct EwramData_EntityData *entity)
{
    struct EwramData *ew;
    struct EwramData_EntityData *p2;
    struct BgLayerState *bg;
    s32 x;
    s32 y;
    s32 speed;
    s32 vx;
    struct EwramData_EntityData *fx;

    entity->hidden = Entity_IsInRect(entity, -0x20, 0, 0x130, 0xD0) ? 0 : 1;
    switch (entity->state)
    {
    case 0: {
        register u32 flags asm("r1") = gEwramData->gameState.bossEventFlags; /* FAKEMATCH: pinned register */
        u32 scratch = 1;
        register u32 one asm("sl") = scratch; /* FAKEMATCH: pinned register */
        { u32 test = one;
          test &= flags;
        if (test)
            goto open;
        }
        if (!BgPalette_GetPendingFades())
        {
            BgPalette_StopColorFade();
            ObjPalette_StopColorFade();
        }
        { s32 counter = entity->timer + 1;
          entity->timer = counter;
        if (counter & one)
        {
            Effect_SpawnShared(entity->posX.whole + RandomSigned(0x80000), entity->posY.whole + ((RandomNumberGenerator() % 0x30 - 0x30) << 16), 0x10, 5);
        }
        }
        speed = 0x40000;
        fx = Effect_CreateFromTable(entity->posX.whole + RandomSigned(speed) + speed, entity->posY.whole + ((RandomNumberGenerator() % 0x30 - 0x30) << 16), RandomSigned(0x400) + 0x1000, RandomSigned(0x400), 7, 5);
        if (fx != NULL)
            fx->spriteLayer = 0xE;
        p2 = gEwramData->playerState.player;
        if (p2->unk_4F4.whole & 0x80)
            break;
        if (!Entity_IsInRect(entity, p2->posX.part16.integer - 0x1C, p2->posY.part16.integer, 0x38, 0x22))
            break;
        PlaySong(0x111);
        BgPalette_StartColorFade(1, 1, 1, 0x4000, 0x10000, 0x10);
        ObjPalette_StartColorFade(1, 1, 1, 0x4000, 0x10000, 0x10);
        ew = gEwramData;
        ew->playerState.pendingHitDamage = one;
        ew->playerState.hitReaction = 3;
        ew->playerState.hitSourceX = ew->playerState.player->posX.whole + 0x80000;
        ew->playerState.hitSourceY = ew->playerState.player->posY.whole;
        Player_SetKnockbackVelocity(speed, -0x10000);
        break;
    }
    case 1:
        SetPlayerInput(1, entity->flipX ? 0x20 : 0x10);
        if (entity->timer)
        {
            entity->timer--;
            break;
        }
    open:
        entity->state = 2;
        break;
    case 2:
        if (gEwramData->gameState.linkPlay)
            SetPlayerInput(1, entity->flipX ? 0x20 : 0x10);
        if (entity->unk_4F8.whole == 0)
        {
            x = (s16)entity->posX.part16.integer;
            bg = &gEwramData->bgInfo[1];
            x += (s16)bg->xPos.part16.integer;
            y = (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer;
            Collision_ClearBlock(x, y - 8);
            Collision_ClearBlock(x, y - 0x18);
            Collision_ClearBlock(x, y - 0x28);
        }
        if (++entity->unk_4F8.whole > 0x2F)
        {
            gEwramData->gameState.stateFlags &= 0xFFFFFEFE;
            if (gEwramData->gameState.linkPlay)
                entity->state++;
        }
        break;
    case 3:
        SetPlayerInput(1, entity->flipX ? 0x20 : 0x10);
        break;
    }
}
