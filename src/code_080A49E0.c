#include "code_08009A0.h"
#include "chaos_entity.h"
#include "code/code_08039340.h"
#include "code_080109F4.h"
#include "code_08011DD0.h"
#include "code_08033CAC.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"
extern void Chaos2_StepPaletteSequence(struct EwramData_EntityData *);
extern void Chaos2_SpawnSpinningDebris(struct EwramData_EntityData *, s32, s32);
extern void Chaos2_SpawnShard(struct EwramData_EntityData *, s32, s32);

/* 0x10000..0x2FFFF; the masked random value lives in r1 as in the original. */
#define RANDOM_SPEED() (({ register u32 r asm("r1") = RandomNumberGenerator() & 0x1FFFF; r; }) + 0x10000) /* FAKEMATCH: pinned register */

/**
 * @brief Defeat state (state 3): hold the player, run the palette sequence and
 * scatter spinning debris and shards (Chaos2_SpawnSpinningDebris /
 * Chaos2_SpawnShard) before the ending.
 */
void Chaos2_UpdateDefeat(struct EwramData_EntityData *entity)
{
    s32 i;
    u8 anim;

    SetPlayerInput(2, 0);
    switch (entity->subState)
    {
    case 0:
        Chaos2_StepPaletteSequence(entity);
        if (!CHAOS2_DEFEAT_READY(entity))
            break;
        entity->subState++;
        Anim_StartIndexed(entity, 0x11, 2, 0);
        break;
    case 1:
        if (!entity->animTimer)
        {
            anim = entity->animStep;
            if (anim == 1 || anim == 3 || anim == 4 || anim == 5)
                PlaySong(0x167);
        }
        if (!entity->animDone)
            break;
        entity->subState++;
        entity->timer = 0x3C;
        PlaySong(0x168);
        for (i = 0; i < 4; i++)
            Chaos2_SpawnSpinningDebris(entity, RandomNumberGenerator() & 0xFFFF, RANDOM_SPEED());
        for (i = 0; i < 6; i++)
            Chaos2_SpawnShard(entity, RandomNumberGenerator() & 0xFFFF, RANDOM_SPEED());
        entity->hidden = 1;
        break;
    case 2:
        if (entity->timer > 0x38)
        {
            Chaos2_SpawnSpinningDebris(entity, RandomNumberGenerator() & 0xFFFF, RANDOM_SPEED());
            for (i = 0; i < 4; i++)
                Chaos2_SpawnShard(entity, RandomNumberGenerator() & 0xFFFF, RANDOM_SPEED());
        }
        if ((s8)--entity->timer > 0)
            break;
        BgPalette_StartColorFade(0, 0, 0, 0x10000, 0, 0x3C);
        ObjPalette_StartColorFade(0, 0, 0, 0x10000, 0, 0x3C);
        PlaySong(0x1010);
        entity->subState++;
        entity->timer = 0x3A;
        break;
    case 3:
        if (--entity->timer > 1)
            break;
        gEwramData->gameState.bossEventFlags |= 0x80;
        Player_SetSelectedTarget(NULL);
        gEwramData->gameState.stateFlags = (gEwramData->gameState.stateFlags & ~2) | 0x4000;
        RoomJump_SetDestination((void *)0x0852362C, 0, 0, 0x80, 0x70);
        RoomJump_Begin(&gEwramData->gameState);
        gEwramData->gameState.stateFlags = (gEwramData->gameState.stateFlags & ~1) | 0x800000;
        gEwramData->startPauseEnabled = 0;
        break;
    }
}
