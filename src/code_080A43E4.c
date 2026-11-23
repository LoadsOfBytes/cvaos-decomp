/* Chaos_UpdateIdle. */
#include "code_08009A0.h"
#include "code_08013960.h"
#include "code_08033CAC.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

/**
 * @brief A43E4 | Sub-state 0: start the animation (Anim_StartIndexed with
 * 0, 0, 0) and advance the sub-state.
 */
void Chaos_UpdateIdle(struct EwramData_EntityData *entity)
{
    if (entity->subState == 0)
    {
        Anim_StartIndexed(entity, 0, 0, 0);
        entity->subState++;
    }
}

/* allocation of 10 words in the core's unk_504 (second form) */
struct Chaos2Data {
    s32 graphics;
    s32 palette;
    s32 palette2;
    s32 timer;    /* 0x0C */
    s32 count;    /* 0x10 */
    s32 summoned; /* 0x14 */
    s32 paletteStep;
    s32 paletteFrameTime;
    s32 paletteSequenceDone;
};

extern void *Heap_Alloc(u16);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern u8 EnemyChaos2Create(struct EwramData_EntityData *);
extern void EnemyChaos2Update(struct EwramData_EntityData *);
extern void Chaos2_OnHit(struct EwramData_EntityData *);
extern void Chaos2_CreateAffineEffect(struct EwramData_EntityData *);

/**
 * @brief A4408 | Turn the core into the second form of Chaos (3000 HP,
 * box 0x1010F8F8 = x -8, y -8, w 16, h 16, hit by Chaos2_OnHit). Returns
 * 1 when the core must be deleted.
 */
s32 Chaos_StartSecondForm(struct EwramData_EntityData *core)
{
    u32 box = 0x1010F8F8;
    struct Chaos2Data *data;

    core->updateFunc = (u32 *)EnemyChaos2Update;
    data = Heap_Alloc(10);
    core->ext.chaos.data.value = (s32)data;
    if (data != NULL)
    {
        if (EnemyChaos2Create(core))
        {
            Anim_StartIndexed(core, 8, 2, 0);
            Anim_Update(core);
            Enemy_ResetFromTemplate(core);
            Hit_Enable(core, 8, NULL, (s32)Chaos2_OnHit);
            HitBox_SetBody(core, &box);
            core->hit.flags &= ~4;
            core->hit.flags |= 2;
            core->spriteLayer = 0x1A;
            core->hp = 3000;
            core->state = 0;
            Player_SetSelectedTarget(core);
            gEwramData->gameState.stateFlags |= 2;
            gEwramData->gameState.stateFlags |= 1;
            gEwramData->gameState.stateFlags |= 0x200000;
            gEwramData->gameState.stateFlags |= 0x800000;
            gEwramData->startPauseEnabled = 0;
            core->timer = 0;
            data->timer = RandomNumberGenerator() % 128 + 60;
            core->ext.chaos2.partCount = 4;
            core->ext.chaos2.baseDefense = core->defense;
            Chaos2_CreateAffineEffect(core);
            Music_SetCurrent(0x12);
            return 0;
        }
        EntityDelete(core);
    }
    return 1;
}

extern u8 sChaos2PaletteSequence2[];
extern u8 sChaos2PaletteSequence3[];
extern void Chaos2_StepPaletteSequence(struct EwramData_EntityData *);
extern void Chaos2_SpawnShot(struct EwramData_EntityData *, s32, s32, s32);
extern void Chaos2_SummonPart(struct EwramData_EntityData *, s32, s32, s32, s32);
extern void Chaos2_SpawnEffectAbove(struct EwramData_EntityData *, s32);
extern void Chaos_StartPaletteSequence(struct EwramData_EntityData *, void *);

#define ANGLE(e) ((e)->velX.whole)
#define SPIN_SPEED(e) ((e)->accelX.whole)

/**
 * @brief A4508 | Second form AI: after a random pause either fire a ring of
 * 12 shots (Chaos2_SpawnShot) while turning, or once summon the four
 * Chaos2_SummonPart parts after the sequence sChaos2PaletteSequence3.
 */
void Chaos2_UpdateAttack(struct EwramData_EntityData *core)
{
    struct Chaos2Data *data = (struct Chaos2Data *)core->ext.chaos.data.value;

    switch (core->subState)
    {
    case 0:
        if (data->timer != 0)
        {
            data->timer--;
            break;
        }
        data->timer = (RandomNumberGenerator() & 0x7F) + 180;
        if (RandomNumberGenerator() & 1)
        {
            core->subState = 1;
            data->count = 0;
            Chaos_StartPaletteSequence(core, sChaos2PaletteSequence2);
            core->timer = 0x38;
            if (!(RandomNumberGenerator() & 1))
                SPIN_SPEED(core) = 0x1556;
            else
                SPIN_SPEED(core) = -0x1555;
            ANGLE(core) = 0x4000;
        }
        else if (data->summoned == 0)
        {
            core->subState = 10;
            Chaos_StartPaletteSequence(core, sChaos2PaletteSequence3);
            core->timer = 0;
        }
        else
            data->timer = (RandomNumberGenerator() & 0x7F) + 180;
        break;
    case 1:
        Chaos2_StepPaletteSequence(core);
        if ((s8)--core->timer <= 0)
        {
            core->subState++;
            core->timer = 10;
        }
        break;
    case 2:
        if (core->timer++ > 5)
        {
            s32 x, y;

            core->timer = 0;
            x = Sin(ANGLE(core) + 0x4000);
            x = FixedMultiply(x, 0x180000);
            y = Sin(ANGLE(core));
            y = FixedMultiply(y, 0x180000);
            Chaos2_SpawnShot(core, x, y, ANGLE(core));
            PlaySong(0x145);
            ANGLE(core) += SPIN_SPEED(core);
            if (++data->count > 11)
            {
                core->subState = 0;
                data->timer = RandomNumberGenerator() % 128 + 180;
            }
        }
        break;
    case 10:
        Chaos2_StepPaletteSequence(core);
        if (core->timer <= 59)
        {
            core->timer++;
            if ((core->timer & 3) == 0)
                Chaos2_SpawnEffectAbove(core, ((core->timer >> 2) & 7) + 9);
        }
        if (data->paletteSequenceDone != 0)
            core->subState++;
        break;
    case 11:
        Chaos2_SummonPart(core, 0, 0, -0x60000, -0x30000);
        Chaos2_SummonPart(core, 0, 0, 0x60000, -0x30000);
        Chaos2_SummonPart(core, 0, 0, -0x60000, 0x30000);
        Chaos2_SummonPart(core, 0, 0, 0x60000, 0x30000);
        data->summoned = 1;
        core->subState = 0;
        core->timer = 0;
        data->timer = RandomNumberGenerator() % 128 + 180;
        PlaySong(0x85);
        break;
    }
}
